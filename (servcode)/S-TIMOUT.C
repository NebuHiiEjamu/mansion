/* S-TIMOUT.C 1/14/97 JAB */

#include "s-server.h"
#include "s-timout.h"

AuxRegistrationRec	*TranslateToAlternateSignonRecord(LogonInfoPtr inBuf, AuxRegistrationRec *outBuf)
{
	memset(outBuf,0,sizeof(AuxRegistrationRec));
	outBuf->counter = inBuf->counter;
	outBuf->crc = inBuf->crc;
	BlockMove(inBuf->userName,outBuf->userName,inBuf->userName[0]+1);
	return outBuf;
}
