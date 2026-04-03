---
layout: page
title: System Design
---

System design and implementation guide for the SCServo_Linux SDK.

## Overview

The SCServo_Linux SDK is a high-performance Linux C++ library for controlling Feetech serial bus servo motors. It provides native C++17 support for SMS/STS, SCSCL, and HLSCL series servos with position, velocity, PWM, and force control capabilities. The SDK features multi-servo synchronization, comprehensive feedback, and EEPROM configuration tools.

**Key Features:**

- **Multi-Protocol Support:** SMS/STS, SCSCL, and HLSCL series with unified API
- **Multiple Control Modes:** Position (servo), Velocity (closed-loop wheel), PWM (open-loop), Force/Torque
- **Multi-Servo Operations:** Synchronized Write and Broadcast for efficient coordination
- **Comprehensive Feedback:** Position, speed, load, voltage, temperature, current
- **Configuration Tools:** EEPROM programming, midpoint calibration, ID management
- **Platform Optimized:** Linux x86_64, ARM64, Raspberry Pi with native C++17

---

## System Architecture

<div align="center">
  <img src="assets/img/system_architecture.svg" alt="System Architecture" width="600"/>
</div>

The SCServo_Linux SDK implements a **layered architecture** with four distinct tiers:

**Architecture Layers:**

1. **Serial Communication Layer (`SCSerial`)** - POSIX-compliant serial I/O operations
2. **Protocol Base Layer (`SCS`)** - Shared packet construction and CRC calculations
3. **Protocol Implementation Layer** - Protocol-specific control methods (`SMS_STS`, `SCSCL`, `HLSCL`)
4. **Application Layer** - User code integrating servo control functionality

### Class Hierarchy

```
SCSerial (hardware I/O)
    ↓
SCS (base protocol operations)
    ↓
├─ SMS_STS (implements position, velocity, PWM)
├─ SCSCL (implements position, PWM)
└─ HLSCL (implements position, velocity, force)

Supporting utilities:
├─ ServoUtils (helper functions)
├─ SyncWriteBuffer (synchronized writes)
└─ ServoErrors (error codes and handling)
```

---

## Hardware Support Matrix

The library targets three servo protocol families with different capabilities:

| Protocol Family | Models | Resolution | Baud Rates | Control Modes |
| --- | --- | --- | --- | --- |
| **SMS/STS** | STS3215*, STS3032, STS3250, SMS40 | 12-bit (0-4095) | 1M / 115.2k | Position, Velocity, PWM |
| **SCSCL** | SC09, SC15 | 10-bit (0-1023) | 1M / 115.2k | Position, PWM only |
| **HLSCL** | HLS series | 12-bit (0-4095) | 1M / 115.2k | Position, Velocity, Force |

\* **Testing Note:** Only STS3215 extensively tested; other models require verification.

**Connection Requirements:**

- **Serial Interface:** USB-to-Serial adapter (FTDI, CP2102, CH340) or direct UART
- **Wiring:** TX/RX/GND (half-duplex serial communication)
- **Power Supply:** External power required (6-14V depending on model)
- **Data Bus:** Daisy-chainable for multi-servo setups (up to 253 servos)

⚠️ **Bus Compatibility Warning:** Do not mix different motor types (e.g., STS3215 and SC09) on the same serial bus. Different series may use incompatible protocols, baud rates, or memory maps.

---

## Operating Modes

Each protocol family supports different control modes optimized for specific use cases:

| Mode | Use Case | Control Parameters | Protocol Support | Key Functions |
| --- | --- | --- | --- | --- |
| **Mode 0: Position** | Robot arms, pan-tilt mechanisms | Position, Speed, Acceleration/Time | SMS/STS, SCSCL, HLSCL | `WritePosEx()`, `WritePos()`, `SyncWritePosEx()` |
| **Mode 1: Velocity** | Wheeled mobile robots, continuous rotation | Speed (±3400 steps/s), Acceleration | SMS/STS, HLSCL | `WriteSpe()`, `RegWriteSpe()`, `SyncWriteSpe()` |
| **Mode 2: PWM** | Direct motor power control | PWM duty cycle (-1000 to +1000) | SMS/STS, SCSCL | `WritePwm()`, `WritePWM()` (SCSCL) |
| **Mode 2: Force/Torque** | Grippers, tensioning, compliant manipulation | Torque (±1000) | HLSCL only | `WriteEle()` |

