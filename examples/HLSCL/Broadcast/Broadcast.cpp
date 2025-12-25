/**
 * @file Broadcast.cpp
 * @brief Broadcast command example for controlling multiple HLSCL servos simultaneously
 *
 * @details
 * This example demonstrates the broadcast feature where a single command controls all servos
 * on the serial bus at once. Using broadcast ID (0xFE = 254), all connected servos execute
 * the same position command synchronously without individual addressing.
 *
 * Hardware Requirements:
 * - One or more Feetech HLS series servos (any IDs)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 * - All servos connected to same serial bus (daisy-chained)
 *
 * Key Features Demonstrated:
 * - Broadcast ID (0xFE): Command all servos simultaneously
 * - Synchronized motion: All servos move together
 * - Efficient bus usage: Single packet controls multiple servos
 * - Position control with speed, acceleration, and torque
 *
 * Usage:
 * @code
 * ./Broadcast /dev/ttyUSB0
 * # All connected servos will move in unison
 * @endcode
 *
 * Motion Sequence:
 * 1. Broadcast command to move all servos to position 4095
 * 2. Wait for motion to complete
 * 3. Broadcast command to move all servos to position 0
 * 4. Wait for motion to complete
 * 5. Repeat cycle indefinitely
 *
 * Parameter Details:
 * - ID: 0xFE (254) = Broadcast to all servos on bus
 * - Position: P=4095 (maximum) and P=0 (minimum)
 * - Speed: V=60 steps/s (60 × 0.732 = 43.92 RPM)
 * - Acceleration: A=50 (50 × 8.7 = 435 deg/s²)
 * - Torque limit: T=500 (500 × 6.5 = 3250 mA max current)
 *
 * Advantages of Broadcast:
 * - Perfect synchronization: All servos start moving at exactly the same time
 * - Reduced bus traffic: One packet instead of N individual commands
 * - Simplified code: No loops needed for multiple servos
 * - Lower latency: Faster than sequential individual commands
 *
 * @note Broadcast commands do not generate response packets from servos.
 *       This is normal behavior and reduces bus collisions.
 *
 * @note All servos receive the same position target. For different positions
 *       per servo, use SyncWritePosEx() instead of broadcast.
 *
 * @warning Ensure all servos have adequate clearance for the commanded positions.
 *          Mechanical collisions can damage motors or mechanisms.
 *
 * @warning Cannot read feedback from broadcast commands. Use individual IDs
 *          or FeedBack() with specific servo IDs for status monitoring.
 *
 * @see HLSCL::WritePosEx()
 * @see HLSCL::SyncWritePosEx() for different positions per servo
 * @see HLSCL::begin()
 */

#include <iostream>
#include "SCServo.h"

HLSCL hlscl;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!hlscl.begin(115200, argv[1])){
        std::cout<<"Failed to init HLS motor!"<<std::endl;
        return 0;
    }
	while(1){
		// Servo (broadcast 0xFE) with max speed V=60×0.732=43.92rpm, acceleration A=50×8.7deg/s²,
		// max torque current T=500×6.5=3250mA, move to P1=4095 position
		hlscl.WritePosEx(0xfe, 4095, 60, 50, 500);
		std::cout<<"pos = "<<4095<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);

		// Servo (broadcast 0xFE) with max speed V=60×0.732=43.92rpm, acceleration A=50×8.7deg/s²,
		// max torque current T=500×6.5=3250mA, move to P0=0 position
		hlscl.WritePosEx(0xfe, 0, 60, 50, 500);
		std::cout<<"pos = "<<0<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);
	}
	hlscl.end();
	return 1;
}

