/**
 * @file ProgramEprom.cpp
 * @brief Persistent EEPROM parameter programming for SMS/STS protocol servos
 * 
 * @details
 * This example demonstrates how to permanently modify servo configuration parameters
 * stored in EEPROM. The procedure involves unlocking EEPROM write protection, writing
 * the desired parameter(s), and re-locking EEPROM to prevent accidental corruption.
 * This is essential for factory configuration, servo ID changes, and limit customization.
 * 
 * Hardware Requirements:
 * - Feetech SMS or STS protocol servo (ID: 1, will be changed to 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model
 * - Serial connection at 115200 baud
 * - Stable power during EEPROM write operations
 * 
 * Key Features Demonstrated:
 * - unLockEeprom(): Disable EEPROM write protection
 * - writeByte(): Write single-byte parameters (e.g., servo ID)
 * - LockEeprom(): Re-enable EEPROM write protection
 * - Persistent configuration storage across power cycles
 * - Safe EEPROM write procedure with protection
 * 
 * Usage:
 * @code
 * # Servo with ID=1 will be changed to ID=2
 * ./ProgramEprom /dev/ttyUSB0
 * # After running, servo responds to ID=2 (persists across reboots)
 * @endcode
 * 
 * EEPROM Write Procedure (CRITICAL):
 * 1. unLockEeprom(ID) - Disable write protection for target servo
 * 2. writeByte/writeWord() - Write parameter(s) to EEPROM
 * 3. LockEeprom(NEW_ID) - Re-enable write protection (use new ID if changed)
 * 4. Power cycle servo to fully apply some changes
 * 
 * Common EEPROM Parameters:
 * - SMS_STS_ID: Servo ID (1-253, 254=broadcast)
 * - MIN/MAX_ANGLE_LIMIT: Position limits (0-4095)
 * - MAX_TEMPERATURE: Temperature shutdown limit (°C)
 * - MAX_VOLTAGE/MIN_VOLTAGE: Voltage operating range (0.1V units)
 * - BAUD_RATE: Serial communication speed
 * - RETURN_DELAY_TIME: Response delay (microseconds)
 * 
 * @note EEPROM has limited write endurance (~100,000 cycles per cell). Use this
 *       for configuration only, never in control loops or frequent updates.
 * 
 * @warning ALWAYS lock EEPROM after programming to prevent accidental corruption
 *          from stray commands or electrical noise.
 * 
 * @warning When changing servo ID, use the NEW ID in the LockEeprom() call, as the
 *          servo will respond to the new ID immediately after writeByte().
 * 
 * @warning Ensure stable power during EEPROM writes. Power loss during write can
 *          corrupt servo configuration, potentially requiring factory reset.
 * 
 * @warning Some parameters (like baud rate) require power cycle to take effect.
 *          Test changes before deploying to production systems.
 * 
 * @see SMS_STS::unLockEeprom()
 * @see SMS_STS::LockEeprom()
 * @see SMS_STS::writeByte()
 * @see SMS_STS::writeWord()
 */
#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

	sm_st.unLockEeprom(1);//Enable EEPROM save function
	std::cout<<"unLock Eeprom"<<std::endl;
	sm_st.writeByte(1, SMS_STS_ID, 2);//ID
	std::cout<<"write ID:"<<2<<std::endl;
	sm_st.LockEeprom(2);//Disable EEPROM save function
	std::cout<<"Lock Eeprom"<<std::endl;
	sm_st.end();
	return 1;
}