**Important Mode Behavior:**

- **Mode 0 (Position):** Motor moves to target position and holds using closed-loop control
- **Mode 1 (Velocity):** Motor maintains target speed using encoder feedback (SMS/STS, HLSCL only)
- **Mode 2 (PWM):** Open-loop control with no feedback; actual speed depends on load (SMS/STS, SCSCL)
- **Mode 2 (Force):** Maintains constant torque regardless of position or speed (HLSCL only)

**SCSCL Mode Numbering Note:** SCSCL uses different mode numbering - Mode 0 = Position, Mode 1 = PWM (no velocity mode).

---

## Communication Patterns

The SDK implements four synchronization strategies for multi-servo coordination:

| Strategy | Timing Accuracy | Packet Count | Use Case | Example Function |
| --- | --- | --- | --- | --- |
| **Direct Write** | ~10ms skew | Per-servo | Single servo testing | `WritePosEx()`, `WriteSpe()` |
| **Register Write** | ~1ms skew | Per-servo + Action | General multi-servo | `RegWritePosEx()` + `RegWriteAction()` |
| **Synchronized Write** | <1ms skew | Single packet | Production systems | `SyncWritePosEx()`, `SyncWriteSpe()` |
| **Broadcast** | Simultaneous | Single packet | Identical commands to all servos | `WritePosEx(0xFE, ...)` |

**Synchronization Benefits:**

- **Direct Write:** Simple API, good for single servo or testing
- **Register Write:** Better timing than direct write, commands triggered by single action packet
- **Synchronized Write:** Best timing accuracy, single packet contains all servo commands
- **Broadcast:** Fastest, sends identical command to all servos (ID 0xFE)

---

## Protocol Packet Structure

All protocols use a common packet framework with CRC validation:

```
[Header] [ID] [Length] [Instruction] [Parameters...] [CRC-Low] [CRC-High]
```

**Packet Components:**

- **Header:** Protocol-specific start bytes (0xFF 0xFF for SMS/STS)
- **ID:** Servo ID (1-253) or broadcast (0xFE)
- **Length:** Payload length including instruction and parameters
- **Instruction:** Command type (read, write, sync write, etc.)
- **Parameters:** Command-specific data (position, speed, etc.)
- **CRC:** 16-bit cyclic redundancy check for error detection

**CRC Calculation:**

The SDK implements polynomial-based CRC calculation in `SCS::crc()` method for packet integrity verification. All received packets are validated before processing.

---

## Serial Interface Implementation

The `SCSerial` class encapsulates POSIX termios operations for half-duplex serial communication:

**Serial Configuration:**

- **Device Access:** Opens with `O_RDWR | O_NOCTTY | O_NONBLOCK` flags
- **Termios Setup:** Configures for half-duplex, raw mode, no parity
- **Timeout Handling:** Implements timeout via `select()` system call
- **Thread Safety:** Provides thread-safe read/write operations
- **Device Paths:** `/dev/ttyUSB*` or `/dev/ttyACM*` on Linux

**Baud Rate Support:**

Configurable baud rates: 9600, 19200, 38400, 57600, 115200, 500000, 1000000
- Default: 1000000 (1M) for STS series
- Default: 115200 for SMS series
- Configurable via EEPROM

**Permissions:**

User must belong to `dialout` group for serial port access:
```bash
sudo usermod -a -G dialout $USER
```

---

## Feedback and State Reading

The SDK provides comprehensive feedback from servo motors:

| Feedback Type | Unit | Read Function | Description |
| --- | --- | --- | --- |
| **Position** | Steps (0-4095 or 0-1023) | `ReadPos()` | Current angular position |
| **Velocity** | Steps/s (±3400 max) | `ReadSpeed()` | Current rotation speed |
| **Load** | ±1000 (percentage) | `ReadLoad()` | Motor load (-100% to +100%) |
| **Voltage** | 0.1V units | `ReadVoltage()` | Supply voltage |
| **Temperature** | °C | `ReadTemper()` | Internal temperature |
| **Current** | 6.5mA units | `ReadCurrent()` | Motor current draw |
| **Motion Status** | Boolean | `ReadMove()` | Is motor moving (1) or stopped (0) |

