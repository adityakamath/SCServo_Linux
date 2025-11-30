/**
 * @file WriteSpeed.cpp
 * @brief Example: Write speed commands for wheel mode
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
s16 Speed1[3] = {-1700, -1700, -1700}; //forward
s16 Speed2[3] = {1700, 1700, 1700}; //reverse
u8 Acc[3] = {50, 50, 50}; // 0 to 254

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
        int mode_ret = sm_st.writeByte(ID[i], SMS_STS_MODE, 1); // Set velocity mode
        std::cout << "Set Operating_Mode=1 (velocity) for motor " << (int)ID[i] << " (ret=" << mode_ret << ")" << std::endl;
        usleep(100000);
        int acc_ret = sm_st.writeByte(ID[i], SMS_STS_ACC, Acc[i]); // Set acceleration
        std::cout << "Set Acceleration=" << (int)Acc[i] << " for motor " << (int)ID[i] << " (ret=" << acc_ret << ")" << std::endl;
        usleep(100000);
        int torque_ret = sm_st.EnableTorque(ID[i], 1);
        std::cout << "Enable Torque for motor " << (int)ID[i] << " (ret=" << torque_ret << ")" << std::endl;
        usleep(100000);
    }
    usleep(500000);
    
	while(1){
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Speed1[i], Acc[i]); //ID, Speed=2000 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<50<<"%"<<std::endl;
		sleep(2);
        
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Zero[i], Acc[i]); //ID, Speed=0 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<0<<"%"<<std::endl;
		sleep(2);

        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Speed2[i], Acc[i]); //ID, Speed=-2000 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WriteSpe(ID[i], Zero[i], Acc[i]); //ID, Speed=0 steps/s, Acc=50*100 steps/s^2
        }
		std::cout<<"Speed = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

