/**
 * @file SyncRead.cpp
 * @brief Example: Synchronized read from multiple servos
 * 
 * Demonstrates usage of SCServo library functions for Feetech serial servos.
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

