// U-TIMOUT.H

#ifndef U_TIMOUT

#define DemoTimeOut		(60L*60L*5L)
#define GuestTimeOut	(60L*60L*40L)
#define GUIDMAGIC		0xA1B2C3D4
#define USEGUESTTIMEOUT	0

typedef struct {
	time_t	lastTimerCheck;
	int		lastRoomIDRequest;
} TimeOutLocalnfo;

extern AuxRegistrationRec	gAuxRegRec;


// Hook Routines:
void AuxSecurityInit();		// used when security check shows guest reg#
void BeginSessionTimer();	// used when guest signs on
void EndSessionTimer();		// used when guest signs off or is disconnected
void UsageIdle();			// used while guest is online
Boolean ApproveSignOn();	// used to approve use of connect menu command
void AlternateLogon();
void SetDesiredRoom(char *roomname);
void DemoTimerDisplayIdle();
void DemoTimerDisplay(Rect *rr);
void AltLogonReply(AuxRegistrationRec *reply);
void GrantMemberAccess();

// Internal Routines
void UpdateTimer();
unsigned long ComputePuidCRC(unsigned LONG counter);
void MyHideFile(short volNum, long dirID, StringPtr fileName);

// Local Routines: WriteAuxFile();  LoadAuxFile();  CreateAuxFile()
//
void WriteAuxFile(AuxRegistrationRec *auxRec, Boolean changeExpected);
Boolean LoadAuxFile(AuxRegistrationRec *auxRec);
void CreateAuxFile(AuxRegistrationRec *auxRec);

#endif