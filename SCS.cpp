/**
 * @file SCS.cpp
 * @brief Feetech serial servo communication protocol layer implementation
 *
 * @details This file implements the low-level protocol for Feetech serial servo
 * communication. It handles packet construction, checksum calculation, command
 * encoding/decoding, and synchronized read/write operations.
 *
 * **Key Responsibilities:**
 * - Protocol packet formatting (header, ID, length, instruction, data, checksum)
 * - Command execution (PING, READ, WRITE, REG_WRITE, REG_ACTION, SYNC_WRITE, SYNC_READ)
 * - Checksum validation
 * - Byte ordering (endianness handling)
 * - Response parsing and error detection
 *
 * @note This is an abstract base class - use concrete implementations (SCSerial)
 * @see SCS.h for class interface documentation
 */
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include "SCS.h"

/**
 * @brief Default constructor for SCS base class
 * 
 * Initializes the servo communication with:
 * - Level = 1 (all instructions except broadcast return acknowledgement)
 * - Error = 0 (no error)
 */
SCS::SCS()
{
	Level = 1;  // All instructions except broadcast return acknowledgement
	Error = 0;
}

/**
 * @brief Constructor with endianness parameter
 * 
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 */
SCS::SCS(u8 End)
{
	Level = 1;
	this->End = End;
	Error = 0;
}

/**
 * @brief Constructor with endianness and response level
 * 
 * @param End Endianness flag (0=little-endian, 1=big-endian)
 * @param Level Response level (1=return ACK, 0=no ACK for broadcast)
 */
SCS::SCS(u8 End, u8 Level)
{
	this->Level = Level;
	this->End = End;
	Error = 0;
}

/**
 * @brief Split one 16-bit value into two 8-bit values (host to servo format)
 * 
 * Handles endianness conversion between host and servo protocols.
 * 
 * @param DataL Pointer to store low byte
 * @param DataH Pointer to store high byte
 * @param Data 16-bit data value to split
 */
void SCS::Host2SCS(u8 *DataL, u8* DataH, u16 Data)
{
	if(End){
		*DataL = (Data>>8);
		*DataH = (Data&0xff);
	}else{
		*DataH = (Data>>8);
		*DataL = (Data&0xff);
	}
}

/**
 * @brief Combine two 8-bit values into one 16-bit value (servo to host format)
 * 
 * Handles endianness conversion between servo and host protocols.
 * 
 * @param DataL Low byte
 * @param DataH High byte
 * @return 16-bit combined value
 */
u16 SCS::SCS2Host(u8 DataL, u8 DataH)
{
	u16 Data;
	if(End){
		Data = DataL;
		Data<<=8;
		Data |= DataH;
	}else{
		Data = DataH;
		Data<<=8;
		Data |= DataL;
	}
	return Data;
}

/**
 * @brief Build and write command packet buffer
 * 
 * Internal function to construct protocol packets with header, ID, length, and checksum.
 * 
 * @param ID Servo ID
 * @param MemAddr Memory address to access
 * @param nDat Pointer to data buffer (NULL for commands without data)
 * @param nLen Data length
 * @param Fun Function code/instruction
 */
void SCS::writeBuf(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen, u8 Fun)
{
	u8 msgLen = 2;
	u8 bBuf[6];
	u8 CheckSum = 0;
	bBuf[0] = 0xff;
	bBuf[1] = 0xff;
	bBuf[2] = ID;
	bBuf[4] = Fun;
	if(nDat){
		msgLen += nLen + 1;
		bBuf[3] = msgLen;
		bBuf[5] = MemAddr;
		writeSCS(bBuf, 6);
		
	}else{
		bBuf[3] = msgLen;
		writeSCS(bBuf, 5);
	}
	CheckSum = ID + msgLen + Fun + MemAddr;
	u8 i = 0;
	if(nDat){
		for(i=0; i<nLen; i++){
			CheckSum += nDat[i];
		}
		writeSCS(nDat, nLen);
	}
	writeSCS(~CheckSum);
}

/**
 * @brief Normal write instruction (synchronous)
 * 
 * Writes data to servo memory and waits for acknowledgement.
 * 
 * @param ID Servo ID (0-253, 0xFE for broadcast)
 * @param MemAddr Memory table address
 * @param nDat Pointer to data to write
 * @param nLen Length of data in bytes
 * @return 1 on success, 0 on failure
 */
int SCS::genWrite(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen)
{
	rFlushSCS();
	writeBuf(ID, MemAddr, nDat, nLen, INST_WRITE);
	wFlushSCS();
	return Ack(ID);
}