**Synchronized Feedback:**

The SDK supports reading feedback from multiple servos efficiently using batch read operations, reducing serial bus overhead.

---

## Configuration and EEPROM Management

The SDK provides tools for persistent servo configuration:

**Configuration Functions:**

- **ID Management:** `unLockEprom()`, `LockEprom()`, `setID()` - Change servo ID
- **Baud Rate:** `setBaudRate()` - Configure communication speed
- **Operating Mode:** `setMode()` - Set position/velocity/PWM/force mode
- **Limits:** Set position limits, voltage limits, temperature limits
- **Midpoint Calibration:** `CalibrationOfs()` - Calibrate center position
- **Factory Reset:** Restore default settings

**EEPROM Protection:**

EEPROM writes require unlocking to prevent accidental configuration changes:

```cpp
servo.unLockEprom(ID);     // Unlock EEPROM
servo.setID(ID, newID);     // Change configuration
servo.LockEprom(ID);        // Lock EEPROM
```

---

## Build System Design

The CMake-based build system uses a two-tier approach:

**Root Configuration** (`CMakeLists.txt`):
- Compiles core library with C++17 standard
- Optimization: `-O3` for aggressive inline expansion
- Position-independent code: `-fPIC` flag
- Output: `build/libSCServo.a` (static library)

**Per-Example Configuration:**
- Links against pre-built static library
- C++11 compatibility for broader platform support
- Independent CMake configuration per example
- Output: Individual executables (statically linked)

**Build Process:**

```bash
mkdir -p build && cd build
cmake ..
make -j4
```

**Library Outputs:**
- Static library: `build/libSCServo.a` (~450KB)
- Example executables: `examples/*/executable` (~500KB each)

---

## Error Handling Strategy

Error codes defined in `ServoErrors.h` cover all failure scenarios:

**Error Categories:**

- **Communication Errors:** Timeouts, CRC failures, no response
- **Parameter Errors:** Invalid servo IDs, out-of-range values
- **Hardware Errors:** EEPROM write protection, overheat, overload
- **Protocol Errors:** Invalid packet format, wrong instruction

**Error Handling Pattern:**

Functions return error codes that can be checked:

```cpp
int result = servo.WritePosEx(1, 2048, 1000, 50);
if (result < 0) {
    // Handle error
}
```

---

## Performance Characteristics

**Latency:**
- Direct Write: ~5ms per servo
- Synchronized Write: <1ms for all servos in single packet
- Serial I/O: Non-blocking with configurable timeouts

**Binary Size:**
- Static library: ~450KB
- Example executable: ~500KB (includes library)

**Optimization:**
- Compiled with `-O3` flag for aggressive optimization
- Inline expansion for frequently called functions
- Position-independent code enables future shared library conversion

**Scalability:**
- Supports up to 253 servos on single serial bus
- Daisy-chain topology minimizes wiring complexity
- Synchronized write enables efficient multi-servo control

---

## Example Program Organization

The SDK includes 25+ comprehensive examples organized by protocol:

**SMS_STS Examples:**
- `Ping` - Test servo connectivity
- `WritePos` - Basic position control
- `WritePosEx` - Position with speed and acceleration
- `WriteSpe` - Velocity control
- `SyncWriteSpe` - Synchronized velocity for multiple servos
- `ReadPos` - Read current position
- `Feedback` - Comprehensive state reading
- And more...

**SCSCL Examples:**
- `WritePos` - Position control
- `WritePWM` - PWM mode control
- `ReadPos` - Position feedback
- And more...

**HLSCL Examples:**
- `WritePos` - Position control
- `WriteSpe` - Velocity control
- `WriteEle` - Force/torque control
- `Feedback` - State reading
- And more...

**Sandbox Examples:**
- Advanced synchronization benchmarks
- Multi-servo coordination patterns
- Performance testing utilities

