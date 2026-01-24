# SCServo_Linux

[![Ask DeepWiki (Experimental)](https://deepwiki.com/badge.svg)](https://deepwiki.com/adityakamath/SCServo_Linux)
![License](https://img.shields.io/github/license/adityakamath/SCServo_Linux?label=License)
![Website](https://img.shields.io/badge/Website-kamathrobotics.com-blue?style=flat&logo=hashnode&logoSize=auto&link=https%3A%2F%2Fkamathrobotics.com&link=https%3A%2F%2Fkamathrobotics.com)
![X (formerly Twitter) Follow](https://img.shields.io/twitter/follow/kamathsblog)

> Linux SDK for Feetech serial servo motors with native C++ support

A high-performance Linux SDK for controlling Feetech SMS/STS/SCSCL/HLSCL series serial bus servo motors. Features position, velocity, PWM, and force control with multi-servo synchronization support.

> **📌 About This Fork:** This repository is a fork of Feetech's official [FTServo_Linux](https://gitee.com/ftservo/FTServo_Linux) SDK with enhanced documentation and code quality improvements. Most enhancements were AI-generated, but under strict human supervision. The original repository on Gitee is fully functional and can be used if you prefer the unmodified SDK. This fork focuses on improved usability, comprehensive documentation, and additional examples for the Linux platform.

## Table of Contents

- [Features](#features)
- [Hardware Support](#hardware-support)
- [Installation](#installation)
- [Quick Start](#quick-start)
- [Operating Modes](#operating-modes)
- [Usage Examples](#usage-examples)
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
- **Language**: Native C++ (C++17)
- **Rich Examples**: 25+ comprehensive examples with full documentation

## Hardware Support

### Supported Servo Models

| Protocol | Models | Common Baud Rates* | Position Resolution | Special Features |
|----------|--------|-------------------|---------------------|------------------|
| SMS/STS  | STS3215, STS3032, STS3250, SMS40 | 1000000 (STS), 115200 (SMS) | 12-bit (0-4095) | Standard modes |
| SCSCL    | SC09, SC15 | 115200, 1000000 | 10-bit (0-1023) | Position + PWM |
| HLSCL    | HLS series | 115200, 1000000 | 12-bit (0-4095) | **Includes force/torque mode** |

\* Factory default is typically 1000000 (1M) for most models, but can be reconfigured. Check your specific servo's current setting.

### Connection Requirements

- **Serial Interface**: USB-to-Serial adapter (FTDI, CP2102, CH340) or direct UART
- **Wiring**: TX/RX/GND (half-duplex serial communication)
- **Power Supply**: External power required (6-14V depending on model)
- **Data Bus**: Daisy-chainable for multi-servo setups

> **⚠️ Bus Compatibility Warning:** The motor controller may not support mixing different motor types (e.g., STS3215 and SC09) on the same serial bus. Different series may use incompatible protocols, baud rates, or memory maps. For reliable operation, use motors from the same series on a single bus. If you need to control different motor types, consider using separate serial interfaces.

### Tested Platforms

- Ubuntu 20.04/22.04 (x86_64, ARM64)
- Raspberry Pi OS (Pi 3/4/5, Zero 2)

> **⚠️ Important:** This SDK has only been tested with **STS3215** servo motors. While it should work with other SMS/SCSCL/HLSCL series motors according to protocol specifications, exercise caution and thoroughly test functionality before deploying with other motor models.

## Installation

### Prerequisites

```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

### Build from Source

```bash
# Clone repository
git clone https://github.com/adityakamath/SCServo_Linux.git
cd SCServo_Linux

# Build library
mkdir -p build
cd build
cmake ..
make -j4

# The build produces:
# - build/libSCServo.a (static library)
```

### Serial Port Permissions

Add your user to the dialout group for `/dev/ttyUSB*` access:

```bash
sudo usermod -a -G dialout $USER
# Log out and log back in for changes to take effect
```

Alternatively, use `sudo` for testing:

```bash
sudo ./WritePos /dev/ttyUSB0
```

## Quick Start

### Step 1: Connect Hardware

Connect your motors in a chain to the motor driver board, and connect the motor driver via USB to a Linux computer. Connect the motor driver and the Linux computer to appropriate (check specs) power sources.

> **⚠️ Important:** Each motor must have a unique motor ID, if there are duplicates on the chain, they are not detected. Since new motors always have motor ID 1, first assign each motor unique IDs one by one, and only then connect them to the chain.

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

**⚠️ Important:** Before running any example, double check the baud rate and motor ID(s) in the example code, and change them if needed to match your setup. Then build the example before running it. 

```bash
# Position control example
cd examples/SMS_STS/WritePos
cmake . && make
./WritePos /dev/ttyUSB0

# Servo will oscillate between position 0 and 4095
# Press Ctrl+C to stop (torque automatically disabled)
```

## Operating Modes

### Mode 0: Servo Mode (Position Control)

- **Use case**: Precise position control (robot arms, pan-tilt mechanisms)
- **Control parameters**: Position, Speed, Acceleration (SMS/STS, HLSCL) or Time (SCSCL)
- **Functions**: `WritePosEx()` (SMS/STS, HLSCL), `WritePos()` (SCSCL), `RegWritePosEx()`, `SyncWritePosEx()`
- **Behavior**: Motor moves to target position and holds
- **Protocols**: SMS/STS, SCSCL, HLSCL

### Mode 1: Closed-Loop Wheel Mode (Velocity Control)

- **Use case**: Continuous rotation with speed feedback (wheeled mobile robots)
- **Control parameters**: Speed (±3400 steps/s), Acceleration
- **Functions**: `WriteSpe()`, `RegWriteSpe()` (SMS/STS only), `SyncWriteSpe()` (SMS/STS, HLSCL)
- **Behavior**: Motor maintains target speed using encoder feedback
- **Protocols**: SMS/STS, HLSCL
- **Note**: SCSCL does not support velocity control mode

### Mode 2: Protocol-Specific Modes

Mode 2 behavior differs by protocol. SCSCL uses mode numbering differently (Mode 1 = PWM).

#### SMS/STS Mode 2: Open-Loop Wheel Mode (PWM Control)

- **Use case**: Direct motor power control without feedback
- **Control parameters**: PWM duty cycle (-1000 to +1000, represents -100% to +100% duty cycle)
- **Functions**: `WritePwm()`, `RegWritePwm()`, `SyncWritePwm()`
- **Behavior**: No speed feedback; actual speed depends on load
- **Protocols**: SMS/STS only

#### SCSCL Mode 1: PWM Mode

- **Use case**: Direct motor power control without feedback
- **Control parameters**: PWM duty cycle
- **Functions**: `WritePWM()` (note: capital PWM)
- **Behavior**: Open-loop control via PWM output
- **Protocols**: SCSCL only
- **Note**: SCSCL uses different mode numbering - Mode 0 = Position, Mode 1 = PWM

#### HLSCL Mode 2: Force/Torque Mode (Electric Mode)

- **Use case**: Constant force/torque output (grippers, tensioning, compliant manipulation)
- **Control parameters**: Torque (±1000, negative=CCW, positive=CW)
- **Functions**: `WriteEle()` (HLSCL only)
- **Behavior**: Motor maintains constant torque regardless of position or speed
- **Protocols**: HLSCL only
- **⚠️ Note**: Conversion factors and behavior should be verified against your specific HLS servo model's datasheet.

## Usage Examples

The SDK includes nearly 40 comprehensive examples demonstrating all servo control modes and techniques.

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

The SDK includes multiple example programs in the `examples/` directory, organized by protocol (SMS_STS, SCSCL, HLSCL) and `sandbox` for advanced multi-servo examples. Each example includes inline documentation and can be built individually using CMake.

## Documentation

### Source Code Documentation

All C++ source files and headers include comprehensive Doxygen-style and inline comments:
- Header files (`*.h`) contain detailed function signatures, parameter descriptions, and return values
- Implementation files (`*.cpp`) include inline comments explaining complex logic and algorithms
- Example files feature extensive comments for learning and reference

### Experimental AI-Generated Documentation

> **⚠️ EXPERIMENTAL - USE WITH CAUTION**
> 
> The [`with_ai_docs`](https://github.com/adityakamath/SCServo_Linux/tree/with_ai_docs) branch contains extensive AI-generated documentation (API reference, architecture guide, examples guide, troubleshooting guide). This documentation was **entirely generated by AI** and has **NOT been fully verified** against the actual implementation.

> The AI-generated docs are provided "as-is" for experimental purposes only. When in doubt, trust the code, not the docs.

> DeepWiki provides an additional set of AI-generated documentation. Once again, this has not been manually reviewed, and is purely experimental

## Contributing

Contributions (issues, PRs) are welcome, but this is only a personal project for now, so do not expect active maintenance!

## References & Acknowledgments

This repository is a fork of Feetech's official [FTServo_Linux SDK](https://gitee.com/ftservo/FTServo_Linux) with the following enhancements:

**Improvements in this fork:**
- Enhanced code comments and Doxygen documentation
- Comprehensive English documentation (README, experimental AI-generated documentation including API reference, architecture guide, troubleshooting guide)
- HLSCL protocol support for HLS series servos
- Additional example programs with detailed explanations
- Code quality improvements (includes, error handling, consistency)

---
