/**
 * @file SyncWritePWM.cpp
 * @brief High-performance synchronized PWM control using single-packet protocol
 * 
 * @details
 * This example demonstrates the most efficient method for synchronized open-loop PWM control
 * across multiple servos. SyncWritePwm() packs all servo IDs and PWM values into a single
 * optimized packet, minimizing communication overhead and ensuring perfect synchronization.
 * This is the preferred method for applications requiring simultaneous torque/power control
 * of multiple motors, such as motor testing, open-loop systems, or direct drive applications.
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
 * - SyncWritePwm(): Single-packet synchronized PWM command to multiple servos
 * - Optimal communication efficiency (single packet vs N individual packets)
 * - Perfect synchronization across all servos
 * - Bidirectional PWM for forward/reverse operation
 * - Graceful shutdown with signal handler
 * 
 * Usage:
 * @code
 * ./SyncWritePWM /dev/ttyUSB0
 * # All three servos oscillate between forward and reverse PWM
 * # Single packet per command = minimal latency
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Packet Structure (conceptual):
 * - Command: SYNC_WRITE_PWM
 * - Servo 1: [ID=7, PWM=500]
 * - Servo 2: [ID=8, PWM=500]
 * - Servo 3: [ID=9, PWM=500]
 * - All data in single packet, all servos apply PWM simultaneously
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
 * Performance Benefits:
 * - 66% reduction in bus traffic vs individual PWM commands (3 servos)
 * - Sub-millisecond synchronization accuracy
 * - Scalable to many servos (protocol-dependent limit)
 * - Reduced CPU overhead for high-frequency control loops
 * 
 * Comparison with Alternatives:
 * - vs WritePwm(): 3x fewer packets, better synchronization
 * - vs RegWritePwm(): Single packet vs N+1 packets (N queues + 1 action)
 * - Trade-off: Less flexible than RegWrite but more efficient
 * 
 * @note PWM mode provides maximum torque and response time but no position accuracy
 *       or feedback control. Use for open-loop applications only.
 * 
 * @warning PWM mode bypasses position limits and overload protection. Monitor motor
 *          temperature and current to prevent damage from excessive duty cycles.
 * 
 * @warning All servo IDs in the array must exist on the bus. Missing servos will not
 *          cause errors but will not respond. Use Ping() to verify presence.
 * 
 * @warning Ensure adequate power supply for simultaneous multi-servo PWM operation.
 *          Peak current can exceed single-servo current significantly.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::SyncWritePwm()
 * @see WritePWM.cpp for direct PWM control
 * @see RegWritePWM.cpp for deferred execution alternative
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
        // Initialize motor in PWM/open-loop mode with torque enabled
        int init_ret = sm_st.InitMotor(ID[i], 2, 1);
        std::cout << "Initialize motor " << (int)ID[i] << " in PWM mode (ret=" << init_ret << ")" << std::endl;
        usleep(100000);
    }
    usleep(500000);
    
	while(1){
        sm_st.SyncWritePwm(ID, sizeof(ID)/sizeof(ID[0]), Pwm1); //ID, PWM=500 (50%, forward)
		std::cout<<"PWM = "<<50<<"%"<<std::endl;
		sleep(2);
        
        sm_st.SyncWritePwm(ID, sizeof(ID)/sizeof(ID[0]), Zero); //ID, PWM=0 (0%, stop)
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);

        sm_st.SyncWritePwm(ID, sizeof(ID)/sizeof(ID[0]), Pwm2); //ID, PWM=-500 (50%, reverse)
		std::cout<<"PWM = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        sm_st.SyncWritePwm(ID, sizeof(ID)/sizeof(ID[0]), Zero); //ID, PWM=0 (0%, stop)
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

