/**
 * @file ProgramEprom.cpp
 * @brief EEPROM programming example for changing servo ID on HLSCL protocol servos
 *
 * @details
 * This example demonstrates how to write configuration parameters to servo EEPROM,
 * specifically changing the servo ID from 1 to 2. EEPROM parameters persist across
 * power cycles and include settings like ID, baud rate, position limits, and operating mode.
 *
 * Hardware Requirements:
 * - Feetech HLS series servo (current ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 * - **IMPORTANT**: Only ONE servo should be connected when changing IDs
 *
 * Key Features Demonstrated:
 * - unLockEprom(): Enable EEPROM write access
 * - writeByte(): Write single byte to EEPROM memory address
 * - LockEprom(): Disable EEPROM write access (protection)
 * - EEPROM parameter programming workflow
 *
 * Usage:
 * @code
 * ./ProgramEprom /dev/ttyUSB0
 * # After running, servo ID changes from 1 to 2
 * # Power cycle servo to fully apply new ID
 * @endcode
 *
 * EEPROM Programming Workflow:
 * 1. Unlock EEPROM (enable write access)
 * 2. Write parameter to EEPROM address using old ID
 * 3. Lock EEPROM (disable write access) using NEW ID
 * 4. Changes persist across power cycles
 *
 * Common EEPROM Parameters:
 * - HLSCL_ID (address 5): Servo ID (0-253)
 * - HLSCL_BAUD_RATE (address 6): Communication speed (0-7)
 * - HLSCL_MODE (address 33): Operating mode (0=servo, 1=wheel, 2=force)
 * - HLSCL_MIN_ANGLE_LIMIT_L/H: Minimum position limit
 * - HLSCL_MAX_ANGLE_LIMIT_L/H: Maximum position limit
 *
 * @note After changing ID, use the NEW ID for subsequent commands (including LockEprom).
 *       The example correctly calls LockEprom(2) instead of LockEprom(1).
 *
 * @note EEPROM changes may require power cycling the servo to fully take effect.
 *       Communication with the old ID may still work briefly.
 *
 * @warning EEPROM has limited write cycles (~100,000 writes). Do not write to
 *          EEPROM in loops or frequently. Always Lock after writing.
 *
 * @warning When changing ID, ensure only ONE servo is connected. Multiple servos
 *          with the same ID will cause bus collisions.
 *
 * @warning Always lock EEPROM after programming to prevent accidental writes.
 *          Unlocked EEPROM can be corrupted by stray commands.
 *
 * @see HLSCL::unLockEprom()
 * @see HLSCL::LockEprom()
 * @see HLSCL::writeByte()
 * @see HLSCL::writeWord() for 16-bit parameters
 * @see HLSCL.h memory table definitions
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

	// Enable EEPROM write access
	hlscl.unLockEprom(1);
	std::cout<<"unLock Eprom"<<std::endl;

	// Write new ID value to EEPROM (change ID from 1 to 2)
	hlscl.writeByte(1, HLSCL_ID, 2);
	std::cout<<"write ID:"<<2<<std::endl;

	// Disable EEPROM write access (use NEW ID=2)
	hlscl.LockEprom(2);
	std::cout<<"Lock Eprom"<<std::endl;

	hlscl.end();
	return 1;
}

