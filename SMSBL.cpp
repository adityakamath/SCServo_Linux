/**
 * @file SMSBL.cpp
 * @brief Implementation of SMSBL Series Serial Servo Application Layer
 *
 * @details This file implements the control functions for Feetech SMSBL series
 * serial bus servo motors. Provides position control, velocity control, and
 * acceleration-based motion planning.
 *
 * **Implemented Features:**
 * - Position control with speed and acceleration parameters
 * - Velocity control (wheel mode) with acceleration
 * - Asynchronous (RegWrite) and synchronous (SyncWrite) operations
 * - Comprehensive servo status feedback
 * - EEPROM lock/unlock for parameter persistence
 * - Mode configuration (servo/wheel modes)
 * - Midpoint calibration
 *
 * **Refactoring Improvements:**
 * - Uses ServoUtils for direction bit encoding/decoding (DRY principle)
 * - Uses SyncWriteBuffer for automatic memory management (RAII)
 * - LSP compliance with uniform InitMotor() and Mode() methods
 *
 * @see SMSBL.h for class interface documentation
 * @see SMS_STS.cpp for reference implementation style
 */

#include "INST.h"
#include "SMSBL.h"
#include "SyncWriteBuffer.h"

/**
 * @brief Default constructor - initializes with End byte set to 0
 */
SMSBL::SMSBL()
{
	End = 0;
}

/**
 * @brief Constructor with custom endianness
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 */
SMSBL::SMSBL(u8 End):SCSerial(End)
{
}

/**
 * @brief Constructor with endianness and response level
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 * @param Level Response level (0=ping only, 1=read only, 2=all commands)
 */
SMSBL::SMSBL(u8 End, u8 Level):SCSerial(End, Level)
{
}

/**
 * @brief Write position, speed, and acceleration to SMSBL servo
 * 
 * Extended position command with acceleration and speed control.
 * 
 * @param ID Servo ID
 * @param Position Target position (-32767 to 32767)
 * @param Speed Moving speed (0-3400 steps/s) (0-3400 steps/s)
 * @param ACC Acceleration value (0-254)
 * @return 1 on success, 0 on failure
 */
int SMSBL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	u16 encodedPosition = ServoUtils::encodeSignedValue(Position, SMSBL_DIRECTION_BIT_POS);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, encodedPosition);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return genWrite(ID, SMSBL_ACC, bBuf, 7);
}

/**
 * @brief Register write position command
 * @param ID Servo ID
 * @param Position Target position
 * @param Speed Moving speed (0-3400 steps/s)
 * @param ACC Acceleration
 * @return 1 on success, 0 on failure
 */
int SMSBL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	u16 encodedPosition = ServoUtils::encodeSignedValue(Position, SMSBL_DIRECTION_BIT_POS);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, encodedPosition);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return regWrite(ID, SMSBL_ACC, bBuf, 7);
}

/**
 * @brief Synchronized position write for multiple servos
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Position Array of positions
 * @param Speed Array of speeds
 * @param ACC Array of accelerations
 */
void SMSBL::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[])
{
    SyncWriteBuffer buffer(IDN, 7);
    if(!buffer.isValid()){
        return;  // Allocation failed
    }

    for(u8 i = 0; i<IDN; i++){
		u16 encodedPosition = ServoUtils::encodeSignedValue(Position[i], SMSBL_DIRECTION_BIT_POS);

        u8 bBuf[7];
		u16 V;
		if(Speed){
			V = Speed[i];
		}else{
			V = 0;
		}
		if(ACC){
			bBuf[0] = ACC[i];
		}else{
			bBuf[0] = 0;
		}
        Host2SCS(bBuf+1, bBuf+2, encodedPosition);
        Host2SCS(bBuf+3, bBuf+4, 0);
        Host2SCS(bBuf+5, bBuf+6, V);
        buffer.writeMotorData(i, bBuf, 7);
    }
    syncWrite(ID, IDN, SMSBL_ACC, buffer.getBuffer(), 7);
}

/**
 * @brief Enable wheel mode for continuous rotation
 * @param ID Servo ID
 * @return 1 on success, 0 on failure
 */
/** @brief Set operating mode */
int SMSBL::Mode(u8 ID, u8 mode)
{
	return writeByte(ID, SMSBL_MODE, mode);
}

/**
 * @brief Initialize motor with mode and torque settings
 * @param ID Servo ID
 * @param mode Operating mode
 * @param enableTorque 1 to enable torque, 0 to disable
 * @return 1 on success, 0 on failure
 */
