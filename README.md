# SCServo_Linux
Copy of SCServo_Linux SDK from [Feetech's official repository](https://gitee.com/ftservo/SCServoSDK/tree/master) with personal modifications for the SMS/STS series of serial bus servo motors.

## Quick Start

### Building the Library
1. Clone this repository and enter the directory: `cd SCServo_Linux`
2. Build SCServo libraries: `cmake . && make`
3. Choose an example and enter the directory: `cd examples/SMS_STS/WritePos`
4. Build the example: `cmake . && make`
5. Execute the example: `sudo ./WritePos /dev/ttyUSB0`

**Note:** Adjust `/dev/ttyUSB0` according to the serial port of your device (e.g., `/dev/ttyACM0`, `/dev/ttyUSB1`)

### Serial Port Permissions
If you encounter permission errors, add your user to the dialout group:
```bash
sudo usermod -a -G dialout $USER
# Log out and log back in for changes to take effect
```

Or run with sudo (not recommended for production use):
```bash
sudo ./WritePos /dev/ttyUSB0
```

## Structure

* Communication layer: SCS
* Hardware interface layer: SCSerial
* Application layer:
    * SMSBL SMSCL SCSCL correspond to the three series of Feetech respectively
    * SMS_STS is compatible with SMS/STS series servos

### Definition

```
SMSBL sm;      //Define SMSBL series servo
SMSCL sm;      //Define SMSCL series servo
SCSCL sc;      //Define SCSCL series servo
SMS_STS sm_st; //Define SMS or STS series servo
```

For each interface of SMSCL\SMSBL\SCSCL\SMS_STS, refer to the corresponding header file

### Header files

* INST.h:                  Instruction definition header file
* SCS.h/SCS.cpp:           Communication layer program
* SCSerial.h/SCSerial.cpp: Hardware interface program
* SMSBL.h/SMSBL.cpp:       SMSBL Application layer program
* SMSCL.h/SMSCL.cpp:       SMSCL Application layer program
* SCSCL.h/SCSCL.cpp:       SCSCL Application layer program
* SMS_STS.h/SMS_STS.cpp:   SMS/STS Application layer program

(The memory table is defined in the application layer program header file: SMSBL.h\SMSCL.h\SCSCL.h\SMS_STS.h. There are differences in the definition of the memory table for different series of servos)

```
                               SMSBL class
SCS class<---SCSerial class<---SMSCL class
                               SCSCL class
                               SMS_STS class
```

## Operating Modes

The SMS/STS servos support three operating modes:

### Mode 0: Servo Mode (Position Control)
- **Use case:** Precise position control (e.g., robot arms, pan-tilt mechanisms)
- **Control parameters:** Position, Speed, Acceleration
- **Functions:** `WritePosEx()`, `RegWritePosEx()`, `SyncWritePosEx()`
- **Range:** 0-4095 steps (full rotation varies by model)

### Mode 1: Closed-Loop Wheel Mode (Velocity Control)
- **Use case:** Continuous rotation with speed feedback (e.g., omnidirectional wheels, mobile robots)
- **Control parameters:** Speed, Acceleration
- **Functions:** `WriteSpe()`, `RegWriteSpe()`, `SyncWriteSpe()`
- **Speed range:** ±3400 steps/s (approximately)
- **Note:** Motor maintains target speed using encoder feedback

### Mode 2: Open-Loop Wheel Mode (PWM Control)
- **Use case:** Direct motor power control without feedback
- **Control parameters:** PWM duty cycle
- **Functions:** `WritePwm()`, `RegWritePwm()`, `SyncWritePwm()`
- **PWM range:** -1000 to +1000 (percentage: -100% to +100%)
- **Note:** No speed feedback; actual speed depends on load

## API Modifications & Enhancements

### New Features in `SMS_STS.h/.cpp`

1. **Enhanced Mode Control**
   - **Updated:** `WheelMode(ID)` → `Mode(ID, mode)`
   - **Modes:** 0 (servo), 1 (closed-loop wheel), 2 (open-loop wheel)
   - **Note:** Mode 3 (stepper mode) exists in hardware but is not implemented in this SDK

2. **Speed Control (Mode 1)**
   - `WriteSpe(ID, Speed, Acc)` - Write speed to single motor
   - `RegWriteSpe(ID, Speed, Acc)` - Async write speed (execute with `RegWriteAction()`)
   - `SyncWriteSpe(ID[], IDN, Speed[], Acc[])` - Sync write to multiple motors
   - **Known Issue:** `SyncWriteSpe()` currently not functioning correctly

3. **PWM Control (Mode 2)**
   - `WritePwm(ID, Pwm)` - Write PWM to single motor
   - `RegWritePwm(ID, Pwm)` - Async write PWM
   - `SyncWritePwm(ID[], IDN, Pwm[])` - Sync write PWM to multiple motors
   - **Status:** All PWM functions working correctly

4. **Safety Features**
   - Added `SignalHandler()` to all write examples
   - Disables torque on `CTRL+C` termination
   - Allows manual motor rotation after exit

### Parameters Guide

| Parameter | Type | Range | Unit | Description |
|-----------|------|-------|------|-------------|
| ID | u8 | 0-253 | - | Motor ID (254=broadcast) |
| Position | s16 | 0-4095 | steps | Target position |
| Speed | s16 | ±3400 | steps/s | Target velocity |
| Acc | u8 | 0-254 | ×100 steps/s² | Acceleration |
| Pwm | s16 | ±1000 | 0.1% | PWM duty cycle |

**Speed Conversion:**
- Factory default: 0.0146 rpm
- 2400 steps/s ≈ 35 rpm (approximate, model-dependent)

## Examples

### Official Examples (`examples/SMS_STS/`)
Standard examples with Chinese comments:
- `WritePos` - Position control example
- `WriteSpe` - Speed control example
- `SyncWritePos` - Synchronized position control
- `RegWritePos` - Asynchronous position write
- `Ping` - Motor discovery and ping
- `FeedBack` - Read motor feedback data
- `CalibrationOfs` - Midpoint calibration
- `ProgramEprom` - EEPROM programming
- `Broadcast` - Broadcast commands
- `SyncRead` - Synchronized read

### Custom Examples (`examples/sandbox/`)
Enhanced examples with English comments for 3-omnidirectional-wheel robot:

#### Position Control
- `WritePosition` - Basic position control
- `RegWritePosition` - Async position control
- `SyncWritePosition` - Sync multi-motor position

#### Velocity Control (Mode 1)
- `WriteSpeed` - Basic speed control
- `RegWriteSpeed` - Async speed control
- `SyncWriteSpeed` - Sync multi-motor speed

#### PWM Control (Mode 2)
- `WritePWM` - Basic PWM control
- `RegWritePWM` - Async PWM control
- `SyncWritePWM` - Sync multi-motor PWM

#### Utilities
- `ReadData` - Read all motor parameters
- `MidpointCalib` - Calibrate motor center position

**Note:** Sandbox examples are configured for motor IDs 7, 8, 9 (3-wheel omni robot)

## Hardware Specifications

### Supported Motors
- **STS Series:** STS3215, STS3250, etc.
- **SMS Series:** SMS series servos
- **SMSBL/SMSCL/SCSCL:** Various Feetech series

### Communication
- **Protocol:** Half-duplex asynchronous serial
- **Supported Baud Rates:**
  - 1M (1000000) - Default for STS series
  - 500K, 250K, 128K
  - 115200 - Default for SMS series
  - 76800, 57600, 38400

### Memory Map
- **EPROM (Read-Only):** Model number (addresses 3-4)
- **EPROM (Read/Write):** ID, baud rate, angle limits, offsets, mode (addresses 5-33)
- **SRAM (Read/Write):** Torque enable, acceleration, goal position/speed, lock (addresses 40-55)
- **SRAM (Read-Only):** Present position, speed, load, voltage, temperature, current (addresses 56-70)

See `SMS_STS.h` for complete memory table definitions.

## Troubleshooting

### Common Issues

**1. Permission Denied Error**
```
Error opening serial port: Permission denied
```
**Solution:** Add user to dialout group or use sudo (see Serial Port Permissions section)

**2. Motor Not Responding**
- Check power supply (motors require external power, typically 10-14V)
- Verify correct baud rate (1M for STS, 115200 for SMS)
- Check motor ID matches the code
- Use `Ping` example to discover motors
- Verify serial port device name

**3. Motor Jittering or Unstable**
- Reduce acceleration parameter
- Lower speed values
- Check for mechanical binding
- Verify power supply can provide sufficient current



**5. Build Errors**
- Clean build directory: `rm -rf CMakeCache.txt CMakeFiles/`
- Ensure CMake 3.0+ installed: `cmake --version`
- Check compiler: `g++ --version`

### Debug Tips
1. **Enable verbose output:** Modify `SCSerial.cpp` to print debug messages
2. **Test with Ping:** Use the Ping example to verify communication
3. **Read motor data:** Use ReadData example to check motor status
4. **Serial monitoring:** Use `screen` or `minicom` to monitor serial traffic
   ```bash
   screen /dev/ttyUSB0 1000000
   ```

## Advanced Usage

### Changing Motor ID
```cpp
SMS_STS sm_st;
sm_st.begin(1000000, "/dev/ttyUSB0");
sm_st.unLockEprom(OLD_ID);     // Unlock EEPROM
sm_st.writeByte(OLD_ID, SMS_STS_ID, NEW_ID);  // Write new ID
sm_st.LockEprom(NEW_ID);       // Lock EEPROM
```

### Changing Baud Rate
```cpp
sm_st.unLockEprom(ID);
sm_st.writeByte(ID, SMS_STS_BAUD_RATE, SMS_STS_1M);  // Set to 1M
sm_st.LockEprom(ID);
// Reconnect at new baud rate
```

### Reading Motor Feedback
```cpp
sm_st.FeedBack(ID);            // Trigger feedback read
int pos = sm_st.ReadPos(ID);   // Position: 0-4095
int spd = sm_st.ReadSpeed(ID); // Speed: ±3400 steps/s
int load = sm_st.ReadLoad(ID); // Load: -1000 to +1000
int volt = sm_st.ReadVoltage(ID);  // Voltage: mV
int temp = sm_st.ReadTemper(ID);   // Temperature: °C
int curr = sm_st.ReadCurrent(ID);  // Current: mA
```

### Broadcast Commands
Use motor ID 254 to send commands to all motors simultaneously:
```cpp
sm_st.EnableTorque(254, 1);  // Enable torque on all motors
sm_st.Mode(254, 1);          // Set all motors to velocity mode
```

## Integration with ROS/ROS2

This C++ SDK can be integrated with ROS/ROS2 as an alternative to the Python `scservo-sdk`.

**Advantages:**
- Lower latency
- Direct C++ integration with ROS nodes
- Better performance for high-frequency control

**Note:** The Python `scservo-sdk` is currently used in the `lekiwi_ros2` package and provides similar functionality with easier integration for Python-based nodes.

## Related Projects

- [lekiwi_ros2](https://github.com/yourusername/lekiwi_ros2) - ROS2 integration using Python scservo-sdk
- [SCServo Python SDK](https://pypi.org/project/scservo-sdk/) - Official Python SDK

## License

Based on Feetech's official SDK. See original repository for license details.

## Contributing

Contributions welcome! Please test thoroughly with actual hardware before submitting PRs.

**Known TODOs:**
- Fix `SyncWriteSpe()` function
- Add support for Mode 3 (stepper mode)
- Add more comprehensive examples
- Improve error handling and reporting

## Major Code Improvements (2025)

- Buffer overflow protection added to all buffer operations.
- Variable Length Arrays replaced with safe C++ containers.
- File descriptor leaks fixed throughout the codebase.
- All public classes and methods now use Doxygen documentation in English.
- Safety: Example programs disable motor torque on exit (CTRL+C).

For details, see the source code and example files included in this repository.

## Python Bindings


Python bindings are available for the following classes:
- `SMS_STS`, `SMSCL`, `SMSBL`, `SCSerial`, and `SCSCL` (core Feetech servo classes)
- All safe, default-constructible methods are exposed.
- Array/pointer-based methods are wrapped for Python compatibility.
- The bindings are implemented using [nanobind](https://github.com/wjakob/nanobind) for modern, high-performance Python/C++ integration.

**Usage:**
- Install via `python setup.py develop` or use the built `.so` directly.
- Example usage and API documentation are provided in the source and docstrings.

**Limitations:**
- Only methods with simple types are directly exposed.
- Advanced features (e.g., bulk/sync operations) are available via Python wrapper functions.
- For full hardware control, use the C++ API or extend the Python bindings as needed.
