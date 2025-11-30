/**
 * @file WriteSpe.cpp
 * @brief Example: Write speed commands for wheel mode
 * 
 * Demonstrates usage of SCServo library functions for Feetech serial servos.
 */
/*
Factory speed unit of servo is 0.0146rpm, speed changed to V=2400
*/


#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;

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
	sm_st.Mode(1, 1);//Constant speed mode
	std::cout<<"mode = "<<1<<std::endl;
	while(1){
		sm_st.WriteSpe(1, 2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), rotate
		std::cout<<"speed = "<<2400<<std::endl;
		sleep(2);
		sm_st.WriteSpe(1, 0, 50);//Servo (ID1) with acceleration A=50 (50*100 steps/second^2), stop rotating (V=0)
		std::cout<<"speed = "<<0<<std::endl;
		sleep(2);
		sm_st.WriteSpe(1, -2400, 50);//Servo (ID1) with maximum speed V=2400 (steps/second), acceleration A=50 (50*100 steps/second^2), rotate in reverse
		std::cout<<"speed = "<<-2400<<std::endl;
		sleep(2);
		sm_st.WriteSpe(1, 0, 50);//Servo (ID1) with acceleration A=50 (50*100 steps/second^2), stop rotating (V=0)
		std::cout<<"speed = "<<0<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

