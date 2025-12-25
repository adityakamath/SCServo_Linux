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

	hlscl.unLockEprom(1);//打开EPROM保存功能
	std::cout<<"unLock Eprom"<<std::endl;
	hlscl.writeByte(1, HLSCL_ID, 2);//ID
	std::cout<<"write ID:"<<2<<std::endl;
	hlscl.LockEprom(2);////关闭EPROM保存功能
	std::cout<<"Lock Eprom"<<std::endl;
	hlscl.end();
	return 1;
}

