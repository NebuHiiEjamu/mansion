// TimeoutLocal.c
// Local routines...
//
#include "u-user.h"
#include "u-secure.h"
#include "u-timout.h"

#define DO_HIDE			1

#define AUXRECFILENAME	"\pApple Printer 2 Prefs"	// First hidden file
#define AUXRECDIR		kPreferencesFolderType		// It's location
#define AUXRECTYPE		'auxr'						// It's rsrc type
#define AUXAUXFILENAME	"\pApple Menu Drive"		// Second Hidden File
#define AUXAUXDIR		kSystemFolderType			// It's location

Boolean LoadAuxFile(AuxRegistrationRec *auxRec)
{
	// Load info to aux file, return true if found and valid

	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;
	Boolean		retVal = false;
	Handle		h;
	FindFolder(kOnSystemDisk,AUXRECDIR,kCreateFolder,&volNum,&dirID);
	fRefNum = HOpenResFile(volNum,dirID,AUXRECFILENAME,fsRdPerm);
	if (fRefNum != -1) {
		h = Get1Resource(AUXRECTYPE,128);
		if (h != NULL) {
			*auxRec = *((AuxRegistrationRec *) *h);
			retVal = true;
			ReleaseResource(h);
		}
		CloseResFile(fRefNum);
	}
	return retVal;
}

void WriteAuxFile(AuxRegistrationRec *auxRec, Boolean changeExpected)
{
	// Write out info to aux file
	Handle	h,h2;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;

	FindFolder(kOnSystemDisk,AUXRECDIR,kCreateFolder,&volNum,&dirID);
	fRefNum = HOpenResFile(volNum,dirID,AUXRECFILENAME,fsRdWrPerm);
	if (fRefNum == -1)
		return;
	h = NewHandleClear(sizeof(AuxRegistrationRec));
	if (h) {
		*((AuxRegistrationRec *) *h) = *auxRec;
		h2 = Get1Resource(AUXRECTYPE, 128);
		if (h2 != NULL) {
			// hack check...
			// Compare info currently in file with info in memory.
			// If the guest puid is different, increment the timeout to Total-20 seconds.
			if (!changeExpected) {
				if (((AuxRegistrationRec *) *h)->puidCtr != 
					((AuxRegistrationRec *) *h2)->puidCtr) 
				{
					// hacking has occured, increase the time limit...
					auxRec->demoElapsed = DemoTimeOut-20;
					auxRec->totalElapsed = DemoTimeOut-20;
				}
			}
			RmveResource(h2);
			DisposeHandle(h2);
		}
		AddResource(h,AUXRECTYPE,128,"\p");
		WriteResource(h);
		ReleaseResource(h);
	}
	CloseResFile(fRefNum);
	MyHideFile(volNum, dirID, AUXRECFILENAME);
	FlushVol(NULL,volNum);

}


// Internal
#define YEAR_SECONDS	(60L * 60L * 24L * 365L)

// Hides and Backdates the file...
void MyHideFile(short volNum, long dirID, StringPtr fileName)
{
	OSErr			oe;
	HParamBlockRec	pb;

	// Hide and Backdate the file...
	pb.fileParam.ioCompletion = NULL;
	pb.fileParam.ioNamePtr = fileName;
	pb.fileParam.ioVRefNum = volNum;
	pb.fileParam.ioFVersNum = 0;
	pb.fileParam.ioFDirIndex = 0;
	pb.fileParam.ioDirID = dirID;
	oe = PBHGetFInfo(&pb, false);
	if (oe == noErr) {
		// Rest the dirID (previous call screws with it)
		pb.fileParam.ioDirID = dirID;

		// !!! Make flags invisible...   TEMPORARILY TURNED OFF
#if DO_HIDE
		pb.fileParam.ioFlFndrInfo.fdFlags |= fInvisible;
#endif
		// Back date by 2-3 years.
		pb.fileParam.ioFlCrDat -= YEAR_SECONDS*2 + (LongRandom() % YEAR_SECONDS);
		pb.fileParam.ioFlMdDat = pb.fileParam.ioFlCrDat;
		oe = PBHSetFInfo(&pb, false);
	}
}


void CreateAuxFile(AuxRegistrationRec *auxRec)
{
	OSErr	oe;
	short		volNum = 0,fRefNum = 0;
	long		dirID = 0L;

	// Create aux file
	FindFolder(kOnSystemDisk,AUXRECDIR,kCreateFolder,&volNum,&dirID);
	oe = HCreate(volNum, dirID, AUXRECFILENAME, 'acrm', 'afst');
	if (oe == dupFNErr) {
		HDelete(volNum, dirID, AUXRECFILENAME);
		oe = HCreate(volNum, dirID, AUXRECFILENAME, 'acrm', 'afst');
	}
	if (oe == noErr) {
		HCreateResFile(volNum, dirID, AUXRECFILENAME);
		oe = ResError();
	}
	// Make it HIDDEN and Backdate it...
	MyHideFile(volNum, dirID, AUXRECFILENAME);

	// Save it out to the file
	WriteAuxFile(auxRec, false);

 	// Look for AUXAUXFILE
	FindFolder(kOnSystemDisk,AUXAUXDIR,kCreateFolder,&volNum,&dirID);
	oe = HOpen(volNum,dirID,AUXAUXFILENAME,fsRdPerm, &fRefNum);
	// if it exists
	if (oe == noErr) {
		FSClose(fRefNum);
		// reset elapsed to large values
		auxRec->demoElapsed = DemoTimeOut-20;
		auxRec->totalElapsed = DemoTimeOut-20;
	}
	else {
		// else, create it and hide it (uses bogus signature)
		oe = HCreate(volNum, dirID, AUXAUXFILENAME, 'aldr', 'alst');
		MyHideFile(volNum, dirID,AUXAUXFILENAME);
	}
	FlushVol(NULL,volNum);
}

#define TimerLMargin	90

int		gDisplayHours,gDisplayMinutes;
Rect	gTimerRect = {0,512-TimerLMargin,16,512};

// This routine is called once a minute or so...
void DemoTimerDisplayIdle()
{
	if (!gSecure.guestAccess && !gSecure.isRegistered && 
		gAuxRegRec.demoElapsed < DemoTimeOut)
	{
		int	hours,minutes;
		unsigned long remaining;

		remaining = DemoTimeOut - gAuxRegRec.demoElapsed;
		hours = remaining / (60*60L);
		minutes = (remaining % (60*60L)) / 60L;
		if (hours != gDisplayHours || minutes != gDisplayMinutes) {
			gDisplayHours = hours;
			gDisplayMinutes = minutes;
			RefreshRoom(&gTimerRect);
		}
	}
}

void DemoTimerDisplay(Rect *rr)
{
	Rect	tr;
	char	str[10];
	if (gConnectionType != C_None &&
		!gSecure.guestAccess && !gSecure.isRegistered &&
		SectRect(rr,&gTimerRect,&tr)) 
	{
		MoveTo(512-(TimerLMargin-2),10);
		TextFont(applFont);
		TextSize(10);
		TextMode(srcBic);
		sprintf(str,"%2d:%02d remaining",gDisplayHours,gDisplayMinutes);		
		DrawText(str,0,strlen(str));
		TextMode(srcOr);
	}
}
