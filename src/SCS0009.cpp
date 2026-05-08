/**
 * @file SCS0009.cpp
 * @brief Implementation of SCS0009 Series Serial Servo Application Layer
 *
 * @details This file implements the control functions for Feetech SCS0009 series
 *  This code is quite similar to that used to control the SCSCL series. 
 *  Most of the relevant changes can be found in SCS0009.h which has the values
 *  and register memory locations used in the SCS0009 firmware vs. the SCSCL series. 
 */ 

#include "SCS0009.h"
#include "INST.h"
#include "SyncWriteBuffer.h"
#include <cstring>
#include <iostream>

/**
 * @brief Default constructor - initializes with little-endian byte order
 */
SCS0009::SCS0009() : SCSerial(1) {}

/**
 * @brief Constructor with endianness parameter
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 */
SCS0009::SCS0009(u8 End) : SCSerial(End) {}

/**
 * @brief Constructor with endianness and response level
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 * @param Level Response level (0=ping only, 1=read only, 2=all commands)
 */
SCS0009::SCS0009(u8 End, u8 Level) : SCSerial(End, Level) {}

int SCS0009::WritePos(u8 ID, u16 Position, u16 Time, u16 Speed)
{
	u8 bBuf[6];
	this->Host2SCS(bBuf+0, bBuf+1, Position);
	this->Host2SCS(bBuf+2, bBuf+3, Time);
	this->Host2SCS(bBuf+4, bBuf+5, Speed);
    
	return this->genWrite(ID, SCS0009_GOAL_POSITION_L, bBuf, 6);
}

int SCS0009::RegWritePos(u8 ID, u16 Position, u16 Time, u16 Speed)
{
	u8 bBuf[6];
	this->Host2SCS(bBuf+0, bBuf+1, Position);
	this->Host2SCS(bBuf+2, bBuf+3, Time);
	this->Host2SCS(bBuf+4, bBuf+5, Speed);
    
	return this->regWrite(ID, SCS0009_GOAL_POSITION_L, bBuf, 6);
}

