/**
 * @file SyncWritePos.cpp
 * @brief Synchronized write example for optimal multi-servo control
 *
 * @details
 * This example demonstrates the SyncWritePosEx command, which is the most efficient
 * method for controlling multiple servos. Unlike sequential WritePosEx() calls or
 * buffered RegWritePosEx() commands, SyncWritePosEx sends all servo commands in a
 * single packet, providing perfect synchronization and minimal bus traffic.
 *
 * Hardware Requirements:
 * - Two Feetech HLS series servos (ID: 1 and ID: 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 * - Servos connected to same serial bus (daisy-chained)
 *
 * Key Features Demonstrated:
 * - SyncWritePosEx(): Single-packet multi-servo position control
 * - Array-based parameter specification for multiple servos
 * - Perfect timing synchronization (all servos start simultaneously)
 * - Minimal bus latency (single packet vs N individual packets)
 * - Different positions for each servo in synchronized motion
 *
 * Usage:
 * @code
 * ./SyncWritePos /dev/ttyUSB0
 * # Both servos move in perfect sync to their respective positions
 * @endcode
 *
 * Motion Sequence:
 * 1. Initialize arrays with servo IDs and motion parameters
 * 2. Set position targets in Position[] array
 * 3. Call SyncWritePosEx() with all arrays
 * 4. Both servos start moving at exactly the same moment
 * 5. Wait for motion to complete
 * 6. Repeat with new positions
 *
 * Sync Methods Comparison:
 * - **SyncWritePosEx**: Best - Single packet, perfect sync, minimal latency
 * - RegWrite + Action: Good - Buffered, good sync, two-phase command
 * - Sequential WritePosEx: Poor - Multiple packets, timing skew between servos
 * - Broadcast WritePosEx: Limited - Same position for all servos only
 *
 * Parameter Details:
 * - Servo IDs: ID[0]=1, ID[1]=2
 * - Position: P=4095 (maximum) and P=0 (minimum)
 * - Speed: V=60 steps/s (60 × 0.732 = 43.92 RPM) for both servos
 * - Acceleration: A=50 (50 × 8.7 = 435 deg/s²) for both servos
 * - Torque limit: T=300 (300 × 6.5 = 1950 mA max current) for both servos
 *
 * @note SyncWritePosEx() accepts arrays for all parameters, allowing each servo
 *       to have different target positions, speeds, accelerations, and torque limits.
 *
 * @note This is the recommended method for coordinated multi-servo motion in
 *       robot applications (arms, hexapods, humanoids, etc.).
 *
 * @note SyncWrite commands do not generate response packets, so error checking
 *       must be done separately using feedback reads.
 *
 * @warning Array sizes must match the servo count (parameter 2). Mismatched
 *          array sizes can cause memory access errors or undefined behavior.
 *
 * @warning Ensure all servos have mechanical clearance for their target positions.
 *
 * @see HLSCL::SyncWritePosEx()
 * @see HLSCL::RegWritePosEx() for buffered writes
 * @see HLSCL::WritePosEx() for single servo control
 */

#include <iostream>
#include "SCServo.h"

HLSCL hlscl;

u8 ID[2];
s16 Position[2];
u16 Speed[2];
u8 ACC[2];
u16 Torque[2];

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!hlscl.begin(115200, argv[1])){
        std::cout<<"Failed to init HLS motor!"<<std::endl;
        return 0;
    }

	// Initialize servo IDs
	ID[0] = 1;  // Servo ID1
	ID[1] = 2;  // Servo ID2

	// Initialize motion parameters (constant for this example)
	Speed[0] = 60;    // Max speed V=60×0.732=43.92rpm
	Speed[1] = 60;    // Max speed V=60×0.732=43.92rpm
	ACC[0] = 50;      // Acceleration A=50×8.7deg/s²
	ACC[1] = 50;      // Acceleration A=50×8.7deg/s²
	Torque[0] = 300;  // Max torque current T=300×6.5=1950mA
	Torque[1] = 300;  // Max torque current T=300×6.5=1950mA

	while(1){
		// Servos (ID1/ID2) with max speed V=60×0.732=43.92rpm, acceleration A=50×8.7deg/s²,
		// max torque current T=300×6.5=1950mA, move to P1=4095 position
		Position[0] = 4095;
		Position[1] = 4095;
		hlscl.SyncWritePosEx(ID, 2, Position, Speed, ACC, Torque);
		std::cout<<"pos = "<<4095<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);

		// Servos (ID1/ID2) with max speed V=60×0.732=43.92rpm, acceleration A=50×8.7deg/s²,
		// max torque current T=300×6.5=1950mA, move to P0=0 position
		Position[0] = 0;
		Position[1] = 0;
		hlscl.SyncWritePosEx(ID, 2, Position, Speed, ACC, Torque);
		std::cout<<"pos = "<<0<<std::endl;
		// Wait time calculation: [(P1-P0)/(V×50)]×1000 + [(V×50)/(A×100)]×1000 + 50ms (error margin)
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);
	}
	hlscl.end();
	return 1;
}

