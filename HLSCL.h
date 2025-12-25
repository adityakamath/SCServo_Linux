/**
 * @file HLSCL.h
 * @brief Feetech HLS Series Serial Servo Application Layer
 *
 * @details This file provides the application programming interface for
 * controlling Feetech HLS series serial bus servo motors.
 * Supports three operating modes:
 * - Mode 0: Servo (position control)
 * - Mode 1: Wheel (constant velocity control with feedback)
 * - Mode 2: Electric/Force (constant torque output control)
 *
 * @date 2025.9.27
 */

#ifndef _HLSCL_H
#define _HLSCL_H

//Memory table definition
//-------EPROM (Read only)--------
#define HLSCL_MODEL_L 3
#define HLSCL_MODEL_H 4

//-------EPROM (Read and Write)--------
#define HLSCL_ID 5
#define HLSCL_BAUD_RATE 6
#define HLSCL_SECOND_ID 7
#define HLSCL_MIN_ANGLE_LIMIT_L 9
#define HLSCL_MIN_ANGLE_LIMIT_H 10
#define HLSCL_MAX_ANGLE_LIMIT_L 11
#define HLSCL_MAX_ANGLE_LIMIT_H 12
#define HLSCL_CW_DEAD 26
#define HLSCL_CCW_DEAD 27
#define HLSCL_OFS_L 31
#define HLSCL_OFS_H 32
#define HLSCL_MODE 33

//-------SRAM (Read and Write)--------
#define HLSCL_TORQUE_ENABLE 40
#define HLSCL_ACC 41
#define HLSCL_GOAL_POSITION_L 42
#define HLSCL_GOAL_POSITION_H 43
#define HLSCL_GOAL_TORQUE_L 44
#define HLSCL_GOAL_TORQUE_H 45
#define HLSCL_GOAL_SPEED_L 46
#define HLSCL_GOAL_SPEED_H 47
#define HLSCL_TORQUE_LIMIT_L 48
#define HLSCL_TORQUE_LIMIT_H 49
#define HLSCL_LOCK 55

//-------SRAM (Read only)--------
#define HLSCL_PRESENT_POSITION_L 56
#define HLSCL_PRESENT_POSITION_H 57
#define HLSCL_PRESENT_SPEED_L 58
#define HLSCL_PRESENT_SPEED_H 59
#define HLSCL_PRESENT_LOAD_L 60
#define HLSCL_PRESENT_LOAD_H 61
#define HLSCL_PRESENT_VOLTAGE 62
#define HLSCL_PRESENT_TEMPERATURE 63
#define HLSCL_MOVING 66
#define HLSCL_PRESENT_CURRENT_L 69
#define HLSCL_PRESENT_CURRENT_H 70

// Operating mode values
#define HLSCL_MODE_SERVO 0        // Servo mode (position control)
#define HLSCL_MODE_WHEEL 1        // Wheel mode (constant velocity control)
#define HLSCL_MODE_ELECTRIC 2     // Electric/Force mode (constant torque output)

#include "SCSerial.h"

/**
 * @class HLSCL
 * @brief Application layer interface for HLS series serial servos
 *
 * @details Provides high-level control functions for Feetech HLS series
 * servo motors. Supports three operating modes with complete read/write functionality.
 *
 * **Operating Modes:**
 * - Mode 0: Servo mode (position control) - precise positioning
 * - Mode 1: Wheel mode (velocity control) - constant speed rotation
 * - Mode 2: Electric mode (force control) - constant torque output
 *
 * **Key Features:**
 * - Position control with speed, acceleration, and torque limiting
 * - Velocity control with acceleration and torque parameters
 * - Force/torque control mode (unique to HLS series)
 * - Synchronized multi-servo commands
 * - Buffered command execution with RegWrite
 * - Comprehensive feedback reading (position, speed, load, voltage, temperature, current)
 *
 * @see SCSerial for base serial communication functionality
 */
class HLSCL : public SCSerial
{
public:
	HLSCL();
	HLSCL(u8 End);
	HLSCL(u8 End, u8 Level);

