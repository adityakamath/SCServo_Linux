/**
 * @file SMSCL.cpp
 * @brief Feetech SMSCL series serial servo application layer implementation
 *
 * @details This file implements high-level control functions for Feetech SMSCL
 * series servo motors. It provides position and velocity control modes with
 * LSP-compliant initialization.
 *
 * **Implemented Features:**
 * - Mode 0: Position control with time and speed parameters
 * - Mode 1: Velocity control (wheel mode)
 * - Synchronized writes for multi-motor coordination
 * - Asynchronous writes with RegWriteAction
 * - Comprehensive feedback reading (position, speed, load, voltage, temp, current)
 * - EEPROM configuration management
 *
 * **Refactoring Improvements:**
 * - Uses ServoUtils for direction bit encoding/decoding (DRY principle)
 * - Uses SyncWriteBuffer for automatic memory management (RAII)
 * - LSP compliance with uniform InitMotor() and Mode() methods
 *
 * @note All sync write operations use RAII-based buffer management
 * @see SMSCL.h for class interface and usage examples
 */

#include "SMSCL.h"
#include "INST.h"
#include "SyncWriteBuffer.h"

/** @brief Default constructor for SMSCL servo controller */
SMSCL::SMSCL()
{
	End = 0;
}

SMSCL::SMSCL(u8 End):SCSerial(End)
{
}

/** @brief Constructor with End byte and response level */
SMSCL::SMSCL(u8 End, u8 Level):SCSerial(End, Level)
{
}

/**
 * @brief Write position, speed, and acceleration to servo
 * @param ID Servo ID
 * @param Position Target position (0-1023 for 10-bit servos)
 * @param Speed Moving speed (0-3400 steps/s, 0 = max speed)
 * @param ACC Acceleration value (0-254, units of 100 steps/s²)
 * @return 1 on success, 0 on failure
 */
int SMSCL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	u16 encodedPosition = ServoUtils::encodeSignedValue(Position, SMSCL_DIRECTION_BIT_POS);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, encodedPosition);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return genWrite(ID, SMSCL_ACC, bBuf, 7);
}

/**
 * @brief Register write position command (execute with RegWriteAction)
 * @param ID Servo ID
 * @param Position Target position (0-1023 for 10-bit servos)
 * @param Speed Moving speed (0-3400 steps/s)
 * @param ACC Acceleration value (0-254)
 * @return 1 on success, 0 on failure
 */
int SMSCL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	u16 encodedPosition = ServoUtils::encodeSignedValue(Position, SMSCL_DIRECTION_BIT_POS);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, encodedPosition);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return regWrite(ID, SMSCL_ACC, bBuf, 7);
}

/**
 * @brief Synchronized position write for multiple servos
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Position Array of target positions
 * @param Speed Array of speeds (NULL for 0 speed)
 * @param ACC Array of accelerations (NULL for 0 acceleration)
 */
