/**
 * @file INST.h
 * @brief Feetech serial servo protocol instruction definitions and data types
 *
 * @details This file defines the fundamental protocol instructions, data types,
 * and buffer size constants used across all Feetech servo series. It provides
 * the low-level protocol command set for servo communication.
 *
 * **Protocol Instructions:**
 * - PING: Check servo connection
 * - READ/WRITE: Memory table access
 * - REG_WRITE/REG_ACTION: Asynchronous write operations
 * - SYNC_READ/SYNC_WRITE: Synchronized multi-servo operations
 *
 * **Data Types:**
 * - Signed/unsigned 8, 16, and 32-bit integers
 * - Platform-independent type definitions
 *
 * @note This file is included by all servo class headers (SMS_STS, SMSCL, SMSBL, SCSCL)
 */

#ifndef _INST_H
#define _INST_H

typedef	char s8;
typedef	unsigned char u8;	
typedef	unsigned short u16;	
typedef	short s16;
typedef	unsigned long u32;	
typedef	long s32;

#define INST_PING 0x01
#define INST_READ 0x02
#define INST_WRITE 0x03
#define INST_REG_WRITE 0x04
#define INST_REG_ACTION 0x05
#define INST_SYNC_READ 0x82
#define INST_SYNC_WRITE 0x83

// Buffer size constants
#define SCSERVO_BUFFER_SIZE 255
#define SCSERVO_HEADER_SIZE 6
#define SCSERVO_MAX_DATA_SIZE (SCSERVO_BUFFER_SIZE - SCSERVO_HEADER_SIZE)  // 249 bytes

#endif