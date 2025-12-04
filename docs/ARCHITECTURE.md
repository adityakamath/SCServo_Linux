# Architecture Documentation

> **⚠️ AI-Generated Documentation**: This documentation was generated with AI assistance and may contain inconsistencies with the actual codebase. Always verify class hierarchies, function signatures, and implementation details against the source code when in doubt.

Detailed architecture and design documentation for SCServo_Linux.

## Table of Contents

- [System Overview](#system-overview)
- [Layer Architecture](#layer-architecture)
- [Class Hierarchy](#class-hierarchy)
- [Communication Protocol](#communication-protocol)
- [Data Flow](#data-flow)
- [Memory Management](#memory-management)
- [Thread Safety](#thread-safety)
- [Design Patterns](#design-patterns)

---

## System Overview

SCServo_Linux is a layered C++ library for controlling Feetech serial bus servo motors on Linux platforms. The architecture follows a clear separation of concerns with distinct layers for hardware abstraction, protocol implementation, and application-level APIs.

### Key Design Goals

1. **Portability**: Abstract platform-specific code (POSIX serial I/O) from protocol logic
2. **Extensibility**: Support multiple servo protocols (SMS, STS, SCSCL, SMSBL) with minimal code duplication
3. **Performance**: Efficient packet handling and minimal overhead for real-time robotics applications
4. **Safety**: Resource management (RAII), error handling, and memory safety
5. **Usability**: Simple API for common operations while providing low-level access when needed

---

## Layer Architecture

The library is organized into four distinct layers:

```
┌─────────────────────────────────────────────────────────┐
│                  Application Layer                       │
│  (User Code, Examples, Python Bindings)                 │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│              Protocol API Layer                          │
│  SMS_STS │ SMSCL │ SMSBL │ SCSCL                        │
│  - Motor control functions (WritePosEx, WriteSpe, etc)  │
│  - Memory table definitions                             │
│  - Protocol-specific encoding/decoding                  │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│            Base Protocol Layer (SCS)                     │
│  - Packet construction and parsing                      │
│  - Checksum calculation                                 │
│  - Generic read/write/sync operations                   │
│  - Command encoding (PING, READ, WRITE, SYNC_WRITE)     │
└──────────────────────┬──────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────────┐
│          Hardware Abstraction Layer (SCSerial)           │
│  - POSIX serial I/O (termios)                           │
│  - Baud rate configuration                              │
│  - Timeout management                                   │
│  - Buffer management                                    │
└─────────────────────────────────────────────────────────┘
                       │
                       ▼
              [Physical Hardware]
              (UART/USB → Servo Bus)
```

### Layer Responsibilities

#### 1. Hardware Abstraction Layer (`SCSerial`)

**Purpose**: Isolate platform-specific serial communication code.

**Responsibilities**:
- Open/close serial port
- Configure baud rate, parity, stop bits using termios
- Read/write raw bytes to/from hardware
- Handle timeouts
- Manage transmit buffer

**Key Files**:
- `SCSerial.h` / `SCSerial.cpp`

**Platform Dependencies**:
- Uses POSIX `termios` for serial configuration
- Uses `select()` for timeout handling
- File descriptor management

---

#### 2. Base Protocol Layer (`SCS`)

**Purpose**: Implement core Feetech serial servo protocol logic.

**Responsibilities**:
- Construct protocol packets (header, ID, length, instruction, parameters, checksum)
- Parse response packets
- Calculate checksums
- Implement generic commands:
  - `PING` - Test servo presence
  - `READ_DATA` - Read from memory table
  - `WRITE_DATA` - Write to memory table
  - `REG_WRITE` - Buffered write
  - `ACTION` - Execute buffered commands
  - `SYNC_WRITE` - Write to multiple servos
  - `SYNC_READ` - Read from multiple servos
- Handle byte ordering (little-endian/big-endian)
- Error detection and reporting

**Key Files**:
- `SCS.h` / `SCS.cpp`
- `INST.h` (instruction definitions)

**Protocol Packet Format**:
```
[0xFF] [0xFF] [ID] [LEN] [INST] [PARAM1] [PARAM2] ... [CHECKSUM]
  │      │      │     │     │       │                      │
Header Header  ID  Length Instr Parameters            Checksum
```

**Checksum Calculation**:
```cpp
Checksum = ~(ID + Length + Instruction + Param1 + Param2 + ...) & 0xFF
```

---

#### 3. Protocol API Layer (`SMS_STS`, `SCSCL`, `SMSBL`, `SMSCL`)

**Purpose**: Provide servo-specific APIs and memory table definitions.

**Responsibilities**:
- Define memory table addresses for each protocol
- Implement high-level motor control functions
- Handle protocol-specific data encoding (direction bits, signed values)
- Provide convenience functions (`InitMotor`, `WritePosEx`, `WriteSpe`, etc.)
- Manage EEPROM lock/unlock sequences

**Key Files**:
- `SMS_STS.h` / `SMS_STS.cpp` - SMS/STS series (12-bit, 1M baud default)
- `SCSCL.h` / `SCSCL.cpp` - SCSCL series (10-bit, 115200 baud default)
- `SMSBL.h` / `SMSBL.cpp` - SMSBL series
- `SMSCL.h` / `SMSCL.cpp` - SMSCL series

**Differences Between Protocols**:

| Feature | SMS_STS | SCSCL |
|---------|---------|-------|
| Resolution | 12-bit (0-4095) | 10-bit (0-1023) |
| Default Baud | 1000000 | 115200 |
| Memory Map | Extended | Basic |
| Advanced Features | More sensors | Basic |

---

#### 4. Application Layer

**Purpose**: User-facing code and examples.

**Components**:
- **C++ Examples**: `examples/SMS_STS/`, `examples/sandbox/`
- **Python Bindings**: `src/bindings.cpp` (nanobind)
- **User Applications**: Any code using the library

---

## Class Hierarchy

### UML Class Diagram

```
┌─────────────────────────────────────┐
│             SCS (Abstract)          │
│  ────────────────────────────────── │
│  + u8 Level                         │
│  + u8 End                           │
│  + u8 Error                         │
│  + u8* syncReadRxBuff               │
│  + u8 syncReadRxPacketIndex         │
│  + u8 syncReadRxPacketLen           │
│  + u8* syncReadRxPacket             │
│  + u16 syncReadRxBuffLen            │
│  + u16 syncReadRxBuffMax            │
│  ────────────────────────────────── │
│  + genWrite(ID, Addr, Data, Len)    │
│  + regWrite(ID, Addr, Data, Len)    │
│  + Action(ID)                       │
│  + syncWrite(ID[], ...)             │
│  + Ping(ID)                         │
│  + ReadByte/Word(ID, Addr)          │
│  + WriteByte/Word(ID, Addr, Val)    │
│  ────────────────────────────────── │
│  # virtual writeSCS(data, len) = 0  │
│  # virtual readSCS(data, len) = 0   │
│  # virtual rFlushSCS() = 0          │
│  # virtual wFlushSCS() = 0          │
└──────────────┬──────────────────────┘
               │
               │ inherits
               │
┌──────────────▼──────────────────────┐
│           SCSerial                  │
│  ────────────────────────────────── │
│  - int fd (file descriptor)         │
│  - termios orgopt, curopt           │
│  - u8 txBuf[BUFFER_SIZE]            │
│  - int txBufLen                     │
│  + int Err                          │
│  + ulong IOTimeOut                  │
│  ────────────────────────────────── │
│  + bool begin(baud, port)           │
│  + void end()                       │
│  + int setBaudRate(baud)            │
│  # int writeSCS(data, len)          │
│  # int readSCS(data, len)           │
│  # void rFlushSCS()                 │
│  # void wFlushSCS()                 │
└──────────────┬──────────────────────┘
               │
      ┌────────┼────────┬────────┐
      │        │        │        │
┌─────▼───┐ ┌─▼────┐ ┌─▼────┐ ┌─▼─────┐
│SMS_STS  │ │SMSCL │ │SMSBL │ │SCSCL  │
│         │ │      │ │      │ │       │
│Memory   │ │Memory│ │Memory│ │Memory │
│  Table  │ │Table │ │Table │ │Table  │
│         │ │      │ │      │ │       │
│API      │ │API   │ │API   │ │API    │
│Methods  │ │Method│ │Method│ │Methods│
└─────────┘ └──────┘ └──────┘ └───────┘
```

### Key Classes

#### `SCS` (Base Protocol Class)

**Type**: Abstract base class

**Purpose**: Define common protocol operations

**Key Members**:
- `Level` - Response detail level (舵机返回等级 - servo response level)
- `End` - Endianness flag (0=little-endian, 1=big-endian)
- `Error` - Servo status / last error code
- `syncReadRxBuff` - Buffer for synchronized read operations
- `syncReadRxPacket` - Packet buffer for sync read
- `syncReadRxPacketIndex`, `syncReadRxPacketLen` - Packet tracking
- `syncReadRxBuffLen`, `syncReadRxBuffMax` - Buffer size tracking

**Virtual Methods** (must be implemented by derived class):
```cpp
virtual int writeSCS(unsigned char *nDat, int nLen) = 0;
virtual int readSCS(unsigned char *nDat, int nLen) = 0;
virtual void rFlushSCS() = 0;  // Flush receive buffer
virtual void wFlushSCS() = 0;  // Flush transmit buffer
```

**Public Methods**:
- Packet construction and sending
- Command encoding
- Response parsing
- Generic read/write/sync operations

**Design Pattern**: Template Method Pattern
- Defines algorithm structure in base class
- Derived classes provide platform-specific implementations

---

#### `SCSerial` (Hardware Interface)

**Type**: Concrete class inheriting from `SCS`

**Purpose**: Linux-specific serial I/O implementation

**Key Members**:
- `fd` - File descriptor for serial port
- `orgopt` - Original termios configuration (for restoration on `end()`)
- `curopt` - Current termios configuration
- `txBuf` - Transmit buffer
- `IOTimeOut` - Read timeout in microseconds

**Key Methods**:
```cpp
bool begin(int baudrate, const char* port);
void end() noexcept;
int setBaudRate(int baudRate);
```

**Resource Management**:
- Follows RAII principles
- Destructor calls `end()` to close file descriptor
- Copy constructor deleted (resource ownership)
- Move semantics not implemented (single-owner model)

---

#### `SMS_STS` (Application API)

**Type**: Concrete class inheriting from `SCSerial`

**Purpose**: SMS/STS series servo control

**Memory Table** (partial):
```cpp
#define SMS_STS_ID                     5   // Motor ID
#define SMS_STS_BAUD_RATE              6   // Baud rate
#define SMS_STS_MODE                   33  // Operating mode
#define SMS_STS_TORQUE_ENABLE          40  // Torque on/off
#define SMS_STS_GOAL_POSITION_L        42  // Target pos (low byte)
#define SMS_STS_GOAL_POSITION_H        43  // Target pos (high byte)
#define SMS_STS_PRESENT_POSITION_L     56  // Current pos (low byte)
#define SMS_STS_PRESENT_POSITION_H     57  // Current pos (high byte)
// ... many more
```

**High-Level Methods**:
```cpp
int WritePosEx(u8 ID, s16 Position, u16 Speed, u8 Acc);
int WriteSpe(u8 ID, s16 Speed, u8 Acc);
int WritePwm(u8 ID, s16 Pwm);
int SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 Acc[]);
int FeedBack(u8 ID);
int ReadPos(s8 ID);
// ... many more
```

---

### Utility Classes (2025 Refactoring)

#### `ServoUtils.h`

Direction bit encoding/decoding helpers:

```cpp
// Encode/decode signed values with direction bit
u16 encodeSignedValue(s16 value, u8 directionBit = 15);
s16 decodeSignedValue(u16 encodedValue, u8 directionBit = 15);

// Read signed word from buffer with direction decoding
s16 readSignedWordFromBuffer(const u8* buffer, u8 offsetLow, u8 offsetHigh, u8 directionBit);

// Cache-or-servo read helpers
template<typename ServoType>
int readByteFromCacheOrServo(ServoType& servoInstance, int ID, u8 registerAddr,
                               const u8* cachedBuffer, u8 cacheOffset, int& err);
```

**Purpose**: DRY (Don't Repeat Yourself) compliance - centralize bit manipulation logic.

---

#### `SyncWriteBuffer.h`

RAII-based buffer management:

```cpp
class SyncWriteBuffer {
    u8* buffer;
    size_t size;
public:
    SyncWriteBuffer(size_t sz);
    ~SyncWriteBuffer();
    u8* data();
};
```

**Purpose**: Automatic buffer cleanup, exception safety.

---

#### `ServoErrors.h`

Standardized error codes and result types:

```cpp
enum class ServoError {
    SUCCESS = 0,
    COMM_TIMEOUT = -1,
    COMM_RX_FAIL = -2,
    COMM_TX_FAIL = -3,
    INVALID_PARAMETER = -4,
    ALLOCATION_FAILED = -5,
    REGISTER_WRITE_FAILED = -6,
    CHECKSUM_ERROR = -7,
    UNKNOWN_ERROR = -99
};

class ServoResult {
    ServoError errorCode;
    int value;
public:
    bool isSuccess() const;
    ServoError getError() const;
    int getValue() const;
    operator int() const;  // Backward compatibility
};
```

**Purpose**: Type-safe error handling, future-proofing for exceptions.

---

## Communication Protocol

### Packet Structure

All communication uses the Feetech serial servo protocol:

```
Instruction Packet (Master → Servo):
┌────┬────┬────┬────┬─────┬──────────┬──────────┬─────┬──────────┐
│0xFF│0xFF│ ID │LEN │INST │ PARAM 1  │ PARAM 2  │ ... │CHECKSUM  │
└────┴────┴────┴────┴─────┴──────────┴──────────┴─────┴──────────┘

Status Packet (Servo → Master):
┌────┬────┬────┬────┬──────┬──────────┬──────────┬─────┬──────────┐
│0xFF│0xFF│ ID │LEN │ERROR │ PARAM 1  │ PARAM 2  │ ... │CHECKSUM  │
└────┴────┴────┴────┴──────┴──────────┴──────────┴─────┴──────────┘
```

### Field Definitions

| Field | Size | Description |
|-------|------|-------------|
| Header | 2 bytes | Always 0xFF 0xFF |
| ID | 1 byte | Servo ID (0-253, 254=broadcast) |
| Length | 1 byte | Number of parameters + 2 |
| Instruction/Error | 1 byte | Command code or error status |
| Parameters | N bytes | Command-specific data |
| Checksum | 1 byte | `~(ID + LEN + INST + PARAMS) & 0xFF` |

### Instruction Set

| Code | Name | Direction | Description |
|------|------|-----------|-------------|
| 0x01 | PING | Bi | Test servo presence |
| 0x02 | READ_DATA | Bi | Read memory table |
| 0x03 | WRITE_DATA | Master→Servo | Write memory table |
| 0x04 | REG_WRITE | Master→Servo | Buffered write |
| 0x05 | ACTION | Master→Servo | Execute buffered commands |
| 0x06 | FACTORY_RESET | Master→Servo | Reset to defaults |
| 0x82 | SYNC_WRITE | Master→Servo | Write to multiple servos |
| 0x83 | SYNC_READ | Bi | Read from multiple servos |

### Example: Write Position

**Command**: Move servo ID=1 to position 2048

**Packet Construction**:
```
Header:  0xFF 0xFF
ID:      0x01
Length:  0x07  (5 params + 2)
Inst:    0x03  (WRITE_DATA)
Addr:    0x2A  (SMS_STS_GOAL_POSITION_L = 42)
Param1:  0x00  (2048 & 0xFF = 0)
Param2:  0x08  (2048 >> 8 = 8)
Param3:  0x60  (2400 & 0xFF = 96)
Param4:  0x09  (2400 >> 8 = 9)
Checksum: ~(0x01+0x07+0x03+0x2A+0x00+0x08+0x60+0x09) = 0x__
```

**C++ Implementation** (simplified - actual implementation uses ServoUtils):
```cpp
int SMS_STS::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 Acc) {
    // Encode position with direction bit if negative
    u16 encodedPosition = ServoUtils::encodeSignedValue(Position, SMS_STS_DIRECTION_BIT_POS);

    u8 bBuf[7];
    bBuf[0] = Acc;
    bBuf[1] = encodedPosition & 0xFF;
    bBuf[2] = (encodedPosition >> 8) & 0xFF;
    bBuf[3] = 0;  // Time low byte (unused in PosEx)
    bBuf[4] = 0;  // Time high byte (unused in PosEx)
    bBuf[5] = Speed & 0xFF;
    bBuf[6] = (Speed >> 8) & 0xFF;

    return genWrite(ID, SMS_STS_ACC, bBuf, 7);
}
```

---

## Data Flow

### Write Operation Flow

```
User Code
    │
    ▼
WritePosEx(ID, Pos, Speed, Acc)
    │
    ├─ Prepare parameters (encoding)
    │
    ▼
genWrite(ID, MemAddr, Data, Len)  [SCS layer]
    │
    ├─ Construct packet
    │   ├─ Add header [0xFF, 0xFF]
    │   ├─ Add ID
    │   ├─ Add length
    │   ├─ Add instruction (WRITE_DATA)
    │   ├─ Add memory address
    │   ├─ Add parameters
    │   └─ Calculate and add checksum
    │
    ▼
writeSCS(packet, len)  [SCSerial layer]
    │
    ├─ Copy to txBuf
    │
    ▼
write(fd, txBuf, len)  [POSIX]
    │
    ▼
[Hardware UART] → Servo
```

### Read Operation Flow

```
User Code
    │
    ▼
FeedBack(ID)  or  ReadPos(ID)
    │
    ▼
ReadWord(ID, MemAddr)  [SCS layer]
    │
    ├─ Construct READ_DATA packet
    │
    ▼
writeSCS(packet, len)  [Send command]
    │
    ▼
[Hardware UART] → Servo
    │
[Servo processes and responds]
    │
    ▼
readSCS(buffer, expected_len)  [SCSerial layer]
    │
    ├─ select() with timeout
    │
    ▼
read(fd, buffer, len)  [POSIX]
    │
    ▼
Parse response packet  [SCS layer]
    │
    ├─ Verify header [0xFF, 0xFF]
    ├─ Check ID matches
    ├─ Verify checksum
    ├─ Extract error status
    └─ Extract data
    │
    ▼
Return value to user
```

### Synchronized Write Flow

```
User Code
    │
    ▼
SyncWritePosEx(IDs[], IDN, Pos[], Spd[], Acc[])
    │
    ├─ Construct SYNC_WRITE packet:
    │   [0xFF][0xFF][0xFE][LEN][0x82][StartAddr][DataLen]
    │   [ID1][Data1...]
    │   [ID2][Data2...]
    │   ...
    │   [Checksum]
    │
    ▼
syncWrite(IDs, IDN, MemAddr, Data, DataLen)  [SCS layer]
    │
    ▼
writeSCS(packet, total_len)
    │
    ▼
[Hardware UART] → All Servos (broadcast)
    │
    └─ All servos execute simultaneously
```

**Advantage**: Single packet for multiple servos = better synchronization and less bus traffic.

---

## Memory Management

### Resource Ownership

#### File Descriptors
- **Owned by**: `SCSerial` class
- **Acquired**: `begin()` opens serial port
- **Released**: `end()` closes file descriptor
- **RAII**: Destructor calls `end()` automatically

```cpp
{
    SMS_STS servo;
    servo.begin(1000000, "/dev/ttyUSB0");
    // Use servo...
}  // Destructor automatically closes serial port
```

#### Sync Read Buffer
- **Owned by**: `SCS` class
- **Allocated**: First `syncRead()` call (lazy initialization)
- **Released**: Destructor frees buffer
- **Size**: `SCSERVO_BUFFER_SIZE` (defined in `INST.h`)

#### Transmit Buffer
- **Owned by**: `SCSerial` class
- **Type**: Stack-allocated array `txBuf[SCSERVO_BUFFER_SIZE]`
- **Lifetime**: Matches object lifetime (no manual management)

### Copy Semantics

**Copy Disabled**:
```cpp
// Copy constructor deleted
SCSerial(const SCSerial&) = delete;
SCSerial& operator=(const SCSerial&) = delete;
```

**Reason**: Serial port file descriptors cannot be safely copied (single owner).

**Alternative**: Pass by reference or pointer:
```cpp
void controlServo(SMS_STS& servo) {  // Reference
    servo.WritePosEx(1, 2048, 2400, 50);
}
```

---

## Thread Safety

### Current Status

**Not Thread-Safe**: The library is NOT designed for concurrent access from multiple threads.

### Unsafe Operations

1. **Shared State**:
   - File descriptor (`fd`)
   - Transmit buffer (`txBuf`)
   - Cached feedback data
   - Error state (`Error`, `Err`)

2. **Non-Atomic Sequences**:
   - Write + Read (command-response)
   - Multi-packet operations (Reg Write + Action)

### Safe Usage Patterns

#### Single-Threaded
```cpp
SMS_STS servo;
servo.begin(1000000, "/dev/ttyUSB0");

// All operations in main thread
servo.WritePosEx(1, 2048, 2400, 50);
servo.FeedBack(1);
int pos = servo.ReadPos(-1);
```

#### Multi-Threaded with Mutex
```cpp
#include <mutex>

std::mutex servo_mutex;
SMS_STS servo;

void thread1() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    servo.WritePosEx(1, 2048, 2400, 50);
}

void thread2() {
    std::lock_guard<std::mutex> lock(servo_mutex);
    servo.FeedBack(1);
    int pos = servo.ReadPos(-1);
}
```

#### Multiple Serial Ports
```cpp
// Each bus has its own instance (thread-safe if on different ports)
SMS_STS servo1;  // /dev/ttyUSB0
SMS_STS servo2;  // /dev/ttyUSB1

std::thread t1([&]() { servo1.WritePosEx(1, 2048, 2400, 50); });
std::thread t2([&]() { servo2.WritePosEx(1, 2048, 2400, 50); });
```

---

## Design Patterns

### 1. Template Method Pattern

**Used in**: `SCS` base class

**Purpose**: Define algorithm skeleton while allowing subclasses to customize specific steps.

```cpp
// SCS defines the algorithm
int SCS::genWrite(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen) {
    // 1. Construct packet (common)
    u8 txBuf[128];
    txBuf[0] = 0xFF;
    txBuf[1] = 0xFF;
    // ... build packet

    // 2. Send via virtual method (platform-specific)
    writeSCS(txBuf, txLen);

    // 3. Wait for response if needed (common logic)
    if (Level == 2) {
        readSCS(rxBuf, expectedLen);
        // ... parse response
    }
}

// SCSerial provides concrete implementation
int SCSerial::writeSCS(u8 *nDat, int nLen) {
    return write(fd, nDat, nLen);  // POSIX-specific
}
```

---

### 2. RAII (Resource Acquisition Is Initialization)

**Used throughout**: File descriptors, buffers

**Benefits**:
- Automatic cleanup
- Exception safety
- Clear ownership

**Example**:
```cpp
class SCSerial {
    int fd;  // Resource
public:
    bool begin(int baud, const char* port) {
        fd = open(port, O_RDWR | O_NOCTTY);
        return (fd != -1);
    }

    ~SCSerial() {  // Destructor = cleanup
        end();
    }

    void end() noexcept {
        if (fd != -1) {
            close(fd);
            fd = -1;
        }
    }
};
```

---

### 3. Strategy Pattern (Implicit)

**Used in**: Protocol-specific classes

**Purpose**: Different algorithms for different servo protocols.

```cpp
// Different strategies for different protocols
SMS_STS servo_sts;    // 12-bit, 1M baud strategy
SCSCL servo_scscl;    // 10-bit, 115200 baud strategy

// Same interface, different behavior
servo_sts.WritePosEx(1, 2048, 2400, 50);   // Uses SMS_STS memory map
servo_scscl.WritePos(1, 512, 100, 1200);   // Uses SCSCL memory map
```

---

### 4. Facade Pattern

**Used in**: High-level API methods

**Purpose**: Simplify complex subsystem (memory table, protocol packets).

```cpp
// Complex underlying operations
int InitMotor(u8 ID, u8 mode, u8 torque) {
    // Facade hides complexity:
    // 1. Unlock EEPROM
    // 2. Write mode to address 33
    // 3. Lock EEPROM
    // 4. Write torque to address 40

    int ret = unLockEeprom(ID);
    if(ret == 0) return 0;

    ret = Mode(ID, mode);
    if(ret == 0) return 0;

    ret = LockEeprom(ID);
    if(ret == 0) return 0;

    return EnableTorque(ID, torque);
}

// User sees simple interface
servo.InitMotor(1, 1, 1);  // Easy!
```

---

## Python Bindings Architecture

### nanobind Integration

Python bindings use [nanobind](https://github.com/wjakob/nanobind) for modern, efficient C++/Python interop.

**Binding Code** (`src/bindings.cpp`):
```cpp
#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include "SMS_STS.h"

namespace nb = nanobind;

NB_MODULE(scservo_python, m) {
    nb::class_<SMS_STS>(m, "SMS_STS")
        .def(nb::init<>())
        .def("begin", &SMS_STS::begin)
        .def("end", &SMS_STS::end)
        .def("WritePosEx", &SMS_STS::WritePosEx)
        .def("WriteSpe", &SMS_STS::WriteSpe)
        .def("FeedBack", &SMS_STS::FeedBack)
        .def("ReadPos", &SMS_STS::ReadPos)
        // ... all public methods
        ;
}
```

**Build System** (`CMakeLists.txt`):
```cmake
find_package(Python 3.8 REQUIRED COMPONENTS Interpreter Development.Module)
add_subdirectory(extern/nanobind)

nanobind_add_module(scservo_python src/bindings.cpp)
target_link_libraries(scservo_python PRIVATE SCServo)
```

**Python Usage**:
```python
import scservo_python as sc

servo = sc.SMS_STS()
servo.begin(1000000, "/dev/ttyUSB0")
servo.WritePosEx(1, 2048, 2400, 50)
servo.end()
```