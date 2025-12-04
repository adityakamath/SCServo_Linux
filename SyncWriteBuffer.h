/**
 * @file SyncWriteBuffer.h
 * @brief RAII-based buffer management for synchronized write operations
 *
 * @details This file provides automatic buffer management for sync write operations,
 * eliminating manual allocation/deallocation and preventing memory leaks. Part of
 * Phase 4 refactoring to convert all manual buffer management to RAII pattern.
 *
 * **Benefits:**
 * - Automatic cleanup (no memory leaks)
 * - Exception-safe resource management
 * - Consistent error handling
 * - Type-safe operations
 * - Clear ownership semantics
 *
 * **Replaced Manual Allocations:**
 * - SMS_STS::SyncWritePosEx, SyncWriteSpe, SyncWritePwm
 * - SMSCL::SyncWritePosEx
 * - SMSBL::SyncWritePosEx
 * - SCSCL::SyncWritePos
 *
 * **Usage Example:**
 * @code
 * SyncWriteBuffer buffer(3, 7);  // 3 motors, 7 bytes per motor
 * if (!buffer.isValid()) {
 *     return;  // Allocation failed
 * }
 * // Use buffer - automatic cleanup when buffer goes out of scope
 * servo.syncWrite(ids, 3, addr, buffer.getBuffer(), 7);
 * @endcode
 *
 * @note Uses RAII (Resource Acquisition Is Initialization) for automatic cleanup
 */

#ifndef _SYNC_WRITE_BUFFER_H
#define _SYNC_WRITE_BUFFER_H

#include "SCServo.h"
#include <cstring>

/**
 * @brief RAII wrapper for sync write buffer management
 *
 * This class eliminates the 6+ instances of manual buffer management:
 * - SMS_STS::SyncWritePosEx, SyncWriteSpe, SyncWritePwm
 * - SMSCL::SyncWritePosEx
 * - SMSBL::SyncWritePosEx
 * - SCSCL::SyncWritePwm
 *
 * Benefits:
 * - Automatic cleanup (no memory leaks)
 * - Consistent error handling
 * - Type-safe operations
 * - Clear ownership semantics
 */
class SyncWriteBuffer {
private:
    u8* buffer;
    size_t capacity;
    size_t bytesPerMotor;
    size_t numMotors;

public:
    /**
     * @brief Construct buffer for sync write operations
     * @param motorCount Number of motors in the sync write
     * @param payloadSize Bytes per motor payload
     */
    SyncWriteBuffer(size_t motorCount, size_t payloadSize)
        : buffer(nullptr),
          capacity(motorCount * payloadSize),
          bytesPerMotor(payloadSize),
          numMotors(motorCount) {

        if (capacity > 0) {
            buffer = new u8[capacity];
        }
    }

    /**
     * @brief Destructor - automatically frees buffer
     */
    ~SyncWriteBuffer() {
        if (buffer) {
            delete[] buffer;
            buffer = nullptr;
        }
    }

    // Prevent copying (use move semantics if needed)
    SyncWriteBuffer(const SyncWriteBuffer&) = delete;
    SyncWriteBuffer& operator=(const SyncWriteBuffer&) = delete;

    /**
     * @brief Write data for a specific motor at given index
     * @param motorIndex Motor index (0 to numMotors-1)
     * @param data Data to write
     * @param dataSize Size of data (must be <= bytesPerMotor)
     * @return true on success, false on invalid parameters
     */
    bool writeMotorData(size_t motorIndex, const u8* data, size_t dataSize) {
        if (!buffer || motorIndex >= numMotors || dataSize > bytesPerMotor) {
            return false;
        }

        memcpy(buffer + (motorIndex * bytesPerMotor), data, dataSize);
        return true;
    }

    /**
     * @brief Get raw buffer pointer for passing to syncWrite
     * @return Pointer to buffer, or nullptr if allocation failed
     */
    u8* getBuffer() {
        return buffer;
    }

    /**
     * @brief Get buffer pointer (const version)
     */
    const u8* getBuffer() const {
        return buffer;
    }

    /**
     * @brief Get total buffer size in bytes
     */
    size_t getSize() const {
        return capacity;
    }

    /**
     * @brief Get bytes per motor
     */
    size_t getBytesPerMotor() const {
        return bytesPerMotor;
    }

    /**
     * @brief Get number of motors
     */
    size_t getNumMotors() const {
        return numMotors;
    }

    /**
     * @brief Check if allocation succeeded
     * @return true if buffer is valid, false if allocation failed
     */
    bool isValid() const {
        return buffer != nullptr;
    }

    /**
     * @brief Explicit bool conversion for validity checking
     */
    explicit operator bool() const {
        return isValid();
    }

    /**
     * @brief Fill entire buffer with zeros
     */
    void clear() {
        if (buffer) {
            memset(buffer, 0, capacity);
        }
    }

    /**
     * @brief Fill entire buffer with a specific byte value
     * @param value Byte value to fill with
     */
    void fill(u8 value) {
        if (buffer) {
            memset(buffer, value, capacity);
        }
    }
};

#endif // _SYNC_WRITE_BUFFER_H
