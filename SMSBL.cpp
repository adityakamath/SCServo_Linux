/*
 * SMSBL.cpp
 * Feetech SMSBL Series Serial Servo Application Layer Program
 * Date: 2020.6.17
 * Author: 
 */

#include "INST.h"
#include "SMSBL.h"

/**
 * @brief Default constructor for SMSBL servo controller
 * 
 * Initializes SMSBL (Feetech SMSBL series) servo with default End byte (0).
 */
SMSBL::SMSBL()
{
	End = 0;
}

/**
 * @brief Constructor with custom End byte
 * 
 * @param End Protocol end byte (0 or 1)
 */
SMSBL::SMSBL(u8 End):SCSerial(End)
{
}

/**
 * @brief Constructor with End byte and response level
 * 
 * @param End Protocol end byte (0 or 1)
 * @param Level Response level (0=no response, 1=response enabled)
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
 * @param Speed Moving speed (0-65535)
 * @param ACC Acceleration value (0-254)
 * @return Number of bytes written on success, -1 on error
 */
int SMSBL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
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
	
	return genWrite(ID, SMSBL_ACC, bBuf, 7);
}

/**
 * @brief Register write position command
 * @param ID Servo ID
 * @param Position Target position
 * @param Speed Moving speed
 * @param ACC Acceleration
 * @return Bytes written, -1 on error
 */
int SMSBL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC)
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
    syncWrite(ID, IDN, SMSBL_ACC, offbuf, 7);
    delete[] offbuf;
}

/**
 * @brief Enable wheel mode for continuous rotation
 * @param ID Servo ID
 * @return 1 on success, -1 on error
 */
int SMSBL::WheelMode(u8 ID)
{
	return writeByte(ID, SMSBL_MODE, 1);		
}

/**
 * @brief Write speed for wheel mode
 * @param ID Servo ID
 * @param Speed Target speed
 * @param ACC Acceleration
 * @return Bytes written, -1 on error
 */
int SMSBL::WriteSpe(u8 ID, s16 Speed, u8 ACC)
{
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<15);
	}
	u8 bBuf[2];
	bBuf[0] = ACC;
	int ret = genWrite(ID, SMSBL_ACC, bBuf, 1);
	if(ret != 1){
		Err = 1;
		return -1;
	}
	Host2SCS(bBuf+0, bBuf+1, Speed);

	return genWrite(ID, SMSBL_GOAL_SPEED_L, bBuf, 2);
}

/** @brief Enable/disable servo torque */
int SMSBL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, SMSBL_TORQUE_ENABLE, Enable);
}

/** @brief Unlock EEPROM for writing */
int SMSBL::unLockEprom(u8 ID)
{
	return writeByte(ID, SMSBL_LOCK, 0);
}

/** @brief Lock EEPROM to protect settings */
int SMSBL::LockEprom(u8 ID)
{
	return writeByte(ID, SMSBL_LOCK, 1);
}

/** @brief Calibrate servo center position offset */
int SMSBL::CalibrationOfs(u8 ID)
{
	return writeByte(ID, SMSBL_TORQUE_ENABLE, 128);
}

/** @brief Read all feedback data into memory buffer */
int SMSBL::FeedBack(int ID)
{
	int nLen = Read(ID, SMSBL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		Err = 1;
		return -1;
	}
	Err = 0;
	return nLen;
}

/** @brief Read current position */
int SMSBL::ReadPos(int ID)
{
		if(ID == -1) {
			int Pos = Mem[SMSBL_PRESENT_POSITION_H-SMSBL_PRESENT_POSITION_L];
			Pos <<= 8;
			Pos |= Mem[SMSBL_PRESENT_POSITION_L-SMSBL_PRESENT_POSITION_L];
			if(Pos & (1 << 15)) {
				Pos = -(Pos & ~(1 << 15));
			}
			return Pos;
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_POSITION_L, 15);
}

/** @brief Read current speed */
int SMSBL::ReadSpeed(int ID)
{
		if(ID == -1) {
			int Speed = Mem[SMSBL_PRESENT_SPEED_H-SMSBL_PRESENT_POSITION_L];
			Speed <<= 8;
			Speed |= Mem[SMSBL_PRESENT_SPEED_L-SMSBL_PRESENT_POSITION_L];
			if(Speed & (1 << 15)) {
				Speed = -(Speed & ~(1 << 15));
			}
			return Speed;
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_SPEED_L, 15);
}

/** @brief Read current load */
int SMSBL::ReadLoad(int ID)
{
		if(ID == -1) {
			int Load = Mem[SMSBL_PRESENT_LOAD_H-SMSBL_PRESENT_POSITION_L];
			Load <<= 8;
			Load |= Mem[SMSBL_PRESENT_LOAD_L-SMSBL_PRESENT_POSITION_L];
			if(Load & (1 << 10)) {
				Load = -(Load & ~(1 << 10));
			}
			return Load;
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_LOAD_L, 10);
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
			int Current = Mem[SMSBL_PRESENT_CURRENT_H-SMSBL_PRESENT_POSITION_L];
			Current <<= 8;
			Current |= Mem[SMSBL_PRESENT_CURRENT_L-SMSBL_PRESENT_POSITION_L];
			if(Current & (1 << 15)) {
				Current = -(Current & ~(1 << 15));
			}
			return Current;
		}
		Err = 0;
		return readSignedWord(ID, SMSBL_PRESENT_CURRENT_L, 15);
}

