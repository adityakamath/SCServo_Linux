/**
 * @file MidpointCalib.cpp
 * @brief Multi-servo midpoint calibration utility for mechanical alignment
 * 
 * @details
 * This utility performs batch center position calibration on multiple SMS/STS servos.
 * It's designed for robotics applications where multiple servos need to be aligned to
 * a common mechanical reference frame (e.g., robot joints at home position). The utility
 * initializes each servo in position mode, then sets the current physical position as
 * the logical midpoint (2048 for 12-bit servos, representing π radians or 180°).
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models
 * - Serial connection at 1000000 baud (1Mbps for STS)
 * - Servos must be manually positioned at desired center/home before running
 * 
 * Key Features Demonstrated:
 * - Batch servo initialization in position mode
 * - Multi-servo calibration in single operation
 * - CalibrationOfs(): Set current position as midpoint (2048)
 * - Mode switching sequence for proper calibration
 * - EEPROM persistence of calibration data
 * 
 * Usage:
 * @code
 * # STEP 1: Manually position all servos (IDs 7, 8, 9) to desired home/center
 * # STEP 2: Run calibration utility
 * ./MidpointCalib /dev/ttyUSB0
 * # Output: "Calibration complete!"
 * # STEP 3: Power cycle servos (optional but recommended)
 * # Result: Position 2048 now corresponds to the physical positions from STEP 1
 * @endcode
 * 
 * Calibration Workflow:
 * 1. For each servo (IDs 7, 8, 9):
 *    a. InitMotor(ID, mode=0, torque=1) - Switch to position mode with torque
 *    b. Wait 100ms for mode change to stabilize
 *    c. CalibrationOfs(ID) - Set current position as midpoint (2048)
 *    d. Offset written to EEPROM (persists across power cycles)
 * 2. Display "Calibration complete!"
 * 3. All future position commands are relative to new midpoint
 * 
 * Position Mapping After Calibration:
 * - Position 0: One full rotation CCW from calibrated midpoint
 * - Position 2048: Calibrated physical position (new center/home)
 * - Position 4095: One full rotation CW from calibrated midpoint
 * - Total range: ~360° centered on calibrated position
 * 
 * Typical Use Cases:
 * - Robot arm joint calibration (align all joints to home position)
 * - Multi-axis gimbal alignment
 * - Production line servo configuration
 * - Field service recalibration after mechanical repairs
 * - Compensating for assembly tolerances in multi-servo systems
 * 
 * @note This utility modifies EEPROM on all three servos. EEPROM has ~100,000 write
 *       cycle endurance - use for configuration/calibration, not frequent updates.
 * 
 * @note Position 2048 represents π radians in angular coordinate systems, making it
 *       a natural choice for center position in mathematical models.
 * 
 * @warning Ensure ALL servos are at their desired home positions BEFORE running.
 *          Calibration cannot be undone except by recalibrating or factory reset.
 * 
 * @warning Servos must be powered and holding position during calibration. Unpowered
 *          or free-moving servos will result in incorrect calibration.
 * 
 * @warning After calibration, all previous position values are offset. Update your
 *          control software or recalibrate with known reference positions.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::CalibrationOfs()
 * @see CalibrationOfs.cpp for single-servo calibration example
 */
#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {7, 8, 9};

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<< "argc error!"<<std::endl;
        return 0;
	}
    
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(1000000, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

    for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
        // Initialize motor in position mode with torque enabled
        sm_st.InitMotor(ID[i], 0, 1);
        usleep(100000); // Wait for mode change to take effect
        sm_st.CalibrationOfs(ID[i]); //set starting position as midpoint: 2048 (=Pi radians)
    }

	std::cout<<"Calibration complete!"<<std::endl;
	sm_st.end();
	return 1;
}

