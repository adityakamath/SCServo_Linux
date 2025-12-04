/**
 * @file WriteSpe.cpp
 * @brief Continuous rotation velocity control (wheel mode) for SMS/STS protocol servos
 * 
 * @details
 * This example demonstrates wheel mode operation where the servo operates as a continuous
 * rotation motor rather than a position-controlled servo. The servo is initialized in
 * velocity mode and then commanded to rotate forward, stop, and reverse in a continuous
 * cycle with controlled acceleration profiles.
 * 
 * Hardware Requirements:
 * - Feetech SMS or STS protocol servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 6-12V)
 * - Serial connection at 115200 baud
 * - Servo must support velocity mode (wheel mode)
 * 
 * Key Features Demonstrated:
 * - InitMotor(): Configure servo for velocity mode operation
 * - WriteSpe(): Velocity command with acceleration control
 * - Bidirectional rotation: Forward (+V) and reverse (-V)
 * - Controlled stops with deceleration
 * - Velocity range: -2400 to +2400 steps/second
 * - Acceleration control: A=50 (50×100 steps/second²)
 * 
 * Usage:
 * @code
 * ./WriteSpe /dev/ttyUSB0
 * @endcode
 * 
 * Motion Sequence:
 * 1. Initialize servo in velocity mode with torque enabled
 * 2. Rotate forward @ +2400 steps/s for 2 seconds
 * 3. Stop (V=0) with deceleration for 2 seconds
 * 4. Rotate reverse @ -2400 steps/s for 2 seconds
 * 5. Stop (V=0) with deceleration for 2 seconds
 * 6. Repeat cycle indefinitely
 * 
 * Parameter Details:
 * - InitMotor(ID, mode, torque): mode=1 for velocity mode, torque=1 to enable
 * - WriteSpe(ID, velocity, accel): velocity in steps/s, accel in units of 100 steps/s²
 * - Positive velocity: Counter-clockwise rotation
 * - Negative velocity: Clockwise rotation
 * - Zero velocity: Controlled stop with deceleration
 * 
 * @note Factory servo speed unit is 0.0146rpm per step. V=2400 corresponds to
 *       approximately 35rpm at maximum velocity.
 * 
 * @warning Servo must be in velocity mode for this example to work correctly.
 *          Position mode servos will not respond to velocity commands.
 * 
 * @warning Ensure mechanical load is appropriate for continuous rotation.
 *          Excessive load can cause overheating or motor damage.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::WriteSpe()
 * @see SMS_STS::begin()
 */
/*
Factory speed unit of servo is 0.0146rpm, speed changed to V=2400
*/


#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }
	// Initialize motor in velocity mode with torque enabled
	int init_ret = sm_st.InitMotor(1, 1, 1);
	std::cout << "Initialize motor 1 in velocity mode (ret=" << init_ret << ")" << std::endl;
	usleep(100000); // Wait for mode change to take effect
	while(1){
		sm_st.WriteSpe(1, 2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), rotate
		std::cout<<"speed = "<<2400<<std::endl;
		sleep(2);
		sm_st.WriteSpe(1, 0, 50);//Servo (ID1) with acceleration A=50 (50*100 steps/second^2), stop rotating (V=0)
		std::cout<<"speed = "<<0<<std::endl;
		sleep(2);
		sm_st.WriteSpe(1, -2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), rotate in reverse
		std::cout<<"speed = "<<-2400<<std::endl;
		sleep(2);
		sm_st.WriteSpe(1, 0, 50);//Servo (ID1) with acceleration A=50 (50*100 steps/second^2), stop rotating (V=0)
		std::cout<<"speed = "<<0<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