Each example includes:
- Inline documentation
- Independent CMake configuration
- Clear usage instructions
- Signal handling (Ctrl+C cleanup)

---

## Platform Requirements

**Operating System:**
- Linux kernel 3.10+ required
- Tested: Ubuntu 20.04/22.04, Raspberry Pi OS
- Architecture: x86_64, ARM64

**Build Tools:**
- Compiler: GCC 7+ or Clang 5+ with C++17 support
- CMake: 3.10 or newer
- Make: GNU Make

**Runtime:**
- No external library dependencies
- Static linking produces self-contained executables
- Suitable for embedded systems

**Permissions:**
- User must belong to `dialout` group for serial access
- Alternative: Use `sudo` for testing (not recommended for production)

---

## Key Design Decisions

**Static Library Approach:**

The SDK uses static linking to produce self-contained executables without runtime dependencies. This makes it ideal for embedded systems and simplifies deployment.

**Header-Only Utilities:**

`ServoUtils` and `SyncWriteBuffer` are implemented as headers to avoid link-time complications and enable inline optimization.

**C++ Standard Divergence:**

The library uses C++17 internally for modern features while examples compile with C++11 for broader platform compatibility.

**Position-Independent Code:**

Compiled with `-fPIC` flag, enabling potential future conversion to shared libraries if needed.

**Half-Duplex Serial:**

Implements proper half-duplex communication handling for RS485/TTL serial buses, essential for daisy-chain topology.

**Atomic ACC + Speed in `SyncWriteSpe()`:**

`SyncWriteSpe()` uses a single 7-byte sync-write packet that spans registers
`SMS_STS_ACC` (41) through `SMS_STS_GOAL_SPEED_H` (47): ACC, GOAL_POSITION (set
to 0, ignored in wheel mode), GOAL_TIME (set to 0), and GOAL_SPEED. Writing both
values in one atomic bus transaction ensures that each servo's acceleration
profile and speed target arrive together, with no risk of the ACC value being
lost or arriving ahead of (or behind) its speed command on a busy half-duplex bus.

This is critical for synchronised multi-motor applications such as omni-wheel
robots where different wheels need different per-wheel accelerations (proportional
to their individual velocity deltas) so that all wheels finish ramping at the
same wall-clock time. Sending ACC and GOAL_SPEED separately—even in rapid
succession—creates a window where a servo could receive and execute a new speed
command with its old (stale) ACC register value.

---

## API Structure

**Core Classes:**

- **`SMS_STS`** - Control SMS and STS series servos (position, velocity, PWM)
- **`SCSCL`** - Control SCSCL series servos (position, PWM)
- **`HLSCL`** - Control HLS series servos (position, velocity, force)
- **`SCSerial`** - Low-level serial communication
- **`SCS`** - Base protocol operations (shared by all protocols)

**Unified API Pattern:**

All protocol classes follow consistent naming:
- `begin()` / `end()` - Initialize/cleanup
- `WritePosEx()` - Write position with parameters
- `ReadPos()` - Read current position
- `InitMotor()` - Initialize motor mode and torque
- `EnableTorque()` / `UnlockTorque()` - Control torque

**Usage Example:**

```cpp
#include "SCServo.h"

SMS_STS servo;
servo.begin(1000000, "/dev/ttyUSB0");
servo.InitMotor(1, 0, 1);  // ID=1, Mode=0 (position), Torque=ON
servo.WritePosEx(1, 2048, 1000, 50);  // Move to center position
int pos = servo.ReadPos(1);
servo.end();
```

---

## Additional Resources

- [SCServo_Linux README](https://github.com/adityakamath/SCServo_Linux/blob/main/README.md)
- [Quick Start guide](quick-start.md)
- [DeepWiki AI Documentation](https://deepwiki.com/adityakamath/SCServo_Linux) (Experimental)
- [Original FTServo_Linux SDK](https://gitee.com/ftservo/FTServo_Linux)
- [Feetech Official Website](https://www.feetechrc.com/)
- [Blog Post: Serial Bus Servo Motors in 2025](https://kamathrobotics.com/serial-bus-servo-motors-in-2025)