	/**
	 * @brief Write position command to single servo
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @param Position Target position (0-4095 for 12-bit, 0-1023 for 10-bit)
	 * @param Speed Movement speed (0-3400 steps/s, 0=maximum)
	 * @param ACC Acceleration (0-254, units of 100 steps/s², 0=maximum)
	 * @param Torque Torque limit (0-1000, 0=no limit)
	 * @return 1 on success, 0 on failure
	 */
	int WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC = 0, u16 Torque = 0);

	/**
	 * @brief Buffered position write (executes on RegWriteAction)
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @param Position Target position (0-4095 for 12-bit, 0-1023 for 10-bit)
	 * @param Speed Movement speed (0-3400 steps/s)
	 * @param ACC Acceleration (0-254, units of 100 steps/s²)
	 * @param Torque Torque limit (0-1000)
	 * @return 1 on success, 0 on failure
	 */
	int RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC = 0, u16 Torque = 0);

	/**
	 * @brief Synchronized position write for multiple servos
	 * @param ID Array of servo IDs
	 * @param IDN Number of servos
	 * @param Position Array of target positions
	 * @param Speed Array of speeds
	 * @param ACC Array of accelerations
	 * @param Torque Array of torque limits
	 */
	void SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[], u16 Torque[]);

	/**
	 * @brief Synchronized speed write for multiple servos
	 * @param ID Array of servo IDs
	 * @param IDN Number of servos
	 * @param Speed Array of speeds (±3400 steps/s)
	 * @param ACC Array of accelerations
	 * @param Torque Array of torque limits
	 */
	void SyncWriteSpe(u8 ID[], u8 IDN, s16 Speed[], u8 ACC[], u16 Torque[]);

	/**
	 * @brief Set servo to position control mode (mode 0)
	 * @param ID Servo ID
	 * @return 1 on success, 0 on failure
	 */
	int ServoMode(u8 ID);

	/**
	 * @brief Set servo to constant velocity mode (mode 1)
	 * @param ID Servo ID
	 * @return 1 on success, 0 on failure
	 */
	int WheelMode(u8 ID);

	/**
	 * @brief Set servo to constant torque/force mode (mode 2)
	 * @param ID Servo ID
	 * @return 1 on success, 0 on failure
	 */
	int EleMode(u8 ID);

	/**
	 * @brief Write speed command for constant velocity mode
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @param Speed Target velocity (±3400 steps/s, negative=reverse)
	 * @param ACC Acceleration (0-254)
	 * @param Torque Torque limit (0-1000)
	 * @return 1 on success, 0 on failure
	 */
	int WriteSpe(u8 ID, s16 Speed, u8 ACC = 0, u16 Torque = 0);

	/**
	 * @brief Write torque command for electric/force mode
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @param Torque Target torque (±1000, negative=CCW, positive=CW)
	 * @return 1 on success, 0 on failure
	 */
	int WriteEle(u8 ID, s16 Torque);

	/**
	 * @brief Enable or disable motor torque
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @param Enable 1=enable, 0=disable
	 * @return 1 on success, 0 on failure
	 */
	int EnableTorque(u8 ID, u8 Enable);

	/**
	 * @brief Unlock EEPROM for writing
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @return 1 on success, 0 on failure
	 */
	int unLockEprom(u8 ID);

	/**
	 * @brief Lock EEPROM to prevent writes
	 * @param ID Servo ID (0-253, 254=broadcast)
	 * @return 1 on success, 0 on failure
	 */
	int LockEprom(u8 ID);

	/**
	 * @brief Calibrate servo center position
	 * @param ID Servo ID (0-253)
	 * @return 1 on success, 0 on failure
	 */
	int CalibrationOfs(u8 ID);

	/**
	 * @brief Request all feedback data from servo
	 * @param ID Servo ID (0-253, not broadcast)
	 * @return Size of data on success, -1 on failure
	 */
	int FeedBack(int ID);

	/**
	 * @brief Read current position
	 * @param ID Servo ID or -1 for cached data
	 * @return Position (0-4095) or -1 on error
	 */
	int ReadPos(int ID);

	/**
	 * @brief Read current speed
	 * @param ID Servo ID or -1 for cached data
	 * @return Speed in steps/s (signed) or -1 on error
	 */
	int ReadSpeed(int ID);

	/**
	 * @brief Read current load torque
	 * @param ID Servo ID or -1 for cached data
	 * @return Load (-1000 to +1000) or -1 on error
	 */
	int ReadLoad(int ID);

	/**
	 * @brief Read input voltage
	 * @param ID Servo ID or -1 for cached data
	 * @return Voltage in 0.1V units (e.g., 120=12.0V) or -1 on error
	 */
	int ReadVoltage(int ID);

	/**
	 * @brief Read servo temperature
	 * @param ID Servo ID or -1 for cached data
	 * @return Temperature in °C or -1 on error
	 */
	int ReadTemper(int ID);

	/**
	 * @brief Read movement status
	 * @param ID Servo ID or -1 for cached data
	 * @return 1=moving, 0=stopped, -1=error
	 */
	int ReadMove(int ID);

	/**
	 * @brief Read current draw
	 * @param ID Servo ID or -1 for cached data
	 * @return Current in mA or -1 on error
	 */
	int ReadCurrent(int ID);

private:
	u8 Mem[HLSCL_PRESENT_CURRENT_H-HLSCL_PRESENT_POSITION_L+1];
};

#endif
