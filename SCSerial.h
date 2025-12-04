/**
 * @file SCSerial.h
 * @brief Feetech serial servo hardware interface layer
 *
 * @details This file provides the hardware interface layer for serial communication
 * with Feetech servo motors on Linux platforms. It handles POSIX serial port operations,
 * baud rate configuration, and low-level data transmission/reception.
 *
 * **Key Features:**
 * - POSIX serial port communication (termios)
 * - Configurable baud rates (38400 to 1M)
 * - Timeout handling for robust communication
 * - Resource management (file descriptor ownership)
 * - Buffer management for transmit operations
 *
 * **Inherits From:**
 * - SCS: Protocol layer for command encoding/decoding
 *
 * **Derived Classes:**
 * - SMS_STS: SMS/STS series servo control
 * - SMSCL: SMSCL series servo control
 * - SMSBL: SMSBL series servo control
 * - SCSCL: SCSCL series servo control
 *
 * **Usage Example:**
 * @code
 * SCSerial serial;
 * if (!serial.begin(1000000, "/dev/ttyUSB0")) {
 *     printf("Failed to open serial port\n");
 *     return -1;
 * }
 * // Use serial communication methods
 * serial.end();  // Clean up
 * @endcode
 *
 * @note This class owns the file descriptor and implements RAII cleanup
 * @see SCS for protocol-level operations
 */

#ifndef _SCSERIAL_H
#define _SCSERIAL_H

#include "SCS.h"
#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>

class SCSerial : public SCS
{
public:
	SCSerial();
	SCSerial(u8 End);
	SCSerial(u8 End, u8 Level);

	// Disable copying (Rule of Three/Five) - class owns file descriptor resource
	SCSerial(const SCSerial&) = delete;
	SCSerial& operator=(const SCSerial&) = delete;

protected:
	int writeSCS(unsigned char *nDat, int nLen);// Output nLen bytes
	int readSCS(unsigned char *nDat, int nLen);// Input nLen bytes
	int writeSCS(unsigned char bDat);// Output 1 byte
	void rFlushSCS();//
	void wFlushSCS();//
public:
	unsigned long int IOTimeOut;// Input/output timeout
	int Err;
public:
	virtual int getErr(){  return Err;  }
	virtual int setBaudRate(int baudRate);
	virtual bool begin(int baudRate, const char* serialPort);
	virtual void end() noexcept;
protected:
    int fd;//serial port handle
    struct termios orgopt;//fd ort opt
	struct termios curopt;//fd cur opt
	unsigned char txBuf[SCSERVO_BUFFER_SIZE];
	int txBufLen;
};

#endif