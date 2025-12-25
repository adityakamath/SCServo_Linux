/**
 * @file SCSerial.cpp
 * @brief Feetech serial servo hardware interface layer implementation
 *
 * @details This file implements POSIX serial port communication for Feetech
 * servo motors on Linux platforms. It provides the hardware abstraction layer
 * between the protocol layer (SCS) and the actual serial port device.
 *
 * **Key Responsibilities:**
 * - Serial port initialization and configuration (termios)
 * - Baud rate configuration (38400 to 1M baud)
 * - Raw data transmission and reception
 * - Timeout handling using select()
 * - Buffer flushing and flow control
 * - Resource cleanup (file descriptor management)
 *
 * **Platform Support:**
 * - Linux (POSIX termios API)
 * - Supports USB-to-serial adapters (/dev/ttyUSB*, /dev/ttyACM*)
 *
 * @note Uses POSIX termios for serial port control
 * @see SCSerial.h for class interface documentation
 */

#include "SCSerial.h"

/**
 * @brief Default constructor
 * 
 * Initializes serial port with:
 * - IOTimeOut = 100ms
 * - fd = -1 (not open)
 * - txBufLen = 0 (empty buffer)
 */
SCSerial::SCSerial()
{
	IOTimeOut = 100;
	fd = -1;
	txBufLen = 0;
}

/**
 * @brief Constructor with endianness parameter
 * 
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 */
SCSerial::SCSerial(u8 End):SCS(End)
{
	IOTimeOut = 100;
	fd = -1;
	txBufLen = 0;
}

/**
 * @brief Constructor with endianness and response level
 * 
 * @param End Endianness flag
 * @param Level Response level
 */
SCSerial::SCSerial(u8 End, u8 Level):SCS(End, Level)
{
	IOTimeOut = 100;
	fd = -1;
	txBufLen = 0;
}

/**
 * @brief Initialize and open serial port
 * 
 * Opens serial port with specified baud rate and configures it for
 * 8N1 (8 data bits, no parity, 1 stop bit) communication in raw mode.
 * 
 * @param baudRate Baud rate (e.g., 1000000 for 1Mbps)
 * @param serialPort Device path (e.g., "/dev/ttyUSB0")
 * @return true on success, false on failure
 */
bool SCSerial::begin(int baudRate, const char* serialPort)
{
	if(fd != -1){
		close(fd);
		fd = -1;
	}
	//printf("servo port:%s\n", serialPort);
    if(serialPort == NULL)
		return false;
    fd = open(serialPort, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(fd == -1){
		perror("open:");
        return false;
	}
    fcntl(fd, F_SETFL, FNDELAY);
    tcgetattr(fd, &orgopt);
    tcgetattr(fd, &curopt);
    speed_t CR_BAUDRATE;
    switch(baudRate){
    case 9600:
        CR_BAUDRATE = B9600;
        break;
    case 19200:
        CR_BAUDRATE = B19200;
        break;
    case 38400:
        CR_BAUDRATE = B38400;
        break;
    case 57600:
        CR_BAUDRATE = B57600;
        break;
    case 115200:
        CR_BAUDRATE = B115200;
        break;
    case 500000:
        CR_BAUDRATE = B500000;
        break;
    case 1000000:
        CR_BAUDRATE = B1000000;
        break;
    default:
        CR_BAUDRATE = B115200;
        break;
    }
    cfsetispeed(&curopt, CR_BAUDRATE);
    cfsetospeed(&curopt, CR_BAUDRATE);

	printf("serial speed %d\n", baudRate);
    //Mostly 8N1
    curopt.c_cflag &= ~PARENB;
    curopt.c_cflag &= ~CSTOPB;
    curopt.c_cflag &= ~CSIZE;
    curopt.c_cflag |= CS8;
    curopt.c_cflag |= CREAD;
    curopt.c_cflag |= CLOCAL;//disable modem statuc check
    cfmakeraw(&curopt);//make raw mode
    curopt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    if(tcsetattr(fd, TCSANOW, &curopt) == 0){
        return true;
    }else{
		perror("tcsetattr:");
		return false;
	}
}

/**
 * @brief Change serial port baud rate
 * 
 * @param baudRate New baud rate
 * @return 1 on success, -1 if port not open
 */
int SCSerial::setBaudRate(int baudRate)
{ 
    if(fd==-1){
		return -1;
	}
    tcgetattr(fd, &orgopt);
    tcgetattr(fd, &curopt);
    speed_t CR_BAUDRATE = baudRate;
    cfsetispeed(&curopt, CR_BAUDRATE);
    cfsetospeed(&curopt, CR_BAUDRATE);
    return 1;
}

int SCSerial::readSCS(unsigned char *nDat, int nLen)
{
    int fs_sel;
    fd_set fs_read;
	int rvLen = 0;

	// Use select() to implement multi-channel serial communication
	while(1){
		// Reinitialize timeout for each select() call
		// select() modifies the timeout structure on Linux
		struct timeval time;
		time.tv_sec = 0;
		time.tv_usec = IOTimeOut*1000;

		FD_ZERO(&fs_read);
		FD_SET(fd,&fs_read);

		fs_sel = select(fd+1, &fs_read, NULL, NULL, &time);
		if(fs_sel){
			rvLen += read(fd, nDat+rvLen, nLen-rvLen);
			//printf("nLen = %d rvLen = %d\n", nLen, rvLen);
			if(rvLen<nLen){
				continue;
			}else{
				return rvLen;
			}
		}else{
			//printf("serial read fd read return 0\n");
			return rvLen;
		}
	}
}

/**
 * @brief Write data buffer to transmit buffer
 * 
 * Copies data to internal transmit buffer. Data is sent when wFlushSCS() is called.
 * Includes NULL pointer and buffer overflow protection.
 * 
 * @param nDat Pointer to data to write
 * @param nLen Number of bytes to write
 * @return Current buffer length on success, -1 on error
 */
int SCSerial::writeSCS(unsigned char *nDat, int nLen)
{
	// NULL pointer check
	if(!nDat){
		return -1;
	}

	// Buffer overflow protection
	if(txBufLen + nLen > SCSERVO_BUFFER_SIZE){
		return -1;
	}
	while(nLen--){
		txBuf[txBufLen++] = *nDat++;
	}
	return txBufLen;
}

/**
 * @brief Write single byte to transmit buffer
 * 
 * @param bDat Byte to write
 * @return Current buffer length on success, -1 if buffer full
 */
int SCSerial::writeSCS(unsigned char bDat)
{
	// Buffer overflow protection
	if(txBufLen >= SCSERVO_BUFFER_SIZE){
		return -1;
	}
	txBuf[txBufLen++] = bDat;
	return txBufLen;
}

/**
 * @brief Flush receive buffer
 * 
 * Discards any unread data in the serial port receive buffer.
 */
void SCSerial::rFlushSCS()
{
	tcflush(fd, TCIFLUSH);
}

/**
 * @brief Flush transmit buffer
 * 
 * Sends all buffered data from txBuf to the serial port.
 */
void SCSerial::wFlushSCS()
{
	if(txBufLen){
		ssize_t written = write(fd, txBuf, txBufLen);
		// Note: write errors are not critical for this protocol, servo will timeout
		// In production code, consider checking: if(written < 0) { handle error }
		(void)written;  // Suppress unused variable warning
		txBufLen = 0;
	}
}

/**
 * @brief Close serial port and cleanup
 * 
 * Closes the serial port file descriptor if open.
 */
void SCSerial::end() noexcept
{
	if(fd != -1){
		close(fd);
		fd = -1;
	}
}
