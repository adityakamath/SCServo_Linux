/**
 * @file SCServo.h
 * @brief Master include file for Feetech Serial Servo Library
 *
 * @details This is the main header file that includes all servo series interfaces.
 * Include this single file to access all Feetech servo series classes.
 *
 * **Supported Servo Series:**
 * - SMS_STS: SMS and STS series (3 operating modes: servo, wheel closed-loop, wheel open-loop)
 * - SCSCL: SCSCL series (position control and PWM mode)
 * - HLSCL: HLS series (servo, wheel, and force control modes)
 *
 * **Usage:**
 * @code
 * #include "SCServo.h"
 *
 * SMS_STS servo;
 * servo.begin(1000000, "/dev/ttyUSB0");
 * servo.InitMotor(1, 0, 1);  // ID 1, servo mode, enable torque
 * servo.WritePosEx(1, 2048, 1000, 50);  // Move to position 2048
 * @endcode
 *
 * @note This file only includes headers; link against libSCServo.a for implementations
 * @see SMS_STS.h, SCSCL.h, HLSCL.h for protocol documentation
 */

#ifndef _SCSERVO_H
#define _SCSERVO_H

#include "SCSCL.h"
#include "SMS_STS.h"
#include "HLSCL.h"
#endif