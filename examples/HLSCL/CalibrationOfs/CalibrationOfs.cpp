#include <iostream>
#include "SCServo.h"

HLSCL hlscl;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<< "argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!hlscl.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

	hlscl.CalibrationOfs(1);
	std::cout<<"Calibration Ofs"<<std::endl;
	while(1){
	  int pos = hlscl.ReadPos(1);
	  if(!hlscl.getLastError()){
		  std::cout<<"mid pos:"<<pos<<std::endl;
	  }
	}
	hlscl.end();
	return 1;
}

