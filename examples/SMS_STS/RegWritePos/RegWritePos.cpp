/**
 * @file RegWritePos.cpp
 * @brief Deferred multi-servo position commands with acceleration control for SMS/STS protocol
 * 
 * @details
 * This example demonstrates the Register Write pattern with extended motion parameters
 * (velocity and acceleration) for synchronized multi-servo motion. Commands are queued
 * to each servo's register memory individually, then executed simultaneously with a
 * single RegWriteAction() broadcast. This provides precise synchronization with full
 * control over motion profiles including trapezoidal velocity control.
 * 
 * Hardware Requirements:
 * - Two Feetech SMS or STS protocol servos (IDs: 1, 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models (typically 6-12V)
 * - Serial connection at 115200 baud
 * - Servos must be on same serial bus
 * 
 * Key Features Demonstrated:
 * - RegWritePosEx(): Queue extended position command with velocity and acceleration
 * - RegWriteAction(): Broadcast trigger for simultaneous execution
 * - Deferred execution pattern for precise multi-servo synchronization
 * - Individual motion profile control per servo
 * - Trapezoidal velocity profiles with controlled acceleration
 * - Continuous coordinated oscillation
 * 
 * Usage:
 * @code
 * ./RegWritePos /dev/ttyUSB0
 * @endcode
 * 
 * Execution Pattern:
 * 1. RegWritePosEx(ID1, pos, vel, accel) - Queue command to servo 1 (not executed)
 * 2. RegWritePosEx(ID2, pos, vel, accel) - Queue command to servo 2 (not executed)
 * 3. RegWriteAction() - Broadcast: both servos start motion simultaneously
 * 4. Wait for motion completion (including acceleration ramp time)
 * 5. Repeat with new positions
 * 
 * Motion Profile:
 * - Stage 1: Both servos to position 4095 @ 2400 steps/s, accel 5000 steps/s²
 * - Wait 2187ms (motion + acceleration time)
 * - Stage 2: Both servos to position 0 @ 2400 steps/s, accel 5000 steps/s²
 * - Wait 2187ms
 * - Repeat indefinitely
 * 
 * Parameter Details:
 * - Position: 0 to 4095 (full rotation range)
 * - Velocity: 2400 steps/second (adjustable)
 * - Acceleration: 50 (units of 100 steps/s², so 5000 steps/s²)
 * - Timing: [(P1-P0)/V]*1000 + [V/(A*100)]*1000 milliseconds
 * 
 * Advantages over SyncWritePosEx:
 * - More flexible command sequencing
 * - Can mix different command types
 * - Easier dynamic servo management
 * - Better for complex choreography
 * 
 * @note Factory servo speed unit is 0.0146rpm per step. V=2400 corresponds to
 *       approximately 35rpm at maximum velocity.
 * 
 * @warning All servos must support Register Write functionality. Check firmware
 *          version compatibility.
 * 
 * @warning RegWriteAction() is broadcast - all servos with queued commands will execute.
 *          Ensure all intended servos have been given RegWrite commands.
 * 
 * @see SMS_STS::RegWritePosEx()
 * @see SMS_STS::RegWriteAction()
 * @see SMS_STS::SyncWritePosEx() for alternative synchronization method
 */
/*
Factory speed unit of servo is 0.0146rpm, speed changed to V=2400
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
	while(1){
		sm_st.RegWritePosEx(1, 4095, 2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P1=4095
		sm_st.RegWritePosEx(2, 4095, 2400, 50);//Servo (ID2) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P1=4095
		sm_st.RegWriteAction();
		std::cout<<"pos = "<<4095<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
  
		sm_st.RegWritePosEx(1, 0, 2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P0=0
		sm_st.RegWritePosEx(2, 0, 2400, 50);//Servo (ID2) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P0=0
		sm_st.RegWriteAction();
		std::cout<<"pos = "<<0<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

