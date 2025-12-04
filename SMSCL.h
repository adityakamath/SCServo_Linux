/**
 * @file SMSCL.h
 * @brief Feetech SMSCL series serial servo application layer
 *
 * @details This file provides the application programming interface for
 * controlling Feetech SMSCL series serial bus servo motors. The SMSCL series
 * supports position control mode and velocity control mode.
 *
 * **Operating Modes:**
 * - Mode 0: Servo mode (position control with time and speed parameters)
 * - Mode 1: Wheel mode (continuous rotation with speed control)
 *
 * **Key Features:**
 * - Write operations: immediate, asynchronous (Reg), and synchronized (Sync)
 * - Comprehensive feedback: position, speed, load, voltage, temperature, current
 * - EEPROM management: lock/unlock for persistent configuration
 * - LSP compliant: uniform InitMotor() and Mode() methods
 *
 * **Usage Example:**
 * @code
 * SMSCL servo;
 * servo.begin(1000000, "/dev/ttyUSB0");
 * servo.InitMotor(1, 0, 1);  // ID=1, Mode=0 (servo), Enable torque
 * servo.WritePosEx(1, 2048, 1000, 50);  // Move to center position
 * @endcode
 *
 * @note Remember to call begin() before using any servo methods
 * @note WheelMode() is deprecated - use Mode(ID, 1) instead
 * @see SCSerial for serial communication layer methods
 */

#ifndef _SMSCL_H
#define _SMSCL_H


#define	SMSCL_1M 0
#define	SMSCL_0_5M 1
#define	SMSCL_250K 2
#define	SMSCL_128K 3
#define	SMSCL_115200 4
#define	SMSCL_76800	5
#define	SMSCL_57600	6
#define	SMSCL_38400	7

//Memory table definitions
//-------EEPROM (Read-only)--------
#define SMSCL_VERSION_L 3
#define SMSCL_VERSION_H 4

//-------EEPROM (Read/Write)--------
#define SMSCL_ID 5
#define SMSCL_BAUD_RATE 6
#define SMSCL_RETURN_DELAY_TIME 7
#define SMSCL_RETURN_LEVEL 8
#define SMSCL_MIN_ANGLE_LIMIT_L 9
#define SMSCL_MIN_ANGLE_LIMIT_H 10
#define SMSCL_MAX_ANGLE_LIMIT_L 11
#define SMSCL_MAX_ANGLE_LIMIT_H 12
#define SMSCL_LIMIT_TEMPERATURE 13
#define SMSCL_MAX_LIMIT_VOLTAGE 14
#define SMSCL_MIN_LIMIT_VOLTAGE 15
#define SMSCL_MAX_TORQUE_L 16
#define SMSCL_MAX_TORQUE_H 17
#define SMSCL_ALARM_LED 19
#define SMSCL_ALARM_SHUTDOWN 20
#define SMSCL_COMPLIANCE_P 21
#define SMSCL_COMPLIANCE_D 22
#define SMSCL_COMPLIANCE_I 23
#define SMSCL_PUNCH_L 24
#define SMSCL_PUNCH_H 25
#define SMSCL_CW_DEAD 26
#define SMSCL_CCW_DEAD 27
#define SMSCL_OFS_L 33
#define SMSCL_OFS_H 34
#define SMSCL_MODE 35
#define SMSCL_MAX_CURRENT_L 36
#define SMSCL_MAX_CURRENT_H 37	

//-------SRAM (Read/Write)--------
#define SMSCL_TORQUE_ENABLE 40
#define SMSCL_ACC 41
#define SMSCL_GOAL_POSITION_L 42
#define SMSCL_GOAL_POSITION_H 43
#define SMSCL_GOAL_TIME_L 44
#define SMSCL_GOAL_TIME_H 45
#define SMSCL_GOAL_SPEED_L 46
#define SMSCL_GOAL_SPEED_H 47
#define SMSCL_LOCK 48

//-------SRAM (Read-only)--------
#define SMSCL_PRESENT_POSITION_L 56
#define SMSCL_PRESENT_POSITION_H 57
#define SMSCL_PRESENT_SPEED_L 58
#define SMSCL_PRESENT_SPEED_H 59
#define SMSCL_PRESENT_LOAD_L 60
#define SMSCL_PRESENT_LOAD_H 61
#define SMSCL_PRESENT_VOLTAGE 62
#define SMSCL_PRESENT_TEMPERATURE 63
#define SMSCL_REGISTERED_INSTRUCTION 64
#define SMSCL_MOVING 66
#define SMSCL_PRESENT_CURRENT_L 69
#define SMSCL_PRESENT_CURRENT_H 70

// Direction bit positions
#define SMSCL_DIRECTION_BIT_POS 15
#define SMSCL_LOAD_DIRECTION_BIT_POS 10

#include "SCSerial.h"
#include "INST.h"
#include "ServoErrors.h"
#include "ServoUtils.h"

class SMSCL : public SCSerial
{
public:
	SMSCL();
	SMSCL(u8 End);
	SMSCL(u8 End, u8 Level);
	virtual int WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC = 0);// Write position command for a single servo
	virtual int RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC = 0);// Asynchronous write position command for a single servo (effective after RegWriteAction)
	virtual void SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[]);// Synchronous write position command for multiple servos
	virtual int Mode(u8 ID, u8 mode); // Set operating mode (0=servo, 1=wheel)
	virtual int InitMotor(u8 ID, u8 mode, u8 enableTorque = 1); // Initialize motor with mode and torque (unlocks EEPROM, sets mode, locks EEPROM, enables/disables torque)
	[[deprecated("Use Mode(ID, 1) instead")]] virtual int WheelMode(u8 ID);// Constant speed mode - DEPRECATED: use Mode(ID, 1)
	virtual int WriteSpe(u8 ID, s16 Speed, u8 ACC = 0);// Constant speed mode control command
	virtual int EnableTorque(u8 ID, u8 Enable);// Torque control command
	virtual int unLockEeprom(u8 ID);// EEPROM unlock
	virtual int LockEeprom(u8 ID);// EEPROM lock
	virtual int CalibrationOfs(u8 ID);// Center position calibration - Sets offset register to 128 for midpoint calibration (different from InitMotor which sets operating mode)
	virtual int FeedBack(int ID);// Feedback servo information
	virtual int ReadPos(int ID);// Read position
	virtual int ReadSpeed(int ID);// Read speed
	virtual int ReadLoad(int ID);// Read output voltage percentage to motor (0~1000)
	virtual int ReadVoltage(int ID);// Read voltage
	virtual int ReadTemper(int ID);// Read temperature
	virtual int ReadMove(int ID);// Read movement status
	virtual int ReadCurrent(int ID);// Read current
private:
	u8 Mem[SMSCL_PRESENT_CURRENT_H-SMSCL_PRESENT_POSITION_L+1];
};

#endif