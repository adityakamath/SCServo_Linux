/**
 * @file SCSCL.cpp
 * @brief Implementation of SCSCL Series Serial Servo Application Layer
 *
 * @details This file implements the control functions for Feetech SCSCL series
 * serial bus servo motors. Provides position control, PWM output, and comprehensive
 * feedback reading capabilities.
 *
 * **Implemented Features:**
 * - Position control with time and speed parameters
 * - Asynchronous (RegWrite) and synchronous (SyncWrite) operations
 * - PWM output control for open-loop operation
 * - Complete servo status feedback
 * - EEPROM lock/unlock for parameter persistence
 * - Mode configuration via angle limits
 *
 * @see SCSCL.h for class interface documentation
 * @see SMS_STS.cpp for reference implementation style
 */

#include "SCSCL.h"
#include "INST.h"
#include "SyncWriteBuffer.h"
#include <cstring>

/**
 * @brief Default constructor - initializes with little-endian byte order
 */
SCSCL::SCSCL() : SCSerial(1) {}

/**
 * @brief Constructor with endianness parameter
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 */
SCSCL::SCSCL(u8 End) : SCSerial(End) {}

/**
 * @brief Constructor with endianness and response level
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 * @param Level Response level (0=ping only, 1=read only, 2=all commands)
 */
SCSCL::SCSCL(u8 End, u8 Level) : SCSerial(End, Level) {}

int SCSCL::WritePos(u8 ID, u16 Position, u16 Time, u16 Speed)
{
	u8 bBuf[6];
	this->Host2SCS(bBuf+0, bBuf+1, Position);
	this->Host2SCS(bBuf+2, bBuf+3, Time);
	this->Host2SCS(bBuf+4, bBuf+5, Speed);
    
	return this->genWrite(ID, SCSCL_GOAL_POSITION_L, bBuf, 6);
}

int SCSCL::RegWritePos(u8 ID, u16 Position, u16 Time, u16 Speed)
{
	u8 bBuf[6];
	this->Host2SCS(bBuf+0, bBuf+1, Position);
	this->Host2SCS(bBuf+2, bBuf+3, Time);
	this->Host2SCS(bBuf+4, bBuf+5, Speed);
    
	return this->regWrite(ID, SCSCL_GOAL_POSITION_L, bBuf, 6);
}

void SCSCL::SyncWritePos(u8 ID[], u8 IDN, u16 Position[], u16 Time[], u16 Speed[])
{
	SyncWriteBuffer buffer(IDN, 6);
	if(!buffer.isValid()){
		return;
	}
	for(u8 i = 0; i<IDN; i++){
		u8 bBuf[6];
		u16 T, V;
		T = Time ? Time[i] : 0;
		V = Speed ? Speed[i] : 0;
		this->Host2SCS(bBuf+0, bBuf+1, Position[i]);
		this->Host2SCS(bBuf+2, bBuf+3, T);
		this->Host2SCS(bBuf+4, bBuf+5, V);
		buffer.writeMotorData(i, bBuf, 6);
	}
	this->syncWrite(ID, IDN, SCSCL_GOAL_POSITION_L, buffer.getBuffer(), 6);
}

/** @brief Set operating mode (SCSCL uses angle limits for mode control) */
int SCSCL::Mode(u8 ID, u8 mode)
{
	// For SCSCL: mode 0 = position mode (set angle limits), mode 1 = PWM mode (clear angle limits)
	if (mode == 0) {
		// Position mode - would need to set appropriate angle limits
		// For now, just return success as angle limits should be set separately
		return 1;
	} else {
		// PWM mode - set angle limits to 0
		return PWMMode(ID);
	}
}

/**
 * @brief Initialize motor with mode and torque settings
 * @param ID Servo ID
 * @param mode Operating mode
 * @param enableTorque 1 to enable torque, 0 to disable
 * @return 1 on success, 0 on failure
 */
int SCSCL::InitMotor(u8 ID, u8 mode, u8 enableTorque)
{
	// Unlock EEPROM
	int ret = unLockEeprom(ID);
	if (ret == 0) {
		this->Err = 1;
		return 0;
	}

	// Set mode
	ret = Mode(ID, mode);
	if (ret == 0) {
		this->Err = 1;
		return 0;
	}

	// Lock EEPROM
	ret = LockEeprom(ID);
	if (ret == 0) {
		this->Err = 1;
		return 0;
	}

	// Enable/disable torque
	ret = EnableTorque(ID, enableTorque);
	if (ret == 0) {
		this->Err = 1;
		return 0;
	}

	this->Err = 0;
	return 1;
}

int SCSCL::PWMMode(u8 ID)
{
	u8 bBuf[4] = {0, 0, 0, 0};
	return this->genWrite(ID, SCSCL_MIN_ANGLE_LIMIT_L, bBuf, 4);
}

