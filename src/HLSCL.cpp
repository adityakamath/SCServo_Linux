/**
 * @file HLSCL.cpp
 * @brief Feetech HTS/HLS Series Serial Servo Application Layer Implementation
 *
 * @details This file implements high-level control functions for Feetech HLS
 * series servo motors. Supports three operating modes with complete read/write
 * functionality and LSP-compliant initialization.
 *
 * **Implemented Features:**
 * - Mode 0: Position control with speed, acceleration, and torque
 * - Mode 1: Velocity control (constant velocity wheel mode)
 * - Mode 2: Force/Torque control (constant torque output mode)
 * - Synchronized writes for multi-motor coordination
 * - Buffered writes with RegWriteAction
 * - Comprehensive feedback reading (position, speed, load, voltage, temp, current)
 *
 * **Refactoring Improvements:**
 * - Uses ServoUtils for direction bit encoding/decoding (DRY principle)
 * - Uses SyncWriteBuffer for automatic memory management (RAII)
 * - Standardized error handling with ServoErrors
 *
 * @see HLSCL.h for class interface and usage examples
 */

#include "HLSCL.h"
#include "SyncWriteBuffer.h"

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

/**
 * @brief Write position, speed, acceleration, and torque to servo
 * 
 * Sends single-servo position command with full parameter control.
 * Negative positions are encoded with direction bit using ServoUtils.
 * 
 * @param ID Servo ID
 * @param Position Target position (±4095 steps)
 * @param Speed Moving speed (0-3400 steps/s)
 * @param ACC Acceleration value (0-254)
 * @param Torque Torque limit (0-1000)
 * @return 1 on success, 0 on failure
 */
int HLSCL::WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC, u16 Torque)
{
	u16 encodedPosition = ServoUtils::encodeSignedValue(Position, HLSCL_DIRECTION_BIT_POS);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, encodedPosition);
	Host2SCS(bBuf+3, bBuf+4, Torque);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return genWrite(ID, HLSCL_ACC, bBuf, 7);
}

/**
 * @brief Register write position command (executes on RegWriteAction)
 * 
 * Queues position/speed/acceleration/torque command for later execution.
 * Use with RegWriteAction for synchronized multi-servo motion.
 * 
 * @param ID Servo ID
 * @param Position Target position (±4095 steps)
 * @param Speed Moving speed (0-3400 steps/s)
 * @param ACC Acceleration value (0-254)
 * @param Torque Torque limit (0-1000)
 * @return 1 on success, 0 on failure
 */
int HLSCL::RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC, u16 Torque)
{
	u16 encodedPosition = ServoUtils::encodeSignedValue(Position, HLSCL_DIRECTION_BIT_POS);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, encodedPosition);
	Host2SCS(bBuf+3, bBuf+4, Torque);
	Host2SCS(bBuf+5, bBuf+6, Speed);

	return regWrite(ID, HLSCL_ACC, bBuf, 7);
}

/**
 * @brief Synchronized position write for multiple servos
 * 
 * Uses RAII-based SyncWriteBuffer for automatic memory management.
 * Encodes signed positions using ServoUtils helper.
 * 
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Position Array of target positions (±4095 steps)
 * @param Speed Array of speeds (0-3400 steps/s)
 * @param ACC Array of accelerations (0-254, can be nullptr for 0)
 * @param Torque Array of torque limits (0-1000)
 */
void HLSCL::SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[], u16 Torque[])
{
	SyncWriteBuffer buffer(IDN, 7);
	if (!buffer.isValid()) {
		return;  // Allocation failed
	}

	u8* offbuf = buffer.getBuffer();
	for(u8 i = 0; i < IDN; i++) {
		u16 encodedPosition = ServoUtils::encodeSignedValue(Position[i], HLSCL_DIRECTION_BIT_POS);
		
		if(ACC) {
			offbuf[i*7] = ACC[i];
		} else {
			offbuf[i*7] = 0;
		}
		Host2SCS(offbuf+i*7+1, offbuf+i*7+2, encodedPosition);
		Host2SCS(offbuf+i*7+3, offbuf+i*7+4, Torque[i]);
		Host2SCS(offbuf+i*7+5, offbuf+i*7+6, Speed[i]);
	}
	syncWrite(ID, IDN, HLSCL_ACC, offbuf, 7);
}

/**
 * @brief Synchronized speed write for multiple servos
 * 
 * Uses RAII-based SyncWriteBuffer for automatic memory management.
 * Encodes signed speeds using ServoUtils helper.
 * 
 * @param ID Array of servo IDs
 * @param IDN Number of servos
 * @param Speed Array of speeds (±3400 steps/s)
 * @param ACC Array of accelerations (0-254, can be nullptr for 0)
 * @param Torque Array of torque limits (0-1000)
 */
