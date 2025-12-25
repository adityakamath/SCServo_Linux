/**
 * @file WriteSpe.cpp
 * @brief Continuous rotation velocity control (wheel mode) for HLSCL protocol servos
 *
 * @details
 * This example demonstrates wheel mode operation where the HLS servo operates as a continuous
 * rotation motor with constant velocity control. The servo is configured in wheel mode and
 * commanded to rotate forward, stop, and reverse in a continuous cycle with controlled
 * acceleration and torque limiting.
 *
 * Hardware Requirements:
 * - Feetech HLS series servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 *
 * Key Features Demonstrated:
 * - WheelMode(): Configure servo for constant velocity mode (mode 1)
 * - WriteSpe(): Velocity command with acceleration and torque control
 * - Bidirectional rotation: Forward (+V) and reverse (-V)
 * - Controlled stops with deceleration
 * - Torque limiting for safe operation
 *
 * Usage:
 * @code
 * ./WriteSpe /dev/ttyUSB0
 * @endcode
 *
 * Motion Sequence:
 * 1. Configure servo in wheel mode (constant velocity)
 * 2. Rotate forward @ 60 steps/s (~43.92 RPM) for 5 seconds
 * 3. Stop (V=0) with deceleration for 5 seconds
 * 4. Rotate reverse @ -60 steps/s for 2 seconds
 * 5. Stop (V=0) with deceleration for 2 seconds
 * 6. Repeat cycle indefinitely
 *
 * Parameter Details:
 * - Speed: V=60 steps/s (60 × 0.732 = 43.92 RPM)
 * - Acceleration: A=50 (50 × 8.7 deg/s² acceleration)
 * - Torque limit: T=500 (500 × 6.5 = 3250 mA max current)
 * - Positive velocity: Forward rotation
 * - Negative velocity: Reverse rotation
 * - Zero velocity: Controlled stop with deceleration
 *
 * @note HLS series supports torque limiting in wheel mode, protecting both
 *       the servo and mechanical system from excessive current draw.
 *
 * @warning Ensure mechanical load is appropriate for continuous rotation.
 *          Excessive load can cause overheating or motor damage.
 *
 * @see HLSCL::WheelMode()
 * @see HLSCL::WriteSpe()
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
	// Configure servo for constant velocity mode
	hlscl.WheelMode(1);
	std::cout<<"mode = 1 (wheel mode)"<<std::endl;
	while(1){
		// Servo (ID1) accelerates at A=50×8.7deg/s² to max speed V=60×0.732=43.92rpm,
		// rotates forward continuously with max torque current T=500×6.5=3250mA
		hlscl.WriteSpe(1, 60, 50, 500);
		std::cout<<"speed = 60 (forward)"<<std::endl;
		sleep(5);

		// Servo (ID1) decelerates at A=50×8.7deg/s² to speed 0, stops rotation
		hlscl.WriteSpe(1, 0, 50, 500);
		std::cout<<"speed = 0 (stop)"<<std::endl;
		sleep(5);

		// Servo (ID1) accelerates at A=50×8.7deg/s² to max speed V=-60×0.732=-43.92rpm,
		// rotates reverse continuously with max torque current T=500×6.5=3250mA
		hlscl.WriteSpe(1, -60, 50, 500);
		std::cout<<"speed = -60 (reverse)"<<std::endl;
		sleep(2);

		// Servo (ID1) decelerates at A=50×8.7deg/s² to speed 0, stops rotation
		hlscl.WriteSpe(1, 0, 50, 500);
		std::cout<<"speed = 0 (stop)"<<std::endl;
		sleep(2);
	}
	hlscl.end();
	return 1;
}
