/**
 * @file RegWritePWM.cpp
 * @brief Deferred multi-servo PWM control with synchronized execution
 * 
 * @details
 * This example demonstrates synchronized open-loop PWM control using the Register Write
 * pattern. PWM commands are queued to each servo individually via RegWritePwm(), then
 * executed simultaneously with a single RegWriteAction() broadcast. This provides precise
 * synchronization for multi-servo open-loop systems, essential for coordinated torque
 * control, motor testing, or applications where direct power control is needed.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Adequate power supply for three servos under PWM load
 * - Serial connection at 1000000 baud (1Mbps for STS)
 * - Servos must support PWM mode
 * 
 * Key Features Demonstrated:
 * - InitMotor(ID, mode=2, torque=1): Initialize in PWM/open-loop mode
 * - RegWritePwm(): Queue PWM command to servo register (deferred execution)
 * - RegWriteAction(): Broadcast trigger for simultaneous PWM application
 * - Multi-servo array-based control
 * - Bidirectional PWM (forward/reverse)
 * - Graceful shutdown with signal handler
 * 
 * Usage:
 * @code
 * ./RegWritePWM /dev/ttyUSB0
 * # All three servos oscillate between forward and reverse PWM
 * # Commands execute simultaneously for perfect synchronization
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Execution Pattern:
 * 1. For each servo: RegWritePwm(ID, +500) - Queue forward PWM (not executed yet)
 * 2. RegWriteAction() - Broadcast: all servos apply PWM simultaneously
 * 3. Wait 2 seconds
 * 4. For each servo: RegWritePwm(ID, -500) - Queue reverse PWM
 * 5. RegWriteAction() - All servos reverse simultaneously
 * 6. Wait 2 seconds, repeat
 * 
 * Motion Pattern:
 * - Stage 1: All servos @ +500 PWM (50% duty, forward) for 2s
 * - Stage 2: All servos @ -500 PWM (50% duty, reverse) for 2s
 * - Repeat indefinitely until Ctrl+C
 * 
 * PWM Parameter Details:
 * - Range: -1000 to +1000 (-100% to +100% duty cycle)
 * - PWM=500: 50% forward (CCW)
 * - PWM=-500: 50% reverse (CW)
 * - PWM=0: Stopped (torque still enabled)
 * 
 * Advantages of RegWrite Pattern:
 * - Perfect synchronization across all servos (sub-millisecond accuracy)
 * - Individual command customization per servo
 * - Flexible choreography and sequencing
 * - Lower bus traffic than individual PWM commands
 * 
 * @note PWM mode bypasses position and velocity control loops, providing direct motor
 *       power control for maximum torque and response time.
 * 
 * @warning PWM mode disables position limits and overload protection. Monitor motor
 *          temperature and current to prevent damage.
 * 
 * @warning RegWriteAction() is a broadcast command. Ensure all servos have been given
 *          RegWritePwm() commands or they may execute stale commands from registers.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::RegWritePwm()
 * @see SMS_STS::RegWriteAction()
 * @see WritePWM.cpp for direct PWM control without deferred execution
 * @see SyncWritePWM.cpp for single-packet synchronized PWM alternative
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
        sm_st.InitMotor(ID[i], 2, 1);
        usleep(100000); // Wait for mode change to take effect
    }
    
	while(1){
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWritePwm(ID[i], Pwm1[i]); //ID, PWM=500 (50%, forward)
        }
        sm_st.RegWriteAction();
		std::cout<<"PWM = "<<50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWritePwm(ID[i], Zero[i]); //ID, PWM=0 (0%, stop)
        }
        sm_st.RegWriteAction();
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);

        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWritePwm(ID[i], Pwm2[i]); //ID, PWM=-500 (50%, reverse)
        }
        sm_st.RegWriteAction();
		std::cout<<"PWM = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWritePwm(ID[i], Zero[i]); //ID, PWM=0 (0%, stop)
        }
        sm_st.RegWriteAction();
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

