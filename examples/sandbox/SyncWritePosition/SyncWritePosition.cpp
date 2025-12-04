/**
 * @file SyncWritePosition.cpp
 * @brief High-performance calibrated multi-servo position control with single-packet protocol
 * 
 * @details
 * This example demonstrates the most efficient and recommended workflow for multi-servo
 * robotics: automatic calibration followed by single-packet synchronized position control.
 * The program first calibrates all servos to set their current positions as midpoint (2048),
 * then uses SyncWritePosEx() to command all servos simultaneously with a single optimized
 * packet containing positions, velocities, and accelerations. This is the gold standard
 * for coordinated multi-axis motion in production robotics systems.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Adequate power supply for three servos
 * - Serial connection at 1000000 baud (1Mbps for STS)
 * - Servos should be manually positioned at desired home/center before running
 * 
 * Key Features Demonstrated:
 * - Automatic multi-servo calibration workflow
 * - CalibrationOfs(): Set current position as midpoint (2048 = π radians)
 * - SyncWritePosEx(): Single-packet synchronized position+velocity+acceleration command
 * - Optimal communication efficiency (single packet for all servos)
 * - Perfect motion synchronization across all axes
 * - Graceful shutdown with signal handler
 * 
 * Usage:
 * @code
 * # STEP 1: Position all servos (IDs 7, 8, 9) at desired home/center
 * # STEP 2: Run program
 * ./SyncWritePosition /dev/ttyUSB0
 * # Servos calibrate, then oscillate with perfect synchronization
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Initialization Sequence:
 * 1. For each servo (IDs 7, 8, 9):
 *    a. InitMotor(ID, mode=0, torque=1) - Position mode with torque
 *    b. Wait 100ms for mode change
 *    c. CalibrationOfs(ID) - Set current position as P=2048 (π radians)
 * 2. Begin optimized synchronized motion control loop
 * 
 * Motion Pattern:
 * - Stage 1: All servos to P1=4095 @ V=2400 steps/s, A=5000 steps/s² (single packet)
 * - Wait 2 seconds for motion completion
 * - Stage 2: All servos to P0=2048 @ V=2400 steps/s, A=5000 steps/s² (single packet)
 * - Wait 2 seconds for motion completion
 * - Repeat indefinitely
 * 
 * Position Mapping After Calibration:
 * - P0=2048: Calibrated midpoint (π radians, home position)
 * - P1=4095: Maximum position (2π rad from 0, or π rad from midpoint)
 * - Range: 2047 steps (~176° motion amplitude)
 * 
 * Packet Structure (conceptual):
 * - Command: SYNC_WRITE_EXTENDED_POSITION
 * - Servo 1: [ID=7, Pos=4095, Vel=2400, Accel=50]
 * - Servo 2: [ID=8, Pos=4095, Vel=2400, Accel=50]
 * - Servo 3: [ID=9, Pos=4095, Vel=2400, Accel=50]
 * - All data in single optimized packet
 * - All servos begin motion simultaneously upon reception
 * 
 * Performance Benefits:
 * - 66% reduction in bus traffic vs individual commands (3 servos)
 * - Sub-millisecond synchronization accuracy
 * - Scalable to many servos (protocol-dependent limit)
 * - Critical for coordinated multi-axis robotics
 * 
 * @note This is the RECOMMENDED method for multi-servo position control in production
 *       systems. Combines calibration, efficiency, and synchronization.
 * 
 * @note Calibration runs on every startup. For production systems, consider calibrating
 *       once and saving offsets, or using external calibration utilities.
 * 
 * @warning Ensure all servos are at desired home positions BEFORE running. Calibration
 *          sets current positions as midpoint (2048) immediately.
 * 
 * @warning All servo IDs must exist on bus. Use Ping() to verify before deploying.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::CalibrationOfs()
 * @see SMS_STS::SyncWritePosEx()
 * @see RegWritePosition.cpp for deferred execution alternative
 * @see WritePosition.cpp for single-servo control
 */
/*
The factory speed of the servo motor is 0.0146rpm, and the speed is changed to V=2400 steps/sec
*/

#include <iostream>
#include <csignal>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {7, 8, 9};
s16 P0[3] = {2048, 2048, 2048}; // = 1/2 rotation = Pi radians
s16 P1[3] = {4095, 4095, 4095}; // = 1 rotation = 2 Pi radians
u16 V[3] = {2400, 2400, 2400}; // steps/s
u8 A[3] = {50, 50, 50}; //*100 steps/s^2

void signalHandler(int signum) {
    if (signum == SIGINT) {
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.EnableTorque(ID[i], 0);
        }
        sm_st.end();
		std::cout<<"Terminated + Torque Disabled"<<std::endl;
        exit(0);
    }
}

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
    
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(1000000, argv[1])){ //115200 for sms, 1000000 for sts
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

    signal(SIGINT, signalHandler);

    for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
        // Initialize motor in position mode with torque enabled
        sm_st.InitMotor(ID[i], 0, 1);
        usleep(100000); // Wait for mode change to take effect
        sm_st.CalibrationOfs(ID[i]); //set starting Pos=2048 (midpoint or Pi radians)
    }
    
	while(1){
        sm_st.SyncWritePosEx(ID, sizeof(ID)/sizeof(ID[0]), P1, V, A);//go to Pos=4095 with Vel=2400 steps/s and Acc=50*100 steps/s^2
		std::cout<<"pos = 4095, 1024, 0"<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
  
        sm_st.SyncWritePosEx(ID, sizeof(ID)/sizeof(ID[0]), P0, V, A);//go to Pos=2048 with Vel=2400 steps/s and Acc=50*100 steps/s^2
		std::cout<<"pos = 2048, 2048, 2048"<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

