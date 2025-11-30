/**
 * @file SyncWritePos.cpp
 * @brief Example: Synchronized position write to multiple servos
 * 
 * Demonstrates usage of SCServo library functions for Feetech serial servos.
 */
/*
Factory speed unit of servo is 0.0146rpm, speed changed to V=2400
*/

#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[2] = {1, 2};
s16 Position[2];
u16 Speed[2] = {2400, 2400};
u8 ACC[2] = {50, 50};

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
	while(1){
		Position[0] = 4095;
		Position[1] = 4095;
		sm_st.SyncWritePosEx(ID, sizeof(ID)/sizeof(ID[0]), Position, Speed, ACC);//Servos (ID1/ID2) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P1=4095
		std::cout<<"pos = "<<4095<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
  
		Position[0] = 0;
		Position[1] = 0;
		sm_st.SyncWritePosEx(ID, sizeof(ID)/sizeof(ID[0]), Position, Speed, ACC);//Servos (ID1/ID2) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), move to position P0=0
		std::cout<<"pos = "<<0<<std::endl;
		usleep(2187*1000);//[(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

