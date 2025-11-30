/**
 * @file RegWriteSpeed.cpp
 * @brief Example: Register write speed for synchronized execution
 * 
 * Demonstrates usage of SCServo library functions for Feetech serial servos.
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
        sm_st.Mode(ID[i], 1); //closed loop wheel mode
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

