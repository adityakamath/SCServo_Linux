# API Reference

> **⚠️ AI-Generated Documentation**: This documentation was generated with AI assistance and may contain inconsistencies with the actual codebase. Always verify function signatures, parameter types, and behavior against the source code headers (`SMS_STS.h`, `SCSCL.h`, etc.) when in doubt.

Complete API documentation for the SCServo_Linux library.

## Table of Contents

- [Initialization & Cleanup](#initialization--cleanup)
- [Position Control (Mode 0)](#position-control-mode-0)
- [Velocity Control (Mode 1)](#velocity-control-mode-1)
- [PWM Control (Mode 2)](#pwm-control-mode-2)
- [Force Control (Mode 2 - HLSCL Only)](#force-control-mode-2---hlscl-only)
- [Feedback & Status](#feedback--status)
- [Configuration & Management](#configuration--management)
- [Low-Level Memory Access](#low-level-memory-access)
- [Memory Table Reference](#memory-table-reference)
  - [SMS/STS Series](#smssts-series)
  - [HLSCL Series](#hlscl-series)

---

## Initialization & Cleanup

### `bool begin(int baudrate, const char* port)`

Initialize serial connection to servo bus.

**Parameters:**
- `baudrate` - Communication speed in bps
  - Supported by driver: 9600, 19200, 38400, 57600, 115200, 500000, 1000000
  - Default for STS series: 1000000 (1M)
  - Default for SMS/SCSCL series: 115200
  - Note: 9600 and 19200 are supported by the driver but may not be configurable in servo EEPROM
- `port` - Serial device path
  - Linux: `/dev/ttyUSB0`, `/dev/ttyACM0`, `/dev/ttyS0`, etc.

**Returns:**
- `true` - Connection established successfully
- `false` - Failed to open port (check permissions, device path, or hardware connection)

**Example:**
```cpp
SMS_STS servo;
if (!servo.begin(1000000, "/dev/ttyUSB0")) {
    printf("Failed to initialize!\n");
    return -1;
}
```

**Notes:**
- User must have read/write permissions on serial port (add to `dialout` group)
- Only one instance should control a serial port at a time
- Call `end()` before reinitializing with different parameters

---

### `void end()`

Close serial connection and release resources.

**Example:**
```cpp
servo.end();
```

**Notes:**
- Automatically called by destructor
- Does not disable motor torque - call `EnableTorque(ID, 0)` first if needed
- Safe to call multiple times

---

### `int InitMotor(u8 ID, u8 mode, u8 torque)`

Initialize motor with operating mode and torque setting (convenience function).

**Parameters:**
- `ID` - Motor ID (0-253, 254 for broadcast)
- `mode` - Operating mode
  - `0` - Servo mode (position control)
  - `1` - Closed-loop wheel mode (velocity control)
  - `2` - Open-loop wheel mode (PWM control)
  - `3` - Stepper mode (not implemented in SDK)
- `torque` - Torque enable
  - `0` - Disabled (motor can be manually rotated)
  - `1` - Enabled (motor holds position/speed)

**Returns:**
- `1` - Success
- `0` - Failure (invalid mode parameter or communication error)

**Example:**
```cpp
servo.InitMotor(1, 1, 1);  // Motor ID=1, velocity mode, torque enabled
```

**Notes:**
- Equivalent to calling `Mode(ID, mode)` then `EnableTorque(ID, torque)`
- Mode changes require unlocking EEPROM first (done automatically by this function)

---

## Position Control (Mode 0)

Position control mode moves servos to specific angular positions and holds them.

### `int WritePos(u8 ID, s16 Position, u16 Time, u16 Speed)`

**Legacy position control function.** Move servo to target position over specified time.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Position` - Target position
  - 12-bit servos (SMS_STS): 0-4095
  - 10-bit servos (SCSCL): 0-1023
- `Time` - Time to reach position in milliseconds (0-65535 ms)
- `Speed` - Reserved parameter (not used in this mode)

**Returns:**
- `1` - Success
- `0` - Communication failure

**Notes:**
- This is the older API - prefer `WritePosEx()` for new code
- Time-based movement may not provide smooth acceleration
- Use `WritePosEx()` for better control with speed and acceleration parameters

---

### `int WritePosEx(u8 ID, s16 Position, u16 Speed, u8 Acc)`

**Extended position control.** Move servo to target position with specified speed and acceleration.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Position` - Target position (0-4095 for 12-bit, 0-1023 for 10-bit)
- `Speed` - Movement speed (0-3400 steps/s)
  - 0 = maximum speed
  - Typical: 2400 steps/s ≈ 35 RPM for STS3215
- `Acc` - Acceleration (0-254)
  - Units: 100 steps/s²
  - 0 = maximum acceleration
  - 254 = slowest acceleration (smoother motion)

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
servo.WritePosEx(1, 2048, 2400, 50);  // ID=1, center position, medium speed, medium accel
```

**Notes:**
- Servo must be in Mode 0 (servo mode)
- Higher acceleration values provide smoother motion but slower response

---

### `int RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 Acc)`

**Buffered position write.** Stage position command for later execution.

**Parameters:**
- Same as `WritePosEx()`

**Returns:**
- `1` - Success (command buffered)
- `0` - Communication failure

**Example:**
```cpp
// Stage commands for 3 motors
servo.RegWritePosEx(1, 1024, 2400, 50);
servo.RegWritePosEx(2, 2048, 2400, 50);
servo.RegWritePosEx(3, 3072, 2400, 50);

// Execute all simultaneously
servo.RegWriteAction(0xFE);  // 0xFE = broadcast
```

**Notes:**
- Commands are buffered in each servo's memory
- Call `RegWriteAction(ID)` or `RegWriteAction(0xFE)` to execute
- More precise synchronization than sequential `WritePosEx()` calls
- Each servo can buffer one RegWrite command at a time

---

### `int SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 Acc[])`

**Synchronized position control.** Command multiple servos simultaneously in a single packet.

**Parameters:**
- `ID[]` - Array of motor IDs
- `IDN` - Number of motors in array
- `Position[]` - Array of target positions
- `Speed[]` - Array of speeds
- `Acc[]` - Array of accelerations

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
u8 IDs[3] = {7, 8, 9};
s16 positions[3] = {1024, 2048, 3072};
u16 speeds[3] = {2400, 2400, 2400};
u8 accel[3] = {50, 50, 50};

servo.SyncWritePosEx(IDs, 3, positions, speeds, accel);
```

**Notes:**
- Most efficient method for controlling multiple servos
- All motors start moving at the same time
- Single packet reduces bus traffic compared to individual writes
- Arrays must be same length as `IDN`

---

## Velocity Control (Mode 1)

Velocity control mode provides continuous rotation at a specified speed.

### `int WriteSpe(u8 ID, s16 Speed, u8 Acc)` *(SMS/STS/SCSCL)*

Set motor velocity.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Speed` - Target velocity in steps/s
  - Range: ±3400 steps/s (approximately)
  - Negative values = reverse rotation
  - 0 = stop
  - Typical: 2400 steps/s ≈ 35 RPM for STS3215
- `Acc` - Acceleration (0-254, units of 100 steps/s²)

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
servo.WriteSpe(1, 2400, 254);   // Forward at ~35 RPM
servo.WriteSpe(1, -2400, 254);  // Reverse at ~35 RPM
servo.WriteSpe(1, 0, 254);      // Stop
```

**Notes:**
- Servo must be in Mode 1 (velocity mode)
- Motor maintains speed using encoder feedback
- Speed is regulated regardless of load (within motor limits)
- Use higher Acc values for smoother speed changes

---

### `int WriteSpe(u8 ID, s16 Speed, u8 Acc, u16 Torque)` *(HLSCL only)*

Set motor velocity with torque limiting.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Speed` - Target velocity in steps/s
  - Range: ±3400 steps/s (approximately)
  - Conversion: Speed × 0.732 = RPM (for HLS series)
  - Example: 60 steps/s × 0.732 = 43.92 RPM
- `Acc` - Acceleration (0-254)
  - Units: 8.7 deg/s² per unit
  - Example: A=50 → 50 × 8.7 = 435 deg/s² acceleration
- `Torque` - Torque limit (0-1000)
  - Units: 6.5 mA per unit
  - Example: T=500 → 500 × 6.5 = 3250 mA max current

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
HLSCL hlscl;
hlscl.WheelMode(1);
hlscl.WriteSpe(1, 60, 50, 500);   // 43.92 RPM, A=435 deg/s², 3250mA limit
hlscl.WriteSpe(1, -60, 50, 500);  // Reverse
hlscl.WriteSpe(1, 0, 50, 500);    // Stop with deceleration
```

**Notes:**
- HLS series provides torque limiting in wheel mode for overcurrent protection
- Torque limit protects both servo and mechanical system

---

### `int RegWriteSpe(u8 ID, s16 Speed, u8 Acc)`

Buffered velocity write.

**Parameters:**
- Same as `WriteSpe()`

**Returns:**
- `1` - Success (command buffered)
- `0` - Communication failure

**Example:**
```cpp
servo.RegWriteSpe(1, 2400, 254);
servo.RegWriteSpe(2, 2400, 254);
servo.RegWriteAction(0xFE);  // Start both motors simultaneously
```

---

### `int SyncWriteSpe(u8 ID[], u8 IDN, s16 Speed[], u8 Acc[])`

Synchronized velocity control for multiple motors.

**Parameters:**
- `ID[]` - Array of motor IDs
- `IDN` - Number of motors
- `Speed[]` - Array of velocities
- `Acc[]` - Array of accelerations

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
u8 IDs[3] = {7, 8, 9};
s16 speeds[3] = {2400, -2400, 2400};  // Mixed directions
u8 accel[3] = {254, 254, 254};

servo.SyncWriteSpe(IDs, 3, speeds, accel);
```

**Notes:**
- Essential for omnidirectional and mecanum wheel robots
- All motors start at the same time for coordinated motion

---

## PWM Control (Mode 2)

PWM mode provides direct open-loop power control without speed feedback.

### `int WritePwm(u8 ID, s16 Pwm)`

Set motor PWM duty cycle.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Pwm` - PWM duty cycle
  - Range: -1000 to +1000
  - Represents: -100% to +100% duty cycle
  - Negative = reverse, Positive = forward

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
servo.WritePwm(1, 500);   // 50% forward
servo.WritePwm(1, -500);  // 50% reverse
servo.WritePwm(1, 0);     // Stop
```

**Notes:**
- Servo must be in Mode 2 (PWM mode)
- No speed feedback - actual speed varies with load
- More responsive than velocity mode but less precise
- Useful for applications where exact speed control isn't critical

---

### `int RegWritePwm(u8 ID, s16 Pwm)`

Buffered PWM write.

**Parameters:**
- Same as `WritePwm()`

**Returns:**
- `1` - Success
- `0` - Communication failure

---

### `int SyncWritePwm(u8 ID[], u8 IDN, s16 Pwm[])`

Synchronized PWM control for multiple motors.

**Parameters:**
- `ID[]` - Array of motor IDs
- `IDN` - Number of motors
- `Pwm[]` - Array of PWM values

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
u8 IDs[3] = {7, 8, 9};
s16 pwm[3] = {500, 500, 500};

servo.SyncWritePwm(IDs, 3, pwm);
```

---

## Force Control (Mode 2 - HLSCL Only)

Force/torque control mode provides constant torque output regardless of position or speed. This mode is **unique to HLS series servos** and enables compliant manipulation, grippers, tensioning, and force-feedback applications.

### `int WriteEle(u8 ID, s16 Torque)` *(HLSCL only)*

Set constant torque output.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Torque` - Target torque
  - Range: -1000 to +1000
  - Negative = Counter-clockwise torque
  - Positive = Clockwise torque
  - Magnitude represents torque strength
  - Units: Proportional to motor current (approximately 6.5 mA per unit)

**Returns:**
- `1` - Success
- `0` - Communication failure

**Example:**
```cpp
HLSCL hlscl;
hlscl.begin(115200, "/dev/ttyUSB0");

// Enable force mode
hlscl.EleMode(1);

// Apply constant torque
hlscl.WriteEle(1, 500);   // CW torque
hlscl.WriteEle(1, -500);  // CCW torque
hlscl.WriteEle(1, 0);     // Zero torque (free rotation)
```

**Notes:**
- Servo must be in Mode 2 (electric/force mode) using `EleMode(ID)` or `Mode(ID, 2)`
- Motor maintains constant torque regardless of position or speed
- Useful for constant-force gripping, spring-like behavior, and compliant control
- Monitor current draw and temperature to prevent overheating
- This mode is **NOT available** on SMS/STS/SCSCL series servos

---

### `int EleMode(u8 ID)` *(HLSCL only)*

Configure servo for force/torque control mode.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
hlscl.EleMode(1);  // Enable force mode
hlscl.WriteEle(1, 300);  // Apply constant torque
```

**Notes:**
- Equivalent to calling `Mode(ID, 2)` with EEPROM unlock/lock
- Mode persists across power cycles
- After enabling force mode, use `WriteEle()` to control torque

---

### `int WheelMode(u8 ID)` *(HLSCL)*

Configure servo for constant velocity mode.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
hlscl.WheelMode(1);  // Enable wheel mode
hlscl.WriteSpe(1, 60, 50, 500);  // Rotate at constant speed
```

**Notes:**
- Equivalent to calling `Mode(ID, 1)` with EEPROM unlock/lock
- Mode persists across power cycles
- After enabling wheel mode, use `WriteSpe()` to control velocity

---

## Feedback & Status

### `int FeedBack(u8 ID)`

Request all feedback data from servo.

**Parameters:**
- `ID` - Motor ID (0-253)
  - Note: Broadcast ID (254) not supported for reads

**Returns:**
- `1` - Success, data cached
- `0` - Communication failure

**Example:**
```cpp
if (servo.FeedBack(1) == 1) {
    int pos = servo.ReadPos(-1);
    int speed = servo.ReadSpeed(-1);
    printf("Position: %d, Speed: %d\n", pos, speed);
}
```

**Notes:**
- Must call before using Read functions with ID=-1
- Reads all status data in one packet (efficient)
- Data is cached for subsequent Read calls
- Call periodically to get updated status

---

### `int ReadPos(s8 ID)`

Read current position.

**Parameters:**
- `ID` - Motor ID (0-253) or -1 for cached data

**Returns:**
- Position value (0-4095 for 12-bit, 0-1023 for 10-bit)
- `-1` - Failure

**Example:**
```cpp
servo.FeedBack(1);
int pos = servo.ReadPos(-1);  // Use cached data from FeedBack
// or
int pos = servo.ReadPos(1);   // Direct read (slower)
```

---

### `int ReadSpeed(s8 ID)`

Read current speed.

**Parameters:**
- `ID` - Motor ID or -1 for cached data

**Returns:**
- Speed in steps/s (signed, negative = reverse)
- `-1` - Failure

**Example:**
```cpp
servo.FeedBack(1);
int speed = servo.ReadSpeed(-1);
printf("Current speed: %d steps/s\n", speed);
```

---

### `int ReadLoad(s8 ID)`

Read current load torque.

**Parameters:**
- `ID` - Motor ID or -1 for cached data

**Returns:**
- Load value: -1000 to +1000
  - Negative = CCW load
  - Positive = CW load
  - Magnitude indicates load strength
- `-1` - Failure

**Example:**
```cpp
servo.FeedBack(1);
int load = servo.ReadLoad(-1);
if (abs(load) > 800) {
    printf("Warning: High load detected!\n");
}
```

---

### `int ReadVoltage(s8 ID)`

Read current input voltage.

**Parameters:**
- `ID` - Motor ID or -1 for cached data

**Returns:**
- Voltage in 0.1V units (e.g., 120 = 12.0V)
- `-1` - Failure

**Example:**
```cpp
servo.FeedBack(1);
int voltage = servo.ReadVoltage(-1);
printf("Voltage: %.1fV\n", voltage / 10.0);
```

---

### `int ReadTemper(s8 ID)`

Read current temperature.

**Parameters:**
- `ID` - Motor ID or -1 for cached data

**Returns:**
- Temperature in °C
- `-1` - Failure

**Example:**
```cpp
servo.FeedBack(1);
int temp = servo.ReadTemper(-1);
if (temp > 70) {
    printf("Warning: High temperature %d°C!\n", temp);
    servo.EnableTorque(1, 0);  // Disable motor to cool down
}
```

---

### `int ReadCurrent(s8 ID)`

Read current draw.

**Parameters:**
- `ID` - Motor ID or -1 for cached data

**Returns:**
- Current in mA
- `-1` - Failure

**Example:**
```cpp
servo.FeedBack(1);
int current = servo.ReadCurrent(-1);
printf("Current: %d mA\n", current);
```

---

### `int ReadMove(s8 ID)`

Check if motor is currently moving.

**Parameters:**
- `ID` - Motor ID or -1 for cached data

**Returns:**
- `1` - Motor is moving
- `0` - Motor is stopped
- `-1` - Failure

**Example:**
```cpp
servo.WritePosEx(1, 2048, 2400, 50);
while (servo.ReadMove(1) == 1) {
    usleep(10000);  // Wait 10ms
}
printf("Move complete!\n");
```

---

## Configuration & Management

### `int Ping(u8 ID)`

Test servo presence and connectivity.

**Parameters:**
- `ID` - Motor ID (0-253)

**Returns:**
- Servo ID (Error field from response, typically matches ID parameter) on success
- `-1` - No response or communication failure

**Example:**
```cpp
for (u8 id = 1; id <= 10; id++) {
    if (servo.Ping(id) != -1) {
        printf("Found servo at ID %d\n", id);
    }
}
```

**Note:** On success, Ping() returns the Error field from the servo's response packet, which is typically the servo's ID. Check for `!= -1` rather than `== 1`.

---

### `int Mode(u8 ID, u8 Mode)`

Set operating mode.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Mode` - Operating mode
  - `0` - Servo (position control)
  - `1` - Velocity (closed-loop wheel)
  - `2` - PWM (open-loop wheel)

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
servo.Mode(1, 1);  // Switch to velocity mode
```

**Notes:**
- Requires unlocking EEPROM first (use `unLockEeprom()` before calling, or use `InitMotor()` which handles this automatically)
- Mode persists across power cycles
- Remember to lock EEPROM after changing mode (use `LockEeprom()`)

---

### `int EnableTorque(u8 ID, u8 Enable)`

Enable or disable motor torque.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)
- `Enable` - 1=enable, 0=disable

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
servo.EnableTorque(1, 1);  // Enable - motor holds position
servo.EnableTorque(1, 0);  // Disable - motor can be manually moved
```

**Notes:**
- Torque disabled allows manual positioning
- Always disable torque before manual calibration
- Torque state is not saved to EEPROM

---

### `int CalibrationOfs(u8 ID)`

Set current position as center (midpoint calibration).

**Parameters:**
- `ID` - Motor ID (0-253)

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
// Manually position servo to desired center
servo.EnableTorque(1, 0);      // Disable torque
printf("Position servo, then press Enter...\n");
getchar();

servo.unLockEeprom(1);
servo.CalibrationOfs(1);       // Set as center
servo.LockEeprom(1);
servo.EnableTorque(1, 1);      // Re-enable torque
```

**Notes:**
- Requires EEPROM unlock
- Useful for mechanical alignment
- Affects position readings permanently

---

### `int unLockEeprom(u8 ID)`

Unlock EEPROM for writing.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)

**Returns:**
- `1` - Success
- `0` - Failure

---

### `int LockEeprom(u8 ID)`

Lock EEPROM to prevent accidental changes.

**Parameters:**
- `ID` - Motor ID (0-253, 254=broadcast)

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
servo.unLockEeprom(1);
servo.writeByte(1, SMS_STS_ID, 7);  // Change ID to 7
servo.LockEeprom(7);  // Lock with new ID
```

---

### `int RegWriteAction(u8 ID)`

Execute all buffered RegWrite commands.

**Parameters:**
- `ID` - Motor ID or 0xFE (254) for broadcast

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
servo.RegWritePosEx(1, 1024, 2400, 50);
servo.RegWritePosEx(2, 2048, 2400, 50);
servo.RegWriteAction(0xFE);  // Execute both
```

---

## Low-Level Memory Access

### `int writeByte(u8 ID, u8 MemAddr, u8 bDat)`

Write single byte to memory table.

**Parameters:**
- `ID` - Motor ID
- `MemAddr` - Memory address (see Memory Table Reference)
- `bDat` - Byte value to write

**Returns:**
- `1` - Success
- `0` - Failure

**Example:**
```cpp
servo.unLockEeprom(1);
servo.writeByte(1, SMS_STS_BAUD_RATE, SMS_STS_1M);
servo.LockEeprom(1);
```

---

### `int writeWord(u8 ID, u8 MemAddr, u16 wDat)`

Write 16-bit word to memory table.

**Parameters:**
- `ID` - Motor ID
- `MemAddr` - Memory address
- `wDat` - Word value (will be split into low/high bytes)

**Returns:**
- `1` - Success
- `0` - Failure

---

### `int readByte(u8 ID, u8 MemAddr)`

Read single byte from memory table.

**Parameters:**
- `ID` - Motor ID
- `MemAddr` - Memory address

**Returns:**
- Byte value (0-255)
- `-1` - Failure

---

### `int readWord(u8 ID, u8 MemAddr)`

Read 16-bit word from memory table.

**Parameters:**
- `ID` - Motor ID
- `MemAddr` - Memory address

**Returns:**
- Word value (0-65535)
- `-1` - Failure

---

## Memory Table Reference

Complete memory maps for all supported servo protocols.

### SMS/STS Series

Complete memory map for SMS/STS series servos.

### EEPROM (Read-Only) - Model Information

| Address | Name | Description | Default |
|---------|------|-------------|---------|
| 3-4 | MODEL_L/H | Model number | Varies by model |

### EEPROM (Read/Write) - Persistent Configuration

These settings are saved to EEPROM and persist across power cycles. **Requires unlocking EEPROM before writing.**

| Address | Constant | Description | Range | Default |
|---------|----------|-------------|-------|---------|
| 5 | SMS_STS_ID | Servo ID | 0-253 | 1 |
| 6 | SMS_STS_BAUD_RATE | Baud rate | 0-7 | 4 (115200) or 0 (1M) |
| 9-10 | SMS_STS_MIN_ANGLE_LIMIT_L/H | Minimum position limit | 0-4095 | 0 |
| 11-12 | SMS_STS_MAX_ANGLE_LIMIT_L/H | Maximum position limit | 0-4095 | 4095 |
| 13 | SMS_STS_MAX_TEMPERATURE | Max temperature limit (°C) | 0-100 | 70 |
| 14 | SMS_STS_MAX_VOLTAGE | Max voltage limit (0.1V) | 50-140 | 140 (14V) |
| 15 | SMS_STS_MIN_VOLTAGE | Min voltage limit (0.1V) | 50-140 | 60 (6V) |
| 16-17 | SMS_STS_MAX_TORQUE_L/H | Max torque limit | 0-1000 | 1000 |
| 21 | SMS_STS_ALARM_LED | Alarm LED behavior | Bitfield | 0 |
| 22 | SMS_STS_ALARM_SHUTDOWN | Alarm shutdown behavior | Bitfield | 0 |
| 26 | SMS_STS_CW_DEAD | CW deadzone | 0-255 | 0 |
| 27 | SMS_STS_CCW_DEAD | CCW deadzone | 0-255 | 0 |
| 31-32 | SMS_STS_OFS_L/H | Position offset (calibration) | -2048 to +2047 | 0 |
| 33 | SMS_STS_MODE | Operating mode | 0-3 | 0 |

**Baud Rate Values:**
- 0 = 1000000 bps (1M)
- 1 = 500000 bps (500K)
- 2 = 250000 bps (250K) ⚠️ *Not supported by `begin()`*
- 3 = 128000 bps (128K) ⚠️ *Not supported by `begin()`*
- 4 = 115200 bps
- 5 = 76800 bps ⚠️ *Not supported by `begin()`*
- 6 = 57600 bps
- 7 = 38400 bps

**Note:** Values 2, 3, and 5 can be stored in servo EEPROM but are not supported by the Linux driver's `begin()` function. Only use the supported baud rates: 9600, 19200, 38400, 57600, 115200, 500000, 1000000.

### SRAM (Read/Write) - Runtime Control

These settings control real-time operation and are lost on power cycle.

| Address | Constant | Description | Range | Default |
|---------|----------|-------------|-------|---------|
| 40 | SMS_STS_TORQUE_ENABLE | Torque on/off | 0-1 | 0 |
| 41 | SMS_STS_ACC | Acceleration | 0-254 | 0 |
| 42-43 | SMS_STS_GOAL_POSITION_L/H | Target position | 0-4095 | - |
| 44-45 | SMS_STS_GOAL_TIME_L/H | Time to position (ms) | 0-65535 | - |
| 46-47 | SMS_STS_GOAL_SPEED_L/H | Target speed/velocity | 0-3400 (position mode) or -3400 to +3400 (velocity mode) | - |
| 48 | SMS_STS_TORQUE_LIMIT | Torque limit | 0-100 | 100 |
| 55 | SMS_STS_LOCK | Lock EEPROM | 0-1 | 0 |

### SRAM (Read-Only) - Status

| Address | Constant | Description | Units |
|---------|----------|-------------|-------|
| 56-57 | SMS_STS_PRESENT_POSITION_L/H | Current position | steps (0-4095) |
| 58-59 | SMS_STS_PRESENT_SPEED_L/H | Current speed | steps/s (signed) |
| 60-61 | SMS_STS_PRESENT_LOAD_L/H | Current load | ±1000 |
| 62 | SMS_STS_PRESENT_VOLTAGE | Input voltage | 0.1V |
| 63 | SMS_STS_PRESENT_TEMPERATURE | Temperature | °C |
| 64 | SMS_STS_ASYNC_WRITE_FLAG | Async write status | 0-1 |
| 65 | SMS_STS_SERVO_STATUS | Servo status flags | Bitfield |
| 66 | SMS_STS_MOVING | Motion status | 0-1 |
| 69-70 | SMS_STS_PRESENT_CURRENT_L/H | Current draw | mA |

### Direction Encoding

For speed and load values, direction is encoded in the highest bit:
- **Position/Speed**: Bit 15 indicates direction
- **Load/PWM**: Bit 10 indicates direction

Use the utility functions from `ServoUtils.h`:
```cpp
#include "ServoUtils.h"

// Decode speed value
s16 raw_speed = servo.ReadSpeed(1);
s16 actual_speed = DecodeSpeed(raw_speed);  // Handles sign bit

// Encode speed for writing
s16 speed_to_write = EncodeSpeed(-2400);  // Negative for reverse
```

---

## Error Codes

All write/command functions return `1` for success and `0` for failure. Use `getErr()` to get detailed error code:

```cpp
int ret = servo.WritePosEx(1, 2048, 2400, 50);
if (ret == 0) {
    int err = servo.getErr();
    // Check error code
}
```

Read functions return actual values (position, speed, voltage, etc.) or `-1` on error.

Common error sources:
- **Communication timeout**: Check wiring, baud rate, power
- **Incorrect ID**: Servo not present or wrong ID
- **EEPROM locked**: Call `unLockEeprom()` first
- **Invalid parameter**: Value out of range

---

### HLSCL Series

Complete memory map for HLS series servos (HLSCL protocol).

#### EEPROM (Read-Only) - Model Information

| Address | Constant | Description | Default |
|---------|----------|-------------|---------|
| 3-4 | HLSCL_MODEL_L/H | Model number | Varies by model |

#### EEPROM (Read/Write) - Persistent Configuration

These settings are saved to EEPROM and persist across power cycles. **Requires unlocking EEPROM before writing.**

| Address | Constant | Description | Range | Default |
|---------|----------|-------------|-------|---------|
| 5 | HLSCL_ID | Servo ID | 0-253 | 1 |
| 6 | HLSCL_BAUD_RATE | Baud rate | 0-7 | 4 (115200) |
| 7 | HLSCL_SECOND_ID | Secondary ID | 0-253 | 0 |
| 9-10 | HLSCL_MIN_ANGLE_LIMIT_L/H | Minimum position limit | 0-4095 | 0 |
| 11-12 | HLSCL_MAX_ANGLE_LIMIT_L/H | Maximum position limit | 0-4095 | 4095 |
| 26 | HLSCL_CW_DEAD | CW deadzone | 0-255 | 0 |
| 27 | HLSCL_CCW_DEAD | CCW deadzone | 0-255 | 0 |
| 31-32 | HLSCL_OFS_L/H | Position offset (calibration) | -2048 to +2047 | 0 |
| 33 | HLSCL_MODE | Operating mode | 0-2 | 0 |

**Operating Mode Values:**
- 0 = Servo mode (position control) - `HLSCL_MODE_SERVO`
- 1 = Wheel mode (constant velocity control) - `HLSCL_MODE_WHEEL`
- 2 = Electric/Force mode (constant torque output) - `HLSCL_MODE_ELECTRIC`

**Baud Rate Values:**
- 0 = 1000000 bps (1M)
- 1 = 500000 bps (500K)
- 2 = 250000 bps (250K)
- 3 = 128000 bps (128K)
- 4 = 115200 bps (default)
- 5 = 76800 bps
- 6 = 57600 bps
- 7 = 38400 bps

#### SRAM (Read/Write) - Runtime Control

These settings control real-time operation and are lost on power cycle.

| Address | Constant | Description | Range | Default |
|---------|----------|-------------|-------|---------|
| 40 | HLSCL_TORQUE_ENABLE | Torque on/off | 0-1 | 0 |
| 41 | HLSCL_ACC | Acceleration | 0-254 | 0 |
| 42-43 | HLSCL_GOAL_POSITION_L/H | Target position | 0-4095 | - |
| 44-45 | HLSCL_GOAL_TORQUE_L/H | Target torque (force mode) | -1000 to +1000 | - |
| 46-47 | HLSCL_GOAL_SPEED_L/H | Target speed/velocity | -3400 to +3400 | - |
| 48-49 | HLSCL_TORQUE_LIMIT_L/H | Torque limit | 0-1000 | 1000 |
| 55 | HLSCL_LOCK | Lock EEPROM | 0-1 | 0 |

#### SRAM (Read-Only) - Status

| Address | Constant | Description | Units |
|---------|----------|-------------|-------|
| 56-57 | HLSCL_PRESENT_POSITION_L/H | Current position | steps (0-4095) |
| 58-59 | HLSCL_PRESENT_SPEED_L/H | Current speed | steps/s (signed) |
| 60-61 | HLSCL_PRESENT_LOAD_L/H | Current load | ±1000 |
| 62 | HLSCL_PRESENT_VOLTAGE | Input voltage | 0.1V |
| 63 | HLSCL_PRESENT_TEMPERATURE | Temperature | °C |
| 66 | HLSCL_MOVING | Motion status | 0-1 |
| 69-70 | HLSCL_PRESENT_CURRENT_L/H | Current draw | mA |

#### HLSCL Protocol Differences

Key differences from SMS/STS protocol:

1. **Force Control Mode**: HLSCL supports Mode 2 (Electric/Force mode) for constant torque output via `GOAL_TORQUE` registers
2. **Torque Limiting**: Wheel mode includes torque limit parameter (`WriteSpe()` has 4 parameters instead of 3)
3. **Secondary ID**: HLSCL supports a secondary ID for advanced bus configurations
4. **Default Baud**: HLS series defaults to 115200 bps (vs 1M for STS series)
5. **Position Resolution**: 12-bit (0-4095) like STS, not 10-bit like SCSCL

**Example - Force Mode:**
```cpp
HLSCL hlscl;
hlscl.begin(115200, "/dev/ttyUSB0");

// Enable force mode
hlscl.unLockEprom(1);
hlscl.writeByte(1, HLSCL_MODE, HLSCL_MODE_ELECTRIC);
hlscl.LockEprom(1);

// Apply constant torque
hlscl.writeWord(1, HLSCL_GOAL_TORQUE_L, 500);  // 500 × 6.5 = 3250 mA
```