/**
 * @brief Asynchronous write instruction (register write)
 * 
 * Registers a write command to be executed later with RegWriteAction().
 * Useful for synchronized multi-servo movements.
 * 
 * @param ID Servo ID
 * @param MemAddr Memory table address
 * @param nDat Pointer to data to write
 * @param nLen Length of data in bytes
 * @return 1 on success, 0 on failure
 */
int SCS::regWrite(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen)
{
	rFlushSCS();
	writeBuf(ID, MemAddr, nDat, nLen, INST_REG_WRITE);
	wFlushSCS();
	return Ack(ID);
}

/**
 * @brief Execute all registered write commands
 * 
 * Triggers execution of all commands previously registered with regWrite().
 * Enables synchronized movement of multiple servos.
 * 
 * @param ID Servo ID (use 0xFE for broadcast to all servos)
 * @return 1 on success, 0 on failure
 */
int SCS::RegWriteAction(u8 ID)
{
	rFlushSCS();
	writeBuf(ID, 0, NULL, 0, INST_REG_ACTION);
	wFlushSCS();
	return Ack(ID);
}

/**
 * @brief Synchronous write instruction for multiple servos
 * 
 * Writes the same memory address with different data to multiple servos
 * in a single packet. More efficient than individual writes.
 * 
 * @param ID Array of servo IDs
 * @param IDN Number of servos (array length)
 * @param MemAddr Memory table address (same for all servos)
 * @param nDat Pointer to data buffer (IDN * nLen bytes)
 * @param nLen Length of data per servo
 */
void SCS::syncWrite(u8 ID[], u8 IDN, u8 MemAddr, u8 *nDat, u8 nLen)
{
	rFlushSCS();
	u8 mesLen = ((nLen+1)*IDN+4);
	u8 Sum = 0;
	u8 bBuf[7];
	bBuf[0] = 0xff;
	bBuf[1] = 0xff;
	bBuf[2] = 0xfe;
	bBuf[3] = mesLen;
	bBuf[4] = INST_SYNC_WRITE;
	bBuf[5] = MemAddr;
	bBuf[6] = nLen;
	writeSCS(bBuf, 7);

	Sum = 0xfe + mesLen + INST_SYNC_WRITE + MemAddr + nLen;
	u8 i, j;
	for(i=0; i<IDN; i++){
		writeSCS(ID[i]);
		writeSCS(nDat+i*nLen, nLen);
		Sum += ID[i];
		for(j=0; j<nLen; j++){
			Sum += nDat[i*nLen+j];
		}
	}
	writeSCS(~Sum);
	wFlushSCS();
}

int SCS::writeByte(u8 ID, u8 MemAddr, u8 bDat)
{
	rFlushSCS();
	writeBuf(ID, MemAddr, &bDat, 1, INST_WRITE);
	wFlushSCS();
	return Ack(ID);
}

int SCS::writeWord(u8 ID, u8 MemAddr, u16 wDat)
{
	u8 bBuf[2];
	Host2SCS(bBuf+0, bBuf+1, wDat);
	rFlushSCS();
	writeBuf(ID, MemAddr, bBuf, 2, INST_WRITE);
	wFlushSCS();
	return Ack(ID);
}

// Read instruction
// Servo ID, MemAddr memory table address, return data nData, data length nLen
int SCS::Read(u8 ID, u8 MemAddr, u8 *nData, u8 nLen)
{
	// NULL pointer check
	if(!nData){
		return 0;
	}

	// Validate buffer size against maximum data size
	if(nLen > SCSERVO_MAX_DATA_SIZE){
		return 0;
	}

	rFlushSCS();
	writeBuf(ID, MemAddr, &nLen, 1, INST_READ);
	wFlushSCS();

	u8 bBuf[SCSERVO_BUFFER_SIZE];
	u8 i;
	u8 calSum = 0;
	int Size = readSCS(bBuf, nLen+6);
	//printf("nLen+6 = %d, Size = %d\n", nLen+6, Size);
	if(Size!=(nLen+6)){
		return 0;
	}
	//for(i=0; i<Size; i++){
		//printf("%x\n", bBuf[i]);
	//}
	if(bBuf[0]!=0xff || bBuf[1]!=0xff){
		return 0;
	}
	for(i=2; i<(Size-1); i++){
		calSum += bBuf[i];
	}
	calSum = ~calSum;
	if(calSum!=bBuf[Size-1]){
		return 0;
	}
	memcpy(nData, bBuf+5, nLen);
	Error = bBuf[4];
	return nLen;
}

/**
 * @brief Read single byte from servo memory
 * 
 * Convenience function for reading one byte.
 * 
 * @param ID Servo ID
 * @param MemAddr Memory address to read
 * @return Byte value on success, -1 on timeout/error
 */
int SCS::readByte(u8 ID, u8 MemAddr)
{
	u8 bDat;
	int Size = Read(ID, MemAddr, &bDat, 1);
	if(Size!=1){
		return -1;
	}else{
		return bDat;
	}
}