void HLSCL::SyncWriteSpe(u8 ID[], u8 IDN, s16 Speed[], u8 ACC[], u16 Torque[])
{
	SyncWriteBuffer buffer(IDN, 7);
	if (!buffer.isValid()) {
		return;  // Allocation failed
	}

	u8* offbuf = buffer.getBuffer();
	for(u8 i = 0; i < IDN; i++) {
		u16 encodedSpeed = ServoUtils::encodeSignedValue(Speed[i], HLSCL_DIRECTION_BIT_SPEED);
		
		if(ACC) {
			offbuf[i*7] = ACC[i];
		} else {
			offbuf[i*7] = 0;
		}
		Host2SCS(offbuf+i*7+1, offbuf+i*7+2, 0);
		Host2SCS(offbuf+i*7+3, offbuf+i*7+4, Torque[i]);
		Host2SCS(offbuf+i*7+5, offbuf+i*7+6, encodedSpeed);
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

/**
 * @brief Write speed command for constant velocity mode
 * 
 * Sends single-servo speed command with acceleration and torque control.
 * Negative speeds are encoded with direction bit using ServoUtils.
 * 
 * @param ID Servo ID
 * @param Speed Target velocity (±3400 steps/s)
 * @param ACC Acceleration value (0-254)
 * @param Torque Torque limit (0-1000)
 * @return 1 on success, 0 on failure
 */
int HLSCL::WriteSpe(u8 ID, s16 Speed, u8 ACC, u16 Torque)
{
	u16 encodedSpeed = ServoUtils::encodeSignedValue(Speed, HLSCL_DIRECTION_BIT_SPEED);

	u8 bBuf[7];
	bBuf[0] = ACC;
	Host2SCS(bBuf+1, bBuf+2, 0);
	Host2SCS(bBuf+3, bBuf+4, Torque);
	Host2SCS(bBuf+5, bBuf+6, encodedSpeed);

	return genWrite(ID, HLSCL_ACC, bBuf, 7);
}

/**
 * @brief Write torque command for electric/force mode
 * 
 * Sends single-servo torque command for constant torque output.
 * Negative torques are encoded with direction bit using ServoUtils.
 * 
 * @param ID Servo ID
 * @param Torque Target torque (±1000)
 * @return 1 on success, 0 on failure
 */
int HLSCL::WriteEle(u8 ID, s16 Torque)
{
	u16 encodedTorque = ServoUtils::encodeSignedValue(Torque, HLSCL_DIRECTION_BIT_TORQUE);
	return writeWord(ID, HLSCL_GOAL_TORQUE_L, encodedTorque);
}

int HLSCL::EnableTorque(u8 ID, u8 Enable)
{
	return writeByte(ID, HLSCL_TORQUE_ENABLE, Enable);
}

int HLSCL::unLockEprom(u8 ID)
{
	// Disable torque before unlocking EEPROM
	int ret = EnableTorque(ID, 0);
	if(ret != 1){
		return ret;  // Propagate error if torque disable failed
	}
	
	// Unlock EEPROM
	return writeByte(ID, HLSCL_LOCK, 0);
}

int HLSCL::LockEprom(u8 ID)
{
	return writeByte(ID, HLSCL_LOCK, 1);
}

int HLSCL::CalibrationOfs(u8 ID)
{
	int ret;
	
	// Disable torque before calibration
	ret = EnableTorque(ID, 0);
	if(ret != 1){
		return ret;  // Propagate error
	}
	
	// Unlock EEPROM to allow calibration write
	ret = unLockEprom(ID);
	if(ret != 1){
		return ret;  // Propagate error
	}
	
	// Send calibration command
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

/**
 * @brief Read current position from servo or cached data
 * 
 * Uses ServoUtils for direction bit decoding.
 * 
 * @param ID Servo ID or -1 for cached data
 * @return Position (±4095 steps) or -1 on error
 */
int HLSCL::ReadPos(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		u16 encodedPos = ServoUtils::readWordFromBuffer(Mem, 
			HLSCL_PRESENT_POSITION_L - HLSCL_PRESENT_POSITION_L,
			HLSCL_PRESENT_POSITION_H - HLSCL_PRESENT_POSITION_L);
		return ServoUtils::decodeSignedValue(encodedPos, HLSCL_DIRECTION_BIT_POS);
	} else {
		int Pos = readWord(ID, HLSCL_PRESENT_POSITION_L);
		if(Pos == -1) {
			return -1;
		}
		return ServoUtils::decodeSignedValue(static_cast<u16>(Pos), HLSCL_DIRECTION_BIT_POS);
	}
}

/**
 * @brief Read current speed from servo or cached data
 * 
 * Uses ServoUtils for direction bit decoding.
 * 
 * @param ID Servo ID or -1 for cached data
 * @return Speed (±3400 steps/s) or -1 on error
 */
int HLSCL::ReadSpeed(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		u16 encodedSpeed = ServoUtils::readWordFromBuffer(Mem,
			HLSCL_PRESENT_SPEED_L - HLSCL_PRESENT_POSITION_L,
			HLSCL_PRESENT_SPEED_H - HLSCL_PRESENT_POSITION_L);
		return ServoUtils::decodeSignedValue(encodedSpeed, HLSCL_DIRECTION_BIT_SPEED);
	} else {
		int Speed = readWord(ID, HLSCL_PRESENT_SPEED_L);
		if(Speed == -1) {
			return -1;
		}
		return ServoUtils::decodeSignedValue(static_cast<u16>(Speed), HLSCL_DIRECTION_BIT_SPEED);
	}
}

/**
 * @brief Read current load from servo or cached data
 * 
 * Uses ServoUtils for direction bit decoding (bit 10 for load).
 * 
 * @param ID Servo ID or -1 for cached data
 * @return Load (±1000) or -1 on error
 */
int HLSCL::ReadLoad(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		u16 encodedLoad = ServoUtils::readWordFromBuffer(Mem,
			HLSCL_PRESENT_LOAD_L - HLSCL_PRESENT_POSITION_L,
			HLSCL_PRESENT_LOAD_H - HLSCL_PRESENT_POSITION_L);
		return ServoUtils::decodeSignedValue(encodedLoad, HLSCL_DIRECTION_BIT_LOAD);
	} else {
		int Load = readWord(ID, HLSCL_PRESENT_LOAD_L);
		if(Load == -1) {
			return -1;
		}
		return ServoUtils::decodeSignedValue(static_cast<u16>(Load), HLSCL_DIRECTION_BIT_LOAD);
	}
}

