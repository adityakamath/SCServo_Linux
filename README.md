# SCServo_Linux
Copy of SCServo_Linux SDK from [Feetech's official repository](https://gitee.com/ftservo/SCServoSDK/tree/master) with personal modifications for the SMS/STS series of serial bus servo motors. Uses [the 29/03/2022 release of the original SDK](https://gitee.com/ftservo/SCServoSDK/blob/master/SCServo_Linux_220329.7z).

## Usage

1. Clone this repository and enter the directory: `cd SCServo_Linux`
2. Build SCServo libraries: `cmake . && make`
3. Choose an example and enter the directory: `cd examples/SMS_STS/WritePos`
4. Build the example: `cmake . && make`
5. Execute the example: `sudo ./WritePos /dev/ttyUSB0`
   
Note: Adjust /dev/ttyUSB0 according to the serial port of your device

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

## Modifications

I added and updated the following methods in `SMS_STS.h/.cpp`:
* Updated `WheelMode(ID)` to `Mode(ID, mode)`. Now instead of setting only Mode 1 (closed-loop wheel mode), the function can be used to set either of the following:
    * Mode 0: Servo mode
    * Mode 1: Closed loop wheel mode
    * Mode 2: Open loop wheel mode
* Added `WritePwm(ID, PWM)` to set PWM values in Mode 2
* Added `regWriteSpe(ID, Speed, Acc)` and `syncWriteSpe(ID[], IDN, Speed[], Acc[])` to write Speed values to an individual motor asynchronously, and to multiple motors in sync, respectively, to be used in Mode 1. The asynchronous write function works as expected, but as of now, `syncWriteSpe()` does not function correctly.
* Added `regWritePwm(ID, Speed, Acc)` and `syncWritePwm(ID[], IDN, Speed[], Acc[])` to write PWM values to an individual motor asynchronously, and to multiple motors in sync, respectively, to be used in Mode 2. Both new functions work as expected.
* Added a `SignalHandler()` function to each (Write) example, which disables torque when the termination signal (`CTRL+C`) is received. This stops the motors completely, and allows them to be rotated by hand.

My modified examples used for testing on my robot can be found in `examples/sandbox/`. I've also added examples to support the newly added functions, but specific to my application - a 3 omni-wheeled robot. The normal examples have comments written in Chinese, but I've updated the examples in the sandbox with English comments. 
