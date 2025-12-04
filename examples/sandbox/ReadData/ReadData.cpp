/**
 * @file ReadData.cpp
 * @brief Multi-servo real-time data monitoring with SI unit conversions
 * 
 * @details
 * This example demonstrates comprehensive real-time monitoring of multiple SMS/STS servos
 * with automatic conversion from raw register values to standard engineering units (radians,
 * rad/s, %, V, °C, mA). It's designed for robotics applications requiring human-readable
 * feedback data for closed-loop control, diagnostics, or data logging. The program polls
 * all three servos continuously and displays formatted telemetry until interrupted.
 * 
 * Hardware Requirements:
 * - Three Feetech SMS or STS protocol servos (IDs: 7, 8, 9)
 * - USB-to-Serial adapter or direct serial port
 * - Power supply appropriate for servo models
 * - Serial connection at 1000000 baud (1Mbps for STS, use 115200 for SMS)
 * 
 * Key Features Demonstrated:
 * - Multi-servo polling loop with FeedBack() + ReadXxx()
 * - Unit conversion from raw values to SI/engineering units
 * - Real-time telemetry display for human readability
 * - Graceful shutdown with signal handler (Ctrl+C)
 * - Array-based servo management
 * 
 * Usage:
 * @code
 * ./ReadData /dev/ttyUSB0
 * # Displays continuous telemetry:
 * # Motor=7 Pos=1.234rad Speed=0.456rad/sec Load=23.4% ...
 * # Motor=8 Pos=2.345rad Speed=-0.123rad/sec Load=12.1% ...
 * # Motor=9 Pos=3.456rad Speed=0.789rad/sec Load=45.6% ...
 * # Press Ctrl+C to stop
 * @endcode
 * 
 * Unit Conversions (based on Feetech documentation):
 * - Position: steps → radians
 *   - Raw: 0-4095 (12-bit, 0.088°/step)
 *   - Formula: radians = steps * 2π / 4096
 *   - Example: 2048 steps = π radians = 180°
 * 
 * - Speed: steps/second → radians/second
 *   - Raw: signed integer (+ = CCW, - = CW)
 *   - Formula: rad/s = (steps/s) * 2π / 4096
 *   - Example: 50 steps/s ≈ 0.0767 rad/s ≈ 0.732 RPM
 * 
 * - Load: raw → percentage
 *   - Raw: -1000 to +1000 (signed for direction)
 *   - Formula: percent = raw / 10.0
 *   - Example: 500 raw = 50.0% load
 * 
 * - Voltage: raw → volts
 *   - Raw: integer in 0.1V units
 *   - Formula: volts = raw / 10.0
 *   - Example: 120 raw = 12.0V
 * 
 * - Temperature: raw → degrees Celsius
 *   - Raw: integer in 1°C units
 *   - Formula: °C = raw (no conversion)
 *   - Example: 45 raw = 45°C
 * 
 * - Current: raw → milliamperes
 *   - Raw: integer in 6.5mA units
 *   - Formula: mA = raw * 6.5
 *   - Example: 100 raw = 650mA
 * 
 * - Move: raw → boolean
 *   - Raw: 0 or 1
 *   - Meaning: 0 = stopped/at target, 1 = in motion
 * 
 * Data Sources:
 * - Feetech official documentation: https://www.feetechrc.com/en/2020-05-13_56655.html
 * - Waveshare ST3215 wiki: https://www.waveshare.com/wiki/ST3215_Servo
 * 
 * @note Loop contains division by zero bug: `sizeof(ID)/sizeof(ID[0])/sizeof(ID[0])`
 *       should be `sizeof(ID)/sizeof(ID[0])`. Fix before use or loop won't execute.
 * 
 * @warning High polling rates may saturate serial bus bandwidth. Adjust loop delay
 *          based on application requirements and number of servos.
 * 
 * @see SMS_STS::FeedBack()
 * @see SMS_STS::ReadPos(), ReadSpeed(), ReadLoad(), ReadVoltage(), ReadTemper(), ReadMove(), ReadCurrent()
 */
#include <iostream>
#include <csignal>
#include <cmath>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {7, 8, 9};

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

		for(size_t i=0; i<sizeof(ID)/sizeof(ID[0])/sizeof(ID[0]); i++){
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

