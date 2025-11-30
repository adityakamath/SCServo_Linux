/**
 * @file FeedBack.cpp
 * @brief Example: Read all feedback data from servo
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
	while(1){
		int Pos;
		int Speed;
		int Load;
		int Voltage;
		int Temper;
		int Move;
		int Current;
		//One command returns all servo feedback information
		if(sm_st.FeedBack(1)!=-1){
			Pos = sm_st.ReadPos(-1);//-1 means read cached data, same below
			Speed = sm_st.ReadSpeed(-1);
			Load = sm_st.ReadLoad(-1);
			Voltage = sm_st.ReadVoltage(-1);
			Temper = sm_st.ReadTemper(-1);
			Move = sm_st.ReadMove(-1);
			Current = sm_st.ReadCurrent(-1);
			std::cout<<"pos = "<<Pos<<" ";
			std::cout<<"Speed = "<<Speed<<" ";
			std::cout<<"Load = "<<Load<<" ";
			std::cout<<"Voltage = "<<Voltage<<" ";
			std::cout<<"Temper = "<<Temper<<" ";
			std::cout<<"Move = "<<Move<<" ";
			std::cout<<"Current = "<<Current<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read err"<<std::endl;
			sleep(1);
		}
		//One command reads one parameter
		Pos = sm_st.ReadPos(1);
		if(Pos!=-1){
			std::cout<<"pos = "<<Pos<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read pos err"<<std::endl;
			sleep(1);
		}
		Voltage = sm_st.ReadVoltage(1);
		if(Voltage!=-1){
			std::cout<<"Voltage = "<<Voltage<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Voltage err"<<std::endl;
			sleep(1);
		}

		Temper = sm_st.ReadTemper(1);
		if(Temper!=-1){
			std::cout<<"temperature = "<<Temper<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read temperature err"<<std::endl;
			sleep(1);
		}

		Speed = sm_st.ReadSpeed(1);
		if(Speed!=-1){
			std::cout<<"Speed = "<<Speed<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Speed err"<<std::endl;
			sleep(1);
		}
  
		Load = sm_st.ReadLoad(1);
		if(Load!=-1){
			std::cout<<"Load = "<<Load<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Load err"<<std::endl;
			sleep(1);
		}

		Current = sm_st.ReadCurrent(1);
		if(Current!=-1){
			std::cout<<"Current = "<<Current<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Current err"<<std::endl;
			sleep(1);
		}

		Move = sm_st.ReadMove(1);
		if(Move!=-1){
			std::cout<<"Move = "<<Move<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Move err"<<std::endl;
			sleep(1);
		}
	}
	sm_st.end();
	return 1;
}

