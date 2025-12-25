/**
 * @file CalibrationOfs.cpp
 * @brief Midpoint calibration example for HLSCL protocol servos
 *
 * @details
 * This example demonstrates how to calibrate the servo's center position (midpoint offset).
 * The CalibrationOfs() function sets the current physical position as the new center point (2048)
 * and then continuously displays the calibrated position reading for verification. This is 
 * essential for mechanical alignment when installing servos in robots or mechanisms.
 *
 * Hardware Requirements:
 * - Feetech HLS series servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 *
 * Key Features Demonstrated:
 * - CalibrationOfs(): Set current position as center (midpoint)
 * - ReadPos(): Read calibrated position continuously
 * - EEPROM programming for persistent calibration
 * - Error handling for calibration failures
 * - Graceful shutdown on Ctrl+C
 *
 * Usage:
 * @code
 * # 1. Disable torque and manually position servo to desired center
 * # 2. Run calibration program
 * ./CalibrationOfs /dev/ttyUSB0
 * # 3. Servo's current position is now set as center (2048)
 * # 4. Program displays continuous position readings (press Ctrl+C to exit)
 * @endcode
 *
 * Calibration Process:
 * 1. Before running: Manually position servo to desired center point
 * 2. Program calls CalibrationOfs(1) to save current position as center
 * 3. EEPROM offset register is updated and persists across power cycles
 * 4. All future position readings are relative to this new center
 * 5. Continuous loop displays calibrated position values for verification
 *
 * @note The calibration writes to EEPROM and is permanent until recalibrated.
 *       The servo's physical center is now mapped to position value 2048.
 *
 * @note This function automatically handles EEPROM unlock/lock internally.
 *       No manual lock/unlock calls needed.
 *
 * @warning Calibration affects all position readings permanently.
 *          Document original calibration if you need to restore it later.
 *
 * @warning EEPROM has limited write cycles (~100,000). Do not run
 *          calibration in a continuous loop or frequently.
 *
 * @see HLSCL::CalibrationOfs()
 * @see HLSCL::ReadPos()
 * @see HLSCL::begin()
 */

#include <iostream>
#include <csignal>
#include "SCServo.h"

HLSCL hlscl;
volatile sig_atomic_t running = 1;

void signalHandler(int signum) {
	std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down..." << std::endl;
	running = 0;
}

int main(int argc, char **argv)
{
	// Register signal handler for graceful shutdown
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	
	if(argc<2){
        std::cout<< "argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!hlscl.begin(115200, argv[1])){
        std::cout<<"Failed to init HLS motor!"<<std::endl;
        return 0;
    }

	// Calibrate current position as center (midpoint offset)
	std::cout<<"Performing calibration - setting current position as center (2048)..."<<std::endl;
	int ret = hlscl.CalibrationOfs(1);
	if(ret != 1){
		std::cout<<"ERROR: CalibrationOfs failed! Return code: "<<ret<<std::endl;
		std::cout<<"Possible causes:"<<std::endl;
		std::cout<<"  - Servo ID 1 not found or not responding"<<std::endl;
		std::cout<<"  - Communication error"<<std::endl;
		std::cout<<"  - Failed to disable torque or unlock EEPROM"<<std::endl;
		hlscl.end();
		return 1;
	}
	std::cout<<"Calibration successful! Current position is now center (2048)."<<std::endl;
	std::cout<<"Displaying calibrated position readings (press Ctrl+C to exit)..."<<std::endl;
	std::cout<<std::endl;

	// Continuously read and display calibrated position
	while(running){
		int pos = hlscl.ReadPos(1);
		if(pos >= 0){
			std::cout<<"Position: "<<pos<<" (center = 2048)"<<std::endl;
		} else {
			std::cout<<"ERROR: Failed to read position"<<std::endl;
		}
		usleep(100*1000);  // 100ms interval (10Hz)
	}

	hlscl.end();
	return 0;
}