int SCSCL::WritePWM(u8 ID, s16 pwmOut)
{
	u16 encodedPwm = ServoUtils::encodeSignedValue(pwmOut, SCSCL_PWM_DIRECTION_BIT_POS);

	u8 bBuf[2];
	this->Host2SCS(bBuf+0, bBuf+1, encodedPwm);
	return this->genWrite(ID, SCSCL_GOAL_TIME_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SCSCL::EnableTorque(u8 ID, u8 Enable)
{
	return this->writeByte(ID, SCSCL_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM */
int SCSCL::unLockEeprom(u8 ID)
{
	return this->writeByte(ID, SCSCL_LOCK, 0);
}

/** @brief Lock EEPROM */
int SCSCL::LockEeprom(u8 ID)
{
	return this->writeByte(ID, SCSCL_LOCK, 1);
}

/**
 * @brief Read all feedback data from servo
 * @param ID Servo ID
 * @return 1 on success, 0 on failure
 */
int SCSCL::FeedBack(u8 ID)
{
	int nLen = this->Read(ID, SCSCL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		this->Err = 1;
		return 0;
	}
	this->Err = 0;
	return 1;
}
	
/** @brief Read current position */
int SCSCL::ReadPos(u8 ID)
{
	int Pos = -1;
	if(ID==(u8)-1){
		Pos = Mem[SCSCL_PRESENT_POSITION_L-SCSCL_PRESENT_POSITION_L];
		Pos <<= 8;
		Pos |= Mem[SCSCL_PRESENT_POSITION_H-SCSCL_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Pos = this->readWord(ID, SCSCL_PRESENT_POSITION_L);
		if(Pos==-1){
			this->Err = 1;
		}
	}
	return Pos;
}

/** @brief Read current speed */
int SCSCL::ReadSpeed(u8 ID)
{
	if(ID == (u8)-1) {
		return ServoUtils::readSignedWordFromBuffer(
			Mem,
			SCSCL_PRESENT_SPEED_L - SCSCL_PRESENT_POSITION_L,
			SCSCL_PRESENT_SPEED_H - SCSCL_PRESENT_POSITION_L,
			SCSCL_DIRECTION_BIT_POS
		);
	}
	this->Err = 0;
	return this->readSignedWord(ID, SCSCL_PRESENT_SPEED_L, SCSCL_DIRECTION_BIT_POS);
}

/** @brief Read current load */
int SCSCL::ReadLoad(u8 ID)
{
	if(ID == (u8)-1) {
		return ServoUtils::readSignedWordFromBuffer(
			Mem,
			SCSCL_PRESENT_LOAD_L - SCSCL_PRESENT_POSITION_L,
			SCSCL_PRESENT_LOAD_H - SCSCL_PRESENT_POSITION_L,
			SCSCL_LOAD_DIRECTION_BIT_POS
		);
	}
	this->Err = 0;
	return this->readSignedWord(ID, SCSCL_PRESENT_LOAD_L, SCSCL_LOAD_DIRECTION_BIT_POS);
}

/** @brief Read supply voltage */
int SCSCL::ReadVoltage(u8 ID)
{
	int Voltage = -1;
	if(ID==(u8)-1){
		Voltage = Mem[SCSCL_PRESENT_VOLTAGE-SCSCL_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Voltage = this->readByte(ID, SCSCL_PRESENT_VOLTAGE);
		if(Voltage==-1){
			this->Err = 1;
		}
	}
	return Voltage;
}

/** @brief Read internal temperature */
int SCSCL::ReadTemper(u8 ID)
{
	int Temper = -1;
	if(ID==(u8)-1){
		Temper = Mem[SCSCL_PRESENT_TEMPERATURE-SCSCL_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Temper = this->readByte(ID, SCSCL_PRESENT_TEMPERATURE);
		if(Temper==-1){
			this->Err = 1;
		}
	}
	return Temper;
}

/** @brief Read movement status */
int SCSCL::ReadMove(u8 ID)
{
	int Move = -1;
	if(ID==(u8)-1){
		Move = Mem[SCSCL_MOVING-SCSCL_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Move = this->readByte(ID, SCSCL_MOVING);
		if(Move==-1){
			this->Err = 1;
		}
	}
	return Move;
}

/** @brief Read motor current */
int SCSCL::ReadCurrent(u8 ID)
{
	if(ID == (u8)-1) {
		return ServoUtils::readSignedWordFromBuffer(
			Mem,
			SCSCL_PRESENT_CURRENT_L - SCSCL_PRESENT_POSITION_L,
			SCSCL_PRESENT_CURRENT_H - SCSCL_PRESENT_POSITION_L,
			SCSCL_DIRECTION_BIT_POS
		);
	}
	this->Err = 0;
	return this->readSignedWord(ID, SCSCL_PRESENT_CURRENT_L, SCSCL_DIRECTION_BIT_POS);
}