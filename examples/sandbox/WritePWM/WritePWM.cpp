/*
The factory speed of the servo motor is 0.0146rpm, and the speed is changed to V=2400 steps/sec
*/

#include <iostream>
#include <csignal>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {11, 12, 13};
s16 Zero[3] = {0, 0, 0};

s16 Pwm1[3] = {500, 500, 500};
s16 Pwm2[3] = {-500, -500, -500};

void signalHandler(int signum) {
    if (signum == SIGINT) {
        for(int i=0; i<sizeof(ID); i++){
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

    for(int i=0; i<sizeof(ID); i++){
        sm_st.Mode(ID[i], 2); //open loop wheel mode
    }
    
	while(1){
        for(int i=0; i<sizeof(ID); i++){
            sm_st.WritePwm(ID[i], Pwm1[i]); //ID, PWM=500 (50%, forward)
        }
		std::cout<<"PWM = "<<50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID); i++){
            sm_st.WritePwm(ID[i], Zero[i]); //ID, PWM=0 (0%, stop)
        }
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);

        for(int i=0; i<sizeof(ID); i++){
            sm_st.WritePwm(ID[i], Pwm2[i]); //ID, PWM=-500 (50%, reverse)
        }
		std::cout<<"PWM = "<<-50<<"%"<<std::endl;
		sleep(2);
        
        for(int i=0; i<sizeof(ID); i++){
            sm_st.WritePwm(ID[i], Zero[i]); //ID, PWM=0 (0%, stop)
        }
		std::cout<<"PWM = "<<0<<"%"<<std::endl;
		sleep(2);
	}
	sm_st.end();
	return 1;
}

