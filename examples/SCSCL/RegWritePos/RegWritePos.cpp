/**
 * @file RegWritePos.cpp
 * @brief Deferred multi-servo position commands with synchronized execution for SCSCL protocol
 * 
 * @details
 * This example demonstrates the Register Write pattern for synchronized multi-servo motion.
 * Unlike SyncWritePos which sends all data in a single packet, RegWritePos allows individual
 * commands to be queued to each servo's register memory, then executed simultaneously with
 * a single RegWriteAction() broadcast. This pattern is useful when different servos need
 * different command types or when building complex motion sequences.
 * 
 * Hardware Requirements:
 * - Two Feetech SCSCL protocol servos (IDs: 1, 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models
 * - Serial connection at 115200 baud
 * - Servos must be on same serial bus
 * 
 * Key Features Demonstrated:
 * - RegWritePos(): Queue position command to servo's register without execution
 * - RegWriteAction(): Broadcast trigger to execute all queued commands simultaneously
 * - Deferred execution pattern for precise multi-servo synchronization
 * - Individual command customization per servo
 * - Continuous coordinated oscillation
 * 
 * Usage:
 * @code
 * ./RegWritePos /dev/ttyUSB0
 * @endcode
 * 
 * Execution Pattern:
 * 1. RegWritePos(ID1, 1000, ...) - Queue command to servo 1 (not executed yet)
 * 2. RegWritePos(ID2, 1000, ...) - Queue command to servo 2 (not executed yet)
 * 3. RegWriteAction() - Broadcast: both servos execute simultaneously
 * 4. Wait for motion completion
 * 5. Repeat with new positions
 * 
 * Motion Profile:
 * - Stage 1: Both servos to position 1000 @ 1500 steps/s
 * - Stage 2: Both servos to position 20 @ 1500 steps/s
 * - Repeat indefinitely
 * 
 * Parameter Details:
 * - Position: Target position for each servo
 * - Time: 0 (use speed control, not time-based control)
 * - Speed: 1500 steps/second
 * - Timing: [(P1-P0)/V]*1000 + 100ms for motion completion
 * 
 * Advantages over SyncWritePos:
 * - More flexible command sequencing
 * - Can mix different command types (position, speed, etc.)
 * - Easier to add/remove servos dynamically
 * - Better for complex choreography
 * 
 * @note Factory servo speed unit is 0.0146rpm per step. V=1500 corresponds to
 *       approximately 22rpm at maximum velocity.
 * 
 * @warning All servos must support Register Write functionality. Older firmware
 *          versions may not implement this feature.
 * 
 * @warning RegWriteAction() is a broadcast command - ensure all servos have been
 *          given RegWrite commands or they may execute stale commands.
 * 
 * @see SCSCL::RegWritePos()
 * @see SCSCL::RegWriteAction()
 * @see SCSCL::SyncWritePos() for alternative synchronization method
 */
/*
Factory servo speed unit is 0.0146rpm, speed is V=1500
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
	while(1){
		sc.RegWritePos(1, 1000, 0, 1500);//Servo (ID1) queue command: move to P1=1000 at maximum speed V=1500 steps/second
		sc.RegWritePos(2, 1000, 0, 1500);//Servo (ID2) queue command: move to P1=1000 at maximum speed V=1500 steps/second
		sc.RegWriteAction();
		std::cout<<"pos = "<<1000<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
  
		sc.RegWritePos(1, 20, 0, 1500);//Servo (ID1) queue command: move to P1=20 at maximum speed V=1500 steps/second
		sc.RegWritePos(2, 20, 0, 1500);//Servo (ID2) queue command: move to P1=20 at maximum speed V=1500 steps/second
		sc.RegWriteAction();
		std::cout<<"pos = "<<20<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
	}
	sc.end();
	return 1;
}

