/*
 * SCSCL.cpp
 * Feetech SCSCL Series Serial Servo Application Layer Program
 * Date: 2020.6.17
 * Author: 
 */


#include "SCSCL.h"
#include "INST.h"
#include <cstring>

/** @brief Default constructor for SCSCL servo controller */
SCSCL::SCSCL() : SCSerial(1) {}
SCSCL::SCSCL(u8 End) : SCSerial(End) {}
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
	u8 *offbuf = new u8[IDN * 6];
	if(!offbuf){
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
		memcpy(offbuf + (i * 6), bBuf, 6);
	}
	this->syncWrite(ID, IDN, SCSCL_GOAL_POSITION_L, offbuf, 6);
	delete[] offbuf;
}

int SCSCL::PWMMode(u8 ID)
{
	u8 bBuf[4] = {0, 0, 0, 0};
	return this->genWrite(ID, SCSCL_MIN_ANGLE_LIMIT_L, bBuf, 4);
}

int SCSCL::WritePWM(u8 ID, s16 pwmOut)
{
	if(pwmOut<0){
		pwmOut = -pwmOut;
		pwmOut |= (1<<10);
	}
	u8 bBuf[2];
	this->Host2SCS(bBuf+0, bBuf+1, pwmOut);
	return this->genWrite(ID, SCSCL_GOAL_TIME_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SCSCL::EnableTorque(u8 ID, u8 Enable)
{
	return this->writeByte(ID, SCSCL_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM */
int SCSCL::unLockEprom(u8 ID)
{
	return this->writeByte(ID, SCSCL_LOCK, 0);
}

/** @brief Lock EEPROM */
int SCSCL::LockEprom(u8 ID)
{
	return this->writeByte(ID, SCSCL_LOCK, 1);
}

/** @brief Read all feedback data into memory buffer */
int SCSCL::FeedBack(u8 ID)
{
	int nLen = this->Read(ID, SCSCL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		this->Err = 1;
		return -1;
	}
	this->Err = 0;
	return nLen;
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
		int Speed = Mem[SCSCL_PRESENT_SPEED_L-SCSCL_PRESENT_POSITION_L];
		Speed <<= 8;
		Speed |= Mem[SCSCL_PRESENT_SPEED_H-SCSCL_PRESENT_POSITION_L];
		if(Speed & (1 << 15)) {
			Speed = -(Speed & ~(1 << 15));
		}
		return Speed;
	}
	this->Err = 0;
	return this->readSignedWord(ID, SCSCL_PRESENT_SPEED_L, 15);
}

/** @brief Read current load */
int SCSCL::ReadLoad(u8 ID)
{
	if(ID == (u8)-1) {
		int Load = Mem[SCSCL_PRESENT_LOAD_L-SCSCL_PRESENT_POSITION_L];
		Load <<= 8;
		Load |= Mem[SCSCL_PRESENT_LOAD_H-SCSCL_PRESENT_POSITION_L];
		if(Load & (1 << 10)) {
			Load = -(Load & ~(1 << 10));
		}
		return Load;
	}
	this->Err = 0;
	return this->readSignedWord(ID, SCSCL_PRESENT_LOAD_L, 10);
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
		int Current = Mem[SCSCL_PRESENT_CURRENT_L-SCSCL_PRESENT_POSITION_L];
		Current <<= 8;
		Current |= Mem[SCSCL_PRESENT_CURRENT_H-SCSCL_PRESENT_POSITION_L];
		if(Current & (1 << 15)) {
			Current = -(Current & ~(1 << 15));
		}
		return Current;
	}
	this->Err = 0;
	return this->readSignedWord(ID, SCSCL_PRESENT_CURRENT_L, 15);
}