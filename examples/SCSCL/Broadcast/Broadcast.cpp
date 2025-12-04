/**
 * @file Broadcast.cpp
 * @brief Example: Broadcast position commands to all SCSCL servos
 * 
 * @details Demonstrates the broadcast feature (ID=0xFE) to control multiple
 * servos simultaneously with a single command. All servos on the bus will
 * execute the same position command in perfect synchronization.
 *
 * **Hardware Requirements:**
 * - Multiple SCSCL series servo motors
 * - All servos connected to same serial bus
 * - Each servo must have unique ID (but all respond to broadcast ID 0xFE)
 *
 * **Key Features Demonstrated:**
 * - Broadcast ID usage (0xFE)
 * - Simultaneous multi-servo control
 * - Position control with speed parameter
 * - Timing calculation for motion completion
 *
 * **Usage:**
 * @code{.sh}
 * ./Broadcast /dev/ttyUSB0
 * @endcode
 *
 * **Motion Pattern:**
 * All servos move together:
 * - To position 1000 at 1500 steps/sec
 * - Back to position 20 at 1500 steps/sec
 * - Repeat indefinitely
 *
 * @warning Broadcast commands receive no acknowledgment from servos
 * @note Factory speed unit is 0.0146 rpm, example uses V=1500 steps/sec
 * @see SCSCL::WritePos() for function details
 */

#include <iostream>
#include "SCServo.h"

SCSCL sc;  ///< SCSCL servo controller instance

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
    if(!sc.begin(115200, argv[1])){
        std::cout<<"Failed to init scscl motor!"<<std::endl;
        return 0;
    }
	while(1){
		sc.WritePos(0xfe, 1000, 0, 1500);//舵机(ID1)以最高速度V=1500步/秒,运行至P1=1000
		std::cout<<"pos = "<<1000<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
  
		sc.WritePos(0xfe, 20, 0, 1500);//舵机(ID1)以最高V=1500步/秒,运行至P1=20
		std::cout<<"pos = "<<20<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
	}
	sc.end();
	return 1;
}

