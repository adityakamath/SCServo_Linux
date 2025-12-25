/**
 * @file HLSCL.cpp
 * @brief Feetech HTS/HLS Series Serial Servo Application Layer Implementation
 *
 * @date 2025.9.27
 */

#include "HLSCL.h"

HLSCL::HLSCL()
{
	End = 0;
}

HLSCL::HLSCL(u8 End):SCSerial(End)
{
}

HLSCL::HLSCL(u8 End, u8 Level):SCSerial(End, Level)
{
}

int HLSCL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC, u16 Torque)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, Torque);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return genWrite(ID, HLSCL_ACC, bBuf, 7);
}

int HLSCL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC, u16 Torque)
{
	if(Position<0){
		Position = -Position;
		Position |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, Position);
	Host2SCS(bBuf+3, bBuf+4, Torque);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return regWrite(ID, HLSCL_ACC, bBuf, 7);
}

void HLSCL::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[], u16 Torque[])
{
    u8 offbuf[7*IDN];
    for(u8 i = 0; i<IDN; i++){
		if(Position[i]<0){
			Position[i] = -Position[i];
			Position[i] |= (1<<15);
		}
		if(ACC){
			offbuf[i*7] = ACC[i];
		}else{
			offbuf[i*7] = 0;
		}
        Host2SCS(offbuf+i*7+1, offbuf+i*7+2, Position[i]);
        Host2SCS(offbuf+i*7+3, offbuf+i*7+4, Torque[i]);
        Host2SCS(offbuf+i*7+5, offbuf+i*7+6, Speed[i]);
    }
    syncWrite(ID, IDN, HLSCL_ACC, offbuf, 7);
}

void HLSCL::SyncWriteSpe(u8 ID[], u8 IDN, s16 Speed[], u8 ACC[], u16 Torque[])
{
    u8 offbuf[7*IDN];
    for(u8 i = 0; i<IDN; i++){
		if(Speed[i]<0){
			Speed[i] = -Speed[i];
			Speed[i] |= (1<<15);
		}
		if(ACC){
			offbuf[i*7] = ACC[i];
		}else{
			offbuf[i*7] = 0;
		}
        Host2SCS(offbuf+i*7+1, offbuf+i*7+2, 0);
        Host2SCS(offbuf+i*7+3, offbuf+i*7+4, Torque[i]);
        Host2SCS(offbuf+i*7+5, offbuf+i*7+6, Speed[i]);
    }
    syncWrite(ID, IDN, HLSCL_ACC, offbuf, 7);
}

int HLSCL::WheelMode(u8 ID)
{
	return writeByte(ID, HLSCL_MODE, 1);
}

int HLSCL::EleMode(u8 ID)
{
	return writeByte(ID, HLSCL_MODE, 2);
}

int HLSCL::WriteSpe(u8 ID, s16 Speed, u8 ACC, u16 Torque)
{
	if(Speed<0){
		Speed = -Speed;
		Speed |= (1<<15);
	}
	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, 0);
	Host2SCS(bBuf+3, bBuf+4, Torque);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return genWrite(ID, HLSCL_ACC, bBuf, 7);
}

int HLSCL::WriteEle(u8 ID, s16 Torque)
{
	if(Torque<0){
		Torque = -Torque;
		Torque |= (1<<15);
	}
	return writeWord(ID, HLSCL_GOAL_TORQUE_L, Torque);
}

int HLSCL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, HLSCL_TORQUE_ENABLE, Enable);
}

int HLSCL::unLockEprom(u8 ID)
{
	EnableTorque(ID, 0);
	return writeByte(ID, HLSCL_LOCK, 0);
}

int HLSCL::LockEprom(u8 ID)
{
	return writeByte(ID, HLSCL_LOCK, 1);
}

int HLSCL::CalibrationOfs(u8 ID)
{
	EnableTorque(ID, 0);
	unLockEprom(ID);
	return writeByte(ID, HLSCL_TORQUE_ENABLE, 128);  // 128 = Calibration command
}

int HLSCL::FeedBack(int ID)
{
	int nLen = Read(ID, HLSCL_PRESENT_POSITION_L, Mem, sizeof(Mem));
	if(nLen!=sizeof(Mem)){
		return -1;
	}
	return nLen;
}

int HLSCL::ReadPos(int ID)
{
	int Pos = -1;
	if(ID==-1){
		Pos = Mem[HLSCL_PRESENT_POSITION_H-HLSCL_PRESENT_POSITION_L];
		Pos <<= 8;
		Pos |= Mem[HLSCL_PRESENT_POSITION_L-HLSCL_PRESENT_POSITION_L];
	}else{
		Pos = readWord(ID, HLSCL_PRESENT_POSITION_L);
	}
	if(Pos&(1<<15)){
		Pos = -(Pos&~(1<<15));
	}

	return Pos;
}

int HLSCL::ReadSpeed(int ID)
{
	int Speed = -1;
	if(ID==-1){
		Speed = Mem[HLSCL_PRESENT_SPEED_H-HLSCL_PRESENT_POSITION_L];
		Speed <<= 8;
		Speed |= Mem[HLSCL_PRESENT_SPEED_L-HLSCL_PRESENT_POSITION_L];
	}else{
		Speed = readWord(ID, HLSCL_PRESENT_SPEED_L);
	}
	if(Speed&(1<<15)){
		Speed = -(Speed&~(1<<15));
	}
	return Speed;
}

int HLSCL::ReadLoad(int ID)
{
	int Load = -1;
	if(ID==-1){
		Load = Mem[HLSCL_PRESENT_LOAD_H-HLSCL_PRESENT_POSITION_L];
		Load <<= 8;
		Load |= Mem[HLSCL_PRESENT_LOAD_L-HLSCL_PRESENT_POSITION_L];
	}else{
		Load = readWord(ID, HLSCL_PRESENT_LOAD_L);
	}
	if(Load&(1<<10)){
		Load = -(Load&~(1<<10));
	}
	return Load;
}

int HLSCL::ReadVoltage(int ID)
{
	int Voltage = -1;
	if(ID==-1){
		Voltage = Mem[HLSCL_PRESENT_VOLTAGE-HLSCL_PRESENT_POSITION_L];
	}else{
		Voltage = readByte(ID, HLSCL_PRESENT_VOLTAGE);
	}
	return Voltage;
}

int HLSCL::ReadTemper(int ID)
{
	int Temper = -1;
	if(ID==-1){
		Temper = Mem[HLSCL_PRESENT_TEMPERATURE-HLSCL_PRESENT_POSITION_L];
	}else{
		Temper = readByte(ID, HLSCL_PRESENT_TEMPERATURE);
	}
	return Temper;
}

int HLSCL::ReadMove(int ID)
{
	int Move = -1;
	if(ID==-1){
		Move = Mem[HLSCL_MOVING-HLSCL_PRESENT_POSITION_L];
	}else{
		Move = readByte(ID, HLSCL_MOVING);
	}
	return Move;
}

int HLSCL::ReadCurrent(int ID)
{
	int Current = -1;
	if(ID==-1){
		Current = Mem[HLSCL_PRESENT_CURRENT_H-HLSCL_PRESENT_POSITION_L];
		Current <<= 8;
		Current |= Mem[HLSCL_PRESENT_CURRENT_L-HLSCL_PRESENT_POSITION_L];
	}else{
		Current = readWord(ID, HLSCL_PRESENT_CURRENT_L);
	}
	if(Current&(1<<15)){
		Current = -(Current&~(1<<15));
	}
	return Current;
}

int HLSCL::ServoMode(u8 ID)
{
	return writeByte(ID, HLSCL_MODE, 0);
}
