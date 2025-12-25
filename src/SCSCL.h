/**
 * @file SCSCL.h
 * @brief Feetech SCSCL Series Serial Servo Application Layer
 *
 * @details This file provides the application programming interface for
 * controlling Feetech SCSCL series serial bus servo motors.
 * Supports two primary operating modes:
 * - Servo mode: Position control with time and speed parameters
 * - PWM mode: Direct PWM output control (open-loop)
 *
 * **Key Features:**
 * - Position control with time and speed settings
 * - PWM output control for open-loop operation
 * - Feedback reading (position, speed, load, voltage, temperature, current)
 * - EEPROM lock/unlock for parameter persistence
 * - Mode switching via angle limit configuration
 *
 * **Memory Map:**
 * - EEPROM (Read-only): Model number, version
 * - EEPROM (Read/Write): ID, baud rate, angle limits, dead zones
 * - SRAM (Read/Write): Torque enable, goal position/time/speed, EEPROM lock
 * - SRAM (Read-only): Present position, speed, load, voltage, temperature, current, moving status
 *
 * @note SCSCL series uses angle limit configuration for mode switching
 * @see SMS_STS.h for reference implementation
 */

#ifndef _SCSCL_H
#define _SCSCL_H
#include "INST.h"
#include "SCSerial.h"
#include "ServoErrors.h"
#include "ServoUtils.h"

// Baud rate definitions
#define	SCSCL_1M 0
#define	SCSCL_0_5M 1
#define	SCSCL_250K 2
#define	SCSCL_128K 3
#define	SCSCL_115200 4
#define	SCSCL_76800	5
#define	SCSCL_57600	6
#define	SCSCL_38400	7

// Memory table definitions
//-------EEPROM (Read-only)--------
#define SCSCL_VERSION_L 3
#define SCSCL_VERSION_H 4

//-------EEPROM (Read/Write)--------
#define SCSCL_ID 5
#define SCSCL_BAUD_RATE 6
#define SCSCL_MIN_ANGLE_LIMIT_L 9
#define SCSCL_MIN_ANGLE_LIMIT_H 10
#define SCSCL_MAX_ANGLE_LIMIT_L 11
#define SCSCL_MAX_ANGLE_LIMIT_H 12
#define SCSCL_CW_DEAD 26
#define SCSCL_CCW_DEAD 27

//-------SRAM (Read/Write)--------
#define SCSCL_TORQUE_ENABLE 40
#define SCSCL_GOAL_POSITION_L 42
#define SCSCL_GOAL_POSITION_H 43
#define SCSCL_GOAL_TIME_L 44
#define SCSCL_GOAL_TIME_H 45
#define SCSCL_GOAL_SPEED_L 46
#define SCSCL_GOAL_SPEED_H 47
#define SCSCL_LOCK 48

//-------SRAM (Read-only)--------
#define SCSCL_PRESENT_POSITION_L 56
#define SCSCL_PRESENT_POSITION_H 57
#define SCSCL_PRESENT_SPEED_L 58
#define SCSCL_PRESENT_SPEED_H 59
#define SCSCL_PRESENT_LOAD_L 60
#define SCSCL_PRESENT_LOAD_H 61
#define SCSCL_PRESENT_VOLTAGE 62
#define SCSCL_PRESENT_TEMPERATURE 63
#define SCSCL_MOVING 66
#define SCSCL_PRESENT_CURRENT_L 69
#define SCSCL_PRESENT_CURRENT_H 70

// Direction bit positions
#define SCSCL_DIRECTION_BIT_POS 15
#define SCSCL_LOAD_DIRECTION_BIT_POS 10
// SCSCL uses bit 10 for PWM direction in open-loop mode
#define SCSCL_PWM_DIRECTION_BIT_POS 10

// Missing register definitions (needed for full compatibility)
#define SCSCL_ACC 41
#define SCSCL_MODE 35
#define SCSCL_OFS_L 33
#define SCSCL_OFS_H 34

/**
 * @class SCSCL
 * @brief Application layer interface for SCSCL series serial servos
 *
 * @details Provides high-level control functions for Feetech SCSCL series
 * servo motors. Supports position control and PWM output modes.
 *
 * **Core Functions:**
 * - WritePos: Write position with time and speed
 * - RegWritePos: Asynchronous position write
 * - SyncWritePos: Synchronized multi-servo position control
 * - WritePWM: Direct PWM output control
 * - Mode/InitMotor: Operating mode configuration
 * - FeedBack: Read all servo status
 * - Read* methods: Individual parameter reading
 *
 * **Inheritance:**
 * Inherits from SCSerial for serial communication and SCS protocol handling
 *
 * @see SMS_STS for similar servo series interface
 * @see SCSerial for communication layer
 */
class SCSCL : public SCSerial
{
public:
	SCSCL();
	SCSCL(u8 End);
	SCSCL(u8 End, u8 Level);


	virtual int WritePos(u8 ID, u16 Position, u16 Time, u16 Speed = 0);
	virtual int RegWritePos(u8 ID, u16 Position, u16 Time, u16 Speed = 0);
	virtual void SyncWritePos(u8 ID[], u8 IDN, u16 Position[], u16 Time[], u16 Speed[]);
	virtual int Mode(u8 ID, u8 mode); // Set operating mode (implementation via angle limits for SCSCL)
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
	u8 Mem[SCSCL_PRESENT_CURRENT_H-SCSCL_PRESENT_POSITION_L+1];
};

#endif