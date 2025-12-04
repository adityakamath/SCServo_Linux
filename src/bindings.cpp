/**
 * @file bindings.cpp
 * @brief Python bindings for SCServo_Linux library using nanobind
 *
 * @details This file provides Python bindings for the core Feetech servo classes.
 * Only safe, default-constructible methods are exposed to Python for security
 * and compatibility. Array-based C++ methods are wrapped with lambda functions
 * that accept Python lists and perform automatic conversion.
 *
 * **Binding Strategy:**
 * - Only default constructors are bound (parameterized constructors are unsafe in Python)
 * - Array/pointer parameters are wrapped with lambda functions for Python list compatibility
 * - Named arguments (nb::arg) provide clear Python API
 * - Default parameters are preserved where applicable
 *
 * **Usage Example (Python):**
 * @code{.py}
 * import scservo_python as sc
 * servo = sc.SMS_STS()
 * servo.begin(1000000, "/dev/ttyUSB0")
 * servo.InitMotor(1, 0, 1)  # ID=1, Mode=0 (servo), Enable torque
 * servo.SyncWriteSpe([1,2,3], [100,200,300], [50,50,50])
 * @endcode
 *
 * @note This module uses nanobind for modern, high-performance Python/C++ integration
 * @see https://github.com/wjakob/nanobind for nanobind documentation
 */

#include <nanobind/nanobind.h>
#include <nanobind/stl/vector.h>
#include <nanobind/stl/string.h>
#include "SMS_STS.h"
#include "SMSCL.h"
#include "SMSBL.h"
#include "SCS.h"

namespace nb = nanobind;

/**
 * @brief Python module definition for scservo_python
 *
 * @details Defines the Python module "scservo_python" containing bindings for
 * SMS_STS, SMSCL, SMSBL, and SCSerial classes. The SCS class is intentionally
 * not bound as it is an abstract base class.
 *
 * **Bound Classes:**
 * - SMS_STS: Full-featured SMS/STS series servo control (most complete binding)
 * - SMSCL: SMSCL series servo control (basic binding)
 * - SMSBL: SMSBL series servo control (basic binding)
 * - SCSerial: Low-level serial communication and error handling
 *
 * **Not Bound:**
 * - SCS: Abstract base class, cannot be instantiated
 */
NB_MODULE(scservo_python, m) {
    m.doc() = "Python bindings for SCServo_Linux core classes (safe constructors and methods only)";

    /**
     * SMS_STS class binding
     *
     * Provides comprehensive bindings for SMS/STS series servo motors.
     * Includes initialization, torque control, synchronized speed writing,
     * and feedback reading capabilities.
     */
    nb::class_<SMS_STS>(m, "SMS_STS")
        .def(nb::init<>())  // Default constructor only (safe for Python)

        // Serial communication methods
        .def("begin", &SMS_STS::begin)  // Initialize serial port: begin(baud_rate, "/dev/ttyUSB0")
        .def("end", &SMS_STS::end)      // Close serial port

        // Low-level write method
        .def("writeByte", &SMS_STS::writeByte)  // Write single byte to register

        // Motor initialization and control (LSP compliant)
        .def("InitMotor", &SMS_STS::InitMotor,
             nb::arg("ID"), nb::arg("mode"), nb::arg("enableTorque") = 1)  // Initialize motor with mode and torque setting

        /**
         * Synchronized speed write with Python list support
         *
         * Lambda wrapper converts Python lists to C++ arrays for SyncWriteSpe().
         * Enables efficient multi-motor velocity control from Python.
         *
         * @param py_ids Python list of motor IDs (u8)
         * @param py_speeds Python list of target speeds (s16)
         * @param py_accs Python list of accelerations (u8)
         *
         * Example: servo.SyncWriteSpe([1,2,3], [100,200,300], [50,50,50])
         */
        .def("SyncWriteSpe", [](SMS_STS &self, nb::list py_ids, nb::list py_speeds, nb::list py_accs) {
            size_t n = py_ids.size();
            std::vector<u8> ids(n);
            std::vector<s16> speeds(n);
            std::vector<u8> accs(n);
            // Convert Python lists to C++ vectors
            for (size_t i = 0; i < n; ++i) {
                ids[i] = nb::cast<u8>(py_ids[i]);
                speeds[i] = nb::cast<s16>(py_speeds[i]);
                accs[i] = nb::cast<u8>(py_accs[i]);
            }
            // Call C++ method with array pointers
            self.SyncWriteSpe(ids.data(), n, speeds.data(), accs.data());
        })

        // Torque and feedback methods
        .def("EnableTorque", &SMS_STS::EnableTorque)  // Enable/disable motor torque
        .def("FeedBack", &SMS_STS::FeedBack)          // Trigger feedback data read
        .def("ReadSpeed", &SMS_STS::ReadSpeed);       // Read current motor speed

    /**
     * SMSCL class binding
     *
     * Basic bindings for SMSCL series servo motors.
     * Currently only includes serial communication methods.
     */
    nb::class_<SMSCL>(m, "SMSCL")
        .def(nb::init<>())
        .def("begin", &SMSCL::begin)  // Initialize serial port
        .def("end", &SMSCL::end);     // Close serial port

    /**
     * SMSBL class binding
     *
     * Basic bindings for SMSBL series servo motors.
     * Currently only includes torque control.
     */
    nb::class_<SMSBL>(m, "SMSBL")
        .def(nb::init<>())
        .def("EnableTorque", &SMSBL::EnableTorque);  // Enable/disable motor torque

    /**
     * SCS class is intentionally NOT bound
     *
     * SCS is an abstract base class that cannot be instantiated.
     * Use concrete implementations (SMS_STS, SMSCL, SMSBL) instead.
     */

    /**
     * SCSerial class binding
     *
     * Low-level serial communication and error handling.
     * Provides access to error status from serial operations.
     */
    nb::class_<SCSerial>(m, "SCSerial")
        .def(nb::init<>())
        .def("getErr", &SCSerial::getErr);  // Get last error code from serial operations
}
