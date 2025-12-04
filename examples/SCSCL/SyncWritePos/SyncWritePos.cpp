/**
 * @file SyncWritePos.cpp
 * @brief Synchronized multi-servo position control for SCSCL protocol servos
 * 
 * @details
 * This example demonstrates synchronized position commands to multiple SCSCL servos
 * using a single SyncWritePos() call. This is essential for coordinated multi-axis
 * motion where timing synchronization between servos is critical. All servos receive
 * their commands simultaneously and begin motion at the same time.
 * 
 * Hardware Requirements:
 * - Two Feetech SCSCL protocol servos (IDs: 1, 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models
 * - Serial connection at 115200 baud
 * - Servos must be on same serial bus
 * 
 * Key Features Demonstrated:
 * - SyncWritePos(): Simultaneous position command to multiple servos
 * - Array-based servo ID management
 * - Array-based position and speed parameters
 * - Coordinated multi-servo motion
 * - Continuous synchronized oscillation
 * 
 * Usage:
 * @code
 * ./SyncWritePos /dev/ttyUSB0
 * @endcode
 * 
 * Motion Profile:
 * - Both servos move to position 1000 @ 1500 steps/s simultaneously
 * - Wait 754ms for motion completion
 * - Both servos move to position 20 @ 1500 steps/s simultaneously
 * - Wait 754ms for motion completion
 * - Repeat indefinitely
 * 
 * Parameter Details:
 * - ID[2]: Array of servo IDs to command {1, 2}
 * - Position[2]: Target positions for each servo
 * - Speed[2]: Velocities for each servo (1500 steps/second)
 * - Servo count: 2 servos in this example
 * - Time parameter: 0 (use speed control, not time control)
 * 
 * Timing Calculation:
 * - Motion time ≈ [(P1-P0)/V]*1000 + 100 milliseconds
 * - For P0=20 to P1=1000 at V=1500: [(1000-20)/1500]*1000 + 100 ≈ 754ms
 * 
 * @note Factory servo speed unit is 0.0146rpm per step. V=1500 corresponds to
 *       approximately 22rpm at maximum velocity.
 * 
 * @warning All servos in the ID array must exist on the bus. Missing servos
 *          will not cause errors but will not move.
 * 
 * @warning Ensure adequate power supply for simultaneous multi-servo acceleration.
 *          Insufficient power can cause position errors or servo resets.
 * 
 * @see SCSCL::SyncWritePos()
 * @see SCSCL::begin()
 */
/*
Factory servo speed unit is 0.0146rpm, speed is V=1500
*/

#include <iostream>
#include "SCServo.h"

SCSCL sc;

u8 ID[2] = {1, 2};
u16 Position[2];
u16 Speed[2];

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
		Position[0] = 1000;
		Position[1] = 1000;
		Speed[0] = 1500;
		Speed[1] = 1500;
		sc.SyncWritePos(ID, 2, Position, 0, Speed);//Servos (ID1/ID2) move to position P1=1000 at maximum speed V=1500 steps/second
		std::cout<<"pos = "<<1000<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
  
		Position[0] = 20;
		Position[1] = 20;
		Speed[0] = 1500;
		Speed[1] = 1500;
		sc.SyncWritePos(ID, 2, Position, 0, Speed);//舵机((ID1/ID2))以最高速度V=1500步/秒,运行至P1=20
		std::cout<<"pos = "<<20<<std::endl;
		usleep(754*1000);//[(P1-P0)/V]*1000+100
	}
	sc.end();
	return 1;
}

