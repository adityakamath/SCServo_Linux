/*
 * SMS_STS.cpp
 * Feetech SMS/STS Series Serial Servo Application Layer Program
 * Date: 2021.12.8
 * Author: 
 */

#include "INST.h"
#include "SMS_STS.h"

/**
 * @brief Default constructor
 * 
 * Initializes SMS/STS servo controller with default End byte (0).
 */
SMS_STS::SMS_STS()
{
	End = 0;
}

/**
 * @brief Constructor with custom End byte
 * 
 * @param End Protocol end byte (0 or 1)
 */
SMS_STS::SMS_STS(u8 End):SCSerial(End)
{
}

/**
 * @brief Constructor with End byte and response level
 * 
 * @param End Protocol end byte (0 or 1)
 * @param Level Response level (0=no response, 1=response for read/write commands)
 */
SMS_STS::SMS_STS(u8 End, u8 Level):SCSerial(End, Level)
{
}

/**
 * @brief Write position, speed, and acceleration to servo (extended command)
 * 
 * Sends single-servo position command with acceleration and speed control.
 * Negative positions are converted to absolute value with direction bit set.
 * 
 * @param ID Servo ID
 * @param Position Target position (-4095 to 4095, negative = CCW)
 * @param Speed Moving speed (0-65535, 0 = max speed)
 * @param ACC Acceleration value (0-254)
 * @return Number of bytes written on success, -1 on error
 */
int SMS_STS::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<SMS_STS_DIRECTION_BIT_POS);  // Set direction bit
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return genWrite(ID, SMS_STS_ACC, bBuf, 7);
}

/**
 * @brief Register write position command (executes on RegWriteAction)
 * 
 * Queues position/speed/acceleration command for later execution.
 * Use with RegWriteAction for synchronized multi-servo motion.
 * 
 * @param ID Servo ID
 * @param Position Target position (-4095 to 4095)
 * @param Speed Moving speed (0-65535)
 * @param ACC Acceleration value (0-254)
 * @return Number of bytes written on success, -1 on error
 */
int SMS_STS::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<SMS_STS_DIRECTION_BIT_POS);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return regWrite(ID, SMS_STS_ACC, bBuf, 7);
}

/**
 * @brief Synchronized position write for multiple servos
 * 
 * Sends position/speed/acceleration commands to multiple servos simultaneously.
 * All servos receive commands in single transmission for coordinated motion.
 * 
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Position Array of target positions
 * @param Speed Array of speeds (NULL for 0 speed)
 * @param ACC Array of accelerations (NULL for 0 acceleration)
 */
void SMS_STS::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[])
{
    // Use dynamic allocation instead of VLA for C++ compliance
	u8 *offbuf = new u8[IDN * 7];
	if(!offbuf){
		return;  // Allocation failed
	}
	for(u8 i = 0; i<IDN; i++){
		if(Position[i]<0){
			Position[i] = -Position[i];
			Position[i] |= (1<<SMS_STS_DIRECTION_BIT_POS);
		}
		u8 bBuf[7];
		u16 V = Speed ? Speed[i] : 0;
		bBuf[0] = ACC ? ACC[i] : 0;
		Host2SCS(bBuf+1, bBuf+2, Position[i]);
		Host2SCS(bBuf+3, bBuf+4, 0);
		Host2SCS(bBuf+5, bBuf+6, V);
		memcpy(offbuf + (i * 7), bBuf, 7);
	}
	syncWrite(ID, IDN, SMS_STS_ACC, offbuf, 7);
	delete[] offbuf;
}

/**
 * @brief Set servo operating mode
 * 
 * Configures servo for different operation modes:
 * - Mode 0: Servo position mode (default)
 * - Mode 1: Wheel closed-loop speed mode
 * - Mode 2: Wheel open-loop power mode
 * 
 * @param ID Servo ID
 * @param mode Operating mode (0-2)
 * @return 1 on success, -1 on invalid mode
 */
int SMS_STS::Mode(u8 ID, u8 mode)
{
	// Modes: 0 (servo), 1 (wheel closed-loop), 2 (wheel open-loop), 3 (stepper - not implemented)
	if(!(mode == SMS_STS_MODE_SERVO || mode == SMS_STS_MODE_WHEEL_CLOSED || mode == SMS_STS_MODE_WHEEL_OPEN)){
		Err = 1;
		return -1;
	}
	Err = 0;
	return writeByte(ID, SMS_STS_MODE, mode);
}

