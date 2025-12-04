/**
 * @file ProgramEprom.cpp
 * @brief Multi-parameter persistent EEPROM programming for SCSCL protocol servos
 * 
 * @details
 * This example demonstrates comprehensive EEPROM parameter programming for Feetech
 * SCSCL servos, including servo ID change and angle limit configuration. The procedure
 * shows how to write multiple parameters in a single EEPROM unlock session, which is
 * more efficient and safer than multiple unlock/lock cycles. This is essential for
 * factory configuration, servo customization, and production setup.
 * 
 * Hardware Requirements:
 * - Feetech SCSCL protocol servo (ID: 1, will be changed to 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model
 * - Serial connection at 115200 baud
 * - Stable, uninterrupted power during EEPROM write operations
 * 
 * Key Features Demonstrated:
 * - unLockEeprom(): Disable EEPROM write protection
 * - writeByte(): Write single-byte parameters (servo ID)
 * - writeWord(): Write two-byte parameters (angle limits)
 * - Multiple parameter writes in single unlock session
 * - LockEeprom(): Re-enable EEPROM write protection
 * - Angle limit configuration for restricted motion range
 * 
 * Usage:
 * @code
 * # Servo ID=1 will be changed to ID=2 with angle limits [20, 1000]
 * ./ProgramEprom /dev/ttyUSB0
 * # After running:
 * # - Servo responds to ID=2
 * # - Motion range restricted to 20-1000 (out of 0-1023)
 * # - Configuration persists across power cycles
 * @endcode
 * 
 * EEPROM Write Procedure (CRITICAL):
 * 1. unLockEeprom(1) - Disable write protection for servo ID=1
 * 2. writeByte(1, SCSCL_ID, 2) - Change servo ID from 1 to 2
 * 3. writeWord(2, MIN_ANGLE, 20) - Set min angle limit (use new ID)
 * 4. writeWord(2, MAX_ANGLE, 1000) - Set max angle limit
 * 5. LockEeprom(2) - Re-enable write protection (use new ID)
 * 
 * Configuration Details:
 * - Servo ID: Changed from 1 to 2
 * - Min Angle Limit: 20 (~7° from zero, prevents over-extension)
 * - Max Angle Limit: 1000 (~344° from zero, ~95% of full range)
 * - Range: 980 positions (~337° usable rotation)
 * 
 * Common SCSCL EEPROM Parameters:
 * - SCSCL_ID: Servo ID (1-253)
 * - SCSCL_MIN_ANGLE_LIMIT_L: Minimum position limit (0-1023)
 * - SCSCL_MAX_ANGLE_LIMIT_L: Maximum position limit (0-1023)
 * - SCSCL_MAX_TEMPERATURE: Temperature shutdown limit
 * - SCSCL_MAX_VOLTAGE: Maximum voltage limit
 * - SCSCL_MIN_VOLTAGE: Minimum voltage limit
 * 
 * @note SCSCL servos use 10-bit position resolution (0-1023) vs SMS/STS 12-bit (0-4095).
 * 
 * @note EEPROM has limited write endurance (~100,000 cycles). Use for configuration
 *       only, never in control loops or frequent updates.
 * 
 * @warning ALWAYS lock EEPROM after programming. Unlocked EEPROM is vulnerable to
 *          corruption from electrical noise or spurious commands.
 * 
 * @warning After changing servo ID with writeByte(), all subsequent commands must
 *          use the NEW ID (2 in this example), not the old ID.
 * 
 * @warning Angle limits are enforced by servo firmware. Commands outside limits will
 *          be clamped to [min, max] range. Test limits before deploying to production.
 * 
 * @warning Ensure stable power during EEPROM writes. Power loss can corrupt configuration,
 *          potentially requiring factory reset or firmware reflash.
 * 
 * @see SCSCL::unLockEeprom()
 * @see SCSCL::LockEeprom()
 * @see SCSCL::writeByte()
 * @see SCSCL::writeWord()
 */
#include <iostream>
#include "SCServo.h"

SCSCL sc;

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

	sc.unLockEeprom(1);// Enable EEPROM save function
	std::cout<<"unLock Eeprom"<<std::endl;
	sc.writeByte(1, SCSCL_ID, 2);//ID
	std::cout<<"write ID:"<<2<<std::endl;
	sc.writeWord(2, SCSCL_MIN_ANGLE_LIMIT_L, 20);
	std::cout<<"write min angle limit:"<<20<<std::endl;
	sc.writeWord(2, SCSCL_MAX_ANGLE_LIMIT_L, 1000);
	std::cout<<"write max angle limit:"<<1000<<std::endl;
	sc.LockEeprom(2);// Disable EEPROM save function
	std::cout<<"Lock Eeprom"<<std::endl;
	sc.end();
	return 1;
}

