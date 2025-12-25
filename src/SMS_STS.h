/**
 * @file SMS_STS.h
 * @brief Feetech SMS/STS Series Serial Servo Application Layer
 *
 * @details This file provides the application programming interface for
 * controlling Feetech SMS and STS series serial bus servo motors.
 * Supports three operating modes:
 * - Mode 0: Servo (position control)
 * - Mode 1: Closed-loop wheel (velocity control with feedback)
 * - Mode 2: Open-loop wheel (PWM control without feedback)
 */

#ifndef _SMS_STS_H
#define _SMS_STS_H

// Baud rate definition
#define	SMS_STS_1M 0
#define	SMS_STS_0_5M 1
#define	SMS_STS_250K 2
#define	SMS_STS_128K 3
#define	SMS_STS_115200 4
#define	SMS_STS_76800 5
#define	SMS_STS_57600 6
#define	SMS_STS_38400 7

//Memory table definition
//-------EEPROM (Read only)--------
#define SMS_STS_MODEL_L 3
#define SMS_STS_MODEL_H 4

//-------EEPROM (Read and Write)--------
#define SMS_STS_ID 5
#define SMS_STS_BAUD_RATE 6
#define SMS_STS_MIN_ANGLE_LIMIT_L 9
#define SMS_STS_MIN_ANGLE_LIMIT_H 10
#define SMS_STS_MAX_ANGLE_LIMIT_L 11
#define SMS_STS_MAX_ANGLE_LIMIT_H 12
#define SMS_STS_CW_DEAD 26
#define SMS_STS_CCW_DEAD 27
#define SMS_STS_OFS_L 31
#define SMS_STS_OFS_H 32
#define SMS_STS_MODE 33

//-------SRAM (Read and Write)--------
#define SMS_STS_TORQUE_ENABLE 40
#define SMS_STS_ACC 41
#define SMS_STS_GOAL_POSITION_L 42
#define SMS_STS_GOAL_POSITION_H 43
#define SMS_STS_GOAL_TIME_L 44
#define SMS_STS_GOAL_TIME_H 45
#define SMS_STS_GOAL_SPEED_L 46
#define SMS_STS_GOAL_SPEED_H 47
#define SMS_STS_LOCK 55

//-------SRAM (Read only)--------
#define SMS_STS_PRESENT_POSITION_L 56
#define SMS_STS_PRESENT_POSITION_H 57
#define SMS_STS_PRESENT_SPEED_L 58
#define SMS_STS_PRESENT_SPEED_H 59
#define SMS_STS_PRESENT_LOAD_L 60
#define SMS_STS_PRESENT_LOAD_H 61
#define SMS_STS_PRESENT_VOLTAGE 62
#define SMS_STS_PRESENT_TEMPERATURE 63
#define SMS_STS_MOVING 66
#define SMS_STS_PRESENT_CURRENT_L 69
#define SMS_STS_PRESENT_CURRENT_H 70

// Bit position constants for data encoding
#define SMS_STS_DIRECTION_BIT_POS 15    // Bit position for direction flag in position/speed
#define SMS_STS_LOAD_DIRECTION_BIT_POS 10  // Bit position for direction flag in load/PWM

// Operating mode values
#define SMS_STS_MODE_SERVO 0        // Servo mode (position control)
#define SMS_STS_MODE_WHEEL_CLOSED 1 // Wheel mode - closed loop (velocity control)
#define SMS_STS_MODE_WHEEL_OPEN 2   // Wheel mode - open loop (PWM control)
#define SMS_STS_MODE_STEPPER 3      // Stepper mode (not implemented in SDK)

// Special servo IDs
#define SMS_STS_BROADCAST_ID 0xFE   // Broadcast ID for all servos

// Calibration command
#define SMS_STS_CALIBRATION_CMD 128 // Command value for midpoint calibration

#include "SCSerial.h"
#include "INST.h"
#include "ServoErrors.h"
#include "ServoUtils.h"

/**
 * @class SMS_STS
 * @brief Application layer interface for SMS/STS series serial servos
 *
 * @details Provides high-level control functions for Feetech SMS and STS series
 * servo motors. Supports three operating modes with complete read/write functionality.
 *
 * **Operating Modes:**
 * - Mode 0: Servo mode (position control) - precise positioning
 * - Mode 1: Wheel mode closed-loop (velocity control) - speed feedback
 * - Mode 2: Wheel mode open-loop (PWM control) - direct motor power
 *
 * **Key Features:**
 * - Write operations: immediate, asynchronous (Reg), and synchronized (Sync)
 * - Comprehensive feedback: position, speed, load, voltage, temperature, current
 * - EEPROM management: lock/unlock for persistent configuration
 * - LSP compliant: uniform InitMotor() and Mode() methods
 *
 * **Usage Example:**
 * @code
 * SMS_STS servo;
 * servo.begin(1000000, "/dev/ttyUSB0");
 * servo.InitMotor(1, 0, 1);  // ID=1, Mode=0 (servo), Enable torque
 * servo.WritePosEx(1, 2048, 1000, 50);  // Move to center position
 * @endcode
 *
 * @note Remember to call begin() before using any servo methods
 * @see SCSerial for serial communication layer methods
 */
