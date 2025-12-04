/**
 * @file FeedBack.cpp
 * @brief Comprehensive servo feedback data reading example for SCSCL protocol servos
 * 
 * @details
 * This example demonstrates real-time reading of all available feedback data from
 * Feetech SCSCL protocol servos. It shows how to efficiently query and display
 * position, speed, load, voltage, temperature, motion status, and current draw
 * using a single feedback command followed by multiple read operations.
 * 
 * Hardware Requirements:
 * - Feetech SCSCL protocol servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model
 * - Serial connection at 115200 baud
 * 
 * Key Features Demonstrated:
 * - Single FeedBack() call to request all servo data
 * - ReadPos(-1): Read position from last feedback (no new query)
 * - ReadSpeed(-1): Read speed from last feedback
 * - ReadLoad(-1): Read load torque from last feedback
 * - ReadVoltage(-1): Read supply voltage from last feedback
 * - ReadTemper(-1): Read internal temperature from last feedback
 * - ReadMove(-1): Read motion status from last feedback
 * - ReadCurrent(-1): Read current draw from last feedback
 * - Continuous monitoring loop for real-time data display
 * 
 * Usage:
 * @code
 * ./FeedBack /dev/ttyUSB0
 * @endcode
 * 
 * Parameter Details:
 * - ID=1: Target servo to read feedback from
 * - ReadXxx(-1): -1 indicates reading from previously fetched data (no new serial query)
 * 
 * @note The -1 parameter in Read functions retrieves data from the last FeedBack() call
 *       without sending a new query, making bulk data reading very efficient.
 * 
 * @warning Ensure servo ID matches the physical servo configuration before running.
 *          Invalid IDs will result in no data being returned.
 * 
 * @see SCSCL::FeedBack()
 * @see SCSCL::ReadPos()
 * @see SCSCL::ReadSpeed()
 * @see SCSCL::ReadLoad()
 * @see SCSCL::ReadVoltage()
 * @see SCSCL::ReadTemper()
 * @see SCSCL::ReadMove()
 * @see SCSCL::ReadCurrent()
 */
#include <iostream>
#include "SCServo.h"

SCSCL sc;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sc.begin(115200, argv[1])){
        std::cout<<"Failed to init scscl motor!"<<std::endl;
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
		// Single command retrieves all feedback data
		if(sc.FeedBack(1)!=-1){
			Pos = sc.ReadPos(-1);      // -1 indicates reading from previously fetched data, same below
			Speed = sc.ReadSpeed(-1);
			Load = sc.ReadLoad(-1);
			Voltage = sc.ReadVoltage(-1);
			Temper = sc.ReadTemper(-1);
			Move = sc.ReadMove(-1);
			Current = sc.ReadCurrent(-1);
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
		//һ��ָ���һ����������
		Pos = sc.ReadPos(1);
		if(Pos!=-1){
			std::cout<<"pos = "<<Pos<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read pos err"<<std::endl;
			sleep(1);
		}
		Voltage = sc.ReadVoltage(1);
		if(Voltage!=-1){
			std::cout<<"Voltage = "<<Voltage<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Voltage err"<<std::endl;
			sleep(1);
		}

		Temper = sc.ReadTemper(1);
		if(Temper!=-1){
			std::cout<<"temperature = "<<Temper<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read temperature err"<<std::endl;
			sleep(1);
		}

		Speed = sc.ReadSpeed(1);
		if(Speed!=-1){
			std::cout<<"Speed = "<<Speed<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Speed err"<<std::endl;
			sleep(1);
		}
  
		Load = sc.ReadLoad(1);
		if(Load!=-1){
			std::cout<<"Load = "<<Load<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Load err"<<std::endl;
			sleep(1);
		}

		Current = sc.ReadCurrent(1);
		if(Current!=-1){
			std::cout<<"Current = "<<Current<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Current err"<<std::endl;
			sleep(1);
		}

		Move = sc.ReadMove(1);
		if(Move!=-1){
			std::cout<<"Move = "<<Move<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Move err"<<std::endl;
			sleep(1);
		}
	}
	sc.end();
	return 1;
}