/**
 * @brief Write speed command for wheel mode
 * 
 * Controls servo speed in wheel mode. Negative speeds reverse direction.
 * 
 * @param ID Servo ID
 * @param Speed Target speed (-32767 to 32767, negative = reverse)
 * @param ACC Acceleration value (0-254)
 * @return Number of bytes written on success, -1 on error
 */
int SMS_STS::WriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<SMS_STS_DIRECTION_BIT_POS);
	}
	u8 bBuf[2];
	bBuf[0] = ACC;
	int ret = genWrite(ID, SMS_STS_ACC, bBuf, 1);
	if(ret != 1){
		Err = 1;
		return -1;
	}
	Host2SCS(bBuf+0, bBuf+1, Speed);

	return genWrite(ID, SMS_STS_GOAL_SPEED_L, bBuf, 2);
}

/**
 * @brief Register write speed command (executes on RegWriteAction)
 * 
 * Queues speed command for later synchronized execution.
 * 
 * @param ID Servo ID
 * @param Speed Target speed (-32767 to 32767)
 * @param ACC Acceleration value (0-254)
 * @return Number of bytes written on success, -1 on error
 */
int SMS_STS::RegWriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<SMS_STS_DIRECTION_BIT_POS);
	}
	u8 bBuf[2];
	bBuf[0] = ACC;
	int ret = regWrite(ID, SMS_STS_ACC, bBuf, 1);
	if(ret != 1){
		Err = 1;
		return -1;
	}
	Host2SCS(bBuf+0, bBuf+1, Speed);

	return regWrite(ID, SMS_STS_GOAL_SPEED_L, bBuf, 2);
}

/**
 * @brief Synchronized speed write for multiple servos
 * 
 * Sends speed commands to multiple servos in wheel mode simultaneously.
 * 
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Speed Array of target speeds
 * @param ACC Array of accelerations (NULL for 0 acceleration)
 */
void SMS_STS::SyncWriteSpe(u8 ID[], u8 IDN, s16 Speed[], u8 ACC[])
{
    // Use dynamic allocation instead of VLA for C++ compliance
	u8 *offbuf = new u8[IDN * 2];
	if(!offbuf){
		return;  // Allocation failed
	}
	for(u8 i = 0; i<IDN; i++){
		if(Speed[i]<0){
			Speed[i] = -Speed[i];
			Speed[i] |= (1<<SMS_STS_DIRECTION_BIT_POS);
		}
		u8 bBuf[2];
		bBuf[0] = ACC ? ACC[i] : 0;
		genWrite(ID[i], SMS_STS_ACC, bBuf, 1);
		Host2SCS(bBuf+0, bBuf+1, Speed[i]);
		memcpy(offbuf + (i * 2), bBuf, 2);
	}
	syncWrite(ID, IDN, SMS_STS_GOAL_SPEED_L, offbuf, 2);
	delete[] offbuf;
}

/**
 * @brief Write PWM output for open-loop mode
 * 
 * Directly controls motor PWM in open-loop wheel mode.
 * Negative values reverse direction.
 * 
 * @param ID Servo ID
 * @param Pwm PWM value (-1000 to 1000, negative = reverse)
 * @return Number of bytes written on success, -1 on error
 */
int SMS_STS::WritePwm(u8 ID, s16 Pwm)
{
    if(Pwm<0){
        Pwm = -Pwm;
        Pwm |= (1<<SMS_STS_LOAD_DIRECTION_BIT_POS);
    }
    u8 bBuf[2];
    Host2SCS(bBuf+0, bBuf+1, Pwm);
    
    return genWrite(ID, SMS_STS_GOAL_TIME_L, bBuf, 2);
}

/**
 * @brief Register write PWM command (executes on RegWriteAction)
 * 
 * Queues PWM command for synchronized execution.
 * 
 * @param ID Servo ID
 * @param Pwm PWM value (-1000 to 1000)
 * @return Number of bytes written on success, -1 on error
 */
int SMS_STS::RegWritePwm(u8 ID, s16 Pwm)
{
    if(Pwm<0){
        Pwm = -Pwm;
        Pwm |= (1<<SMS_STS_LOAD_DIRECTION_BIT_POS);
    }
    u8 bBuf[2];
    Host2SCS(bBuf+0, bBuf+1, Pwm);
    
    return regWrite(ID, SMS_STS_GOAL_TIME_L, bBuf, 2);
}

