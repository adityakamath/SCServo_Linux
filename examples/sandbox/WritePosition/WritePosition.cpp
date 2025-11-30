/**
 * @file WritePosition.cpp
 * @brief Example: Write position commands to servo
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
s16 P0 = 2048; // = 1/2 rotation = Pi radians
s16 P1 = 4095; // = 1 rotation = 2*Pi radians
u16 V = 2400; // steps/s
u8 A = 50; //*100 steps/s^2

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
        int mode_ret = sm_st.writeByte(ID[i], SMS_STS_MODE, 0); // Set position mode
        std::cout << "Set Operating_Mode=0 (position) for motor " << (int)ID[i] << " (ret=" << mode_ret << ")" << std::endl;
        usleep(100000);
        int acc_ret = sm_st.writeByte(ID[i], SMS_STS_ACC, A); // Set acceleration
        std::cout << "Set Acceleration=" << (int)A << " for motor " << (int)ID[i] << " (ret=" << acc_ret << ")" << std::endl;
        usleep(100000);
        sm_st.CalibrationOfs(ID[i]); //set starting Pos=2048 (midpoint or Pi radians)
        int torque_ret = sm_st.EnableTorque(ID[i], 1);
        std::cout << "Enable Torque for motor " << (int)ID[i] << " (ret=" << torque_ret << ")" << std::endl;
        usleep(100000);
    }
    usleep(500000);
    
	while(1){
        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePosEx(ID[i], P1, V, A);//go to Pos=4095 with Vel=2400 steps/s and Acc=50*100 steps/s^2
        }
		std::cout<<"pos = "<<static_cast<int>(P1)<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000

        for(size_t i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            sm_st.WritePosEx(ID[i], P0, V, A);//go to Pos=2048 with Vel=2400 steps/s and Acc=50*100 steps/s^2
        }
		std::cout<<"pos = "<<static_cast<int>(P0)<<std::endl;
		sleep(2);//execution time = 2s, max = [(P1-P0)/V]*1000+[V/(A*100)]*1000
	}
	sm_st.end();
	return 1;
}