/**
 * @brief Read 16-bit word from servo memory
 * 
 * Reads two consecutive bytes and combines them into a 16-bit value.
 * Handles endianness conversion automatically.
 * 
 * @param ID Servo ID
 * @param MemAddr Memory address to read (reads 2 bytes)
 * @return 16-bit word value on success, -1 on timeout/error
 */
int SCS::readWord(u8 ID, u8 MemAddr)
{	
	u8 nDat[2];
	int Size;
	u16 wDat;
	Size = Read(ID, MemAddr, nDat, 2);
	if(Size!=2)
		return -1;
	wDat = SCS2Host(nDat[0], nDat[1]);
	return wDat;
}

/**
 * @brief Ping servo to check if it's online
 * 
 * Sends ping command and waits for response to verify servo connectivity.
 * 
 * @param ID Servo ID to ping
 * @return Servo ID on success, -1 on timeout (servo not responding)
 */
int	SCS::Ping(u8 ID)
{
	rFlushSCS();
	writeBuf(ID, 0, NULL, 0, INST_PING);
	wFlushSCS();
	Error = 0;

	u8 bBuf[6];
	u8 i;
	u8 calSum = 0;
	int Size = readSCS(bBuf, 6);
	if(Size!=6){
		return -1;
	}
	if(bBuf[0]!=0xff || bBuf[1]!=0xff){
		return -1;
	}
	if(bBuf[2]!=ID && ID!=0xfe){
		return -1;
	}
	if(bBuf[3]!=2){
		return -1;
	}
	for(i=2; i<(Size-1); i++){
		calSum += bBuf[i];
	}
	calSum = ~calSum;
	if(calSum!=bBuf[Size-1]){
		return -1;
	}
	Error = bBuf[2];
	return Error;
}

/**
 * @brief Wait for and validate acknowledgment packet from servo
 * 
 * Reads response packet and validates checksum. Only waits if response
 * level is enabled (Level != 0) and not broadcasting (ID != 0xfe).
 * 
 * @param ID Servo ID to receive ack from
 * @return 1 on success (valid ack received), 0 on failure or when Level=0
 */
int	SCS::Ack(u8 ID)
{
	Error = 0;
	if(ID!=0xfe && Level){
		u8 bBuf[6];
		u8 i;
		u8 calSum = 0;
		int Size = readSCS(bBuf, 6);
		if(Size!=6){
			return 0;
		}
		if(bBuf[0]!=0xff || bBuf[1]!=0xff){
			return 0;
		}
		if(bBuf[2]!=ID){
			return 0;
		}
		if(bBuf[3]!=2){
			return 0;
		}
		for(i=2; i<(Size-1); i++){
			calSum += bBuf[i];
		}
		calSum = ~calSum;
		if(calSum!=bBuf[Size-1]){
			return 0;
		}
		Error = bBuf[4];
	}
	return 1;
}

/**
 * @brief Transmit sync read command and receive all responses
 * 
 * Sends INST_SYNC_READ command to multiple servos and reads all responses
 * into internal buffer for later parsing.
 * 
 * @param ID Array of servo IDs to read from
 * @param IDN Number of servo IDs in array
 * @param MemAddr Starting memory address to read
 * @param nLen Number of bytes to read per servo
 * @return Total bytes received in syncReadRxBuff
 */
int	SCS::syncReadPacketTx(u8 ID[], u8 IDN, u8 MemAddr, u8 nLen)
{
	rFlushSCS();
	syncReadRxPacketLen = nLen;
	u8 checkSum = (4+0xfe)+IDN+MemAddr+nLen+INST_SYNC_READ;
	u8 i;
	writeSCS(0xff);
	writeSCS(0xff);
	writeSCS(0xfe);
	writeSCS(IDN+4);
	writeSCS(INST_SYNC_READ);
	writeSCS(MemAddr);
	writeSCS(nLen);
	for(i=0; i<IDN; i++){
		writeSCS(ID[i]);
		checkSum += ID[i];
	}
	checkSum = ~checkSum;
	writeSCS(checkSum);
	wFlushSCS();
	
	syncReadRxBuffLen = readSCS(syncReadRxBuff, syncReadRxBuffMax);
	return syncReadRxBuffLen;
}

/**
 * @brief Initialize sync read buffer for multiple servo responses
 * 
 * Allocates buffer sized for expected number of servo responses.
 * Cleans up any existing buffer first to prevent memory leaks.
 * 
 * @param IDN Number of servos to read from
 * @param rxLen Number of data bytes expected per servo response
 */
