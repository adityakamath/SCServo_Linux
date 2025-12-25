/**
 * @file ServoUtils.h
 * @brief Utility functions for SCServo library implementing DRY principle
 *
 * @details This file provides reusable utility functions that eliminate code
 * duplication across the servo library.
 *
 * **Key Functions:**
 * - Direction bit encoding/decoding for signed values
 * - Buffer read operations with direction handling
 * - Cached vs direct servo read abstraction
 * - Motor ID validation
 *
 * **Eliminated Duplication:**
 * - 13+ instances of direction encoding logic
 * - 16+ instances of direction decoding logic
 * - 12+ instances of cache-or-servo read patterns
 *
 * **Usage Example:**
 * @code
 * u16 encoded = ServoUtils::encodeSignedValue(-100, 15);  // Encode with direction bit
 * s16 decoded = ServoUtils::decodeSignedValue(encoded, 15);  // Decode back to -100
 * @endcode
 *
 * @note All functions are inline for zero-overhead abstraction
 * @see SMS_STS.cpp, SCSCL.cpp, HLSCL.cpp for usage examples
 */

#ifndef _SERVO_UTILS_H
#define _SERVO_UTILS_H

#include "SCServo.h"
#include "ServoErrors.h"

namespace ServoUtils {

/**
 * @brief Encode a signed value with direction bit
 * @param value Signed value to encode
 * @param directionBit Bit position for direction flag (default 15)
 * @return Encoded unsigned value with direction bit set if negative
 *
 * This eliminates 13+ instances of duplicate direction encoding logic across:
 * SMS_STS.cpp, SCSCL.cpp, HLSCL.cpp
 */
inline u16 encodeSignedValue(s16 value, u8 directionBit = 15) {
    if (value < 0) {
        u16 absValue = static_cast<u16>(-value);
        return absValue | (1 << directionBit);
    }
    return static_cast<u16>(value);
}

/**
 * @brief Decode a value with direction bit
 * @param encodedValue Encoded unsigned value
 * @param directionBit Bit position for direction flag (default 15)
 * @return Decoded signed value
 *
 * This eliminates 16+ instances of duplicate direction decoding logic
 */
inline s16 decodeSignedValue(u16 encodedValue, u8 directionBit = 15) {
    if (encodedValue & (1 << directionBit)) {
        return -static_cast<s16>(encodedValue & ~(1 << directionBit));
    }
    return static_cast<s16>(encodedValue);
}

/**
 * @brief Read a 16-bit word from memory buffer and combine bytes
 * @param buffer Memory buffer containing data
 * @param offsetLow Offset for low byte (relative to buffer start)
 * @param offsetHigh Offset for high byte (relative to buffer start)
 * @return Combined 16-bit value (high byte << 8 | low byte)
 */
inline u16 readWordFromBuffer(const u8* buffer, u8 offsetLow, u8 offsetHigh) {
    u16 value = buffer[offsetHigh];
    value <<= 8;
    value |= buffer[offsetLow];
    return value;
}

/**
 * @brief Read a 16-bit signed value from memory buffer with direction decoding
 * @param buffer Memory buffer containing data
 * @param offsetLow Offset for low byte
 * @param offsetHigh Offset for high byte
 * @param directionBit Bit position for direction flag
 * @return Decoded signed value
 *
 * This eliminates the repeated pattern of reading two bytes, combining them,
 * and extracting the direction bit found in 16+ locations
 */
inline s16 readSignedWordFromBuffer(const u8* buffer, u8 offsetLow, u8 offsetHigh, u8 directionBit) {
    u16 value = readWordFromBuffer(buffer, offsetLow, offsetHigh);
    return decodeSignedValue(value, directionBit);
}

/**
 * @brief Read a single byte from cached buffer or directly from servo
 * @param servoInstance Reference to servo object (SMS_STS, SCSCL, HLSCL, etc)
 * @param ID Motor ID (-1 to read from cache, >=0 to read from servo)
 * @param registerAddr Register address to read from servo
 * @param cachedBuffer Cached memory buffer
 * @param cacheOffset Offset in cache buffer
 * @param err Error flag output (set to 0 on success, 1 on failure)
 * @return Byte value read, or -1 on error
 *
 * This eliminates 12+ instances of the same read pattern:
 * - If ID == -1: read from cache
 * - Otherwise: read from servo and update error flag
 */
template<typename ServoType>
inline int readByteFromCacheOrServo(
    ServoType& servoInstance,
    int ID,
    u8 registerAddr,
    const u8* cachedBuffer,
    u8 cacheOffset,
    int& err
) {
    if (ID == -1) {
        return cachedBuffer[cacheOffset];
    } else {
        err = 0;
        int value = servoInstance.readByte(ID, registerAddr);
        if (value == -1) {
            err = 1;
        }
        return value;
    }
}

/**
 * @brief Read a signed word from cached buffer or directly from servo
 * @param servoInstance Reference to servo object
 * @param ID Motor ID (-1 for cache, >=0 for servo)
 * @param registerAddrLow Low byte register address
 * @param cachedBuffer Cached memory buffer
 * @param offsetLow Low byte offset in cache
 * @param offsetHigh High byte offset in cache
 * @param directionBit Direction bit position
 * @param err Error flag output
 * @return Signed word value, or -1 on error
 *
 * Combines the common pattern of reading signed words with direction bits
 */
template<typename ServoType>
inline s16 readSignedWordFromCacheOrServo(
    ServoType& servoInstance,
    int ID,
    u8 registerAddrLow,
    const u8* cachedBuffer,
    u8 offsetLow,
    u8 offsetHigh,
    u8 directionBit,
    int& err
) {
    if (ID == -1) {
        return readSignedWordFromBuffer(cachedBuffer, offsetLow, offsetHigh, directionBit);
    } else {
        err = 0;
        s16 value = servoInstance.readSignedWord(ID, registerAddrLow, directionBit);
        if (value == -1) {
            err = 1;
        }
        return value;
    }
}

/**
 * @brief Check if an ID represents a cached read request
 * @param ID Motor ID value
 * @return true if ID is -1 (cached read), false otherwise
 *
 * This provides a semantic wrapper around the -1 comparison pattern
 * found throughout the codebase, improving readability
 */
inline bool isCachedRead(int ID) {
    return ID == -1;
}

/**
 * @brief Validate motor ID is in acceptable range
 * @param ID Motor ID to validate
 * @return true if ID is valid (0-253 or -1 for cached), false otherwise
 */
inline bool isValidMotorID(int ID) {
    return (ID >= 0 && ID <= 253) || ID == -1;
}

} // namespace ServoUtils

#endif // _SERVO_UTILS_H
