# SCServo Examples Guide

> **⚠️ AI-Generated Documentation**: This documentation was generated with AI assistance and may contain inconsistencies with the actual codebase. Always verify commands, function names, and procedures against the source code and examples when in doubt.

Complete reference for all example programs included with the SCServo_Linux library.

## Table of Contents

- [Overview](#overview)
- [Building Examples](#building-examples)
- [Official SMS_STS Examples](#official-sms_sts-examples)
- [Official SCSCL Examples](#official-scscl-examples)
- [Sandbox Examples (C++)](#sandbox-examples-c)
- [Python Examples](#python-examples)
- [Example Categories](#example-categories)

## Overview

This SDK includes 25+ example programs demonstrating various servo control techniques:

- **Official SMS_STS Examples** (`examples/SMS_STS/`): Original Feetech SDK examples for SMS/STS protocol servos with English translations and documentation
- **Official SCSCL Examples** (`examples/SCSCL/`): Original Feetech SDK examples for SCSCL protocol servos with English translations and documentation
- **Official HLSCL Examples** (`examples/HLSCL/`): Original Feetech SDK examples for HLSCL protocol servos with English translations and documentation
- **Sandbox Examples** (`examples/sandbox/`): Enhanced examples with detailed comments and advanced features, specifically for STS3215 motors
- **Python Examples** (`examples/sandbox/python/`): Python demonstrations using nanobind bindings

All examples include:
- Comprehensive Doxygen documentation
- Signal handlers for safe shutdown (Ctrl+C)
- Error handling and validation
- Hardware requirements and usage instructions

### IMPORTANT: Motor ID Configuration

**Before building and running any example**, verify the motor IDs in the source code match your actual servo IDs. Most examples use hardcoded IDs:
- Official SMS_STS/SCSCL examples: Usually ID 1 or IDs 1, 2
- Sandbox examples: Usually IDs 7, 8, 9

If your servos use different IDs, edit the ID constants in the `.cpp` file before compiling. For example:

```cpp
// Change this:
const u8 ID[3] = {7, 8, 9};

// To match your servos:
const u8 ID[3] = {1, 2, 3};
```

See [TROUBLESHOOTING.md](TROUBLESHOOTING.md#motor-ids-dont-match) for more details on changing servo IDs.

## Building Examples

### Individual Example

```bash
cd examples/SMS_STS/Ping
cmake . && make
./Ping /dev/ttyUSB0
```

### All Sandbox Examples (Parallel Build)

```bash
cd /home/ubuntu/SCServo_Linux/examples/sandbox
for dir in WritePosition RegWritePosition SyncWritePosition WriteSpeed RegWriteSpeed SyncWriteSpeed WritePWM RegWritePWM SyncWritePWM MidpointCalib ReadData; do
    echo "=== Building $dir ==="
    cd $dir
    make
    cd ..
done
```

### Python Examples

```bash
cd examples/sandbox/python
python3 sync_write_speed.py
```

---

## Official SMS_STS Examples

These examples are from the original Feetech SDK, enhanced with English documentation.

### Ping

**File:** `examples/SMS_STS/Ping/Ping.cpp`

**Purpose:** Test servo connectivity and verify servo ID

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 115200

**What it demonstrates:**
- `Ping()` - Verify servo presence and communication
- Basic diagnostics and ID verification
- Error detection for unresponsive servos

**Usage:**
```bash
./Ping /dev/ttyUSB0
# Success: "ID:1 PingOK!"
# Failure: "Ping servo ID error!"
```

**Key Functions:**
- `servo.Ping(ID)` - Returns servo ID on success, -1 on failure

---

### WritePos

**File:** `examples/SMS_STS/WritePos/WritePos.cpp`

**Purpose:** Basic position control with time parameter

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `WritePos()` - Time-based position control
- Oscillating motion between two positions
- Basic movement commands

**Usage:**
```bash
./WritePos /dev/ttyUSB0
# Servo oscillates between position 1000 and 3000
```

**Key Functions:**
- `servo.WritePos(ID, Position, Time, Speed)` - Time-based position movement

**Note:** For new code, prefer `WritePosEx()` which uses speed and acceleration instead of time.

---

### RegWritePos

**File:** `examples/SMS_STS/RegWritePos/RegWritePos.cpp`

**Purpose:** Asynchronous position control with RegWrite/Action

**Hardware:**
- Multiple servos
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `RegWritePos()` - Stage commands without executing
- `RegWriteAction()` - Execute all staged commands simultaneously
- Synchronized start for multiple servos

**Usage:**
```bash
./RegWritePos /dev/ttyUSB0
```

**Key Functions:**
- `servo.RegWritePos(ID, Position, Time, Speed)` - Buffer command
- `servo.RegWriteAction(0xFE)` - Execute all buffered commands

---

### SyncWritePos

**File:** `examples/SMS_STS/SyncWritePos/SyncWritePos.cpp`

**Purpose:** Synchronized position control for multiple servos

**Hardware:**
- Multiple servos
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `SyncWritePosEx()` - Send commands to multiple servos in single packet
- Perfectly synchronized movement start
- Array-based multi-servo control

**Usage:**
```bash
./SyncWritePos /dev/ttyUSB0
```

**Key Functions:**
- `servo.SyncWritePosEx(IDs[], IDN, Position[], Speed[], Acc[])` - Synchronized multi-servo control

---

### FeedBack

**File:** `examples/SMS_STS/FeedBack/FeedBack.cpp`

**Purpose:** Read servo status and feedback data

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `FeedBack()` - Request all servo feedback data
- `ReadPos()`, `ReadSpeed()`, `ReadLoad()` - Read cached values
- `ReadVoltage()`, `ReadTemper()`, `ReadCurrent()` - Read status
- Continuous monitoring loop

**Usage:**
```bash
./FeedBack /dev/ttyUSB0
# Displays: Position, Speed, Load, Voltage, Temperature, Current
```

**Key Functions:**
- `servo.FeedBack(ID)` - Request feedback (caches data)
- `servo.ReadPos(-1)` - Read from cache (fast, no communication)
- `servo.ReadPos(ID)` - Direct read (slower, requires communication)

---

### WriteSpe

**File:** `examples/SMS_STS/WriteSpe/WriteSpe.cpp`

**Purpose:** Velocity control (wheel mode)

**Hardware:**
- 1 servo in velocity mode (ID: 1, Mode: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `WriteSpe()` - Velocity control command
- `InitMotor()` - Initialize servo in velocity mode
- Continuous rotation with speed control
- Acceleration profiles

**Usage:**
```bash
./WriteSpe /dev/ttyUSB0
# Servo rotates continuously at controlled speed
```

**Key Functions:**
- `servo.InitMotor(ID, mode, torque)` - Initialize mode and torque
- `servo.WriteSpe(ID, Speed, Acc)` - Set velocity (-3400 to +3400 steps/s)

---

### CalibrationOfs

**File:** `examples/SMS_STS/CalibrationOfs/CalibrationOfs.cpp`

**Purpose:** Midpoint calibration (set current position as center)

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `CalibrationOfs()` - Set current position as midpoint
- EEPROM unlock/lock sequence
- Persistent calibration storage

**Usage:**
```bash
./CalibrationOfs /dev/ttyUSB0
# Manually position servo, then program sets that as center
```

**Key Functions:**
- `servo.unLockEeprom(ID)` - Enable EEPROM writes
- `servo.CalibrationOfs(ID)` - Save current position as offset
- `servo.LockEeprom(ID)` - Protect EEPROM from changes

---

### ProgramEprom

**File:** `examples/SMS_STS/ProgramEprom/ProgramEprom.cpp`

**Purpose:** Configure servo EEPROM settings

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `writeByte()`, `writeWord()` - Write to memory table
- EEPROM configuration (ID, baud rate, limits, mode)
- Persistent settings that survive power cycle

**Usage:**
```bash
./ProgramEprom /dev/ttyUSB0
# Configures servo EEPROM settings
```

**Key Functions:**
- `servo.writeByte(ID, MemAddr, Value)` - Write single byte
- `servo.writeWord(ID, MemAddr, Value)` - Write 16-bit word

**Warning:** Always unlock EEPROM before writing, and lock after!

---

### SyncRead

**File:** `examples/SMS_STS/SyncRead/SyncRead.cpp`

**Purpose:** Read data from multiple servos efficiently

**Hardware:**
- Multiple servos
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- Efficient bulk reading from multiple servos
- Synchronized read operations
- Status monitoring for servo arrays

**Usage:**
```bash
./SyncRead /dev/ttyUSB0
```

---

### Broadcast

**File:** `examples/SMS_STS/Broadcast/Broadcast.cpp`

**Purpose:** Send commands to all servos simultaneously

**Hardware:**
- Multiple servos on bus
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- Broadcast commands using ID 254 (0xFE)
- Controlling all servos with single command
- Efficient bus utilization

**Usage:**
```bash
./Broadcast /dev/ttyUSB0
```

**Key Functions:**
- Any command with `ID = 254` broadcasts to all servos

**Warning:** Be cautious with broadcast EEPROM writes - affects all servos!

---

## Official SCSCL Examples

These examples are from the original Feetech SDK for SCSCL protocol servos (SC09, SC15), enhanced with English documentation.

**Key Differences from SMS_STS:**
- **Protocol:** SCSCL vs SMS_STS
- **Resolution:** 10-bit (0-1023) vs 12-bit (0-4095)
- **Default Baud Rate:** 115200 vs 1000000
- **Class:** `SCSCL sc;` vs `SMS_STS sm_st;`

### Ping

**File:** [examples/SCSCL/Ping/Ping.cpp](../examples/SCSCL/Ping/Ping.cpp)

**Purpose:** Test servo connectivity and verify servo ID for SCSCL protocol servos

**Hardware:**
- 1 SCSCL servo (ID: 1)
- Baud rate: 115200

**What it demonstrates:**
- `Ping()` - Verify servo presence and communication
- Basic diagnostics and ID verification
- Error detection for unresponsive servos

**Usage:**
```bash
cd examples/SCSCL/Ping
cmake . && make
./Ping /dev/ttyUSB0
# Success: "ID:1 PingOK!"
# Failure: "Ping servo ID error!"
```

**Key Functions:**
- `sc.Ping(ID)` - Returns servo ID on success, -1 on failure

---

### WritePos

**File:** [examples/SCSCL/WritePos/WritePos.cpp](../examples/SCSCL/WritePos/WritePos.cpp)

**Purpose:** Basic position control for SCSCL servos

**Hardware:**
- 1 SCSCL servo (ID: 1)
- Baud rate: 115200

**What it demonstrates:**
- `WritePos()` - Position control with speed parameter
- Oscillating motion between two positions
- Timing calculation for motion completion

**Usage:**
```bash
cd examples/SCSCL/WritePos
cmake . && make
./WritePos /dev/ttyUSB0
# Servo oscillates between position 20 and 1000
```

**Motion Pattern:**
- Move to position 1000 at 1500 steps/s
- Wait 754ms for completion: [(1000-20)/1500]*1000+100
- Move to position 20 at 1500 steps/s
- Wait 754ms for completion
- Repeat indefinitely

**Key Functions:**
- `sc.WritePos(ID, Position, Time, Speed)` - Position control
  - ID: Servo ID (1)
  - Position: Target position (0-1023 for SCSCL)
  - Time: 0 (use speed control, not time-based)
  - Speed: 1500 steps/second

**Note:** Factory speed unit is 0.0146 rpm per step.

---

### RegWritePos

**File:** [examples/SCSCL/RegWritePos/RegWritePos.cpp](../examples/SCSCL/RegWritePos/RegWritePos.cpp)

**Purpose:** Deferred multi-servo position commands with synchronized execution

**Hardware:**
- 2 SCSCL servos (IDs: 1, 2)
- Baud rate: 115200

**What it demonstrates:**
- `RegWritePos()` - Queue position command without execution
- `RegWriteAction()` - Broadcast trigger to execute all queued commands
- Deferred execution pattern for precise synchronization
- Individual command customization per servo

**Usage:**
```bash
cd examples/SCSCL/RegWritePos
cmake . && make
./RegWritePos /dev/ttyUSB0
```

**Execution Pattern:**
1. `RegWritePos(ID1, 1000, ...)` - Queue command to servo 1 (not executed yet)
2. `RegWritePos(ID2, 1000, ...)` - Queue command to servo 2 (not executed yet)
3. `RegWriteAction()` - Broadcast: both servos execute simultaneously
4. Wait for motion completion
5. Repeat with new positions

**Motion Profile:**
- Stage 1: Both servos to position 1000 @ 1500 steps/s
- Stage 2: Both servos to position 20 @ 1500 steps/s
- Repeat indefinitely

**Advantages over SyncWritePos:**
- More flexible command sequencing
- Can mix different command types
- Easier to add/remove servos dynamically
- Better for complex choreography

**Key Functions:**
- `sc.RegWritePos(ID, Position, Time, Speed)` - Queue position command
- `sc.RegWriteAction()` - Execute all queued commands

**Warning:** All servos must support Register Write functionality. Older firmware may not implement this.

---

### SyncWritePos

**File:** [examples/SCSCL/SyncWritePos/SyncWritePos.cpp](../examples/SCSCL/SyncWritePos/SyncWritePos.cpp)

**Purpose:** Synchronized multi-servo position control with single packet

**Hardware:**
- 2 SCSCL servos (IDs: 1, 2)
- Baud rate: 115200

**What it demonstrates:**
- `SyncWritePos()` - Simultaneous position command to multiple servos
- Array-based servo ID management
- Array-based position and speed parameters
- Coordinated multi-servo motion
- Most efficient multi-servo synchronization

**Usage:**
```bash
cd examples/SCSCL/SyncWritePos
cmake . && make
./SyncWritePos /dev/ttyUSB0
```

**Motion Profile:**
- Both servos move to position 1000 @ 1500 steps/s simultaneously
- Wait 754ms for motion completion
- Both servos move to position 20 @ 1500 steps/s simultaneously
- Wait 754ms for motion completion
- Repeat indefinitely

**Parameter Details:**
- `ID[2]` - Array of servo IDs: {1, 2}
- `Position[2]` - Target positions for each servo
- `Speed[2]` - Velocities for each servo (1500 steps/s)
- Servo count: 2
- Time parameter: 0 (speed control mode)

**Key Functions:**
- `sc.SyncWritePos(ID[], count, Position[], Time, Speed[])` - Synchronized multi-servo control

**Warning:** Ensure adequate power supply for simultaneous multi-servo acceleration.

---

### FeedBack

**File:** [examples/SCSCL/FeedBack/FeedBack.cpp](../examples/SCSCL/FeedBack/FeedBack.cpp)

**Purpose:** Comprehensive servo feedback data reading

**Hardware:**
- 1 SCSCL servo (ID: 1)
- Baud rate: 115200

**What it demonstrates:**
- `FeedBack()` - Request all servo feedback data in single command
- `ReadPos(-1)` - Read position from last feedback (no new query)
- `ReadSpeed(-1)` - Read speed from cached data
- `ReadLoad(-1)` - Read load torque from cache
- `ReadVoltage(-1)` - Read supply voltage from cache
- `ReadTemper(-1)` - Read internal temperature from cache
- `ReadMove(-1)` - Read motion status from cache
- `ReadCurrent(-1)` - Read current draw from cache
- Efficient bulk data reading vs individual queries

**Usage:**
```bash
cd examples/SCSCL/FeedBack
cmake . && make
./FeedBack /dev/ttyUSB0
# Displays: Position, Speed, Load, Voltage, Temperature, Motion, Current
```

**Two Reading Methods:**

**Method 1: Cached Read (Efficient)**
```cpp
sc.FeedBack(1);                // Request all data once
int pos = sc.ReadPos(-1);      // Read from cache (fast)
int speed = sc.ReadSpeed(-1);  // Read from cache
```

**Method 2: Direct Read (Individual Queries)**
```cpp
int pos = sc.ReadPos(1);       // Direct query (slower)
int voltage = sc.ReadVoltage(1);  // Separate query
```

**Key Functions:**
- `sc.FeedBack(ID)` - Request and cache all feedback data
- `sc.ReadPos(-1)` - Read cached position (-1 = use cache)
- `sc.ReadPos(ID)` - Direct position query (requires serial communication)

**Note:** The -1 parameter retrieves data from the last FeedBack() call without sending a new query, making bulk data reading very efficient.

---

### Broadcast

**File:** [examples/SCSCL/Broadcast/Broadcast.cpp](../examples/SCSCL/Broadcast/Broadcast.cpp)

**Purpose:** Broadcast position commands to all servos simultaneously

**Hardware:**
- Multiple SCSCL servos on same bus
- Each servo must have unique ID
- Baud rate: 115200

**What it demonstrates:**
- Broadcast ID usage (0xFE = 254)
- Simultaneous multi-servo control with single command
- Position control with speed parameter
- Timing calculation for motion completion

**Usage:**
```bash
cd examples/SCSCL/Broadcast
cmake . && make
./Broadcast /dev/ttyUSB0
# All servos on bus move together
```

**Motion Pattern:**
- All servos move to position 1000 at 1500 steps/s
- Wait 754ms for completion
- All servos move to position 20 at 1500 steps/s
- Wait 754ms for completion
- Repeat indefinitely

**Key Functions:**
- `sc.WritePos(0xFE, Position, Time, Speed)` - Broadcast to all servos
  - 0xFE = Broadcast ID (all servos respond)
  - Position: 1000 or 20
  - Time: 0 (use speed control)
  - Speed: 1500 steps/s

**Warning:** Broadcast commands receive no acknowledgment from servos. You cannot verify if the command was received.

**Warning:** Be cautious with broadcast EEPROM writes - affects ALL servos on the bus!

---

### ProgramEprom

**File:** [examples/SCSCL/ProgramEprom/ProgramEprom.cpp](../examples/SCSCL/ProgramEprom/ProgramEprom.cpp)

**Purpose:** Multi-parameter persistent EEPROM programming

**Hardware:**
- 1 SCSCL servo (ID: 1, will be changed to 2)
- Baud rate: 115200
- Stable, uninterrupted power required

**What it demonstrates:**
- `unLockEeprom()` - Disable EEPROM write protection
- `writeByte()` - Write single-byte parameters (servo ID)
- `writeWord()` - Write two-byte parameters (angle limits)
- Multiple parameter writes in single unlock session
- `LockEeprom()` - Re-enable EEPROM write protection
- Angle limit configuration for restricted motion range

**Usage:**
```bash
cd examples/SCSCL/ProgramEprom
cmake . && make
./ProgramEprom /dev/ttyUSB0
# Servo ID changes from 1 to 2
# Angle limits set to [20, 1000]
# Configuration persists across power cycles
```

**EEPROM Write Procedure (CRITICAL ORDER):**
1. `unLockEeprom(1)` - Disable write protection for servo ID=1
2. `writeByte(1, SCSCL_ID, 2)` - Change servo ID from 1 to 2
3. `writeWord(2, MIN_ANGLE, 20)` - Set min angle limit (use new ID=2)
4. `writeWord(2, MAX_ANGLE, 1000)` - Set max angle limit
5. `LockEeprom(2)` - Re-enable write protection (use new ID=2)

**Configuration Details:**
- Servo ID: Changed from 1 to 2
- Min Angle Limit: 20 (~7° from zero)
- Max Angle Limit: 1000 (~344° from zero, ~95% of full 0-1023 range)
- Usable Range: 980 positions (~337° rotation)

**Common SCSCL EEPROM Parameters:**
- `SCSCL_ID` - Servo ID (1-253)
- `SCSCL_MIN_ANGLE_LIMIT_L` - Minimum position limit (0-1023)
- `SCSCL_MAX_ANGLE_LIMIT_L` - Maximum position limit (0-1023)
- `SCSCL_MAX_TEMPERATURE` - Temperature shutdown limit
- `SCSCL_MAX_VOLTAGE` - Maximum voltage limit
- `SCSCL_MIN_VOLTAGE` - Minimum voltage limit

**Key Functions:**
- `sc.unLockEeprom(ID)` - Enable EEPROM writes
- `sc.writeByte(ID, Address, Value)` - Write 1-byte parameter
- `sc.writeWord(ID, Address, Value)` - Write 2-byte parameter
- `sc.LockEeprom(ID)` - Disable EEPROM writes (protect from corruption)

**Warning:** EEPROM has limited write endurance (~100,000 cycles). Use for configuration only, never in control loops.

**Warning:** ALWAYS lock EEPROM after programming. Unlocked EEPROM is vulnerable to corruption from electrical noise.

**Warning:** After changing servo ID, all subsequent commands must use the NEW ID (2 in this example).

**Warning:** Ensure stable power during EEPROM writes. Power loss can corrupt configuration, potentially requiring factory reset.

---

### WritePWM

**File:** [examples/SCSCL/WritePWM/WritePos.cpp](../examples/SCSCL/WritePWM/WritePos.cpp)

**Purpose:** Direct PWM control (open-loop wheel mode) for SCSCL servos

**Hardware:**
- 1 SCSCL servo in PWM mode (ID: 1, Mode: 2)
- Baud rate: 115200

**What it demonstrates:**
- `PWMMode()` - Initialize servo in PWM mode
- `WritePWM()` - Direct motor power control
- Open-loop control without encoder feedback
- PWM duty cycle specification (-1000 to +1000)
- Bidirectional rotation control

**Usage:**
```bash
cd examples/SCSCL/WritePWM
cmake . && make
./WritePWM /dev/ttyUSB0
# Servo rotates forward, stops, reverses, stops, repeat
```

**Motion Pattern:**
- Set PWM mode: `PWMMode(1)`
- PWM = +500 (forward rotation) - 2 seconds
- PWM = 0 (stop) - 2 seconds
- PWM = -500 (reverse rotation) - 2 seconds
- PWM = 0 (stop) - 2 seconds
- Repeat indefinitely

**Key Functions:**
- `sc.PWMMode(ID)` - Initialize servo in PWM mode (Mode 2)
- `sc.WritePWM(ID, PWM)` - Set PWM duty cycle
  - PWM range: -1000 to +1000
  - Represents: -100% to +100% duty cycle
  - Positive = forward, Negative = reverse

**Note:** No speed feedback in PWM mode. Actual speed depends on load and voltage.

**Note:** Unlike velocity mode (Mode 1), PWM mode has no closed-loop speed control.

---

## Sandbox Examples (C++)

Enhanced examples with detailed documentation and advanced features.

### WritePosition

**File:** `examples/sandbox/WritePosition/WritePosition.cpp`

**Purpose:** Multi-servo position control with velocity/acceleration profiles

**Hardware:**
- 3 servos (IDs: 7, 8, 9)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `WritePosEx()` - Extended position control
- Sequential commands to multiple servos
- Trapezoidal velocity profiles
- Oscillating motion pattern
- Signal handler for safe shutdown

**Usage:**
```bash
./WritePosition /dev/ttyUSB0
# Three servos oscillate between 2048 and 4095
# Press Ctrl+C to stop safely
```

**Key Concepts:**
- Speed: 0-3400 steps/s
- Acceleration: 0-254 (×100 steps/s²)
- Sequential execution (slight timing stagger)

---

### RegWritePosition

**File:** `examples/sandbox/RegWritePosition/RegWritePosition.cpp`

**Purpose:** Asynchronous position control with perfect synchronization

**Hardware:**
- 3 servos (IDs: 7, 8, 9)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `RegWritePosEx()` - Buffer commands
- `RegWriteAction()` - Synchronized execution
- Zero timing difference between servo starts
- Coordinated multi-servo choreography

**Usage:**
```bash
./RegWritePosition /dev/ttyUSB0
```

**Advantage over WritePosition:** All servos start moving at exactly the same instant.

---

### SyncWritePosition

**File:** `examples/sandbox/SyncWritePosition/SyncWritePosition.cpp`

**Purpose:** Most efficient synchronized multi-servo position control

**Hardware:**
- 3 servos (IDs: 7, 8, 9)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `SyncWritePosEx()` - Single packet for multiple servos
- Minimum bus traffic for multi-servo control
- Array-based parameter specification
- Perfectly synchronized movement

**Usage:**
```bash
./SyncWritePosition /dev/ttyUSB0
```

**Best for:** Applications requiring coordinated motion of 3+ servos with minimal latency.

---

### WriteSpeed

**File:** `examples/sandbox/WriteSpeed/WriteSpeed.cpp`

**Purpose:** Velocity control (wheel mode) for multiple servos

**Hardware:**
- 3 servos in velocity mode (IDs: 7, 8, 9, Mode: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `WriteSpe()` - Individual velocity commands
- Continuous rotation control
- Speed ramping and acceleration
- Sequential velocity control

**Usage:**
```bash
./WriteSpeed /dev/ttyUSB0
# Motors rotate at controlled speeds
```

**Speed Range:** -3400 to +3400 steps/s (~50 RPM max)

---

### RegWriteSpeed

**File:** `examples/sandbox/RegWriteSpeed/RegWriteSpeed.cpp`

**Purpose:** Asynchronous velocity control

**Hardware:**
- 3 servos in velocity mode (IDs: 7, 8, 9, Mode: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `RegWriteSpe()` - Buffer velocity commands
- `RegWriteAction()` - Synchronized velocity changes
- Coordinated speed transitions

**Usage:**
```bash
./RegWriteSpeed /dev/ttyUSB0
```

---

### SyncWriteSpeed

**File:** `examples/sandbox/SyncWriteSpeed/SyncWriteSpeed.cpp`

**Purpose:** Synchronized velocity control for multiple servos

**Hardware:**
- 3 servos in velocity mode (IDs: 7, 8, 9, Mode: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `SyncWriteSpe()` - Bulk velocity command
- Omnidirectional robot control patterns
- Mixed direction control (forward/reverse)
- Minimum latency multi-motor coordination

**Usage:**
```bash
./SyncWriteSpeed /dev/ttyUSB0
```

**Ideal for:** Mobile robots with multiple drive wheels.

---

### WritePWM

**File:** `examples/sandbox/WritePWM/WritePWM.cpp`

**Purpose:** Direct PWM control (open-loop wheel mode)

**Hardware:**
- 3 servos in PWM mode (IDs: 7, 8, 9, Mode: 2)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `WritePwm()` - Direct motor power control
- Open-loop control without encoder feedback
- PWM duty cycle specification
- No speed regulation (speed varies with load)

**Usage:**
```bash
./WritePWM /dev/ttyUSB0
```

**PWM Range:** -1000 to +1000 (represents -100% to +100% duty cycle)

---

### RegWritePWM

**File:** `examples/sandbox/RegWritePWM/RegWritePWM.cpp`

**Purpose:** Asynchronous PWM control

**Hardware:**
- 3 servos in PWM mode (IDs: 7, 8, 9, Mode: 2)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `RegWritePwm()` - Buffer PWM commands
- `RegWriteAction()` - Synchronized PWM changes

**Usage:**
```bash
./RegWritePWM /dev/ttyUSB0
```

---

### SyncWritePWM

**File:** `examples/sandbox/SyncWritePWM/SyncWritePWM.cpp`

**Purpose:** Synchronized PWM control for multiple servos

**Hardware:**
- 3 servos in PWM mode (IDs: 7, 8, 9, Mode: 2)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- `SyncWritePwm()` - Bulk PWM command
- Efficient multi-servo open-loop control

**Usage:**
```bash
./SyncWritePWM /dev/ttyUSB0
```

---

### MidpointCalib

**File:** `examples/sandbox/MidpointCalib/MidpointCalib.cpp`

**Purpose:** Interactive midpoint calibration utility

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- Interactive calibration workflow
- User prompts for manual positioning
- EEPROM programming sequence
- Calibration verification

**Usage:**
```bash
./MidpointCalib /dev/ttyUSB0
# Follow prompts to position servo manually
```

**Workflow:**
1. Disable torque (manual positioning enabled)
2. User manually positions servo to desired center
3. Program saves position as offset in EEPROM
4. Verification moves confirm calibration

---

### ReadData

**File:** `examples/sandbox/ReadData/ReadData.cpp`

**Purpose:** Comprehensive servo status monitoring

**Hardware:**
- 1 servo (ID: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- Complete feedback data collection
- Real-time status display
- Monitoring loop implementation
- All Read* functions

**Usage:**
```bash
./ReadData /dev/ttyUSB0
# Continuous display of position, speed, load, voltage, temp, current
```

**Displays:**
- Position: 0-4095 steps
- Speed: ±3400 steps/s
- Load: -1000 to +1000
- Voltage: 0.1V units (e.g., 120 = 12.0V)
- Temperature: °C
- Current: mA

---

## Python Examples

### sync_write_speed.py

**File:** `examples/sandbox/python/sync_write_speed.py`

**Purpose:** Python demonstration of synchronized velocity control

**Hardware:**
- 3 servos in velocity mode (IDs: 7, 8, 9, Mode: 1)
- Baud rate: 1000000 (1M)

**What it demonstrates:**
- Python API usage via nanobind bindings
- `SyncWriteSpe()` from Python
- Signal handling in Python (Ctrl+C)
- Speed sweep testing
- Real-time speed measurement
- Smooth ramping with intermediate steps
- Data collection and analysis

**Usage:**
```bash
python3 sync_write_speed.py
```

**Features:**
- Safe shutdown handler
- Speed range: -2400 to +2400 steps/s
- Measured vs commanded speed analysis
- Midpoint offset calculation
- Graceful motor deceleration

**Key Python Functions:**
```python
servo = SMS_STS()
servo.begin(baud_rate, port)
servo.InitMotor(id, mode, torque)
servo.SyncWriteSpe(ids, speeds, accs)
servo.FeedBack(id)
servo.ReadSpeed(id)
servo.EnableTorque(id, enable)
servo.end()
```

---

## Example Categories

### By Control Method

**Position Control:**
- WritePos (basic)
- WritePosition (with velocity/accel)
- RegWritePosition (asynchronous)
- SyncWritePosition (synchronized)

**Velocity Control:**
- WriteSpe
- WriteSpeed
- RegWriteSpeed
- SyncWriteSpeed

**PWM Control:**
- WritePWM
- RegWritePWM
- SyncWritePWM

### By Synchronization

**Sequential (Simple):**
- WritePosition
- WriteSpeed
- WritePWM

**Asynchronous (RegWrite/Action):**
- RegWritePosition
- RegWriteSpeed
- RegWritePWM

**Synchronized (Single Packet):**
- SyncWritePosition
- SyncWriteSpeed
- SyncWritePWM

### By Function

**Diagnostics:**
- Ping - Connectivity test
- FeedBack - Status monitoring
- ReadData - Comprehensive monitoring

**Configuration:**
- ProgramEprom - EEPROM configuration
- CalibrationOfs - Basic calibration
- MidpointCalib - Interactive calibration

**Multi-Servo:**
- SyncWrite* - Efficient bulk commands
- RegWrite* - Synchronized execution
- Broadcast - All-servo commands

---

## Common Patterns

### Signal Handler (Safe Shutdown)

All examples include signal handlers to safely stop motors on Ctrl+C:

```cpp
void signal_handler(int signal) {
    printf("\nCtrl+C detected! Stopping motors...\n");
    // Stop motors
    for(int i = 0; i < 3; i++) {
        sm_st.WriteSpe(ID[i], 0, 254);  // Stop
        sm_st.EnableTorque(ID[i], 0);   // Disable torque
    }
    sm_st.end();
    exit(0);
}

signal(SIGINT, signal_handler);
```

### EEPROM Write Sequence

Always unlock, write, then lock EEPROM:

```cpp
sm_st.unLockEeprom(ID);              // 1. Unlock
sm_st.writeByte(ID, ADDR, VALUE);    // 2. Write
sm_st.LockEeprom(ID);                 // 3. Lock
```

### Cached vs Direct Read

```cpp
// Cached read (fast, no bus traffic)
sm_st.FeedBack(ID);           // Request data once
int pos = sm_st.ReadPos(-1);  // Read from cache

// Direct read (slower, requires communication)
int pos = sm_st.ReadPos(ID);  // Direct query
```

---

## Troubleshooting Examples

### Example Won't Build

```bash
# Clean and rebuild
cd examples/sandbox/WritePosition
rm -rf CMakeCache.txt CMakeFiles/
cmake .
make
```

### Permission Denied

```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and log back in

# Or use sudo (not recommended for production)
sudo ./WritePosition /dev/ttyUSB0
```

### Servo Not Responding

1. Check power supply (external power required)
2. Verify baud rate matches servo configuration
3. Confirm servo ID is correct
4. Test with Ping example first
5. Check wiring (TX, RX, GND)

### Motor IDs Don't Match

Examples use specific IDs:
- Official examples: Usually ID 1
- Sandbox examples: Usually IDs 7, 8, 9

Modify the ID constants in example source code to match your servos.

---

## See Also

- **[API Reference](API.md)** - Complete function documentation
- **[TROUBLESHOOTING.md](TROUBLESHOOTING.md)** - Detailed troubleshooting guide
- **[ARCHITECTURE.md](ARCHITECTURE.md)** - System architecture
- **[README.md](../README.md)** - Getting started guide