class SMS_STS : public SCSerial
{
public:
	/** @brief Default constructor */
	SMS_STS();
	/** @brief Constructor with protocol end byte
	 *  @param End Protocol end byte (0 or 1) */
	SMS_STS(u8 End);
	/** @brief Constructor with protocol end byte and response level
	 *  @param End Protocol end byte (0 or 1)
	 *  @param Level Response level (0=no response, 1=response enabled) */
	SMS_STS(u8 End, u8 Level);
	/** @brief Write position to single servo (Mode 0)
	 *  @param ID Servo ID (0-253, 254=broadcast)
	 *  @param Position Target position (0-4095 steps)
	 *  @param Speed Movement speed (0-3400 steps/s)
	 *  @param ACC Acceleration (0-254, units of 100 steps/s²)
	 *  @return 1 on success, 0 on failure */
	virtual int WritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC = 0);

	/** @brief Async write position (execute with RegWriteAction)
	 *  @param ID Servo ID
	 *  @param Position Target position
	 *  @param Speed Movement speed
	 *  @param ACC Acceleration
	 *  @return 1 on success, 0 on failure */
	virtual int RegWritePosEx(u8 ID, s16 Position, u16 Speed, u8 ACC = 0);

	/** @brief Sync write position to multiple servos
	 *  @param ID Array of servo IDs
	 *  @param IDN Number of servos
	 *  @param Position Array of target positions
	 *  @param Speed Array of speeds (NULL for 0)
	 *  @param ACC Array of accelerations (NULL for 0) */
	virtual void SyncWritePosEx(u8 ID[], u8 IDN, s16 Position[], u16 Speed[], u8 ACC[]);

	/** @brief Set operating mode
	 *  @param ID Servo ID
	 *  @param mode 0=servo, 1=wheel closed-loop, 2=wheel open-loop
	 *  @return 1 on success, 0 on failure */
	virtual int Mode(u8 ID, u8 mode);

	/** @brief Initialize motor (unlock EEPROM → set mode → lock EEPROM → enable torque)
	 *  @param ID Servo ID
	 *  @param mode Operating mode (0/1/2)
	 *  @param enableTorque 1=enable, 0=disable (default: 1)
	 *  @return 1 on success, 0 on failure
	 *  @note LSP compliant - available on all servo classes */
	virtual int InitMotor(u8 ID, u8 mode, u8 enableTorque = 1);

	/** @brief Write speed to single servo (Mode 1)
	 *  @param ID Servo ID
	 *  @param Speed Target speed (-3400 to +3400 steps/s)
	 *  @param ACC Acceleration
	 *  @return 1 on success, 0 on failure */
	virtual int WriteSpe(u8 ID, s16 Speed, u8 ACC = 0);

	/** @brief Async write speed */
    virtual int RegWriteSpe(u8 ID, s16 Speed, u8 ACC = 0);

	/** @brief Sync write speed to multiple servos */
    virtual void SyncWriteSpe(u8 ID[], u8 IDN, s16 Speed[], u8 ACC[]);

	/** @brief Write PWM to single servo (Mode 2)
	 *  @param ID Servo ID
	 *  @param Pwm PWM duty cycle (±1000 = ±100%)
	 *  @return 1 on success, 0 on failure */
    virtual int WritePwm(u8 ID, s16 Pwm);

	/** @brief Async write PWM */
    virtual int RegWritePwm(u8 ID, s16 Pwm);

	/** @brief Sync write PWM to multiple servos */
    virtual void SyncWritePwm(u8 ID[], u8 IDN, s16 Pwm[]);

	/** @brief Enable/disable motor torque
	 *  @param ID Servo ID
	 *  @param Enable 1=enable, 0=disable (free-moving)
	 *  @return 1 on success, 0 on failure */
	virtual int EnableTorque(u8 ID, u8 Enable);

	/** @brief Unlock EEPROM for writing configuration
	 *  @param ID Servo ID
	 *  @return 1 on success, 0 on failure */
	virtual int unLockEeprom(u8 ID);

	/** @brief Lock EEPROM to protect configuration
	 *  @param ID Servo ID
	 *  @return 1 on success, 0 on failure */
	virtual int LockEeprom(u8 ID);

	/** @brief Calibrate servo midpoint position
	 *  @param ID Servo ID
	 *  @return 1 on success, 0 on failure
	 *  @note Sets offset register to 128 (different from InitMotor which sets operating mode) */
	virtual int CalibrationOfs(u8 ID);

	/** @brief Read all feedback data into internal buffer
	 *  @param ID Servo ID
	 *  @return 1 on success, 0 on failure
	 *  @note Call before using Read* methods with ID=-1 for cached reads */
	virtual int FeedBack(int ID);

	/** @brief Read current position
	 *  @param ID Servo ID, or -1 to read from cache (after FeedBack)
	 *  @return Position (0-4095), -1 on error */
	virtual int ReadPos(int ID);

	/** @brief Read current speed
	 *  @param ID Servo ID, or -1 for cached read
	 *  @return Speed (±3400 steps/s), -1 on error */
	virtual int ReadSpeed(int ID);

	/** @brief Read motor load
	 *  @param ID Servo ID, or -1 for cached read
	 *  @return Load (±1000 = ±100% PWM), -1 on error */
	virtual int ReadLoad(int ID);

	/** @brief Read supply voltage
	 *  @param ID Servo ID, or -1 for cached read
	 *  @return Voltage in 0.1V units, -1 on error */
	virtual int ReadVoltage(int ID);

	/** @brief Read internal temperature
	 *  @param ID Servo ID, or -1 for cached read
	 *  @return Temperature in °C, -1 on error */
	virtual int ReadTemper(int ID);

	/** @brief Read movement status
	 *  @param ID Servo ID, or -1 for cached read
	 *  @return 1=moving, 0=stopped, -1=error */
	virtual int ReadMove(int ID);

	/** @brief Read motor current
	 *  @param ID Servo ID, or -1 for cached read
	 *  @return Current in milliamps, -1 on error */
	virtual int ReadCurrent(int ID);
private:
	u8 Mem[SMS_STS_PRESENT_CURRENT_H-SMS_STS_PRESENT_POSITION_L+1];
};

#endif