/**
 * @file WriteSpeed.cpp
 * @brief Multi-servo continuous rotation velocity control (wheel mode)
 * 
 * @details
 * This example demonstrates coordinated velocity control (wheel mode) of multiple SMS/STS
 * servos using individual WriteSpe() commands. Servos operate as continuous rotation motors
 * rather than position-controlled servos, making this ideal for wheeled robots, conveyor
 * systems, or any application requiring sustained rotation. Commands are sent sequentially
 * to each servo, which is acceptable for most wheel-based applications.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Adequate power supply for sustained rotation
 * - Serial connection at 1000000 baud (1Mbps for STS)
 * - Servos must support velocity mode (wheel mode)
 * 
 * Key Features Demonstrated:
 * - InitMotor(ID, mode=1, torque=1): Initialize in velocity/wheel mode
 * - WriteSpe(): Velocity command with acceleration control
 * - Multi-servo sequential control via loop iteration
 * - Bidirectional rotation: Forward and reverse
 * - Controlled acceleration/deceleration
 * - Graceful shutdown with signal handler (Ctrl+C)
 * 
 * Usage:
 * @code
 * ./WriteSpeed /dev/ttyUSB0
 * # Three servos cycle: forward → stop → reverse → stop
 * # Commands sent sequentially (slight stagger in start times)
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Motion Pattern:
 * - Stage 1: All servos @ -1700 steps/s (forward, 50% max) for 5 seconds
 * - Stage 2: All servos @ 0 steps/s (stopped) for 5 seconds
 * - Stage 3: All servos @ +1700 steps/s (reverse, 50% max) for 5 seconds
 * - Stage 4: All servos @ 0 steps/s (stopped) for 5 seconds
 * - Repeat indefinitely
 * 
 * Speed Parameter Details:
 * - Maximum: ±3400 steps/second (~50rpm)
 * - Forward: Speed1=-1700 (negative = CCW, 50% max)
 * - Reverse: Speed2=+1700 (positive = CW, 50% max)
 * - Stop: Zero=0 (stopped, torque still enabled)
 * - Acceleration: Acc=50 (50×100 = 5000 steps/s²)
 * 
 * Command Execution:
 * - Loop sends WriteSpe() to each servo sequentially
 * - Servo 7 starts ~1ms before servo 8, ~2ms before servo 9
 * - For wheel-based robots, this stagger is typically acceptable
 * - For critical synchronization, use SyncWriteSpe() instead
 * 
 * Velocity Mode Characteristics:
 * - Continuous rotation without position limits
 * - Speed regulation maintains commanded velocity under varying load
 * - Zero command stops rotation but maintains holding torque
 * - Smooth acceleration/deceleration with configurable ramp rate
 * 
 * Advantages:
 * - Simple code structure for independent wheel control
 * - Easy to customize individual servo speeds
 * - Good for differential drive robots or asynchronous motion
 * 
 * Trade-offs:
 * - Slight timing stagger (typically <5ms total)
 * - Higher bus traffic than synchronized methods
 * - Not ideal for precision multi-wheel odometry
 * 
 * Applications:
 * - Differential drive robots
 * - Conveyor belt systems
 * - Continuous rotation mechanisms
 * - Motor testing and characterization
 * 
 * @note Maximum speed of 3400 steps/s corresponds to approximately 50rpm for typical
 *       SMS/STS servos. This example uses 50% max (1700 steps/s) for safety.
 * 
 * @note Velocity mode provides continuous rotation without position limits, unlike
 *       position mode which constrains motion to 0-4095 range.
 * 
 * @warning For precision multi-wheel robots requiring accurate odometry and straight-line
 *          motion, use SyncWriteSpe() for perfect speed synchronization.
 * 
 * @warning High-speed continuous rotation generates significant heat. Monitor servo
 *          temperature and implement thermal management for sustained operation.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::WriteSpe()
 * @see SyncWriteSpeed.cpp for velocity testing and calibration
 * @see RegWriteSpeed.cpp for deferred execution alternative
 */
/*
The factory speed of the servo motor is 0.0146rpm, and the speed is changed to V=2400 steps/sec
*/

#include <iostream>
#include <csignal>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {7, 8, 9};
s16 Zero[3] = {0, 0, 0};

//max speed = 3400 steps/s, using 50% here
s16 Speed1[3] = {-1700, -1700, -1700}; //forward
s16 Speed2[3] = {1700, 1700, 1700}; //reverse
u8 Acc[3] = {50, 50, 50}; // 0 to 254

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
        // Initialize motor in velocity mode with torque enabled
        int init_ret = sm_st.InitMotor(ID[i], 1, 1);
        std::cout << "Initialize motor " << (int)ID[i] << " in velocity mode (ret=" << init_ret << ")" << std::endl;
        usleep(100000);
        int acc_ret = sm_st.writeByte(ID[i], SMS_STS_ACC, Acc[i]); // Set acceleration
        std::cout << "Set Acceleration=" << (int)Acc[i] << " for motor " << (int)ID[i] << " (ret=" << acc_ret << ")" << std::endl;
        usleep(100000);
    }
    usleep(500000);
    
	while(1){
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Speed1[i], Acc[i]); //ID, Speed=2000 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<50<<"%"<<std::endl;
		sleep(2);
        
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Zero[i], Acc[i]); //ID, Speed=0 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<0<<"%"<<std::endl;
		sleep(2);

        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Speed2[i], Acc[i]); //ID, Speed=-2000 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Zero[i], Acc[i]); //ID, Speed=0 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

