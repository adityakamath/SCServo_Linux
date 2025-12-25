/**
 * @file ServoErrors.h
 * @brief Standardized error handling for SCServo SDK
 *
 * @details This file provides structured error codes and result types for
 * consistent error handling across the servo SDK.
 *
 * **Components:**
 * - ServoError enum: Named error codes for all failure modes
 * - ServoResult class: Type-safe result wrapper with backward compatibility
 * - Helper functions: Semantic error checking predicates
 *
 * **Benefits:**
 * - Improved debugging with named error codes
 * - Type-safe error handling with ServoResult
 * - Backward compatible with existing int return values
 * - Clear semantics for cached vs direct reads
 *
 * **Usage Example:**
 * @code
 * ServoResult result = servo.WritePosEx(1, 2048, 1000);
 * if (result.isSuccess()) {
 *     printf("Success! Value: %d\n", result.getValue());
 * } else {
 *     printf("Error: %d\n", static_cast<int>(result.getError()));
 * }
 *
 * // Or use backward-compatible int conversion:
 * int retval = servo.WritePosEx(1, 2048, 1000);
 * if (retval == -1) { // handle error
 * }
 * @endcode
 */

#ifndef _SERVOERRORS_H
#define _SERVOERRORS_H

/**
 * @brief Standard error codes for servo operations
 *
 * Replaces inconsistent use of -1 return values with
 * named error codes for better debugging and error handling.
 */
enum class ServoError {
    SUCCESS = 0,              // Operation completed successfully
    COMM_TIMEOUT = -1,        // Communication timeout
    COMM_RX_FAIL = -2,        // Receive failure
    COMM_TX_FAIL = -3,        // Transmit failure
    INVALID_PARAMETER = -4,   // Invalid parameter passed
    ALLOCATION_FAILED = -5,   // Memory allocation failed
    REGISTER_WRITE_FAILED = -6, // Register write operation failed
    CHECKSUM_ERROR = -7,      // Checksum validation failed
    UNKNOWN_ERROR = -99       // Unknown error occurred
};

/**
 * @brief Result wrapper for servo operations
 *
 * Provides backward compatibility with int return values
 * while supporting structured error handling.
 *
 * Usage:
 *   ServoResult result = someServoOperation();
 *   if (result.isSuccess()) {
 *       int value = result.getValue();
 *   } else {
 *       ServoError err = result.getError();
 *       // Handle error
 *   }
 *
 * Or for backward compatibility:
 *   int value = someServoOperation(); // Implicit conversion
 *   if (value == -1) { ... }
 */
class ServoResult {
private:
    ServoError errorCode;
    int value;

public:
    /**
     * @brief Construct a result with error code and value
     * @param err Error code
     * @param val Associated value (default 0)
     */
    ServoResult(ServoError err, int val = 0)
        : errorCode(err), value(val) {}

    /**
     * @brief Construct a successful result with value
     * @param val Value to return
     */
    ServoResult(int val)
        : errorCode(ServoError::SUCCESS), value(val) {}

    /**
     * @brief Check if operation was successful
     * @return true if no error occurred
     */
    bool isSuccess() const {
        return errorCode == ServoError::SUCCESS;
    }

    /**
     * @brief Get the error code
     * @return ServoError enum value
     */
    ServoError getError() const {
        return errorCode;
    }

    /**
     * @brief Get the associated value
     * @return Integer value
     */
    int getValue() const {
        return value;
    }

    /**
     * @brief Implicit conversion to int for backward compatibility
     * @return Value on success, error code on failure
     */
    operator int() const {
        return isSuccess() ? value : static_cast<int>(errorCode);
    }
};

/**
 * @brief Helper function to check for invalid motor ID
 *
 * Provides a named function instead of magic -1 comparison.
 * Used to check if ID refers to cached data rather than a specific motor.
 *
 * @param ID Motor ID to check
 * @return true if ID == -1 (refers to cached data)
 */
inline bool isInvalidID(int ID) {
    return ID == -1;
}

/**
 * @brief Helper function to check for cached data request
 *
 * More semantic alternative to isInvalidID when checking
 * if we should read from cached buffer vs querying servo.
 *
 * @param ID Motor ID to check
 * @return true if ID == -1 (use cached data)
 */
inline bool isCachedRequest(int ID) {
    return ID == -1;
}

/**
 * @brief Check if a return value indicates an error
 *
 * Helper for legacy code that returns -1 on error.
 *
 * @param returnValue Value to check
 * @return true if value indicates error (< 0)
 */
inline bool isError(int returnValue) {
    return returnValue < 0;
}

/**
 * @brief Check if a return value indicates success
 *
 * Helper for legacy code that returns -1 on error.
 *
 * @param returnValue Value to check
 * @return true if value indicates success (>= 0)
 */
inline bool isSuccess(int returnValue) {
    return returnValue >= 0;
}

#endif // _SERVOERRORS_H

