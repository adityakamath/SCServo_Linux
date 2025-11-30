/*
 * SMSCL.cpp
 * Feetech SMSCL Series Serial Servo Application Layer Program
 * Date: 2020.6.17
 * Author: 
 */

#include "SMSCL.h"
#include "INST.h"

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

/** @brief Write position, speed, and acceleration */
int SMSCL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return genWrite(ID, SMSCL_ACC, bBuf, 7);
}

/** @brief Register write position command */
int SMSCL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, 0);
	Host2SCS(bBuf+5, bBuf+6, Speed);
	
	return regWrite(ID, SMSCL_ACC, bBuf, 7);
}

/** @brief Synchronized position write for multiple servos */
void SMSCL::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[])
{
    // Use dynamic allocation instead of VLA for C++ compliance
    u8 *offbuf = new u8[IDN * 7];
    if(!offbuf){
        return;  // Allocation failed
    }

    for(u8 i = 0; i<IDN; i++){
		if(Position[i]<0){
			Position[i] = -Position[i];
			Position[i] |= (1<<15);
		}
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
        Host2SCS(bBuf+1, bBuf+2, Position[i]);
        Host2SCS(bBuf+3, bBuf+4, 0);
        Host2SCS(bBuf+5, bBuf+6, V);
        memcpy(offbuf + (i * 7), bBuf, 7);
    }
    syncWrite(ID, IDN, SMSCL_ACC, offbuf, 7);
    delete[] offbuf;
}

/** @brief Enable wheel mode */
int SMSCL::WheelMode(u8 ID)
{
	return writeByte(ID, SMSCL_MODE, 1);		
}

/** @brief Write speed for wheel mode */
int SMSCL::WriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<15);
	}
	u8 bBuf[2];
	bBuf[0] = ACC;
	int ret = genWrite(ID, SMSCL_ACC, bBuf, 1);
	if(ret != 1){
		Err = 1;
		return -1;
	}
	Host2SCS(bBuf+0, bBuf+1, Speed);

	return genWrite(ID, SMSCL_GOAL_SPEED_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SMSCL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, SMSCL_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM */
int SMSCL::unLockEprom(u8 ID)
{
	return writeByte(ID, SMSCL_LOCK, 0);
}

/** @brief Lock EEPROM */
int SMSCL::LockEprom(u8 ID)
{
	return writeByte(ID, SMSCL_LOCK, 1);
}

/** @brief Calibrate center position */
int SMSCL::CalibrationOfs(u8 ID)
{
	return writeByte(ID, SMSCL_TORQUE_ENABLE, 128);
}

/** @brief Read all feedback data into memory buffer */
int SMSCL::FeedBack(int ID)
{
	int nLen = Read(ID, SMSCL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return -1;
	}
	Err = 0;
	return nLen;
}

/** @brief Read current position */
int SMSCL::ReadPos(int ID)
{
		if(ID == -1) {
			int Pos = Mem[SMSCL_PRESENT_POSITION_H-SMSCL_PRESENT_POSITION_L];
			Pos <<= 8;
			Pos |= Mem[SMSCL_PRESENT_POSITION_L-SMSCL_PRESENT_POSITION_L];
			if(Pos & (1 << 15)) {
				Pos = -(Pos & ~(1 << 15));
			}
			return Pos;
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_POSITION_L, 15);
}

/** @brief Read current speed */
int SMSCL::ReadSpeed(int ID)
{
		if(ID == -1) {
			int Speed = Mem[SMSCL_PRESENT_SPEED_H-SMSCL_PRESENT_POSITION_L];
			Speed <<= 8;
			Speed |= Mem[SMSCL_PRESENT_SPEED_L-SMSCL_PRESENT_POSITION_L];
			if(Speed & (1 << 15)) {
				Speed = -(Speed & ~(1 << 15));
			}
			return Speed;
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_SPEED_L, 15);
}

/** @brief Read current load */
int SMSCL::ReadLoad(int ID)
{	
		if(ID == -1) {
			int Load = Mem[SMSCL_PRESENT_LOAD_H-SMSCL_PRESENT_POSITION_L];
			Load <<= 8;
			Load |= Mem[SMSCL_PRESENT_LOAD_L-SMSCL_PRESENT_POSITION_L];
			if(Load & (1 << 10)) {
				Load = -(Load & ~(1 << 10));
			}
			return Load;
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_LOAD_L, 10);
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
			int Current = Mem[SMSCL_PRESENT_CURRENT_H-SMSCL_PRESENT_POSITION_L];
			Current <<= 8;
			Current |= Mem[SMSCL_PRESENT_CURRENT_L-SMSCL_PRESENT_POSITION_L];
			if(Current & (1 << 15)) {
				Current = -(Current & ~(1 << 15));
			}
			return Current;
		}
		Err = 0;
		return readSignedWord(ID, SMSCL_PRESENT_CURRENT_L, 15);
}

