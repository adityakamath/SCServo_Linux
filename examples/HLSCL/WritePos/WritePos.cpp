/**
 * @file WritePos.cpp
 * @brief Position control (servo mode) example for HLSCL protocol servos
 *
 * @details
 * This example demonstrates basic position control where the servo moves back and forth
 * between minimum position (0) and maximum position (4095) continuously. The servo uses
 * acceleration control for smooth motion and torque limiting for safe operation.
 *
 * Hardware Requirements:
 * - Feetech HLS series servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 *
 * Key Features Demonstrated:
 * - WritePosEx(): Extended position control with speed, acceleration, and torque
 * - Bidirectional position movement (0 ↔ 4095)
 * - Motion timing calculation for synchronized operations
 * - Torque limiting for motor protection
 *
 * Usage:
 * @code
 * ./WritePos /dev/ttyUSB0
 * @endcode
 *
 * Motion Sequence:
 * 1. Move to position 4095 (maximum) with specified speed/accel/torque
 * 2. Wait for motion to complete using calculated timing
 * 3. Move to position 0 (minimum) with same parameters
 * 4. Wait for motion to complete
 * 5. Repeat cycle indefinitely
 *
 * Parameter Details:
 * - Position: P=4095 (12-bit maximum) and P=0 (minimum)
 * - Speed: V=60 steps/s (60 × 0.732 = 43.92 RPM)
 * - Acceleration: A=50 (50 × 8.7 = 435 deg/s²)
 * - Torque limit: T=500 (500 × 6.5 = 3250 mA max current)
 *
 * Motion Timing Formula:
 * - Time = [(P1-P0)/(V×50)] + [(V×50)/(A×100)] + 50ms (error margin)
 * - This ensures program waits for motion completion before next command
 *
 * @note The timing calculation prevents issuing new commands before
 *       the servo reaches its target position.
 *
 * @warning Ensure adequate mechanical clearance for full range motion.
 *          Obstacles can cause excessive load and motor damage.
 *
 * @see HLSCL::WritePosEx()
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
		// Servo (ID1) with max speed V=60×0.732=43.92rpm, acceleration A=50×8.7deg/s²,
		// max torque current T=500×6.5=3250mA, move to P1=4095 position
		hlscl.WritePosEx(1, 4095, 60, 50, 500);
		std::cout<<"pos = "<<4095<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);

		// Servo (ID1) with max speed V=60×0.732=43.92rpm, acceleration A=50×8.7deg/s²,
		// max torque current T=500×6.5=3250mA, move to P0=0 position
		hlscl.WritePosEx(1, 0, 60, 50, 500);
		std::cout<<"pos = "<<0<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);
	}
	hlscl.end();
	return 1;
}