/**
 * @brief Read voltage from servo or cached data
 * 
 * @param ID Servo ID or -1 for cached data
 * @return Voltage in 0.1V units (e.g., 120=12.0V) or -1 on error
 */
int HLSCL::ReadVoltage(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		return Mem[HLSCL_PRESENT_VOLTAGE - HLSCL_PRESENT_POSITION_L];
	} else {
		return readByte(ID, HLSCL_PRESENT_VOLTAGE);
	}
}

/**
 * @brief Read temperature from servo or cached data
 * 
 * @param ID Servo ID or -1 for cached data
 * @return Temperature in °C or -1 on error
 */
int HLSCL::ReadTemper(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		return Mem[HLSCL_PRESENT_TEMPERATURE - HLSCL_PRESENT_POSITION_L];
	} else {
		return readByte(ID, HLSCL_PRESENT_TEMPERATURE);
	}
}

/**
 * @brief Read movement status from servo or cached data
 * 
 * @param ID Servo ID or -1 for cached data
 * @return 1=moving, 0=stopped, -1=error
 */
int HLSCL::ReadMove(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		return Mem[HLSCL_MOVING - HLSCL_PRESENT_POSITION_L];
	} else {
		return readByte(ID, HLSCL_MOVING);
	}
}

/**
 * @brief Read current from servo or cached data
 * 
 * Uses ServoUtils for direction bit decoding.
 * 
 * @param ID Servo ID or -1 for cached data
 * @return Current in mA or -1 on error
 */
int HLSCL::ReadCurrent(int ID)
{
	if(ServoUtils::isCachedRead(ID)) {
		u16 encodedCurrent = ServoUtils::readWordFromBuffer(Mem,
			HLSCL_PRESENT_CURRENT_L - HLSCL_PRESENT_POSITION_L,
			HLSCL_PRESENT_CURRENT_H - HLSCL_PRESENT_POSITION_L);
		return ServoUtils::decodeSignedValue(encodedCurrent, HLSCL_DIRECTION_BIT_CURRENT);
	} else {
		int Current = readWord(ID, HLSCL_PRESENT_CURRENT_L);
		if(Current == -1) {
			return -1;
		}
		return ServoUtils::decodeSignedValue(static_cast<u16>(Current), HLSCL_DIRECTION_BIT_CURRENT);
	}
}

int HLSCL::ServoMode(u8 ID)
{
	return writeByte(ID, HLSCL_MODE, 0);
}
