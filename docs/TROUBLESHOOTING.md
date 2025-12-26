# Troubleshooting Guide

> **⚠️ AI-Generated Documentation**: This documentation was generated with AI assistance and may contain inconsistencies with the actual codebase. Always verify commands, function names, and procedures against the source code and examples when in doubt.

Comprehensive troubleshooting guide for common issues with SCServo_Linux.

## Table of Contents

- [Connection Issues](#connection-issues)
- [Motor Behavior Problems](#motor-behavior-problems)
- [Communication Errors](#communication-errors)
- [Build and Installation Issues](#build-and-installation-issues)
- [Performance Issues](#performance-issues)
- [Advanced Debugging](#advanced-debugging)

---

## Connection Issues

### "Failed to init motor" or "Failed to open serial port"

**Symptoms:**
- `begin()` returns `false`
- Error message: "Failed to initialize serial bus"
- Cannot establish communication with servos

**Root Causes & Solutions:**

#### 1. Permission Denied

**Diagnosis:**
```bash
ls -l /dev/ttyUSB0
# Output: crw-rw---- 1 root dialout ...
```
User is not in the `dialout` group.

**Solution:**
```bash
# Add user to dialout group
sudo usermod -a -G dialout $USER

# Verify membership
groups $USER

# Log out and log back in for changes to take effect
```

**Temporary Fix** (for testing only):
```bash
sudo chmod 666 /dev/ttyUSB0
```

#### 2. Incorrect Serial Port

**Diagnosis:**
```bash
# List all serial devices
ls /dev/tty*

# Common USB-serial devices
ls /dev/ttyUSB*   # USB-to-Serial adapters
ls /dev/ttyACM*   # CDC-ACM devices (some Arduino boards)
ls /dev/ttyS*     # Built-in serial ports

# Check which device was just connected
dmesg | grep tty | tail -5
```

**Solution:**
Use the correct device path in your code:
```cpp
servo.begin(1000000, "/dev/ttyUSB0");  // Adjust device name
```

#### 3. Device Already in Use

**Diagnosis:**
```bash
# Check if port is already open
lsof /dev/ttyUSB0
```

**Solution:**
- Close other programs using the serial port (Arduino IDE, minicom, screen, etc.)
- Kill hanging processes: `sudo fuser -k /dev/ttyUSB0`
- Only one program can access a serial port at a time

#### 4. Faulty USB Cable or Adapter

**Diagnosis:**
```bash
# Monitor USB connections
dmesg -w
# Then disconnect and reconnect adapter

# Check USB device info
lsusb
```

**Solution:**
- Try different USB cable (some cables are charge-only, not data)
- Try different USB port
- Try different USB-to-Serial adapter
- Check for loose connections

---

### "Ping servo ID error" or No Response from Servo

**Symptoms:**
- `Ping(ID)` returns `-1` (communication failure)
- Servo doesn't respond to commands
- LED on servo may or may not be lit

**Root Causes & Solutions:**

#### 1. Wiring Issues

**Diagnosis:**
Check connections:
```
USB-Serial Adapter          Servo
    TX      ---------->     RX (Data In)
    RX      <----------     TX (Data Out)
    GND     <--------->     GND
                            VCC (to external PSU)
```

**Common Mistakes:**
- TX/RX swapped (servo receives on TX, sends on RX for half-duplex)
- Missing ground connection
- Loose connection in daisy-chain

**Solution:**
- For half-duplex serial, TX and RX may need to be tied together at adapter side (depends on adapter)
- Ensure solid ground connection
- Check all connections in multi-servo setups

#### 2. Incorrect Baud Rate

**Diagnosis:**
Servo factory defaults:
- STS series: Usually 1000000 (1M)
- SMS series: Usually 115200

**Solution:**
Try different baud rates:
```cpp
// Try common rates
int baud_rates[] = {1000000, 115200, 500000, 57600};
for (int baud : baud_rates) {
    if (servo.begin(baud, "/dev/ttyUSB0")) {
        if (servo.Ping(1) != -1) {
            printf("Found servo at %d baud\n", baud);
            break;
        }
    }
    servo.end();
}
```

#### 3. Incorrect Motor ID

**Diagnosis:**
Default factory ID is usually 1, but may have been changed.

**Solution:**
Scan for all IDs:
```cpp
printf("Scanning for servos...\n");
for (u8 id = 0; id < 254; id++) {
    if (servo.Ping(id) != -1) {
        printf("Found servo at ID %d\n", id);
    }
}
```

Or use the Ping example:
```bash
cd /home/ubuntu/SCServo_Linux
mkdir -p build && cd build
cmake .. && make
./examples/SMS_STS/Ping/Ping /dev/ttyUSB0
```

#### 4. Power Supply Issues

**Symptoms:**
- Servo LED is dim or off
- Servo powers on but doesn't respond consistently
- Works initially but fails under load

**Diagnosis:**
```cpp
servo.FeedBack(1);
int voltage = servo.ReadVoltage(-1);
printf("Voltage: %.1fV\n", voltage / 10.0);
```

**Solution:**
- Ensure external power supply is connected (servos draw significant current)
- Check power supply voltage (typically 6-14V depending on model)
- Verify power supply can provide sufficient current (2-3A per servo recommended)
- Check for voltage drops under load
- Use separate power for logic and motors if necessary

#### 5. Broadcast ID Interference

**Problem:**
Using broadcast ID (254) when only one servo is expected.

**Solution:**
- Use specific IDs for individual servos
- Only use broadcast (254) for commands to all servos
- Cannot use broadcast for read operations

---

## Motor Behavior Problems

### Servo Moves but Positions are Incorrect

**Symptoms:**
- Servo moves to wrong position
- Position readings don't match expected values
- Servo "jumps" to unexpected angles

**Root Causes & Solutions:**

#### 1. Calibration Offset

**Diagnosis:**
The servo's mechanical center doesn't match the software center (2048 for 12-bit).

**Solution:**
Run midpoint calibration:

```bash
cd /home/ubuntu/SCServo_Linux
mkdir -p build && cd build
cmake .. && make
./examples/sandbox/MidpointCalib/MidpointCalib /dev/ttyUSB0
```

Or manually:
```cpp
// Disable torque and manually position servo to desired center
servo.EnableTorque(1, 0);
printf("Position servo to center, press Enter...\n");
getchar();

// Save this position as center
servo.unLockEeprom(1);
servo.CalibrationOfs(1);
servo.LockEeprom(1);
servo.EnableTorque(1, 1);
```

#### 2. Wrong Protocol Class

**Problem:**
Using SMS_STS class for SCSCL servo or vice versa.

**Diagnosis:**
- SCSCL servos: 10-bit resolution (0-1023)
- SMS_STS servos: 12-bit resolution (0-4095)

**Solution:**
Use the correct class for your servo model:
```cpp
SCSCL servo;    // For SC series (10-bit)
SMS_STS servo;  // For SMS/STS series (12-bit)
```

#### 3. Angle Limits

**Problem:**
Software angle limits restrict movement.

**Diagnosis:**
```cpp
servo.FeedBack(1);
int min_angle = servo.readWord(1, SMS_STS_MIN_ANGLE_LIMIT_L);
int max_angle = servo.readWord(1, SMS_STS_MAX_ANGLE_LIMIT_L);
printf("Limits: %d to %d\n", min_angle, max_angle);
```

**Solution:**
Reset angle limits:
```cpp
servo.unLockEeprom(1);
servo.writeByte(1, SMS_STS_MIN_ANGLE_LIMIT_L, 0 & 0xFF);
servo.writeByte(1, SMS_STS_MIN_ANGLE_LIMIT_H, (0 >> 8) & 0xFF);
servo.writeByte(1, SMS_STS_MAX_ANGLE_LIMIT_L, 4095 & 0xFF);
servo.writeByte(1, SMS_STS_MAX_ANGLE_LIMIT_H, (4095 >> 8) & 0xFF);
servo.LockEeprom(1);
```

#### 4. Mechanical Binding

**Symptoms:**
- Servo can't reach target position
- High load readings
- Servo stalls or vibrates

**Diagnosis:**
```cpp
servo.FeedBack(1);
int load = servo.ReadLoad(-1);
printf("Load: %d\n", load);  // High magnitude indicates binding
```

**Solution:**
- Check for mechanical obstructions
- Reduce load on servo
- Ensure smooth mechanical linkages
- Verify servo torque rating is sufficient

---

### Servo Doesn't Move After Fresh Boot

**Symptoms:**
- Code that worked before suddenly fails after reboot
- `Ping()` succeeds but movement commands fail
- Error: "Failed to init motor"

**Root Cause:**
This was a known bug in older SDK versions where baud rate constants were not properly defined for Linux termios.

**Diagnosis:**
Check if you're using raw integer baud rates:
```cpp
// WRONG (old buggy code)
servo.begin(1000000, "/dev/ttyUSB0");  // May fail on some systems
```

**Solution:**
Ensure you have the latest version of SCSerial.cpp with proper baud rate mapping:

```cpp
// Check SCSerial.cpp has this pattern:
int SCSerial::setBaudRate(int baudRate) {
    switch(baudRate) {
        case 1000000: CR_BAUDRATE = B1000000; break;
        case 500000:  CR_BAUDRATE = B500000;  break;
        case 115200:  CR_BAUDRATE = B115200;  break;
        // ... etc
    }
}
```

If you have old code, update from the repository:
```bash
cd SCServo_Linux
git pull origin main
rm -rf build/
mkdir -p build && cd build
cmake .. && make -j4
```

---

### Motor Jittering or Unstable

**Symptoms:**
- Servo vibrates or oscillates around target position
- Erratic movement
- Noisy operation

**Root Causes & Solutions:**

#### 1. Acceleration Too High

**Problem:**
Acceleration parameter set too low (0 = maximum acceleration = very aggressive).

**Solution:**
Increase acceleration value for smoother motion:
```cpp
// Too aggressive
servo.WritePosEx(1, 2048, 2400, 0);   // Accel=0 (max)

// Better
servo.WritePosEx(1, 2048, 2400, 50);  // Medium accel
servo.WritePosEx(1, 2048, 2400, 100); // Smooth accel
```

#### 2. Speed Too High for Load

**Problem:**
Servo can't maintain requested speed due to load.

**Solution:**
- Reduce speed parameter
- Reduce mechanical load
- Use larger/stronger servo
- Monitor load:
```cpp
servo.FeedBack(1);
int load = servo.ReadLoad(-1);
if (abs(load) > 800) {
    printf("Warning: Load too high!\n");
}
```

#### 3. Power Supply Issues

**Problem:**
Voltage fluctuations cause erratic behavior.

**Diagnosis:**
```cpp
servo.FeedBack(1);
int voltage = servo.ReadVoltage(-1);
printf("Voltage: %.1fV\n", voltage / 10.0);
```

**Solution:**
- Use regulated power supply
- Add capacitors near servo power input (100µF-1000µF)
- Ensure power wires are thick enough (low resistance)
- Check for voltage drops under load

#### 4. Mechanical Resonance

**Problem:**
Natural frequency of mechanical system matches control frequency.

**Solution:**
- Change acceleration/speed parameters
- Add mechanical damping
- Adjust load or linkage stiffness

---

## Communication Errors

### High-Speed Communication Errors at 1Mbps

**Symptoms:**
- Intermittent command failures
- Dropped packets
- Works at 115200 but not 1000000

**Root Causes & Solutions:**

#### 1. Cable Quality

**Problem:**
Long or unshielded cables introduce noise and signal degradation.

**Solution:**
- Use shielded twisted-pair cable for data lines
- Keep cable length under 1 meter for 1Mbps
- Use quality USB-to-Serial adapter (FTDI FT232 recommended)
- For longer distances, reduce baud rate to 500000 or 115200

#### 2. Electrical Noise

**Problem:**
Motor switching noise interferes with serial communication.

**Solution:**
- Separate power and signal wires
- Add ferrite beads on motor power wires
- Use twisted pair for TX/RX lines
- Keep signal wires away from motor power wires
- Add 100nF ceramic capacitor across motor power pins

#### 3. Timing Issues

**Problem:**
Insufficient delays between commands cause buffer overruns.

**Solution:**
Add small delays:
```cpp
servo.WritePosEx(1, 2048, 2400, 50);
usleep(1000);  // 1ms delay
servo.WritePosEx(2, 2048, 2400, 50);
usleep(1000);

// Or use SyncWrite for multiple motors (more efficient)
servo.SyncWritePosEx(IDs, 3, positions, speeds, accel);
```

#### 4. USB-Serial Adapter Limitations

**Problem:**
Some cheap CH340/CP2102 adapters have issues at high baud rates.

**Solution:**
- Use genuine FTDI FT232 adapter
- Update USB-serial driver
- Check `dmesg` for USB errors
- Try different USB port (preferably USB 2.0/3.0, not hub)

---

### Random Communication Failures

**Symptoms:**
- Sporadic errors
- Works most of the time, fails occasionally
- `Ping()` sometimes succeeds, sometimes fails

**Root Causes & Solutions:**

#### 1. Buffer Overrun

**Problem:**
Sending commands too fast without waiting for response.

**Solution:**
```cpp
// Check return value
int ret = servo.WritePosEx(1, 2048, 2400, 50);
if (ret == 0) {
    printf("Command failed!\n");
    int err = servo.getErr();
    printf("Error code: %d\n", err);
}
```

Add delays or use synchronous operations properly.

#### 2. Multi-Threading Issues

**Problem:**
Multiple threads accessing same serial port.

**Solution:**
- Use mutex to protect serial port access
- Ensure only one thread calls servo functions at a time
- Consider using separate servo instances for different buses

#### 3. Packet Collisions

**Problem:**
Multiple masters on same bus (rare in typical setups).

**Solution:**
- Only one master (computer) should control a servo bus
- If using multiple computers, use separate serial ports/buses
- Implement proper bus arbitration if multi-master is required

---

## Build and Installation Issues

### CMake Configuration Errors

**Symptoms:**
- `cmake .` fails
- Missing dependencies errors
- Version mismatch errors

**Solutions:**

#### CMake Version Too Old
```bash
cmake --version  # Check version
# Need CMake 3.5+

# Update on Ubuntu
sudo apt-get update
sudo apt-get install cmake

# Or install latest from pip
pip3 install cmake --upgrade
```

#### Missing Build Tools
```bash
sudo apt-get install build-essential git
```

#### Clean Build
```bash
# Remove old build directory
cd /home/ubuntu/SCServo_Linux
rm -rf build/

# Rebuild
mkdir -p build && cd build
cmake ..
make -j4
```

---

### Compilation Errors

**Symptoms:**
- C++ errors during `make`
- Undefined references
- Syntax errors

**Solutions:**

#### C++17 Support
```bash
# Check compiler version (need GCC 7+)
g++ --version

# Update if needed (Ubuntu)
sudo apt-get install g++-9
```

#### Missing Headers
```bash
# Install development packages
sudo apt-get install build-essential linux-headers-$(uname -r)
```

---

### Python Binding Errors

**Symptoms:**
- `pip install -e .` fails
- Cannot import `scservo_python`
- nanobind errors

**Solutions:**

#### Python Development Headers Missing
```bash
sudo apt-get install python3-dev python3-pip
```

#### nanobind Submodule Not Initialized
```bash
cd SCServo_Linux
git submodule update --init --recursive
mkdir -p build && cd build
cmake .. && make -j4
```

#### Wrong Python Version
```bash
# Check Python version (need 3.8+)
python3 --version

# Specify Python version explicitly
cmake -DPYTHON_EXECUTABLE=/usr/bin/python3.10 .
```

#### Module Not Found After Install
```bash
# Check where module was installed
pip3 show scservo-python

# Add to Python path if needed
export PYTHONPATH=/home/ubuntu/SCServo_Linux:$PYTHONPATH

# Or reinstall
pip3 uninstall scservo-python
cd SCServo_Linux
pip3 install -e .
```

---

## Performance Issues

### Slow Communication

**Problem:**
Commands take longer than expected.

**Diagnosis:**
```cpp
#include <chrono>

auto start = std::chrono::high_resolution_clock::now();
servo.WritePosEx(1, 2048, 2400, 50);
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
printf("Command took %ld µs\n", duration.count());
```

**Solutions:**

1. **Use SyncWrite for multiple motors** (faster than individual writes):
```cpp
// Slow (3 separate packets)
servo.WritePosEx(1, 1024, 2400, 50);
servo.WritePosEx(2, 2048, 2400, 50);
servo.WritePosEx(3, 3072, 2400, 50);

// Fast (1 packet)
u8 IDs[3] = {1, 2, 3};
s16 pos[3] = {1024, 2048, 3072};
u16 spd[3] = {2400, 2400, 2400};
u8 acc[3] = {50, 50, 50};
servo.SyncWritePosEx(IDs, 3, pos, spd, acc);
```

2. **Increase baud rate** (if cables allow):
```cpp
servo.begin(1000000, "/dev/ttyUSB0");  // vs 115200
```

3. **Reduce unnecessary reads**:
```cpp
// Use FeedBack once, then cached reads
servo.FeedBack(1);
int pos = servo.ReadPos(-1);     // Cached (fast)
int speed = servo.ReadSpeed(-1); // Cached (fast)
```

---

### High CPU Usage

**Problem:**
Program uses excessive CPU polling servos.

**Solution:**
Add delays in polling loops:
```cpp
// Bad (busy wait)
while (servo.ReadMove(1) == 1) {
    // No delay - 100% CPU usage
}

// Good
while (servo.ReadMove(1) == 1) {
    usleep(10000);  // 10ms delay - low CPU
}
```

---

## Advanced Debugging

### Enable Verbose Output

Modify `SCSerial.cpp` to add debug prints:

```cpp
// In readSCS() function
int SCSerial::readSCS(unsigned char *nDat, int nLen) {
    printf("Reading %d bytes...\n", nLen);
    int ret = read(fd, nDat, nLen);
    printf("Read %d bytes:", ret);
    for(int i = 0; i < ret; i++) {
        printf(" %02X", nDat[i]);
    }
    printf("\n");
    return ret;
}

// In writeSCS() function
int SCSerial::writeSCS(unsigned char *nDat, int nLen) {
    printf("Writing %d bytes:", nLen);
    for(int i = 0; i < nLen; i++) {
        printf(" %02X", nDat[i]);
    }
    printf("\n");
    return write(fd, nDat, nLen);
}
```

Rebuild and run to see all serial traffic.

---

### Monitor Serial Traffic

Using `minicom`:
```bash
sudo apt-get install minicom
minicom -D /dev/ttyUSB0 -b 1000000
# Press Ctrl+A then Z for help menu
# Press Ctrl+A then X to exit
```

Using `screen`:
```bash
screen /dev/ttyUSB0 1000000
# Press Ctrl+A then K to kill session
```

---

### Check USB Connection

```bash
# Monitor USB events in real-time
dmesg -w
# Then disconnect/reconnect adapter

# Check USB device info
lsusb -v | grep -A 5 "USB-Serial"

# Check kernel messages for errors
dmesg | grep -i usb | tail -20
```

---

### Logic Analyzer

For serious debugging, use a logic analyzer (e.g., Saleae, PulseView):
- Capture TX/RX signals
- Decode UART protocol
- Check timing, baud rate, packet structure
- Identify noise or signal integrity issues

---

### Test with Known-Good Hardware

Isolate the problem:
1. Test servo with official Feetech software (Windows)
2. Test with different USB-serial adapter
3. Test with different servo
4. Test on different computer
5. Test with shorter cable

---

## Common Error Messages

### Error: "Device or resource busy"

**Cause:** Serial port already in use.

**Solution:**
```bash
sudo fuser -k /dev/ttyUSB0  # Kill processes using port
```

---

### Error: "Input/output error"

**Cause:** Hardware disconnection or driver issue.

**Solution:**
- Unplug and replug USB adapter
- Check `dmesg` for USB errors
- Reload USB-serial driver: `sudo modprobe -r ftdi_sio && sudo modprobe ftdi_sio`

---

### Error: "No such file or directory"

**Cause:** Wrong serial port path.

**Solution:**
```bash
ls /dev/ttyUSB* /dev/ttyACM*  # Find correct device
```

---

## Getting Help

If you're still stuck:

1. **Check existing issues**: [GitHub Issues](https://github.com/adityakamath/SCServoArduino/issues)

2. **Collect debug information**:
   - OS version: `uname -a`
   - Servo model
   - Baud rate
   - USB-serial adapter model
   - Minimal code example
   - Error messages and output

3. **Create detailed issue report** with:
   - Steps to reproduce
   - Expected vs actual behavior
   - Debug output
   - Photos of wiring if relevant

4. **Test with examples first**:
   ```bash
   cd /home/ubuntu/SCServo_Linux
   mkdir -p build && cd build
   cmake .. && make
   ./examples/SMS_STS/Ping/Ping /dev/ttyUSB0
   ```
