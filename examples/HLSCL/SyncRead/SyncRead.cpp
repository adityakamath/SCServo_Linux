/*
同步读指令，回读ID1与ID2两个舵机的位置与速度信息
*/

#include <iostream>
#include "SCServo.h"

SMS_STS sms_sts;
uint8_t ID[] = {1, 2};
uint8_t rxPacket[4];
int16_t Position;
int16_t Speed;

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<<"argc error!"<<std::endl;
        return 0;
	}
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sms_sts.begin(115200, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }
	sms_sts.syncReadBegin(sizeof(ID), sizeof(rxPacket), 5);//10*10*2=200us<5ms
	while(1){
		sms_sts.syncReadPacketTx(ID, sizeof(ID), SMS_STS_PRESENT_POSITION_L, sizeof(rxPacket));//同步读指令包发送
		for(uint8_t i=0; i<sizeof(ID); i++){
			//接收ID[i]同步读返回包
			if(!sms_sts.syncReadPacketRx(ID[i], rxPacket)){
				std::cout<<"ID:"<<(int)ID[i]<<" sync read error!"<<std::endl;
				continue;//接收解码失败
			}
			Position = sms_sts.syncReadRxPacketToWrod(15);//解码两个字节 bit15为方向位,参数=0表示无方向位
			Speed = sms_sts.syncReadRxPacketToWrod(15);//解码两个字节 bit15为方向位,参数=0表示无方向位
			std::cout<<"ID:"<<int(ID[i])<<" Position:"<<Position<<" Speed:"<<Speed<<std::endl;
		}
		usleep(10*1000);
	}
	sms_sts.syncReadEnd();
	sms_sts.end();
	return 1;
}

