
#ifndef _TCPROUTINES_
#define _TCPROUTINES_


#ifndef __MACTCP__
#include "MacTCP.h"
#endif

OSErr OpenTCPDriver(void);				/* opens the MacTCP driver */

OSErr LowKillTCP(						/* kills a pending MacTCP driver call		*/
	TCPiopb *pBlock);

// 6/26/95
OSErr LowTCPCreateStream(				/* creates a MacTCP stream 					*/
	StreamPtr *streamPtr,				/* stream created (returned)			*/
	Ptr connectionBuffer,				/* pointer to connection buffer			*/
	unsigned long connBufferLen,		/* length of connection buffer			*/
	TCPNotifyUPP notifPtr);				/* address of Async. Notification Rtn.	*/
	
OSErr LowTCPWaitForConnection(			/* listens for a TCP connection to be opened*/
	StreamPtr streamPtr,					/* stream pointer for connection		*/
	b_8 timeout,							/* timeout for listen					*/
	ip_addr *remoteHost,					/* remote host to listen for (returned)	*/
	tcp_port *remotePort,					/* remote port to listen on (returned)	*/
	ip_addr *localHost,						/* local host number (returned)			*/
	tcp_port *localPort,					/* local port to listen on (returned)	*/
	Boolean async,							/* true if call to be made async		*/
	TCPiopb **returnBlock);					/* address of parameter block used		*/

OSErr LowFinishTCPWaitForConn(			/* called when LowTCPWaitForConn. completes	*/
	TCPiopb *pBlock,						/* parameter block used for wait call	*/
	ip_addr *remoteHost,					/* remote host connected to (returned)	*/
	tcp_port *remotePort,					/* remote port connected to (returned)	*/
	ip_addr *localHost,						/* local host- our ip number (returned)	*/
	tcp_port *localPort);					/* local port connected to (returned)	*/
	
OSErr LowTCPOpenConnection(				/* actively attempts to connect to a host	*/
	StreamPtr streamPtr,					/* stream to use for this connection	*/
	b_8 timeout,							/* timeout value for open				*/
	ip_addr remoteHost,						/* ip number of host to connect to		*/
	tcp_port remotePort,					/* tcp port number of host to connect to*/
	ip_addr *localHost,						/* local host ip number (returned)		*/
	tcp_port *localPort);					/* local port number used for connection*/

OSErr LowTCPOpenConnectionAsync(		// 6/15/95 JAB
	StreamPtr streamPtr,
	b_8 timeout,
	ip_addr remoteHost,
	tcp_port remotePort,
	tcp_port localPort,
	TCPiopb **returnBlock);

OSErr LowFinishTCPOpenConn(				// 6/15/95 JAB
	TCPiopb *pBlock,
	ip_addr *localHost,
	tcp_port *localPort);

OSErr LowTCPSendData(					/* send data along an open connection		*/
	StreamPtr streamPtr,					/* stream identifier to send data on	*/
	b_8 timeout,							/* timeout for this send				*/
	Boolean push,							/* true if "push" bit to be set			*/
	Boolean urgent,							/* true if "urgent" bit to be set		*/
	Ptr wdsPtr,								/* write data structure (len/data pairs)*/
	Boolean async,							/* true if call is to be asynchronous	*/
	TCPiopb **returnBlock);					/* parameter block used (returned)		*/
	
OSErr LowFinishTCPSend(					/* called when TCPSendData completes		*/
	TCPiopb *pBlock);						/* parameter block used in call			*/

OSErr LowTCPRecvData(					/* waits for data to be send on a TCP stream*/
	StreamPtr	streamPtr,					/* stream waiting for data on			*/
	b_8		timeout,					/* timeout for receive					*/
	Boolean		*urgent,					/* value of urgent flag (returned)		*/
	Boolean		*mark,						/* value of mark flag (returned)		*/
	Ptr			rcvBuff,					/* buffer to store received data		*/
	unsigned short *rcvLen,					/* length of data received				*/
	Boolean 	async,						/* true if call is asynchronous			*/
	TCPiopb		**returnBlock);				/* parameter block used (returned)		*/
	
OSErr LowFinishTCPRecv(					/* called when LowTCPRecvData completes		*/
	TCPiopb		*pBlock,					/* parameter block used in call			*/
	Boolean		*urgent,					/* value of urgent flag (returned)		*/
	Boolean		*mark,						/* value of mark flag (returned)		*/
	unsigned short *rcvLen);				/* length of data received				*/
	
OSErr LowTCPNoCopyRcv(					/* receives data & doesn't copy to ext. buf.*/
	StreamPtr	streamPtr,					/* stream waiting for data on			*/
	b_8		timeout,					/* timeout for receive					*/
	Boolean		*urgent,					/* value of urgent flag (returned)		*/
	Boolean		*mark,						/* value of mark flag (returned)		*/
	Ptr			rdsPtr,						/* pointer to read data struct data/len */
	short		numEntry,					/* number of entries in read data struct*/
	Boolean		async,						/* true if call is asynchronous			*/
	TCPiopb		**returnBlock);				/* parameter block used (returned)		*/
	
OSErr LowFinishTCPNoCopyRcv(			/* called when LowTCPNoCopyRcv finishes		*/
	TCPiopb *pBlock,						/* parameter block used	in call			*/
	Boolean *urgent,						/* value of urgent flag (returned)		*/
	Boolean *mark);							/* value of mark flag (returned)		*/

OSErr LowTCPBfrReturn(					/* returns a buffer used in LowTCPNoCopyRcv	*/
	StreamPtr	streamPtr,					/* stream on which data was received	*/
	Ptr			rdsPtr);					/* pointer to buffer (read data struct)	*/
	
OSErr LowTCPClose(						/* closes a connection on a given TCP stream*/
	StreamPtr	streamPtr,					/* stream identifier for connection		*/
	b_8		timeout);					/* timeout for close command			*/
	
OSErr LowTCPAbort(						/* aborts a TCP connection non-gracefully	*/
	StreamPtr streamPtr);					/* stream identifier for connection		*/
	
OSErr LowTCPRelease(					/* releases a TCP connection stream			*/
	StreamPtr	streamPtr,					/* stream to be released				*/
	Ptr			*recvPtr,					/* connection buffer (returned)			*/
	unsigned long *recvLen);

OSErr LowTCPStatus(						/* returns status information for a stream	*/
	StreamPtr	streamPtr,					/* stream to get status of				*/
	TCPStatusPB	*theStatus);				/* status (see MacTCP manual for info)	*/
	
OSErr LowTCPGlobalInfo(					/* returns status information for all MacTCP*/
	Ptr	*tcpParam,							/* TCP parameters(returned) (see manual)*/
	Ptr	*tcpStat);							/* TCP statistics(returned) (see manual)*/

OSErr GetMyIP(							/* get local IP address */
	ip_addr *ipnum);						/* ip address (returned) */

OSErr MyCloseConnection(unsigned long stream);


#endif _TCPROUTINES_