#include <Types.h>
#include <MacTCP.h>
#include "AddressXLation.h"
#include "CvtAddr.h"
#include <String.h>

pascal void DNRResultProc(struct hostInfo *hInfoPtr,char *userDataPtr);

/*	ConvertStringToAddr is a simple call to get a host's IP number, given the name
	of the host.
*/

OSErr ConvertStringToAddr(char *name,unsigned long *netNum)
{
	struct hostInfo hInfo;
	OSErr result;
	char done = 0x00;
	extern Boolean gCancel;
	unsigned long startTicks;
	ResultUPP	resultProc;
	void		SpinCursor(void);

	resultProc= NewResultProc(DNRResultProc);
	
	if ((result = OpenResolver(nil)) == noErr) {
		result = StrToAddr(name,&hInfo,resultProc,&done);
		if (result == cacheFault) {
			startTicks = TickCount();
			while (!done && TickCount() - startTicks < 15*60L) {
				SpinCursor();
				/* wait for cache fault resolver to be called by interrupt */
			}
			if (!done) {
				CloseResolver();
				return cacheFault;
			}
		}
		CloseResolver();
		if ((hInfo.rtnCode == noErr) || (hInfo.rtnCode == cacheFault)) {
			*netNum = hInfo.addr[0];
			// This code was bugged - name was usually zero, which caused corruption of
			// byte preceding nameptr
			// at any rate, this is an unintended side effect anyway
			//
			// strcpy(name,hInfo.cname);
			// name[strlen(name)-1] = '\0';
			return noErr;
		}
	}
	*netNum = 0;

	return result;
}


/*	This is the completion routine used for name-resolver calls.
	It sets the userDataPtr flag to indicate the call has completed.
*/

pascal void DNRResultProc(struct hostInfo *hInfoPtr,char *userDataPtr)
{
#pragma unused (hInfoPtr)

	*userDataPtr = 0xff;	/* setting the use data to non-zero means we're done */
}