void SMSCL::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[])
{
    SyncWriteBuffer buffer(IDN, 7);
    if(!buffer.isValid()){
        return;  // Allocation failed
    }

    for(u8 i = 0; i<IDN; i++){
		u16 encodedPosition = ServoUtils::encodeSignedValue(Position[i], SMSCL_DIRECTION_BIT_POS);

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
    syncWrite(ID, IDN, SMSCL_ACC, buffer.getBuffer(), 7);
}

/** @brief Set operating mode */
int SMSCL::Mode(u8 ID, u8 mode)
{
	return writeByte(ID, SMSCL_MODE, mode);
}

/**
 * @brief Initialize motor with mode and torque settings
 * @param ID Servo ID
 * @param mode Operating mode
 * @param enableTorque 1 to enable torque, 0 to disable
 * @return 1 on success, 0 on failure
 */
int SMSCL::InitMotor(u8 ID, u8 mode, u8 enableTorque)
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

/** @brief Enable wheel mode */
int SMSCL::WheelMode(u8 ID)
{
	return Mode(ID, 1);
}

/** @brief Write speed for wheel mode */
int SMSCL::WriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	u16 encodedSpeed = ServoUtils::encodeSignedValue(Speed, SMSCL_DIRECTION_BIT_POS);

	u8 bBuf[2];
	bBuf[0] = ACC;
	int ret = genWrite(ID, SMSCL_ACC, bBuf, 1);
	if(ret != 1){
		Err = 1;
		return -1;
	}
	Host2SCS(bBuf+0, bBuf+1, encodedSpeed);

	return genWrite(ID, SMSCL_GOAL_SPEED_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SMSCL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, SMSCL_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM */
int SMSCL::unLockEeprom(u8 ID)
{
	return writeByte(ID, SMSCL_LOCK, 0);
}

/** @brief Lock EEPROM */
int SMSCL::LockEeprom(u8 ID)
{
	return writeByte(ID, SMSCL_LOCK, 1);
}

/** @brief Calibrate center position */
int SMSCL::CalibrationOfs(u8 ID)
{
	return writeByte(ID, SMSCL_TORQUE_ENABLE, 128);
}

/**
 * @brief Read all feedback data from servo
 * @param ID Servo ID
 * @return 1 on success, 0 on failure
 */
int SMSCL::FeedBack(int ID)
{
	int nLen = Read(ID, SMSCL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return 0;
	}
	Err = 0;
	return 1;
}

/** @brief Read current position */
int SMSCL::ReadPos(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSCL_PRESENT_POSITION_L - SMSCL_PRESENT_POSITION_L,
				SMSCL_PRESENT_POSITION_H - SMSCL_PRESENT_POSITION_L,
				SMSCL_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_POSITION_L, SMSCL_DIRECTION_BIT_POS);
}

/** @brief Read current speed */
int SMSCL::ReadSpeed(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSCL_PRESENT_SPEED_L - SMSCL_PRESENT_POSITION_L,
				SMSCL_PRESENT_SPEED_H - SMSCL_PRESENT_POSITION_L,
				SMSCL_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_SPEED_L, SMSCL_DIRECTION_BIT_POS);
}

/** @brief Read current load */
int SMSCL::ReadLoad(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSCL_PRESENT_LOAD_L - SMSCL_PRESENT_POSITION_L,
				SMSCL_PRESENT_LOAD_H - SMSCL_PRESENT_POSITION_L,
				SMSCL_LOAD_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_LOAD_L, SMSCL_LOAD_DIRECTION_BIT_POS);
}

/** @brief Read supply voltage */
int SMSCL::ReadVoltage(int ID)
{	
	int Voltage = -1;
	if(ID==-1){
		Voltage = Mem[SMSCL_PRESENT_VOLTAGE-SMSCL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Voltage = readByte(ID, SMSCL_PRESENT_VOLTAGE);
		if(Voltage==-1){
			Err = 1;
		}
	}
	return Voltage;
}

/** @brief Read internal temperature */
int SMSCL::ReadTemper(int ID)
{	
	int Temper = -1;
	if(ID==-1){
		Temper = Mem[SMSCL_PRESENT_TEMPERATURE-SMSCL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Temper = readByte(ID, SMSCL_PRESENT_TEMPERATURE);
		if(Temper==-1){
			Err = 1;
		}
	}
	return Temper;
}

/** @brief Read movement status */
int SMSCL::ReadMove(int ID)
{
	int Move = -1;
	if(ID==-1){
		Move = Mem[SMSCL_MOVING-SMSCL_PRESENT_POSITION_L];	
	}else{
		Err = 0;
		Move = readByte(ID, SMSCL_MOVING);
		if(Move==-1){
			Err = 1;
		}
	}
	return Move;
}

/** @brief Read motor current */
int SMSCL::ReadCurrent(int ID)
{
		if(ID == -1) {
			return ServoUtils::readSignedWordFromBuffer(
				Mem,
				SMSCL_PRESENT_CURRENT_L - SMSCL_PRESENT_POSITION_L,
				SMSCL_PRESENT_CURRENT_H - SMSCL_PRESENT_POSITION_L,
				SMSCL_DIRECTION_BIT_POS
			);
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_CURRENT_L, SMSCL_DIRECTION_BIT_POS);
}

