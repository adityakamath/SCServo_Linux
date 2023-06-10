# SCServo_Linux
Copy of SCServo_Linux SDK from [Feetech's official repository](https://gitee.com/ftservo/SCServoSDK/tree/master) with personal modifications for the SMS/STS series of serial bus servo motors. Uses [the 29/03/2022 release of the original SDK](https://gitee.com/ftservo/SCServoSDK/blob/master/SCServo_Linux_220329.7z).

## Usage

1. Clone this repository and enter the directory: `cd SCServo_Linux`
2. Build SCServo libraries: `cmake . && make`
3. Choose an example and enter the directory: `cd examples/SMS_STS/WritePos`
4. Build the example: `cmake . && make`
5. Execute the example: `sudo ./WritePos /dev/ttyUSB0`
   
Note: Adjust /dev/ttyUSB0 according to the serial port of your device
Note: The example provided is for the WritePos program in the SMS/STS folder under the aforementioned directory. Choose the appropriate testing directory based on your specific requirements. 

## Structure

* Communication layer: SCS
* Hardware interface layer: SCSerial
* Application layer:
    * SMSBL SMSCL SCSCL correspond to the three series of Feetech respectively
    * SMS_STS is compatible with SMS/STS series servos

### Definition

```
SMSBL sm;      //Define SMSBL series servo
SMSCL sm;      //Define SMSCL series servo
SCSCL sc;      //Define SCSCL series servo
SMS_STS sm_st; //Define SMS or STS series servo
```

For each interface of SMSCL\SMSBL\SCSCL\SMS_STS, refer to the corresponding header file

### Header files

* INST.h:                  Instruction definition header file
* SCS.h/SCS.cpp:           Communication layer program
* SCSerial.h/SCSerial.cpp: Hardware interface program
* SMSBL.h/SMSBL.cpp:       SMSBL Application layer program
* SMSCL.h/SMSCL.cpp:       SMSCL Application layer program
* SCSCL.h/SCSCL.cpp:       SCSCL Application layer program
* SMS_STS.h/SMS_STS.cpp:   SMS/STS Application layer program

(The memory table is defined in the application layer program header file: SMSBL.h\SMSCL.h\SCSCL.h\SMS_STS.h. There are differences in the definition of the memory table for different series of servos)

```
                               SMSBL class
SCS class<---SCSerial class<---SMSCL class
                               SCSCL class
                               SMS_STS class
```

## Modified examples

My modified examples used for testing on my robot can be found in `examples/sandbox/`. The normal examples have comments written in Chinese, but I've updated the examples in the sandbox with English comments. 
