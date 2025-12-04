/**
 * @file SyncRead.cpp
 * @brief Synchronized bulk feedback reading from multiple SMS/STS protocol servos
 * 
 * @details
 * This example demonstrates efficient bulk reading of position and speed data from
 * multiple servos using the Sync Read protocol. Unlike individual FeedBack() calls
 * to each servo, Sync Read sends a single query command and receives individual
 * responses from each servo, significantly reducing communication overhead for
 * multi-servo systems.
 * 
 * Hardware Requirements:
 * - Two Feetech SMS or STS protocol servos (IDs: 1, 2)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models
 * - Serial connection at 115200 baud
 * - Servos must be on same serial bus
 * 
 * Key Features Demonstrated:
 * - syncReadBegin(): Initialize sync read buffers for expected data size
 * - syncReadPacketTx(): Broadcast sync read query to all servos
 * - syncReadPacketRx(): Receive and decode individual servo responses
 * - Bulk position and speed reading from multiple servos
 * - Efficient multi-servo polling loop
 * - Error handling for missing or corrupted responses
 * 
 * Usage:
 * @code
 * ./SyncRead /dev/ttyUSB0
 * @endcode
 * 
 * Sync Read Protocol Flow:
 * 1. syncReadBegin() - Allocate buffers for 2 servos, 4 bytes per servo
 * 2. syncReadPacketTx() - Broadcast: "All servos, send position+speed data"
 * 3. For each servo:
 *    - syncReadPacketRx() - Receive 4-byte packet (2 bytes position, 2 bytes speed)
 *    - Decode position (little-endian int16)
 *    - Decode speed (little-endian int16)
 * 4. Display all data, repeat
 * 
 * Data Format:
 * - rxPacket[0:1]: Position low/high bytes (int16_t, little-endian)
 * - rxPacket[2:3]: Speed low/high bytes (int16_t, little-endian)
 * - Starting address: SMS_STS_PRESENT_POSITION_L (0x38)
 * - Data length: 4 bytes per servo
 * 
 * Performance Benefits:
 * - Single broadcast query vs N individual queries
 * - Reduced bus traffic and latency
 * - Better synchronization of multi-servo state
 * - Scalable to many servos (up to protocol limits)
 * 
 * @note Position values are signed 16-bit integers. Speed values can be positive
 *       (forward) or negative (reverse).
 * 
 * @warning If a servo fails to respond, syncReadPacketRx() will timeout and return
 *          false. The loop continues to read other servos, but missing data may
 *          indicate communication issues or servo errors.
 * 
 * @warning Ensure sufficient timeout values for all servos to respond. Adding more
 *          servos increases total response time.
 * 
 * @see SMS_STS::syncReadBegin()
 * @see SMS_STS::syncReadPacketTx()
 * @see SMS_STS::syncReadPacketRx()
 * @see SMS_STS::FeedBack() for single-servo alternative
 */
/*
Synchronous read command, reads back position and speed information of two servos ID1 and ID2
*/

#include <iostream>
#include <cstdint>
#include "SCServo.h"

SMS_STS sm_st;
uint8_t ID[] = {1, 2};
uint8_t rxPacket[4];
int16_t Position;
int16_t Speed;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }
	sm_st.syncReadBegin(sizeof(ID)/sizeof(ID[0]), sizeof(rxPacket));
	while(1){
		sm_st.syncReadPacketTx(ID, sizeof(ID)/sizeof(ID[0]), SMS_STS_PRESENT_POSITION_L, sizeof(rxPacket));//Synchronous read command packet transmission
		for(uint8_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
			//Receive ID[i] synchronous read return packet
			if(!sm_st.syncReadPacketRx(ID[i], rxPacket)){
				std::cout<<"ID:"<<(int)ID[i]<<" sync read error!"<<std::endl;
				continue;//Reception decoding failed
			}
			Position = sm_st.syncReadRxPacketToWrod(15);//Decode two bytes, bit15 is direction bit, parameter=0 means no direction bit
			Speed = sm_st.syncReadRxPacketToWrod(15);//Decode two bytes, bit15 is direction bit, parameter=0 means no direction bit
			std::cout<<"ID:"<<int(ID[i])<<" Position:"<<Position<<" Speed:"<<Speed<<std::endl;
		}
		usleep(10*1000);
	}
	sm_st.syncReadEnd();
	sm_st.end();
	return 1;
}

