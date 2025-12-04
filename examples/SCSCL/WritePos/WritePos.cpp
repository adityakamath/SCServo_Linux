/**
 * @file WritePos.cpp
 * @brief Example: Write position commands to SCSCL servo
 * 
 * @details Demonstrates basic position control using the SCSCL servo class.
 * This example moves a servo back and forth between two positions using the
 * WritePos() function with time and speed parameters.
 *
 * **Hardware Requirements:**
 * - SCSCL series servo motor
 * - Serial port connection (USB-to-TTL or direct UART)
 * - Proper power supply for servo (typically 6-12V)
 *
 * **Key Features Demonstrated:**
 * - Serial port initialization at 115200 baud
 * - Position control with speed parameter
 * - Timing calculation: [(P1-P0)/V]*1000+100 ms
 * - Continuous motion loop
 *
 * **Usage:**
 * @code{.sh}
 * ./WritePos /dev/ttyUSB0
 * @endcode
 *
 * **Motion Pattern:**
 * - Move to position 1000 at 1500 steps/sec
 * - Wait for motion to complete (754ms)
 * - Move to position 20 at 1500 steps/sec  
 * - Wait for motion to complete (754ms)
 * - Repeat indefinitely
 *
 * @note Factory speed unit is 0.0146 rpm, example uses V=1500 steps/sec
 * @note Servo ID is hardcoded to 1
 * @see SCSCL::WritePos() for function details
 */

#include <iostream>
#include "SCServo.h"

SCSCL sc;  ///< SCSCL servo controller instance

/**
 * @brief Main function - demonstrates position control
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
		sc.WritePos(1, 1000, 0, 1500);//舵机(ID1)以最高速度V=1500步/秒,运行至P1=1000
		std::cout<<"pos = "<<1000<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
  
		sc.WritePos(1, 20, 0, 1500);//舵机(ID1)以最高V=1500步/秒,运行至P1=20
		std::cout<<"pos = "<<20<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
	}
	sc.end();
	return 1;
}

