/**
 * @file WritePosition.cpp
 * @brief Multi-servo position control with velocity and acceleration profiles
 * 
 * @details
 * This example demonstrates coordinated position control of multiple SMS/STS servos using
 * individual WritePosEx() commands with configurable velocity and acceleration. Unlike
 * synchronized methods (SyncWritePosEx, RegWritePos), this approach sends commands
 * sequentially, which may result in slight timing differences but offers simpler code
 * structure for applications where sub-millisecond synchronization is not critical.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Adequate power supply for three servos
 * - Serial connection at 1000000 baud (1Mbps for STS)
 * 
 * Key Features Demonstrated:
 * - WritePosEx(): Position command with velocity and acceleration control
 * - Multi-servo sequential control via loop iteration
 * - Trapezoidal velocity profiles for smooth motion
 * - Oscillating motion pattern between two positions
 * - Graceful shutdown with signal handler (Ctrl+C)
 * 
 * Usage:
 * @code
 * ./WritePosition /dev/ttyUSB0
 * # Three servos oscillate between midpoint (2048) and max (4095)
 * # Commands sent sequentially (slight stagger in start times)
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Motion Pattern:
 * - Stage 1: All servos to P1=4095 @ V=2400 steps/s, A=5000 steps/s²
 * - Wait 2187ms for motion completion
 * - Stage 2: All servos to P0=2048 @ V=2400 steps/s, A=5000 steps/s²
 * - Wait 2187ms for motion completion
 * - Repeat indefinitely
 * 
 * Position Details:
 * - P0=2048: Midpoint (π radians, 180° equivalent)
 * - P1=4095: Maximum (2π radians from 0, ~360°)
 * - Range: 2047 steps (~176° motion amplitude)
 * 
 * Motion Profile Parameters:
 * - Velocity: V=2400 steps/second (~35rpm)
 * - Acceleration: A=50 (50×100 = 5000 steps/s²)
 * - Timing: [(P1-P0)/V]*1000 + [V/(A*100)]*1000 ≈ 2187ms
 * 
 * Command Execution:
 * - Loop sends WritePosEx() to each servo sequentially
 * - Servo 7 starts ~1ms before servo 8, ~2ms before servo 9
 * - For most applications, this stagger is imperceptible
 * - For critical synchronization, use SyncWritePosEx() instead
 * 
 * Advantages:
 * - Simpler code structure (no arrays of parameters)
 * - Easier to customize individual servo commands
 * - Good for asynchronous or independent servo motion
 * 
 * Trade-offs:
 * - Slight timing stagger between servos (typically <5ms total)
 * - Higher bus traffic than synchronized methods
 * - Not recommended for precision multi-axis robotics
 * 
 * @note Factory servo speed unit is 0.0146rpm per step. V=2400 corresponds to
 *       approximately 35rpm at maximum velocity.
 * 
 * @warning For applications requiring precise multi-servo synchronization (e.g., walking
 *          robots, coordinated arms), use SyncWritePosEx() instead for sub-millisecond
 *          accuracy.
 * 
 * @see SMS_STS::WritePosEx()
 * @see SyncWritePosition.cpp for synchronized alternative
 * @see RegWritePosition.cpp for deferred execution alternative
 */
/*
The factory speed of the servo motor is 0.0146rpm, and the speed is changed to V=2400 steps/sec
*/

#include <iostream>
#include <csignal>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {7, 8, 9};
s16 P0 = 2048; // = 1/2 rotation = Pi radians
s16 P1 = 4095; // = 1 rotation = 2*Pi radians
u16 V = 2400; // steps/s
u8 A = 50; //*100 steps/s^2

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
        int init_ret = sm_st.InitMotor(ID[i], 0, 1);
        std::cout << "Initialize motor " << (int)ID[i] << " in position mode (ret=" << init_ret << ")" << std::endl;
        usleep(100000);
        int acc_ret = sm_st.writeByte(ID[i], SMS_STS_ACC, A); // Set acceleration
        std::cout << "Set Acceleration=" << (int)A << " for motor " << (int)ID[i] << " (ret=" << acc_ret << ")" << std::endl;
        usleep(100000);
        sm_st.CalibrationOfs(ID[i]); //set starting Pos=2048 (midpoint or Pi radians)
        usleep(100000);
    }
    usleep(500000);
    
	while(1){
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePosEx(ID[i], P1, V, A);//go to Pos=4095 with Vel=2400 steps/s and Acc=50*100 steps/s^2
        }
		std::cout<<"pos = "<<static_cast<int>(P1)<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000

        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePosEx(ID[i], P0, V, A);//go to Pos=2048 with Vel=2400 steps/s and Acc=50*100 steps/s^2
        }
		std::cout<<"pos = "<<static_cast<int>(P0)<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