int SMSBL::InitMotor(u8 ID, u8 mode, u8 enableTorque)
{
	// Unlock EEPROM
	int ret = unLockEeprom(ID);
	if (ret == 0) {
		Err = 1;
		return 0;
	}

	// Set mode
	ret = Mode(ID, mode);
	if (ret == 0) {
		Err = 1;
		return 0;
	}

	// Lock EEPROM
	ret = LockEeprom(ID);
	if (ret == 0) {
		Err = 1;
		return 0;
	}

	// Enable/disable torque
	ret = EnableTorque(ID, enableTorque);
	if (ret == 0) {
		Err = 1;
		return 0;
	}

	Err = 0;
	return 1;
}

int SMSBL::WheelMode(u8 ID)
{
	return Mode(ID, 1);
}

/**
 * @brief Write speed for wheel mode
 * @param ID Servo ID
 * @param Speed Target speed (-3400 to +3400 steps/s)
 * @param ACC Acceleration
 * @return 1 on success, 0 on failure
 */
int SMSBL::WriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	u16 encodedSpeed = ServoUtils::encodeSignedValue(Speed, SMSBL_DIRECTION_BIT_POS);

	u8 bBuf[2];
	bBuf[0] = ACC;
	int ret = genWrite(ID, SMSBL_ACC, bBuf, 1);
	if(ret != 1){
		Err = 1;
		return -1;
	}
	Host2SCS(bBuf+0, bBuf+1, encodedSpeed);

	return genWrite(ID, SMSBL_GOAL_SPEED_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SMSBL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, SMSBL_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM for writing */
int SMSBL::unLockEeprom(u8 ID)
{
	return writeByte(ID, SMSBL_LOCK, 0);
}

/** @brief Lock EEPROM to protect settings */
int SMSBL::LockEeprom(u8 ID)
{
	return writeByte(ID, SMSBL_LOCK, 1);
}

/** @brief Calibrate servo center position offset */
int SMSBL::CalibrationOfs(u8 ID)
{
	return writeByte(ID, SMSBL_TORQUE_ENABLE, 128);
}

/**
 * @brief Read all feedback data from servo
 * @param ID Servo ID
 * @return 1 on success, 0 on failure
 */
int SMSBL::FeedBack(int ID)
{
	int nLen = Read(ID, SMSBL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return 0;
	}
	Err = 0;
	return 1;
}

/** @brief Read current position */
int SMSBL::ReadPos(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSBL_PRESENT_POSITION_L - SMSBL_PRESENT_POSITION_L,
				SMSBL_PRESENT_POSITION_H - SMSBL_PRESENT_POSITION_L,
				SMSBL_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_POSITION_L, SMSBL_DIRECTION_BIT_POS);
}

/** @brief Read current speed */
int SMSBL::ReadSpeed(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSBL_PRESENT_SPEED_L - SMSBL_PRESENT_POSITION_L,
				SMSBL_PRESENT_SPEED_H - SMSBL_PRESENT_POSITION_L,
				SMSBL_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_SPEED_L, SMSBL_DIRECTION_BIT_POS);
}

/** @brief Read current load */
int SMSBL::ReadLoad(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSBL_PRESENT_LOAD_L - SMSBL_PRESENT_POSITION_L,
				SMSBL_PRESENT_LOAD_H - SMSBL_PRESENT_POSITION_L,
				SMSBL_LOAD_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_LOAD_L, SMSBL_LOAD_DIRECTION_BIT_POS);
}

/** @brief Read supply voltage */
int SMSBL::ReadVoltage(int ID)
{	
	int Voltage = -1;
	if(ID==-1){
		Voltage = Mem[SMSBL_PRESENT_VOLTAGE-SMSBL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Voltage = readByte(ID, SMSBL_PRESENT_VOLTAGE);
		if(Voltage==-1){
			Err = 1;
		}
	}
	return Voltage;
}

/** @brief Read internal temperature */
int SMSBL::ReadTemper(int ID)
{	
	int Temper = -1;
	if(ID==-1){
		Temper = Mem[SMSBL_PRESENT_TEMPERATURE-SMSBL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Temper = readByte(ID, SMSBL_PRESENT_TEMPERATURE);
		if(Temper==-1){
			Err = 1;
		}
	}
	return Temper;
}

/** @brief Read movement status */
int SMSBL::ReadMove(int ID)
{
	int Move = -1;
	if(ID==-1){
		Move = Mem[SMSBL_MOVING-SMSBL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Move = readByte(ID, SMSBL_MOVING);
		if(Move==-1){
			Err = 1;
		}
	}
	return Move;
}

/** @brief Read motor current */
int SMSBL::ReadCurrent(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSBL_PRESENT_CURRENT_L - SMSBL_PRESENT_POSITION_L,
				SMSBL_PRESENT_CURRENT_H - SMSBL_PRESENT_POSITION_L,
				SMSBL_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_CURRENT_L, SMSBL_DIRECTION_BIT_POS);
}

