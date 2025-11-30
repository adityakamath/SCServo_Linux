/**
 * @file ProgramEprom.cpp
 * @brief Example: Program EEPROM parameters
 * 
 * Demonstrates usage of SCServo library functions for Feetech serial servos.
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

	sm_st.unLockEprom(1);//Enable EPROM save function
	std::cout<<"unLock Eprom"<<std::endl;
	sm_st.writeByte(1, SMSBL_ID, 2);//ID
	std::cout<<"write ID:"<<2<<std::endl;
	sm_st.LockEprom(2);//Disable EPROM save function
	std::cout<<"Lock Eprom"<<std::endl;
	sm_st.end();
	return 1;
}

