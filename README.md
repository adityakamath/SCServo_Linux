# SCServo_Linux

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/adityakamath/SCServo_Linux) ![Tested Motor](https://img.shields.io/badge/Tested-STS3215-success?logo=data:image/svg+xml;base64,PHN2ZyB4bWxucz0iaHR0cDovL3d3dy53My5vcmcvMjAwMC9zdmciIHZpZXdCb3g9IjAgMCAyNCAyNCI+PHBhdGggZmlsbD0iI2ZmZiIgZD0iTTEyIDJDNi40OCAyIDIgNi40OCAyIDEyczQuNDggMTAgMTAgMTAgMTAtNC40OCAxMC0xMFMxNy41MiAyIDEyIDJ6bTAgMThjLTQuNDEgMC04LTMuNTktOC04czMuNTktOCA4LTggOCAzLjU5IDggOC0zLjU5IDgtOCA4em0tMi05aDR2NmgtNHptMC00aDR2MmgtNnoiLz48L3N2Zz4=)
![Supported Motors](https://img.shields.io/badge/Supported-SMS%2FSTS%20%7C%20SCSCL%20%7C%20HLSCL-blue) ![GitHub License](https://img.shields.io/github/license/adityakamath/SCServo_Linux)
![X (formerly Twitter) Follow](https://img.shields.io/twitter/follow/kamathsblog)

> Linux SDK for Feetech serial servo motors with C++ and Python support

A high-performance Linux SDK for controlling Feetech SMS/STS/SCSCL/HLSCL series serial bus servo motors. Features position, velocity, PWM, and force control with multi-servo synchronization support.

> **📌 About This Fork:** This repository is a fork of Feetech's official [FTServo_Linux](https://gitee.com/ftservo/FTServo_Linux) SDK with enhanced documentation, code quality improvements, and Python bindings. Most enhancements were AI-generated, but under strict human supervision. The original repository on Gitee is fully functional and can be used if you prefer the unmodified SDK. This fork focuses on improved usability, comprehensive documentation, and additional examples for the Linux platform.

## Table of Contents

- [Features](#features)
- [Hardware Support](#hardware-support)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Operating Modes](#operating-modes)
- [Usage Examples](#usage-examples)
- [Python Bindings](#python-bindings)
- [Documentation](#documentation)
- [Contributing](#contributing)
- [License](#license)

## Features

- **Protocol Support**: SMS/STS, SCSCL, HLSCL series
- **Control Modes**: Position (servo with velocity/acceleration), Velocity (closed-loop wheel), PWM (open-loop wheel), Force/Torque (constant force output - HLSCL only)
- **Multi-Servo Operations**: Synchronized Write / Broadcast commands
- **Comprehensive Feedback**: Position, speed, load, voltage, temperature, current readings
- **Configuration Tools**: EEPROM programming, midpoint calibration, ID management
- **Platform Optimized**: Linux (x86_64, ARM64), Raspberry Pi tested
- **Language Bindings**: Native C++ (C++17) and Python 3.8+ (via nanobind)
- **Rich Examples**: 25+ comprehensive examples with full documentation

## Hardware Support

### Supported Servo Models

| Protocol | Models | Default Baud Rate | Position Resolution | Special Features |
|----------|--------|-------------------|---------------------|------------------|
| SMS/STS  | STS3215, STS3032, STS3250, SMS40 | 1000000 (1M) | 12-bit (0-4095) | Standard modes |
| SCSCL    | SC09, SC15 | 115200 | 10-bit (0-1023) | Position + PWM |
| HLSCL    | HLS series | 115200 | 12-bit (0-4095) | **Includes force/torque mode** |

### Connection Requirements

- **Serial Interface**: USB-to-Serial adapter (FTDI, CP2102, CH340) or direct UART
- **Wiring**: TX/RX/GND (half-duplex serial communication)
- **Power Supply**: External power required (6-14V depending on model)
- **Data Bus**: Daisy-chainable for multi-servo setups

> **⚠️ Bus Compatibility Warning:** The motor controller may not support mixing different motor types (e.g., STS3215 and SC09) on the same serial bus. Different series may use incompatible protocols, baud rates, or memory maps. For reliable operation, use motors from the same series on a single bus. If you need to control different motor types, consider using separate serial interfaces.

### Tested Platforms

- Ubuntu 20.04/22.04 (x86_64, ARM64)
- Raspberry Pi OS (Pi 3/4/5, Zero 2)
- Other Linux distributions with POSIX serial support

> **⚠️ Important:** This SDK has only been extensively tested with **STS3215** servo motors. While it should work with other SMS/STS/SCSCL series motors according to protocol specifications, exercise caution and thoroughly test functionality before deploying with other motor models.

## Installation

### Prerequisites

```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

### Build from Source

```bash
# Clone repository with submodules
git clone --recursive https://github.com/adityakamath/SCServoLinux.git
cd SCServo_Linux

# Or if already cloned without --recursive:
# git submodule update --init --recursive

# Build library
cmake .
make -j4

# The build produces:
# - libSCServo.a (static library)
# - scservo_python.cpython-312-*.so (Python module)
```

### Serial Port Permissions

Add your user to the dialout group for `/dev/ttyUSB*` access:

```bash
sudo usermod -a -G dialout $USER
# Log out and log back in for changes to take effect
```

Alternatively, use `sudo` for testing (not recommended for production):

```bash
sudo ./WritePos /dev/ttyUSB0
```

## Quick Start

**⚠️ Note:** Before running any example, double check the baud rate and motor ID in the example code, and change them if needed to match your setup. Then build the example before running it. 

### Step 1: Connect Hardware

```
Servo ----[Data]---- USB-Serial Adapter ----[USB]---- Computer
  |                          |
[GND]----------------------[GND]
  |
[Power]---- External PSU (6-14V)
```

### Step 2: Verify Connection

```bash
# Find your serial port
ls /dev/ttyUSB*  # or /dev/ttyACM*

# Test with Ping example
cd examples/SMS_STS/Ping
cmake . && make
./Ping /dev/ttyUSB0

# Expected output: "ID:1 PingOK!" (if servo ID=1 is connected)
```

### Step 3: Run Your First Example

```bash
# Position control example
cd examples/SMS_STS/WritePos
cmake . && make
./WritePos /dev/ttyUSB0

# Servo will oscillate between position 0 and 4095
# Press Ctrl+C to stop (torque automatically disabled)
```

## Operating Modes

The SMS/STS servos support three distinct operating modes:

### Mode 0: Servo Mode (Position Control)

- **Use case**: Precise position control (robot arms, pan-tilt mechanisms)
- **Control parameters**: Position (0-4095), Speed, Acceleration
- **Functions**: `WritePosEx()`, `RegWritePosEx()`, `SyncWritePosEx()`
- **Behavior**: Motor moves to target position and holds
- **Protocols**: SMS/STS, SCSCL, HLSCL

### Mode 1: Closed-Loop Wheel Mode (Velocity Control)

- **Use case**: Continuous rotation with speed feedback (wheeled mobile robots)
- **Control parameters**: Speed (±3400 steps/s), Acceleration
- **Functions**: `WriteSpe()`, `RegWriteSpe()`, `SyncWriteSpe()`
- **Behavior**: Motor maintains target speed using encoder feedback
- **Protocols**: SMS/STS, HLSCL

### Mode 2: Protocol-Specific Modes

Mode 2 behavior differs by protocol:

#### SMS/STS Mode 2: Open-Loop Wheel Mode (PWM Control)

- **Use case**: Direct motor power control without feedback
- **Control parameters**: PWM duty cycle (-1000 to +1000, represents -100% to +100% duty cycle)
- **Functions**: `WritePwm()`, `RegWritePwm()`, `SyncWritePwm()`
- **Behavior**: No speed feedback; actual speed depends on load
- **Protocols**: SMS/STS only

#### HLSCL Mode 2: Force/Torque Mode (Electric Mode)

- **Use case**: Constant force/torque output (grippers, tensioning, compliant manipulation)
- **Control parameters**: Torque (±1000, negative=CCW, positive=CW)
- **Functions**: `WriteEle()` (HLSCL only)
- **Behavior**: Motor maintains constant torque regardless of position or speed
- **Protocols**: HLSCL only
- **⚠️ Note**: Conversion factors and behavior should be verified against your specific HLS servo model's datasheet.

For detailed parameter specifications, ranges, and memory map, see [API.md](docs/API.md#memory-table-reference).

## Usage Examples

The SDK includes 25+ comprehensive examples demonstrating all servo control modes and techniques.

### Quick Example - Basic Position Control

```cpp
#include "SCServo.h"

int main() {
    SMS_STS servo;
    servo.begin(1000000, "/dev/ttyUSB0");
    servo.WritePosEx(1, 2048, 2400, 50);  // ID=1, center position, 2400 steps/s, accel=50
    servo.end();
}
```

For complete examples with full source code, hardware requirements, and detailed explanations, see **[EXAMPLES.md](docs/EXAMPLES.md)**.

## Python Bindings

The SDK includes high-performance Python bindings via [nanobind](https://github.com/wjakob/nanobind).

### Installation

```bash
cd SCServo_Linux
pip install -e .
```

### Quick Example

```python
import scservo_python as sc

servo = sc.SMS_STS()
servo.begin(1000000, "/dev/ttyUSB0")
servo.WritePosEx(1, 2048, 2400, 50)
servo.end()
```

The Python API mirrors the C++ interface. For a Python example including multi-motor control, velocity sweeps, and data collection, see [EXAMPLES.md - Python Examples](docs/EXAMPLES.md#python-examples).

## Documentation

### Detailed Guides

- **[Examples Guide](docs/EXAMPLES.md)** - Complete catalog of all 25+ examples with full documentation
- **[API Reference](docs/API.md)** - Complete API documentation with all functions, parameters, and memory table reference
- **[Troubleshooting Guide](docs/TROUBLESHOOTING.md)** - Common issues, solutions, and debugging techniques
- **[Architecture Documentation](docs/ARCHITECTURE.md)** - System design, class hierarchy, protocol details, and internals

### Source Code

All C++ source files and headers include comprehensive Doxygen-style and inline comments:
- Header files (`*.h`) contain detailed function signatures, parameter descriptions, and return values
- Implementation files (`*.cpp`) include inline comments explaining complex logic and algorithms
- Example files feature extensive comments for learning and reference

> **Tip:** When the markdown documentation is unclear, refer directly to the source code headers for the most accurate and detailed API information.

## Contributing

Contributions are welcome! Please follow these guidelines:

### Reporting Bugs

- Use GitHub Issues with clear reproduction steps
- Include: OS version, servo model, baud rate, error messages
- Provide minimal code example demonstrating the issue
- Attach debug output if possible

### Pull Requests

- Fork repository and create a feature branch
- Follow existing code style (see examples)
- Add Doxygen comments for new public functions
- Test on real hardware before submitting
- Update README if adding major features
- Ensure all examples build without warnings

## References & Acknowledgments

This repository is a fork of Feetech's official [FTServo_Linux SDK](https://gitee.com/ftservo/FTServo_Linux) with the following enhancements:

**Improvements in this fork:**
- Comprehensive English documentation (README, API reference, architecture guide, troubleshooting guide)
- Python bindings via [nanobind](https://github.com/wjakob/nanobind)
- HLSCL protocol support for HLS series servos
- Enhanced code comments and Doxygen documentation
- Additional example programs with detailed explanations
- Code quality improvements (includes, error handling, consistency)
- Build system improvements and example organization

---

> **⚠️ Note on AI-Generated Content:** This README was written with AI assistance to improve clarity and organization, with all content manually reviewed and verified against the actual implementation. The documentation files in the `docs/` folder ([API.md](docs/API.md), [EXAMPLES.md](docs/EXAMPLES.md), [ARCHITECTURE.md](docs/ARCHITECTURE.md), [TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md)) were primarily AI-generated under human supervision and may contain inconsistencies with the actual codebase. When in doubt, always verify against the source code, header files, and working examples. Issue reports and corrections are welcome via GitHub Issues.
