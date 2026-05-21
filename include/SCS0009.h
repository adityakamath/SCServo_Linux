/**
 * @file SCS0009.h
 * @brief Feetech SCS0009 Series Serial Servo Application Layer
 *
 * @details This file maps very closely to the SCSCL.h file modulo 
 * - a few details 
 */

#ifndef _SCS0009_H
#define _SCS0009_H
#include "INST.h"
#include "SCSerial.h"

// Baud rate definitions
#define	SCS0009_1M 0
#define	SCS0009_0_5M 1
#define	SCS0009_250K 2
#define	SCS0009_128K 3
#define	SCS0009_115200 4
#define	SCS0009_76800	5
#define	SCS0009_57600	6
#define	SCS0009_38400	7

// Memory table definitions
//-------EEPROM (Read-only)--------
#define SCS0009_VERSION_L 3
#define SCS0009_VERSION_H 4

//-------EEPROM (Read/Write)--------
#define SCS0009_ID 5
#define SCS0009_BAUD_RATE 6

// hang-time of how long the servo waits to respond to a host
// Might need to be tuned if you start seeing a lot of unacked status packets or retransmits
// at either end of the conversation
// These registers are present on the SCS0009 but not on some other Feetech servos 
#define SCS0009_RETURN_DELAY        7    // 0x07 — response delay in 2µs units
#define SCS0009_STATUS_RETURN_LEVEL 8    // 0x08 — 0=ping only, 1=read, 2=all

#define SCS0009_MIN_ANGLE_LIMIT_L 9
#define SCS0009_MIN_ANGLE_LIMIT_H 10
#define SCS0009_MAX_ANGLE_LIMIT_L 11
#define SCS0009_MAX_ANGLE_LIMIT_H 12

#define SCS0009_CW_DEAD 26
#define SCS0009_CCW_DEAD 27

//-------SRAM (Read/Write)--------
#define SCS0009_TORQUE_ENABLE 40
#define SCS0009_ACC 41
#define SCS0009_GOAL_POSITION_L 42
#define SCS0009_GOAL_POSITION_H 43
#define SCS0009_GOAL_TIME_L 44
#define SCS0009_GOAL_TIME_H 45
#define SCS0009_GOAL_SPEED_L 46
#define SCS0009_GOAL_SPEED_H 47
#define SCS0009_LOCK 48

//-------SRAM (Read-only)--------
#define SCS0009_PRESENT_POSITION_L 56
#define SCS0009_PRESENT_POSITION_H 57
#define SCS0009_PRESENT_SPEED_L 58
#define SCS0009_PRESENT_SPEED_H 59
#define SCS0009_PRESENT_LOAD_L 60
#define SCS0009_PRESENT_LOAD_H 61
#define SCS0009_PRESENT_VOLTAGE 62
#define SCS0009_PRESENT_TEMPERATURE 63
#define SCS0009_MOVING 66
#define SCS0009_PRESENT_CURRENT_L 69
#define SCS0009_PRESENT_CURRENT_H 70

// Direction bit positions

#define SCS0009_DIRECTION_BIT_POS 15

// hang-time of how long the servo waits to respond to a host
// Might need to be tuned if you start seeing a lot of unacked status packets or retransmits
// at either end of the conversation

/**
 * @class SCS0009
 * @brief Application layer interface for SCS0009 series serial servos
 *
 * **Inheritance:**
 * Inherits from SCSerial for serial communication and SCS protocol handling
 *
 * @see SCSCL and others for similar servo series interface
 * @see SCSerial for communication layer
 */
class SCS0009 : public SCSerial
{
public:
	SCS0009();
	SCS0009(u8 End);
	SCS0009(u8 End, u8 Level);


	virtual int WritePos(u8 ID, u16 Position, u16 Time, u16 Speed = 0);
	virtual int RegWritePos(u8 ID, u16 Position, u16 Time, u16 Speed = 0);
	virtual void SyncWritePos(u8 ID[], u8 IDN, u16 Position[], u16 Time[], u16 Speed[]);
	virtual int Mode(u8 ID, u8 mode); // Set operating mode (implementation via angle limits for SCS0009)
	virtual int InitMotor(u8 ID, u8 mode, u8 enableTorque = 1); // Initialize motor with mode and torque (unlocks EEPROM, sets mode, locks EEPROM, enables/disables torque)
	virtual int PWMMode(u8 ID);
	virtual int WritePWM(u8 ID, s16 pwmOut);
	virtual int EnableTorque(u8 ID, u8 Enable);
	virtual int unLockEeprom(u8 ID);
	virtual int LockEeprom(u8 ID);
	virtual int FeedBack(u8 ID);
	virtual int ReadPos(u8 ID);
	virtual int ReadSpeed(u8 ID);
	virtual int ReadLoad(u8 ID);
	virtual int ReadVoltage(u8 ID);
	virtual int ReadTemper(u8 ID);
	virtual int ReadMove(u8 ID);
	virtual int ReadCurrent(u8 ID);
private:
	u8 Mem[SCS0009_PRESENT_CURRENT_H-SCS0009_PRESENT_POSITION_L+1];
};

#endif
