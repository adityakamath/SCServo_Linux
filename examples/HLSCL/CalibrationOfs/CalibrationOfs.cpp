/**
 * @file CalibrationOfs.cpp
 * @brief Midpoint calibration example for HLSCL protocol servos
 *
 * @details
 * This example demonstrates how to calibrate the servo's center position (midpoint offset).
 * The CalibrationOfs() function sets the current physical position as the new center point (2048)
 * and continuously displays the calibrated position reading. This is essential for mechanical
 * alignment when installing servos in robots or mechanisms.
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
 * - Error handling with getLastError()
 *
 * Usage:
 * @code
 * # 1. Disable torque and manually position servo to desired center
 * # 2. Run calibration program
 * ./CalibrationOfs /dev/ttyUSB0
 * # 3. Servo's current position is now set as center (2048)
 * # 4. Program displays continuous position readings
 * @endcode
 *
 * Calibration Process:
 * 1. Before running: Manually position servo to desired center point
 * 2. Program calls CalibrationOfs(1) to save current position as center
 * 3. EEPROM offset register is updated and persists across power cycles
 * 4. All future position readings are relative to this new center
 * 5. Continuous loop displays calibrated position values
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
 * @see HLSCL::getLastError()
 * @see HLSCL::begin()
 */

#include <iostream>
#include "SCServo.h"

HLSCL hlscl;

int main(int argc, char **argv)
{
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
	hlscl.CalibrationOfs(1);
	std::cout<<"Calibration Ofs"<<std::endl;

	// Continuously read and display calibrated position
	while(1){
	  int pos = hlscl.ReadPos(1);
	  if(!hlscl.getLastError()){
		  std::cout<<"mid pos:"<<pos<<std::endl;
	  }
	}
	hlscl.end();
	return 1;
}

