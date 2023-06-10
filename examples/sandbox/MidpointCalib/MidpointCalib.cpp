#include <iostream>
#include "SCServo.h"

SMS_STS sm_st;

u8 ID[3] = {11, 12, 13};

int main(int argc, char **argv)
{
	if(argc<2){
        std::cout<< "argc error!"<<std::endl;
        return 0;
	}
    
	std::cout<<"serial:"<<argv[1]<<std::endl;
    if(!sm_st.begin(1000000, argv[1])){
        std::cout<<"Failed to init sms/sts motor!"<<std::endl;
        return 0;
    }

    for(int i=0; i<sizeof(ID); i++){
        sm_st.Mode(ID[i], 0); //set to servo mode
        sm_st.CalibrationOfs(ID[i]); //set starting position as midpoint: 2048 (=Pi radians)
    }

	std::cout<<"Calibration complete!"<<std::endl;
	sm_st.end();
	return 1;
}

