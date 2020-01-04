/* S-TIMOUT.H  1/14/97 JAB */

#ifndef _H_S_TIMOUT
#define _H_S_TIMOUT	1

#define DemoTimeOut		(60L*60L*5L)
#define GuestTimeOut	(60L*60L*40L)
#define GUIDMAGIC		0xA1B2C3D4
#define USEGUESTTIMEOUT	0

AuxRegistrationRec	*TranslateToAlternateSignonRecord(LogonInfoPtr inBuf, AuxRegistrationRec *outBuf);


#endif
