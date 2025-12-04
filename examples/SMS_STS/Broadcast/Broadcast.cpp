/**
 * @file Broadcast.cpp
 * @brief Example: Broadcast position commands to all SMS/STS servos
 * 
 * @details Demonstrates the broadcast feature (ID=0xFE) to control multiple
 * SMS/STS servos simultaneously with a single command. Uses WritePosEx which
 * includes speed and acceleration parameters for smooth motion planning.
 *
 * **Hardware Requirements:**
 * - Multiple SMS/STS series servo motors
 * - All servos connected to same serial bus
 * - Each servo must have unique ID (but all respond to broadcast ID 0xFE)
 *
 * **Key Features Demonstrated:**
 * - Broadcast ID usage (0xFE)
 * - Simultaneous multi-servo control  
 * - Position control with speed and acceleration
 * - Timing calculation: [(P1-P0)/V]*1000 + [V/(A*100)]*1000 ms
 *
 * **Usage:**
 * @code{.sh}
 * ./Broadcast /dev/ttyUSB0
 * @endcode
 *
 * **Motion Pattern:**
 * All servos move together:
 * - To position 4095 at 2400 steps/sec with 50 accel
 * - Back to position 0 at 2400 steps/sec with 50 accel
 * - Repeat indefinitely
 *
 * @warning Broadcast commands receive no acknowledgment from servos
 * @note Factory speed unit is 0.0146 rpm, example uses V=2400 steps/sec
 * @see SMS_STS::WritePosEx() for function details
 */

#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;  ///< SMS_STS servo controller instance

/**
 * @brief Main function - broadcasts position commands to all servos
 * @param argc Argument count (must be 2)
 * @param argv Argument vector [program_name, serial_port]
 * @return 1 on success, 0 on error
 */
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
	while(1){
		sm_st.WritePosEx(0xfe, 4095, 2400, 50);//Servo (broadcast) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P1=4095
		std::cout<<"pos = "<<4095<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
  
		sm_st.WritePosEx(0xfe, 0, 2400, 50);//Servo (broadcast) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P0=0
		std::cout<<"pos = "<<0<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

