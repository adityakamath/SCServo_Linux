#include <iostream>
#include <csignal>
#include <cmath>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {11, 12, 13};

void signalHandler(int signum) {
    if (signum == SIGINT) {
		std::cout<<"Terminated!"<<std::endl;
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
    
	while(1){
		float Pos[3];
		float Speed[3];
		float Load[3];
		float Voltage[3];
		float Temper[3];
		float Move[3];
		float Current[3];

		for(int i=0; i<sizeof(ID)/sizeof(ID[0]); i++){
            if(sm_st.FeedBack(ID[i])!=-1){
                // Conversions here: https://www.feetechrc.com/en/2020-05-13_56655.html
    			Pos[i] = sm_st.ReadPos(ID[i])*2*M_PI/4096.0; // 1 step=2*PI/4096.0 rad, 
    			Speed[i] = sm_st.ReadSpeed(ID[i])*2*M_PI/4096.0; // 1 steps/s=2*PI/4096.0 rads/sec (50 steps/s≈0.732RPM https://www.waveshare.com/wiki/ST3215_Servo)
    			Load[i] = sm_st.ReadLoad(ID[i])/10.0; // 0-1000 : 0-100%
                Move[i] = sm_st.ReadMove(ID[i]); // 1True, 0=False
                Temper[i] = sm_st.ReadTemper(ID[i]); // 1 : 1 degree Celcius
    			Voltage[i] = sm_st.ReadVoltage(ID[i])/10.0; // 1 : 0.1V
    			Current[i] = sm_st.ReadCurrent(ID[i])*6.5; // 1 : 6.5mA
                std::cout<<"Motor="<<static_cast<int>(ID[i])<<" ";
    			std::cout<<"Pos="<<Pos[i]<<"rad ";
    			std::cout<<"Speed="<<Speed[i]<<"rad/sec ";
    			std::cout<<"PWM="<<Load[i]<<"% ";
                std::cout<<"Move="<<Move[i]<<" ";
                std::cout<<"Temperature="<<Temper[i]<<"deg ";
    			std::cout<<"Voltage="<<Voltage[i]<<"V ";
    			std::cout<<"Current="<<Current[i]<<"mA ";
                std::cout<<std::endl;
    			
    		}else{
                std::cout<<"Motor="<<static_cast<int>(ID[i])<<" ";
    			std::cout<<"read err"<<std::endl;
    		}
        }
        std::cout<<"------------------------------------------------------"<<std::endl;
        sleep(1);
	}
	sm_st.end();
	return 1;
}