void SCS::syncReadBegin(u8 IDN, u8 rxLen)
{
	// Clean up existing buffer to prevent memory leak
	if(syncReadRxBuff){
		delete[] syncReadRxBuff;
		syncReadRxBuff = NULL;
	}
	syncReadRxBuffMax = IDN*(rxLen+6);
	syncReadRxBuff = new u8[syncReadRxBuffMax];
	// Check allocation success
	if(!syncReadRxBuff){
		syncReadRxBuffMax = 0;
	}
}

/**
 * @brief Cleanup sync read buffer
 * 
 * Deallocates the sync read response buffer. Must be called after
 * completing sync read operations to free memory.
 */
void SCS::syncReadEnd()
{
	if(syncReadRxBuff){
		delete[] syncReadRxBuff;  // Correct: use delete[] for arrays
		syncReadRxBuff = NULL;
	}
}

/**
 * @brief Extract one servo's response from sync read buffer
 * 
 * Parses sync read response buffer to extract packet for specific servo ID.
 * Validates packet format, checksum, and copies data to output buffer.
 * 
 * @param ID Servo ID to extract response for
 * @param nDat Output buffer for response data (must be at least syncReadRxPacketLen bytes)
 * @return Number of data bytes extracted on success, 0 on error/not found
 */
int SCS::syncReadPacketRx(u8 ID, u8 *nDat)
{
	// NULL pointer checks
	if(!nDat || !syncReadRxBuff){
		return 0;
	}

	u16 syncReadRxBuffIndex = 0;
	syncReadRxPacket = nDat;
	syncReadRxPacketIndex = 0;
	while((syncReadRxBuffIndex+6+syncReadRxPacketLen)<=syncReadRxBuffLen){
		u8 bBuf[] = {0, 0, 0};
		u8 calSum = 0;
		while(syncReadRxBuffIndex<syncReadRxBuffLen){
			bBuf[0] = bBuf[1];
			bBuf[1] = bBuf[2];
			bBuf[2] = syncReadRxBuff[syncReadRxBuffIndex++];
			if(bBuf[0]==0xff && bBuf[1]==0xff && bBuf[2]!=0xff){
				break;
			}
		}
		if(bBuf[2]!=ID){
			continue;
		}
		// Bounds check before accessing buffer
		if(syncReadRxBuffIndex >= syncReadRxBuffLen){
			break;
		}
		if(syncReadRxBuff[syncReadRxBuffIndex++]!=(syncReadRxPacketLen+2)){
			continue;
		}
		// Bounds check before accessing buffer
		if(syncReadRxBuffIndex >= syncReadRxBuffLen){
			break;
		}
		Error = syncReadRxBuff[syncReadRxBuffIndex++];
		calSum = ID+(syncReadRxPacketLen+2)+Error;
		for(u8 i=0; i<syncReadRxPacketLen; i++){
			// Bounds check in loop
			if(syncReadRxBuffIndex >= syncReadRxBuffLen){
				return 0;
			}
			syncReadRxPacket[i] = syncReadRxBuff[syncReadRxBuffIndex++];
			calSum += syncReadRxPacket[i];
		}
		calSum = ~calSum;
		// Bounds check before final access
		if(syncReadRxBuffIndex >= syncReadRxBuffLen){
			return 0;
		}
		if(calSum!=syncReadRxBuff[syncReadRxBuffIndex++]){
			return 0;
		}
		return syncReadRxPacketLen;
	}
	return 0;
}

/**
 * @brief Read next byte from sync read response packet
 * 
 * Sequential access to parsed sync read packet data. Advances internal index.
 * 
 * @return Next byte from packet, or -1 if all bytes have been read
 */
int SCS::syncReadRxPacketToByte()
{
	if(syncReadRxPacketIndex>=syncReadRxPacketLen){
		return -1;
	}
	return syncReadRxPacket[syncReadRxPacketIndex++];
}

/**
 * @brief Read next 16-bit word from sync read response packet
 * 
 * Sequential access to parsed sync read packet data. Reads two bytes,
 * converts to host byte order, and optionally handles negative values.
 * 
 * @param negBit Bit position for sign extension (0 = unsigned, >0 = treat as signed with negBit as sign bit)
 * @return 16-bit word value (possibly sign-extended), or -1 if insufficient bytes remain
 */
int SCS::syncReadRxPacketToWrod(u8 negBit)
{
	if((syncReadRxPacketIndex+1)>=syncReadRxPacketLen){
		return -1;
	}
	int Word = SCS2Host(syncReadRxPacket[syncReadRxPacketIndex], syncReadRxPacket[syncReadRxPacketIndex+1]);
	syncReadRxPacketIndex += 2;
	if(negBit){
		if(Word&(1<<negBit)){
			Word = -(Word & ~(1<<negBit));
		}
	}
	return Word;
}