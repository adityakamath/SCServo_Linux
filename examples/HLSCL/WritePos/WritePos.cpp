#include <iostream>
#include "SCServo.h"

HLSCL hlscl;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!hlscl.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }
	while(1){
		//舵机(ID1)以最高速度V=60*0.732=43.92rpm，加速度A=50*8.7deg/s^2，最大扭矩电流T=500*6.5=3250mA，运行至P1=4095位置
		hlscl.WritePosEx(1, 4095, 60, 50, 500);
		std::cout<<"pos = "<<4095<<std::endl;
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);//[(P1-P0)/(V*50)]*1000+[(V*50)/(A*100)]*1000 + 50(误差)
  
		//舵机(ID1)以最高速度V=60*0.732=43.92rpm，加速度A=50*8.7deg/s^2，最大扭矩电流T=500*6.5=3250mA，运行至P0=0位置
		hlscl.WritePosEx(1, 0, 60, 50, 500);
		std::cout<<"pos = "<<0<<std::endl;
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);//[(P1-P0)/(V*50)]*1000+[(V*50)/(A*100)]*1000 + 50(误差)
	}
	hlscl.end();
	return 1;
}

