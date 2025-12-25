/**
 * @file SyncRead.cpp
 * @brief Synchronized read example for efficient multi-servo feedback
 *
 * @details
 * This example demonstrates the synchronized read command for reading position and
 * speed from multiple servos simultaneously. SyncRead reduces bus traffic compared
 * to individual read commands and provides better timing correlation between servo states.
 *
 * Hardware Requirements:
 * - Two Feetech HLS series servos (ID: 1 and ID: 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 * - Servos connected to same serial bus (daisy-chained)
 *
 * Key Features Demonstrated:
 * - syncReadBegin(): Initialize synchronized read session
 * - syncReadPacketTx(): Send synchronized read request to multiple servos
 * - syncReadPacketRx(): Receive and decode response from each servo
 * - syncReadRxPacketToWrod(): Decode 16-bit values with direction bit
 * - syncReadEnd(): Clean up synchronized read session
 *
 * Usage:
 * @code
 * ./SyncRead /dev/ttyUSB0
 * # Continuously displays position and speed for both servos
 * @endcode
 *
 * Read Sequence:
 * 1. Initialize sync read session with timeout
 * 2. Send sync read request for position+speed from servos ID1 and ID2
 * 3. Receive response from ID1, decode position and speed
 * 4. Receive response from ID2, decode position and speed
 * 5. Display values for both servos
 * 6. Repeat at 100Hz (10ms interval)
 *
 * SyncRead vs Individual Reads:
 * - SyncRead: One request packet, N response packets (efficient)
 * - Individual: N request packets, N response packets (more bus traffic)
 * - SyncRead: Better timing correlation between servos
 * - Individual: Sequential reads have timing skew
 *
 * Parameter Details:
 * - Read start address: SMS_STS_PRESENT_POSITION_L (position low byte)
 * - Read length: 4 bytes (2 bytes position + 2 bytes speed)
 * - Timeout: 5ms (sufficient for 2 servos at 115200 baud)
 * - Direction bit: Bit 15 indicates rotation direction
 *
 * @note This example uses SMS_STS class which is compatible with HLSCL protocol.
 *       HLSCL servos can use SMS_STS sync read functions.
 *
 * @note The direction bit (bit 15) encoding:
 *       - 0 = Positive direction (CW)
 *       - 1 = Negative direction (CCW)
 *       syncReadRxPacketToWrod(15) handles this automatically.
 *
 * @warning Timeout must be sufficient for all servos to respond.
 *          Formula: timeout > (servo_count × response_time)
 *          At 115200 baud, ~2ms per servo is safe.
 *
 * @see SMS_STS::syncReadBegin()
 * @see SMS_STS::syncReadPacketTx()
 * @see SMS_STS::syncReadPacketRx()
 * @see SMS_STS::syncReadRxPacketToWrod()
 * @see SMS_STS::syncReadEnd()
 */

#include <iostream>
#include <cstdint>
#include <csignal>
#include "SCServo.h"

SMS_STS sms_sts;
uint8_t ID[] = {1, 2};
uint8_t rxPacket[4];
int16_t Position;
int16_t Speed;
volatile sig_atomic_t running = 1;

void signalHandler(int signum) {
	std::cout << "\nInterrupt signal (" << signum << ") received. Shutting down..." << std::endl;
	running = 0;
}

int main(int argc, char **argv)
{
	// Register signal handlers for graceful shutdown
	signal(SIGINT, signalHandler);
	signal(SIGTERM, signalHandler);
	
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sms_sts.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

	// Initialize sync read: 2 servos, 4 bytes per servo
	sms_sts.syncReadBegin(sizeof(ID), sizeof(rxPacket));

	while(running){
		// Send sync read request for position+speed from both servos
		sms_sts.syncReadPacketTx(ID, sizeof(ID), SMS_STS_PRESENT_POSITION_L, sizeof(rxPacket));

		for(uint8_t i=0; i<sizeof(ID); i++){
			// Receive and decode response from servo ID[i]
			if(!sms_sts.syncReadPacketRx(ID[i], rxPacket)){
				std::cout<<"ID:"<<(int)ID[i]<<" sync read error!"<<std::endl;
				continue;  // Reception/decoding failed
			}

			// Decode 2-byte words: bit15 is direction bit, parameter=15 means handle direction bit
			Position = sms_sts.syncReadRxPacketToWrod(15);  // Decode position (bit15=direction)
			Speed = sms_sts.syncReadRxPacketToWrod(15);     // Decode speed (bit15=direction)

			std::cout<<"ID:"<<int(ID[i])<<" Position:"<<Position<<" Speed:"<<Speed<<std::endl;
		}

		usleep(10*1000);  // 10ms interval (100Hz read rate)
	}

	sms_sts.syncReadEnd();
	sms_sts.end();
	return 0;
}

