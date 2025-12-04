/**
 * @file RegWriteSpeed.cpp
 * @brief Deferred multi-servo velocity control with synchronized execution (wheel mode)
 * 
 * @details
 * This example demonstrates synchronized continuous rotation (wheel mode) using the Register
 * Write pattern for velocity commands. Servos are initialized in velocity mode, then speed
 * commands are queued individually via RegWriteSpe() and executed simultaneously with a
 * single RegWriteAction() broadcast. This provides perfect synchronization for multi-wheel
 * robots, conveyor systems, or any application requiring coordinated continuous rotation.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Adequate power supply for three servos under continuous load
 * - Serial connection at 1000000 baud (1Mbps for STS)
 * - Servos must support velocity mode (wheel mode)
 * 
 * Key Features Demonstrated:
 * - InitMotor(ID, mode=1, torque=1): Initialize in velocity/wheel mode
 * - RegWriteSpe(): Queue velocity command to servo register (deferred execution)
 * - RegWriteAction(): Broadcast trigger for simultaneous speed application
 * - Maximum acceleration (254 = 25400 steps/s²) for rapid response
 * - Bidirectional rotation at maximum speed (3400 steps/s)
 * - Graceful shutdown with signal handler
 * 
 * Usage:
 * @code
 * ./RegWriteSpeed /dev/ttyUSB0
 * # All three servos rotate forward at max speed
 * # Then reverse at max speed
 * # Commands execute simultaneously for perfect synchronization
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Execution Pattern:
 * 1. For each servo: RegWriteSpe(ID, -3400, 254) - Queue forward command (not executed)
 * 2. RegWriteAction() - Broadcast: all servos start rotating simultaneously
 * 3. Wait 2 seconds
 * 4. For each servo: RegWriteSpe(ID, +3400, 254) - Queue reverse command
 * 5. RegWriteAction() - All servos reverse simultaneously
 * 6. Wait 2 seconds, repeat
 * 
 * Motion Pattern:
 * - Stage 1: All servos @ -3400 steps/s (forward, CCW) for 2s
 * - Stage 2: All servos @ +3400 steps/s (reverse, CW) for 2s
 * - Repeat indefinitely until Ctrl+C
 * 
 * Speed Parameter Details:
 * - Range: Typically -3400 to +3400 steps/s (servo model dependent)
 * - Speed=-3400: Maximum forward rotation (CCW)
 * - Speed=+3400: Maximum reverse rotation (CW)
 * - Speed=0: Stopped (torque still enabled)
 * - Acceleration=254: Maximum (25400 steps/s²)
 * 
 * Advantages of RegWrite Pattern:
 * - Perfect synchronization across all servos (sub-millisecond accuracy)
 * - Individual speed customization per servo
 * - Flexible choreography and sequencing
 * - Critical for multi-wheel robots requiring straight-line motion
 * 
 * @note Maximum speed of 3400 steps/s corresponds to approximately 50rpm for typical
 *       SMS/STS servos. Actual maximum may vary by model.
 * 
 * @note Velocity mode provides continuous rotation without position limits, unlike
 *       position mode which has 0-4095 range constraints.
 * 
 * @warning High-speed continuous rotation generates significant heat. Monitor servo
 *          temperature and implement thermal management for sustained operation.
 * 
 * @warning Ensure mechanical load is appropriate for maximum speed. Excessive load
 *          can cause speed regulation failure or motor damage.
 * 
 * @warning RegWriteAction() is broadcast. Ensure all servos have RegWriteSpe() commands
 *          or they may execute stale commands from registers.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::RegWriteSpe()
 * @see SMS_STS::RegWriteAction()
 * @see WriteSpeed.cpp for direct velocity control
 * @see SyncWriteSpeed.cpp for single-packet synchronized alternative
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
s16 Speed1[3] = {-3400, -3400, -3400}; //forward
s16 Speed2[3] = {3400, 3400, 3400}; //reverse
u8 Acc[3] = {254, 254, 254}; // 0 to 254

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
        // Initialize motor in velocity mode with torque enabled
        sm_st.InitMotor(ID[i], 1, 1);
        usleep(100000); // Wait for mode change to take effect
    }
    
	while(1){
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWriteSpe(ID[i], Speed1[i], Acc[i]); //ID, Speed=2000 steps/s, Acc=50*100 steps/s^2
        }
        sm_st.RegWriteAction();
		std::cout<<"Speed = "<<50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWriteSpe(ID[i], Zero[i], Acc[i]); //ID, Speed=0 steps/s, Acc=50*100 steps/s^2
        }
        sm_st.RegWriteAction();
		std::cout<<"Speed = "<<0<<"%"<<std::endl;
		sleep(2);

        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWriteSpe(ID[i], Speed2[i], Acc[i]); //ID, Speed=-2000 steps/s, Acc=50*100 steps/s^2
        }
        sm_st.RegWriteAction();
		std::cout<<"Speed = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.RegWriteSpe(ID[i], Zero[i], Acc[i]); //ID, Speed=0 steps/s, Acc=50*100 steps/s^2
        }
        sm_st.RegWriteAction();
		std::cout<<"Speed = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

