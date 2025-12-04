/**
 * @file WritePWM.cpp
 * @brief Direct PWM output control for open-loop multi-servo operation
 * 
 * @details
 * This example demonstrates open-loop PWM control of multiple SMS/STS servos. In PWM mode,
 * the servo controller outputs a raw PWM duty cycle directly to the motor driver, bypassing
 * position and velocity feedback loops. This mode is useful for applications requiring direct
 * torque/power control, open-loop systems, or when using the servo as a simple motor driver.
 * The example oscillates three servos between forward (+500) and reverse (-500) PWM.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Adequate power supply for three servos under load
 * - Serial connection at 1000000 baud (1Mbps for STS, use 115200 for SMS)
 * - Servos must support PWM mode (check firmware version)
 * 
 * Key Features Demonstrated:
 * - InitMotor(ID, mode=2, torque=1): Initialize servo in PWM/open-loop mode
 * - WritePwm(): Send PWM duty cycle command (-1000 to +1000)
 * - Multi-servo array-based control
 * - Graceful shutdown with signal handler (Ctrl+C)
 * - Bidirectional PWM for forward/reverse operation
 * 
 * Usage:
 * @code
 * ./WritePWM /dev/ttyUSB0
 * # Servos oscillate between forward and reverse PWM
 * # Press Ctrl+C to stop and disable torque safely
 * @endcode
 * 
 * PWM Mode Initialization:
 * - InitMotor(ID, 2, 1): mode=2 for PWM/open-loop, torque=1 to enable
 * - Wait 100ms per servo for mode change
 * - Additional 500ms settling time before starting control loop
 * 
 * Motion Pattern:
 * - Forward PWM: +500 (50% duty cycle, forward) for 2 seconds
 * - Reverse PWM: -500 (50% duty cycle, reverse) for 2 seconds
 * - Repeat indefinitely until Ctrl+C
 * 
 * PWM Parameter Details:
 * - Range: -1000 to +1000 (corresponds to -100% to +100% duty cycle)
 * - PWM=0: Motor stopped (but torque still enabled)
 * - PWM>0: Forward rotation (counter-clockwise)
 * - PWM<0: Reverse rotation (clockwise)
 * - PWM=500: 50% duty cycle forward (~50% of max torque/speed)
 * - PWM=-500: 50% duty cycle reverse
 * 
 * Control Loop Characteristics:
 * - Open-loop: No position or velocity feedback
 * - Direct motor power control
 * - Load-dependent speed (not regulated)
 * - Immediate torque response
 * - No motion profiling or acceleration control
 * 
 * Signal Handler:
 * - Catches SIGINT (Ctrl+C) for safe shutdown
 * - Disables torque on all servos before exit
 * - Prevents sudden stops or power spikes
 * 
 * @note PWM mode provides maximum torque and response time but no position accuracy.
 *       Use position mode for precise motion control.
 * 
 * @warning PWM mode bypasses position limits and overload protection. Monitor motor
 *          current and temperature to prevent damage.
 * 
 * @warning Ensure mechanical load is appropriate. High PWM values with heavy loads
 *          can cause overheating or motor damage.
 * 
 * @warning Always implement signal handler for safe shutdown. Exiting without disabling
 *          torque can leave motors powered and potentially cause mechanical issues.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::WritePwm()
 * @see SMS_STS::EnableTorque()
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

s16 Pwm1[3] = {500, 500, 500};
s16 Pwm2[3] = {-500, -500, -500};

void signalHandler(int signum) {
    if (signum == SIGINT) {
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
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

    for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
        // Initialize motor in PWM/open-loop mode with torque enabled
        int init_ret = sm_st.InitMotor(ID[i], 2, 1);
        std::cout << "Initialize motor " << (int)ID[i] << " in PWM mode (ret=" << init_ret << ")" << std::endl;
        usleep(100000);
    }
    usleep(500000);
    
	while(1){
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePwm(ID[i], Pwm1[i]); //ID, PWM=500 (50%, forward)
        }
		std::cout<<"PWM = "<<50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePwm(ID[i], Zero[i]); //ID, PWM=0 (0%, stop)
        }
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);

        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePwm(ID[i], Pwm2[i]); //ID, PWM=-500 (50%, reverse)
        }
		std::cout<<"PWM = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePwm(ID[i], Zero[i]); //ID, PWM=0 (0%, stop)
        }
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

