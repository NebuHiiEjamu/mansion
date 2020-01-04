// TCPWindow.h
#ifndef _H_UserTCP

#define _H_UserTCP	1

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <serial.h>

#include <MacTCP.h>
#include "TCPHi.h"
#include "TCPRoutines.h"

#define RcveBufferLength	32766L
#define SendBufferLength	16384L
#define LineBufferLength	32766L
#define LenGroupBuffer		16384L	// Shouldn't be larger than send buffer!

typedef struct {
	unsigned long		remoteHost;		// IP Address (currently unused)
	unsigned long		tcpStream;		// tcp Stream
	unsigned long		newStream;		// Connecting Stream
	long				receiveIdx;
	long				groupLen;
	TCPiopb 			*rcveBlock;		// Receive Async Rec
	TCPiopb 			*connectPB;		// Connect Rec
	TCPiopb				*sendBlock;		// Send Block
	char				remoteHostname[128];	// JAB 7/12/96 changed to 128 from 127
	char				*rcveBuffer,*sendBuffer,*groupBuffer;	// Receive Buffer
	// char				*cp;	// Pointer to current recieve char
	// long				cCnt;	// Character Count
	unsigned long		startConnectTime;
	short				connectType; 	// JAB 7/12/96 Moved to bottom for alignment
	short				remotePort;
	Boolean				connectionOpen,receiveFlag,sendFlag,aborting,connecting,timeNotify;
} TCPRecord, *TCPRecordPtr;

#define TCPWinID		130
#define TCPBufSize		RcveBufferLength
#define TCPPort			6667
#define TCPTimeOut		60

extern TCPRecordPtr		gTCPRecord;


unsigned long SwapLong(unsigned long *n);
unsigned short SwapShort(unsigned short *n);
void StoreNetAddress(char *name, long ip_addr, short port);

// New Buffered Output Routines
void IdleTCPSend(Boolean abortOK);
Boolean PostTCPBuffer(Ptr buffer, long bufferLength);
void FlushGroupTCPBuffer(void);	// Wait till Group Buffer is Sent
void SendGroupTCPBuffer(void);


#endif
		
