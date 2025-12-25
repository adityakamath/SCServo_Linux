#include <iostream>
#include "SCServo.h"

HLSCL hlscl;

u8 ID[2];
s16 Position[2];
u16 Speed[2];
u8 ACC[2];
u16 Torque[2];

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
	ID[0] = 1;//舵机ID1
	ID[1] = 2;//舵机ID2
	Speed[0] = 60;//最高速度V=60*0.732=43.92rpm
	Speed[1] = 60;//最高速度V=60*0.732=43.92rpm
	ACC[0] = 50;//加速度A=50*8.7deg/s^2
	ACC[1] = 50;//加速度A=50*8.7deg/s^2
	Torque[0] = 300;//最大扭矩电流T=500*6.5=3250mA
	Torque[1] = 300;//最大扭矩电流T=500*6.5=3250mA
	while(1){
		//舵机(ID1/ID2)以最高速度V=60*0.732=43.92rpm，加速度A=50*8.7deg/s^2，最大扭矩电流T=500*6.5=3250mA，运行至P1=4095位置
		Position[0] = 4095;
		Position[1] = 4095;
		hlscl.SyncWritePosEx(ID, 2, Position, Speed, ACC, Torque);
		std::cout<<"pos = "<<4095<<std::endl;
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);//[(P1-P0)/(V*50)]*1000+[(V*50)/(A*100)]*1000 + 50(误差)
  
		//舵机(ID1/ID2)以最高速度V=60*0.732=43.92rpm，加速度A=50*8.7deg/s^2，最大扭矩电流T=500*6.5=3250mA，运行至P0=0位置
		Position[0] = 0;
		Position[1] = 0;
		hlscl.SyncWritePosEx(ID, 2, Position, Speed, ACC, Torque);
		std::cout<<"pos = "<<0<<std::endl;
		usleep(((4095-0)*1000/(60*50)+(60*50)*10/(50)+50)*1000);//[(P1-P0)/(V*50)]*1000+[(V*50)/(A*100)]*1000 + 50(误差)
	}
	hlscl.end();
	return 1;
}

