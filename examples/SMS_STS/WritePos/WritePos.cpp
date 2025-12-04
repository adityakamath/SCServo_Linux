/**
 * @file WritePos.cpp
 * @brief Position control with velocity and acceleration for SMS/STS protocol servos
 * 
 * @details
 * This example demonstrates smooth position control with configurable velocity and
 * acceleration profiles using the WritePosEx() command. The servo oscillates between
 * two positions (0 and 4095) with controlled motion parameters, showcasing trapezoidal
 * velocity profile generation for smooth, predictable motion.
 * 
 * Hardware Requirements:
 * - Feetech SMS or STS protocol servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 6-12V)
 * - Serial connection at 1000000 baud (1Mbps)
 * 
 * Key Features Demonstrated:
 * - WritePosEx(): Extended position write with velocity and acceleration
 * - Position range: 0 to 4095 (12-bit resolution, ~360° rotation)
 * - Velocity control: V=2400 steps/second
 * - Acceleration control: A=50 (50×100 steps/second²)
 * - Motion timing calculation: [(P1-P0)/V]*1000 + [V/(A*100)]*1000 milliseconds
 * - Continuous oscillation for testing and demonstration
 * 
 * Usage:
 * @code
 * ./WritePos /dev/ttyUSB0
 * @endcode
 * 
 * Motion Profile:
 * - Position 0 → 4095 (full rotation) @ 2400 steps/s with 5000 steps/s² acceleration
 * - Wait 2187ms for motion completion (includes acceleration ramp time)
 * - Position 4095 → 0 (return) @ 2400 steps/s with 5000 steps/s² acceleration
 * - Wait 2187ms for motion completion
 * - Repeat indefinitely
 * 
 * Parameter Details:
 * - Position: 0-4095 (0° to ~360° for most servos)
 * - Velocity: 2400 steps/second (adjustable based on servo capability)
 * - Acceleration: 50 (units of 100 steps/second², so 5000 steps/s²)
 * 
 * @note Factory servo speed unit is 0.0146rpm per step. This example uses V=2400
 *       which corresponds to approximately 35rpm at maximum velocity.
 * 
 * @warning Ensure adequate power supply for servo load during acceleration phases.
 *          Insufficient power can cause position errors or servo resets.
 * 
 * @see SMS_STS::WritePosEx()
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
    if(!sm_st.begin(1000000, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }
	while(1){
		sm_st.WritePosEx(1, 4095, 2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P1=4095
		std::cout<<"pos = "<<4095<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
  
		sm_st.WritePosEx(1, 0, 2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P0=0
		std::cout<<"pos = "<<0<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