void SCS0009::SyncWritePos(u8 ID[], u8 IDN, u16 Position[], u16 Time[], u16 Speed[])
{
	SyncWriteBuffer buffer(IDN, 6);
	if(!buffer.isValid()){
            this->Err = 1;
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
	this->syncWrite(ID, IDN, SCS0009_GOAL_POSITION_L, buffer.getBuffer(), 6);
}

/** @brief Set operating mode (SCS0009 uses angle limits for mode control) */
int SCS0009::Mode(u8 ID, u8 mode)
{
	// For SCS0009: mode 0 = position mode (set angle limits), mode 1 = PWM mode (clear angle limits)
	if (mode == 0) {
                u8 bBuf[4];
                this->Host2SCS(bBuf+0,bBuf+1,0);
                this->Host2SCS(bBuf+2,bBuf+3,1023);
                return this->genWrite(ID, SCS0009_MIN_ANGLE_LIMIT_L,bBuf,4);
        }
        else if (mode == 1) { 
               return PWMMode(ID);
	} else { 
                /* defensively refuse other values for mode-setting */
		return 0; 
	}
}

/**
 * @brief Initialize motor with mode and torque settings
 * @param ID Servo ID
 * @param mode Operating mode
 * @param enableTorque 1 to enable torque, 0 to disable
 * @return 1 on success, 0 on failure
 */
int SCS0009::InitMotor(u8 ID, u8 mode, u8 enableTorque)
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
                LockEeprom(ID);
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

int SCS0009::PWMMode(u8 ID)
{
	u8 bBuf[4] = {0, 0, 0, 0};
	return this->genWrite(ID, SCS0009_MIN_ANGLE_LIMIT_L, bBuf, 4);
}

int SCS0009::WritePWM(u8 ID, s16 pwmOut)
{
	u16 encodedPwm = ServoUtils::encodeSignedValue(pwmOut, SCS0009_DIRECTION_BIT_POS);

	u8 bBuf[2];
	this->Host2SCS(bBuf+0, bBuf+1, encodedPwm);
	return this->genWrite(ID, SCS0009_GOAL_TIME_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SCS0009::EnableTorque(u8 ID, u8 Enable)
{
	return this->writeByte(ID, SCS0009_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM */
int SCS0009::unLockEeprom(u8 ID)
{
	return this->writeByte(ID, SCS0009_LOCK, 0);
}

/** @brief Lock EEPROM */
int SCS0009::LockEeprom(u8 ID)
{
	return this->writeByte(ID, SCS0009_LOCK, 1);
}

/**
 * @brief Read all feedback data from servo
 * @param ID Servo ID
 * @return 1 on success, 0 on failure
 */
int SCS0009::FeedBack(u8 ID)
{
	int nLen = this->Read(ID, SCS0009_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		this->Err = 1;
		return 0;
	}
	this->Err = 0;
	return 1;
}
	
/** @brief Read current position from cache (FeedBack must be called first) or directly from servo */
int SCS0009::ReadPos(u8 ID)
{
	int Pos = -1;
	if(ID==(u8)-1){
		Pos = Mem[SCS0009_PRESENT_POSITION_H-SCS0009_PRESENT_POSITION_L];
		Pos <<= 8;
		Pos |= Mem[SCS0009_PRESENT_POSITION_L-SCS0009_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Pos = this->readWord(ID, SCS0009_PRESENT_POSITION_L);
		if(Pos==-1){
			this->Err = 1;
		}
	}
	return Pos;
}

/** @brief Read current speed */
int SCS0009::ReadSpeed(u8 ID)
{
	if(ID == (u8)-1) {
		return ServoUtils::readSignedWordFromBuffer(
			Mem,
			SCS0009_PRESENT_SPEED_L - SCS0009_PRESENT_POSITION_L,
			SCS0009_PRESENT_SPEED_H - SCS0009_PRESENT_POSITION_L,
			SCS0009_DIRECTION_BIT_POS
		);
	}
	this->Err = 0;
        int speed = this->readSignedWord(ID, SCS0009_PRESENT_SPEED_L, SCS0009_DIRECTION_BIT_POS);
	if (speed == -1) this->Err = 1;
	return speed;
}

/** @brief Read current load */
int SCS0009::ReadLoad(u8 ID)
{
	if(ID == (u8)-1) {
		return ServoUtils::readSignedWordFromBuffer(
			Mem,
			SCS0009_PRESENT_LOAD_L - SCS0009_PRESENT_POSITION_L,
			SCS0009_PRESENT_LOAD_H - SCS0009_PRESENT_POSITION_L,
			SCS0009_DIRECTION_BIT_POS
		);
	}
	this->Err = 0;
        int load = this->readSignedWord(ID, SCS0009_PRESENT_LOAD_L, 
                                            SCS0009_DIRECTION_BIT_POS
                         );
	if (load == -1) this->Err = 1;
	return load;
}

/** @brief Read supply voltage */
int SCS0009::ReadVoltage(u8 ID)
{
	int Voltage = -1;
	if(ID==(u8)-1){
		Voltage = Mem[SCS0009_PRESENT_VOLTAGE-SCS0009_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Voltage = this->readByte(ID, SCS0009_PRESENT_VOLTAGE);
		if(Voltage==-1){
			this->Err = 1;
		}
	}
	return Voltage;
}

/** @brief Read internal temperature */
int SCS0009::ReadTemper(u8 ID)
{
	int Temper = -1;
	if(ID==(u8)-1){
		Temper = Mem[SCS0009_PRESENT_TEMPERATURE-SCS0009_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Temper = this->readByte(ID, SCS0009_PRESENT_TEMPERATURE);
		if(Temper==-1){
			this->Err = 1;
		}
	}
	return Temper;
}

/** @brief Read movement status */
int SCS0009::ReadMove(u8 ID)
{
	int Move = -1;
	if(ID==(u8)-1){
		Move = Mem[SCS0009_MOVING-SCS0009_PRESENT_POSITION_L];
	}else{
		this->Err = 0;
		Move = this->readByte(ID, SCS0009_MOVING);
		if(Move==-1){
			this->Err = 1;
		}
	}
	return Move;
}

/** @brief Read motor current */
int SCS0009::ReadCurrent(u8 ID)
{
	if(ID == (u8)-1) {
		return ServoUtils::readSignedWordFromBuffer(
			Mem,
			SCS0009_PRESENT_CURRENT_L - SCS0009_PRESENT_POSITION_L,
			SCS0009_PRESENT_CURRENT_H - SCS0009_PRESENT_POSITION_L,
			SCS0009_DIRECTION_BIT_POS
		);
	}
	this->Err = 0;
        int current = this->readSignedWord(ID, SCS0009_PRESENT_CURRENT_L,
                                           SCS0009_DIRECTION_BIT_POS
                                      );
	if (current == -1) this->Err = 1;
	return current;
}