/**
 * @brief Synchronized PWM write for multiple servos
 * 
 * Sends PWM commands to multiple servos simultaneously.
 * 
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Pwm Array of PWM values
 */
void SMS_STS::SyncWritePwm(u8 ID[], u8 IDN, s16 Pwm[])
{
    // Use dynamic allocation instead of VLA for C++ compliance
	u8 *offbuf = new u8[IDN * 2];
	if(!offbuf){
		return;  // Allocation failed
	}
	for(u8 i = 0; i<IDN; i++){
		if(Pwm[i]<0){
			Pwm[i] = -Pwm[i];
			Pwm[i] |= (1<<SMS_STS_LOAD_DIRECTION_BIT_POS);
		}
		u8 bBuf[2];
		Host2SCS(bBuf+0, bBuf+1, Pwm[i]);
		memcpy(offbuf + (i * 2), bBuf, 2);
	}
	syncWrite(ID, IDN, SMS_STS_GOAL_TIME_L, offbuf, 2);
	delete[] offbuf;
}

/**
 * @brief Enable or disable servo motor torque
 * 
 * Controls whether servo actively maintains position or is free-moving.
 * 
 * @param ID Servo ID
 * @param Enable 1 to enable torque, 0 to disable (free-moving)
 * @return 1 on success, -1 on error
 */
int SMS_STS::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, SMS_STS_TORQUE_ENABLE, Enable);
}

/**
 * @brief Unlock EEPROM for writing
 * 
 * Allows modification of EEPROM parameters (ID, baud rate, limits, etc.).
 * 
 * @param ID Servo ID
 * @return 1 on success, -1 on error
 */
int SMS_STS::unLockEprom(u8 ID)
{
	return writeByte(ID, SMS_STS_LOCK, 0);
}

/**
 * @brief Lock EEPROM to prevent accidental changes
 * 
 * Protects EEPROM parameters from modification.
 * 
 * @param ID Servo ID
 * @return 1 on success, -1 on error
 */
int SMS_STS::LockEprom(u8 ID)
{
	return writeByte(ID, SMS_STS_LOCK, 1);
}

/**
 * @brief Calibrate servo center position offset
 * 
 * Initiates automatic calibration of servo zero position.
 * Servo must be manually positioned at desired center before calling.
 * 
 * @param ID Servo ID
 * @return 1 on success, -1 on error
 */
int SMS_STS::CalibrationOfs(u8 ID)
{
	return writeByte(ID, SMS_STS_TORQUE_ENABLE, SMS_STS_CALIBRATION_CMD);
}

/**
 * @brief Read all feedback data from servo into memory buffer
 * 
 * Reads position, speed, load, voltage, temperature, movement status, and current.
 * Data is stored in Mem[] array and can be accessed via ReadPos(), ReadSpeed(), etc.
 * 
 * @param ID Servo ID
 * @return Number of bytes read on success, -1 on error
 */
int SMS_STS::FeedBack(int ID)
{
	int nLen = Read(ID, SMS_STS_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return -1;
	}
	Err = 0;
	return nLen;
}

/**
 * @brief Read current servo position
 * 
 * Reads position from servo or from cached Mem[] buffer.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer (after FeedBack())
 * @return Position value (-4095 to 4095), -1 on error
 */
int SMS_STS::ReadPos(int ID)
{
		if(ID == -1) {
			int Pos = Mem[SMS_STS_PRESENT_POSITION_H-SMS_STS_PRESENT_POSITION_L];
			Pos <<= 8;
			Pos |= Mem[SMS_STS_PRESENT_POSITION_L-SMS_STS_PRESENT_POSITION_L];
			if(Pos & (1 << SMS_STS_DIRECTION_BIT_POS)) {
				Pos = -(Pos & ~(1 << SMS_STS_DIRECTION_BIT_POS));
			}
			return Pos;
		}
		Err = 0;
		return readSignedWord(ID, SMS_STS_PRESENT_POSITION_L, SMS_STS_DIRECTION_BIT_POS);
}

/**
 * @brief Read current servo speed
 * 
 * Reads speed from servo or from cached buffer.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer
 * @return Speed value (-32767 to 32767), -1 on error
 */
