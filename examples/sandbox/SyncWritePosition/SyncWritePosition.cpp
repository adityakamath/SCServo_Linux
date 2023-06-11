/*
The factory speed of the servo motor is 0.0146rpm, and the speed is changed to V=2400 steps/sec
*/

#include <iostream>
#include <csignal>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {11, 12, 13};
s16 P0[3] = {2048, 2048, 2048}; // = 1/2 rotation = Pi radians
s16 P1[3] = {4095, 4095, 4095}; // = 1 rotation = 2 Pi radians
u16 V[3] = {2400, 2400, 2400}; // steps/s
u8 A[3] = {50, 50, 50}; //*100 steps/s^2

void signalHandler(int signum) {
    if (signum == SIGINT) {
        sm_st.SyncWritePosEx(ID, sizeof(ID), P0, V, A);//go to Pos=2048 with Vel=2400 steps/s and Acc=50*100 steps/s^2
		std::cout<<"Terminated! pos = 2048, 2048, 2048"<<std::endl;
        sm_st.end();
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
        sm_st.Mode(ID[i], 0); //servo mode
        sm_st.CalibrationOfs(ID[i]); //set starting Pos=2048 (midpoint or Pi radians)
    }
    
	while(1){
        sm_st.SyncWritePosEx(ID, sizeof(ID), P1, V, A);//go to Pos=4095 with Vel=2400 steps/s and Acc=50*100 steps/s^2
		std::cout<<"pos = 4095, 1024, 0"<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
  
        sm_st.SyncWritePosEx(ID, sizeof(ID), P0, V, A);//go to Pos=2048 with Vel=2400 steps/s and Acc=50*100 steps/s^2
		std::cout<<"pos = 2048, 2048, 2048"<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

