/**
 * @file FeedBack.cpp
 * @brief Real-time servo feedback monitoring for HLSCL protocol servos
 *
 * @details
 * This example demonstrates two methods of reading servo feedback data:
 * 1. Bulk read using FeedBack() then accessing cached data with Read*(-1)
 * 2. Individual parameter reads using Read*(ID) for specific values
 *
 * The bulk read method is more efficient for reading multiple parameters,
 * while individual reads are useful for monitoring specific values.
 *
 * Hardware Requirements:
 * - Feetech HLS series servo (ID: 1)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo model (typically 7.4V-12V)
 * - Serial connection at 115200 baud
 *
 * Key Features Demonstrated:
 * - FeedBack(): Efficient bulk read of all servo status registers
 * - ReadPos(): Current position in steps
 * - ReadSpeed(): Current velocity in steps/second
 * - ReadLoad(): Current load torque
 * - ReadVoltage(): Input voltage in 0.1V units
 * - ReadTemper(): Internal temperature in degrees Celsius
 * - ReadMove(): Motion status (1=moving, 0=stopped)
 * - ReadCurrent(): Current draw in milliamps
 * - Comparison of bulk read vs individual read methods
 *
 * Usage:
 * @code
 * ./FeedBack /dev/ttyUSB0
 * @endcode
 *
 * Reading Methods:
 * - Method 1 (Bulk): FeedBack(ID) then Read*(-1) for cached data
 *   - More efficient: Single bus transaction for all parameters
 *   - Lower latency: All data read simultaneously
 *   - Recommended for real-time monitoring
 *
 * - Method 2 (Individual): Direct Read*(ID) calls
 *   - More flexible: Read only needed parameters
 *   - Higher latency: Separate bus transaction per parameter
 *   - Useful for spot-checking specific values
 *
 * @note The -1 parameter in Read*(-1) indicates using cached data from
 *       the most recent FeedBack() call, avoiding redundant bus traffic.
 *
 * @warning Monitor temperature and voltage to prevent servo damage.
 *          Typical safe operating ranges vary by model.
 *
 * @see HLSCL::FeedBack()
 * @see HLSCL::ReadPos(), HLSCL::ReadSpeed(), HLSCL::ReadLoad()
 * @see HLSCL::ReadVoltage(), HLSCL::ReadTemper(), HLSCL::ReadCurrent()
 */

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
        std::cout<<"Failed to init HLS motor!"<<std::endl;
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

		// Method 1: Bulk read - one command returns all feedback data
		if(hlscl.FeedBack(1)!=-1){
			Pos = hlscl.ReadPos(-1);      // -1 means use cached data, same below
			Speed = hlscl.ReadSpeed(-1);
			Load = hlscl.ReadLoad(-1);
			Voltage = hlscl.ReadVoltage(-1);
			Temper = hlscl.ReadTemper(-1);
			Move = hlscl.ReadMove(-1);
			Current = hlscl.ReadCurrent(-1);
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

		// Method 2: Individual reads - one command per parameter
		Pos = hlscl.ReadPos(1);
		if(Pos!=-1){
			std::cout<<"pos = "<<Pos<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read pos err"<<std::endl;
			sleep(1);
		}

		Voltage = hlscl.ReadVoltage(1);
		if(Voltage!=-1){
			std::cout<<"Voltage = "<<Voltage<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Voltage err"<<std::endl;
			sleep(1);
		}

		Temper = hlscl.ReadTemper(1);
		if(Temper!=-1){
			std::cout<<"temperature = "<<Temper<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read temperature err"<<std::endl;
			sleep(1);
		}

		Speed = hlscl.ReadSpeed(1);
		if(Speed!=-1){
			std::cout<<"Speed = "<<Speed<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Speed err"<<std::endl;
			sleep(1);
		}

		Load = hlscl.ReadLoad(1);
		if(Load!=-1){
			std::cout<<"Load = "<<Load<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Load err"<<std::endl;
			sleep(1);
		}

		Current = hlscl.ReadCurrent(1);
		if(Current!=-1){
			std::cout<<"Current = "<<Current<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Current err"<<std::endl;
			sleep(1);
		}

		Move = hlscl.ReadMove(1);
		if(Move!=-1){
			std::cout<<"Move = "<<Move<<std::endl;
			usleep(10*1000);
		}else{
			std::cout<<"read Move err"<<std::endl;
			sleep(1);
		}
	}
	hlscl.end();
	return 1;
}