int SMS_STS::ReadSpeed(int ID)
{
		if(ID == -1) {
			int Speed = Mem[SMS_STS_PRESENT_SPEED_H-SMS_STS_PRESENT_POSITION_L];
			Speed <<= 8;
			Speed |= Mem[SMS_STS_PRESENT_SPEED_L-SMS_STS_PRESENT_POSITION_L];
			if(Speed & (1 << SMS_STS_DIRECTION_BIT_POS)) {
				Speed = -(Speed & ~(1 << SMS_STS_DIRECTION_BIT_POS));
			}
			return Speed;
		}
		Err = 0;
		return readSignedWord(ID, SMS_STS_PRESENT_SPEED_L, SMS_STS_DIRECTION_BIT_POS);
}

/**
 * @brief Read current servo load
 * 
 * Reads load torque from servo or from cached buffer.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer
 * @return Load value (-1000 to 1000), -1 on error
 */
int SMS_STS::ReadLoad(int ID)
{
		if(ID == -1) {
			int Load = Mem[SMS_STS_PRESENT_LOAD_H-SMS_STS_PRESENT_POSITION_L];
			Load <<= 8;
			Load |= Mem[SMS_STS_PRESENT_LOAD_L-SMS_STS_PRESENT_POSITION_L];
			if(Load & (1 << SMS_STS_LOAD_DIRECTION_BIT_POS)) {
				Load = -(Load & ~(1 << SMS_STS_LOAD_DIRECTION_BIT_POS));
			}
			return Load;
		}
		Err = 0;
		return readSignedWord(ID, SMS_STS_PRESENT_LOAD_L, SMS_STS_LOAD_DIRECTION_BIT_POS);
}

/**
 * @brief Read servo supply voltage
 * 
 * Reads operating voltage from servo or from cached buffer.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer
 * @return Voltage in 0.1V units (e.g., 120 = 12.0V), -1 on error
 */
int SMS_STS::ReadVoltage(int ID)
{	
	int Voltage = -1;
	if(ID==-1){
		Voltage = Mem[SMS_STS_PRESENT_VOLTAGE-SMS_STS_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Voltage = readByte(ID, SMS_STS_PRESENT_VOLTAGE);
		if(Voltage==-1){
			Err = 1;
		}
	}
	return Voltage;
}

/**
 * @brief Read servo internal temperature
 * 
 * Reads temperature from servo or from cached buffer.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer
 * @return Temperature in degrees Celsius, -1 on error
 */
int SMS_STS::ReadTemper(int ID)
{	
	int Temper = -1;
	if(ID==-1){
		Temper = Mem[SMS_STS_PRESENT_TEMPERATURE-SMS_STS_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Temper = readByte(ID, SMS_STS_PRESENT_TEMPERATURE);
		if(Temper==-1){
			Err = 1;
		}
	}
	return Temper;
}

/**
 * @brief Read servo movement status
 * 
 * Checks if servo is currently moving or has reached target position.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer
 * @return 1 if moving, 0 if stopped, -1 on error
 */
int SMS_STS::ReadMove(int ID)
{
	int Move = -1;
	if(ID==-1){
		Move = Mem[SMS_STS_MOVING-SMS_STS_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Move = readByte(ID, SMS_STS_MOVING);
		if(Move==-1){
			Err = 1;
		}
	}
	return Move;
}

/**
 * @brief Read servo motor current
 * 
 * Reads motor current from servo or from cached buffer.
 * 
 * @param ID Servo ID, or -1 to read from cached buffer
 * @return Current value in mA, -1 on error
 */
int SMS_STS::ReadCurrent(int ID)
{
		if(ID == -1) {
			int Current = Mem[SMS_STS_PRESENT_CURRENT_H-SMS_STS_PRESENT_POSITION_L];
			Current <<= 8;
			Current |= Mem[SMS_STS_PRESENT_CURRENT_L-SMS_STS_PRESENT_POSITION_L];
			if(Current & (1 << SMS_STS_DIRECTION_BIT_POS)) {
				Current = -(Current & ~(1 << SMS_STS_DIRECTION_BIT_POS));
			}
			return Current;
		}
		Err = 0;
		return readSignedWord(ID, SMS_STS_PRESENT_CURRENT_L, SMS_STS_DIRECTION_BIT_POS);
}

