/**
 * @file Ping.cpp
 * @brief Example: Ping servo to test connectivity
 * 
 * @details Demonstrates the PING command to check if a servo with a specific ID
 * is connected and responsive on the serial bus. This is useful for:
 * - Verifying servo connectivity before operation
 * - Discovering servo IDs on the bus
 * - Troubleshooting communication issues
 *
 * **Hardware Requirements:**
 * - SCSCL series servo motor
 * - Serial port connection
 * - Servo ID must be known (default: ID=1)
 *
 * **Key Features Demonstrated:**
 * - Serial port initialization
 * - PING command usage
 * - Error detection and reporting
 *
 * **Usage:**
 * @code{.sh}
 * ./Ping /dev/ttyUSB0
 * @endcode
 *
 * **Expected Output:**
 * - Success: "ID:1" (or whichever ID responded)
 * - Failure: "Ping servo ID error!"
 *
 * @note Broadcast PING (ID=0xFE) only works when a single servo is on the bus
 * @note This example tests servo ID=1
 * @see SCSCL::Ping() for function details
 */

#include <iostream>
#include "SCServo.h"

SCSCL sc;  ///< SCSCL servo controller instance

/**
 * @brief Main function - tests servo connectivity with PING command
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
	int ID = sc.Ping(1);
	if(ID!=-1){
		std::cout<<"ID:"<<ID<<std::endl;
	}else{
		std::cout<<"Ping servo ID error!"<<std::endl;
	}
	sc.end();
	return 1;
}
