/**
 * @file RegWritePosition.cpp
 * @brief Calibrated multi-servo position control with deferred synchronized execution
 * 
 * @details
 * This example demonstrates a complete workflow for multi-servo robotics applications:
 * automatic midpoint calibration followed by synchronized coordinated motion using the
 * Register Write pattern. The program first calibrates all three servos to set their
 * current positions as the midpoint (2048 = π radians), then oscillates them between
 * midpoint and maximum position with precise synchronized execution via RegWriteAction().
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
 * - CalibrationOfs(): Set current position as midpoint (2048)
 * - RegWritePosEx(): Queue position commands with velocity and acceleration
 * - RegWriteAction(): Simultaneous execution across all servos
 * - Graceful shutdown with signal handler
 * - Coordinated multi-axis motion
 * 
 * Usage:
 * @code
 * # STEP 1: Position all servos (IDs 7, 8, 9) at desired home/center
 * # STEP 2: Run program
 * ./RegWritePosition /dev/ttyUSB0
 * # Servos calibrate, then oscillate between midpoint and max position
 * # Press Ctrl+C to stop safely
 * @endcode
 * 
 * Initialization Sequence:
 * 1. For each servo (IDs 7, 8, 9):
 *    a. InitMotor(ID, mode=0, torque=1) - Position mode with torque
 *    b. Wait 100ms for mode change
 *    c. CalibrationOfs(ID) - Set current position as P=2048
 * 2. Begin synchronized motion control loop
 * 
 * Motion Pattern:
 * - Stage 1: All servos to P1=4095 @ V=2400 steps/s, A=5000 steps/s² (synchronized)
 * - Wait 2187ms for motion completion
 * - Stage 2: All servos to P0=2048 @ V=2400 steps/s, A=5000 steps/s² (synchronized)
 * - Wait 2187ms for motion completion
 * - Repeat indefinitely
 * 
 * Position Mapping After Calibration:
 * - P0=2048: Calibrated midpoint (π radians, 180° equivalent)
 * - P1=4095: Maximum position (2π radians, 360° equivalent, one full rotation from 0)
 * - Range: 2047 steps (~176° motion amplitude)
 * 
 * Execution Pattern:
 * 1. For each servo: RegWritePosEx(ID, pos, vel, accel) - Queue command
 * 2. RegWriteAction() - Broadcast: all servos start motion simultaneously
 * 3. Wait for motion completion (calculated timing)
 * 4. Repeat with next position
 * 
 * Parameter Details:
 * - P0=2048: Midpoint after calibration (π rad)
 * - P1=4095: Maximum position (2π rad relative to 0, or π rad from midpoint)
 * - V=2400: Velocity in steps/second (~35rpm)
 * - A=50: Acceleration (50×100 = 5000 steps/s²)
 * - Timing: [(P1-P0)/V]*1000 + [V/(A*100)]*1000 = ~2187ms
 * 
 * @note Calibration writes to EEPROM (~100,000 cycle endurance). This example calibrates
 *       on every run, which is fine for testing but may not be suitable for production
 *       systems with frequent restarts.
 * 
 * @warning Ensure all servos are at desired home positions BEFORE running. The program
 *          will calibrate current positions as midpoint (2048) immediately.
 * 
 * @warning After calibration, position 2048 corresponds to the physical positions from
 *          startup. All motion is relative to this new reference frame.
 * 
 * @see SMS_STS::InitMotor()
 * @see SMS_STS::CalibrationOfs()
 * @see SMS_STS::RegWritePosEx()
 * @see SMS_STS::RegWriteAction()
 * @see MidpointCalib.cpp for calibration-only utility
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
        sm_st.InitMotor(ID[i], 0, 1);
        usleep(100000); // Wait for mode change to take effect
        sm_st.CalibrationOfs(ID[i]); //set starting Pos=2048 (midpoint or Pi radians)
    }
    
	while(1){
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
    		sm_st.RegWritePosEx(ID[i], P1, V, A);//go to Pos=4095 with Vel=2400 steps/s and Acc=50*100 steps/s^2
        }
        sm_st.RegWriteAction();
		std::cout<<"pos = "<<static_cast<int>(P1)<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000

        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
    		sm_st.RegWritePosEx(ID[i], P0, V, A);//go to Pos=2048 with Vel=2400 steps/s and Acc=50*100 steps/s^2
        }
        sm_st.RegWriteAction();
		std::cout<<"pos = "<<static_cast<int>(P0)<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

