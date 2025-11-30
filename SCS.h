/**
 * @file SCS.h
 * @brief Feetech serial servo communication protocol layer program
 * @date 2022.3.29
 * @author 
 */

#ifndef _SCS_H
#define _SCS_H

#include "INST.h"

/**
 * @class SCS
 * @brief Base class for Feetech serial servo communication protocol
 */
class SCS{
public:
	SCS();
	SCS(u8 End);
	SCS(u8 End, u8 Level);
	virtual ~SCS() {}  // Virtual destructor for proper cleanup in derived classes

	// Disable copying (Rule of Three/Five) - class owns dynamic memory (syncReadRxBuff)
	SCS(const SCS&) = delete;
	SCS& operator=(const SCS&) = delete;

	/**
	 * @brief Write data to servo memory (normal write command)
	 * @param ID Servo ID (0-253, 0xFE for broadcast)
	 * @param MemAddr Memory table address
	 * @param nDat Pointer to data buffer
	 * @param nLen Data length (bytes)
	 * @return 1 on success, 0 on failure
	 */
	int genWrite(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen);

	/**
	 * @brief Asynchronous write command
	 * @note Requires RegWriteAction() to execute
	 */
	int regWrite(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen);

	/**
	 * @brief Execute asynchronous write commands (异步写执行指令)
	 * @param ID Servo ID (default: 0xFE for all servos)
	 * @return 1 on success, 0 on failure
	 */
	int RegWriteAction(u8 ID = 0xfe);

	/**
	 * @brief Synchronous write to multiple servos (同步写指令)
	 * @param ID Array of servo IDs
	 * @param IDN Number of servos
	 * @param MemAddr Memory table start address
	 * @param nDat Data buffer (nLen bytes per servo)
	 * @param nLen Data length per servo
	 */
	void syncWrite(u8 ID[], u8 IDN, u8 MemAddr, u8 *nDat, u8 nLen);

	int writeByte(u8 ID, u8 MemAddr, u8 bDat);//写1个字节
	int writeWord(u8 ID, u8 MemAddr, u16 wDat);//写2个字节

	/**
	 * @brief Read data from servo memory (读指令)
	 * @param ID Servo ID
	 * @param MemAddr Memory table address
	 * @param nData Buffer to store read data
	 * @param nLen Number of bytes to read (max 249)
	 * @return Number of bytes read on success, 0 on failure
	 */
	int Read(u8 ID, u8 MemAddr, u8 *nData, u8 nLen);

	int readByte(u8 ID, u8 MemAddr);//读1个字节
	int readWord(u8 ID, u8 MemAddr);//读2个字节

	/**
	 * @brief Ping servo to check connection (Ping指令)
	 * @param ID Servo ID to ping
	 * @return Servo error status on success, -1 on failure
	 */
	int Ping(u8 ID);
	int syncReadPacketTx(u8 ID[], u8 IDN, u8 MemAddr, u8 nLen);//同步读指令包发送
	int syncReadPacketRx(u8 ID, u8 *nDat);//同步读返回包解码，成功返回内存字节数，失败返回0
	int syncReadRxPacketToByte();//解码一个字节
	int syncReadRxPacketToWrod(u8 negBit=0);//解码两个字节，negBit为方向为，negBit=0表示无方向
	void syncReadBegin(u8 IDN, u8 rxLen);//同步读开始
	void syncReadEnd();//同步读结束
public:
	u8	Level;//舵机返回等级
	u8	End;//处理器大小端结构
	u8	Error;//舵机状态
	u8 syncReadRxPacketIndex;
	u8 syncReadRxPacketLen;
	u8 *syncReadRxPacket;
	u8 *syncReadRxBuff;
	u16 syncReadRxBuffLen;
	u16 syncReadRxBuffMax;
protected:
	virtual int writeSCS(unsigned char *nDat, int nLen) = 0;
	virtual int readSCS(unsigned char *nDat, int nLen) = 0;
	virtual int writeSCS(unsigned char bDat) = 0;
	virtual void rFlushSCS() = 0;
	virtual void wFlushSCS() = 0;
protected:
	void writeBuf(u8 ID, u8 MemAddr, u8 *nDat, u8 nLen, u8 Fun);
	void Host2SCS(u8 *DataL, u8* DataH, u16 Data);//1个16位数拆分为2个8位数
	u16	SCS2Host(u8 DataL, u8 DataH);//2个8位数组合为1个16位数
	int	Ack(u8 ID);//返回应答

	// Helper for reading signed words with direction bit
	int readSignedWord(int ID, u8 addr, u8 signBit) {
		int value = readWord(ID, addr);
		if(value == -1) {
			Error = 1;
			return -1;
		}
		if((value & (1 << signBit))) {
			value = -(value & ~(1 << signBit));
		}
		return value;
	}
};
#endif