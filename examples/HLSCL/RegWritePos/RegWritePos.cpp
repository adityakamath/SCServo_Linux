/**
 * @file RegWritePos.cpp
 * @brief Buffered position write (RegWrite) example for synchronized multi-servo control
 *
 * @details
 * This example demonstrates the RegWrite command pattern where position commands are
 * buffered in each servo's memory and then executed simultaneously using RegWriteAction().
 * This provides better synchronization than sequential WritePosEx() commands, as all
 * servos start moving at exactly the same moment.
 *
 * Hardware Requirements:
 * - Two Feetech HLS series servos (ID: 1 and ID: 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 * - Servos connected to same serial bus (daisy-chained)
 *
 * Key Features Demonstrated:
 * - RegWritePosEx(): Buffer position command in servo memory
 * - RegWriteAction(): Execute all buffered commands simultaneously
 * - Multi-servo synchronized motion
 * - Precise timing synchronization vs sequential writes
 *
 * Usage:
 * @code
 * ./RegWritePos /dev/ttyUSB0
 * # Both servos move together to position 4095, then to 0, repeatedly
 * @endcode
 *
 * Motion Sequence:
 * 1. Buffer position 4095 command in servo ID1
 * 2. Buffer position 4095 command in servo ID2
 * 3. Execute both commands simultaneously with RegWriteAction()
 * 4. Wait for motion to complete
 * 5. Repeat for position 0
 * 6. Cycle continues indefinitely
 *
 * RegWrite vs Direct Write:
 * - RegWrite: Commands buffered, execute together (better sync)
 * - WritePosEx: Immediate execution, sequential (slight delay between servos)
 * - SyncWrite: Best synchronization (single packet, multiple servos)
 *
 * Parameter Details:
 * - Position: P=4095 (maximum) and P=0 (minimum)
 * - Speed: V=60 steps/s (60 × 0.732 = 43.92 RPM)
 * - Acceleration: A=50 (50 × 8.7 = 435 deg/s²)
 * - Torque limit: T=500 (500 × 6.5 = 3250 mA max current)
 *
 * @note RegWriteAction() can use broadcast ID (0xFE) to trigger all buffered servos
 *       or specific ID to trigger only one servo's buffered command.
 *
 * @note Each servo can buffer only ONE RegWrite command at a time. Subsequent
 *       RegWrite commands overwrite the previous buffered command.
 *
 * @warning Ensure both servos have adequate mechanical clearance for full range motion.
 *
 * @see HLSCL::RegWritePosEx()
 * @see HLSCL::RegWriteAction()
 * @see HLSCL::SyncWritePosEx() for even better synchronization
 */

#include <iostream>
#include <csignal>
#include "SCServo.h"

HLSCL hlscl;
volatile bool running = true;

void signalHandler(int signum) {
	std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down..." << std::endl;
	running = false;
}

int main(int argc, char **argv)
{
	// Register signal handler for graceful shutdown
	signal(SIGINT, signalHandler);
	
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!hlscl.begin(115200, argv[1])){
        std::cout<<"Failed to init HLS motor!"<<std::endl;
        return 0;
    }
	while(running){
		// Buffer commands: Servos (ID1/ID2) with max speed V=60×0.732=43.92rpm,
		// acceleration A=50×8.7deg/s², max torque current T=500×6.5=3250mA, move to P1=4095
		hlscl.RegWritePosEx(1, 4095, 60, 50, 500);
		hlscl.RegWritePosEx(2, 4095, 60, 50, 500);
		// Execute both buffered commands simultaneously
		hlscl.RegWriteAction();
		std::cout<<"pos = "<<4095<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);

		if (!running) break;  // Check before next command

		// Buffer commands: Servos (ID1/ID2) with max speed V=60×0.732=43.92rpm,
		// acceleration A=50×8.7deg/s², max torque current T=500×6.5=3250mA, move to P0=0
		hlscl.RegWritePosEx(1, 0, 60, 50, 500);
		hlscl.RegWritePosEx(2, 0, 60, 50, 500);
		// Execute both buffered commands simultaneously
		hlscl.RegWriteAction();
		std::cout<<"pos = "<<0<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);
	}
	hlscl.end();
	return 0;
}

