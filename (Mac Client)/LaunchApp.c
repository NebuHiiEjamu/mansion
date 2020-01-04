// LaunchApp.c		LOCAL
#include <Aliases.h>
#include <Folders.h>
#include "U-USER.H"
#include "U-TIMOUT.H"	// 1/14/97 JAB

void LaunchApp(char *aName);
Boolean FindAppBySig(OSType	sig, FSSpec	*applSpec);
void LaunchAppByFSpec(FSSpec *fSpec);
void LaunchAppBySig(OSType	sig);
OSErr FindAProcess(OSType sig,
							OSType processType,
							 ProcessSerialNumber *psn);

void LaunchAppByFSpec(FSSpec *fSpec)
{
	OSErr				oe;
	LaunchParamBlockRec	gLaunchRec;

	gLaunchRec.launchBlockID = extendedBlock;
	gLaunchRec.launchEPBLength = extendedBlockLen;
	gLaunchRec.launchFileFlags = 0;
	gLaunchRec.launchControlFlags = launchAllow24Bit | launchUseMinimum | launchNoFileFlags;
	gLaunchRec.launchControlFlags |= launchContinue;
	gLaunchRec.launchAppSpec = fSpec;
	gLaunchRec.launchAppParameters = 0L;
	oe = LaunchApplication(&gLaunchRec);
ErrorExit:
	if (oe)
		AddBalloon(NULL,"!!Can't launch application",0);
	else
		gSublaunch = true;
}


void LaunchApp(char *aName)
{
	FSSpec				fSpec;
	Str255				appName;
	Boolean				targetWasFolder,wasAliased;

	// Security Check  7/18/95  disallow apps outside of the plug-ins folder
	if (aName == NULL || aName[0] == 0 ||
		aName[0] == '.' || aName[0] == ':' ||
		aName[0] == '/' || aName[0] == '\\')
	{
		return;
	}
	//
	// System 7 launch
	//
	strcpy((char *) appName,":plugins:");
	strcat((char *) appName,aName);
	CtoPstr((char *) appName);

	FSMakeFSSpec(0,0L,appName,&fSpec);
	ResolveAliasFile(&fSpec,true,&targetWasFolder,&wasAliased);
	LaunchAppByFSpec(&fSpec);
}

// Find Application on local volumes - use the one with the most recent date

Boolean FindAppBySig(OSType	sig, FSSpec	*applSpec)
{
	long			maxCreationDate = 0;
	OSErr			oe;
	HVolumeParam	vol_pb;
	DTPBRec			desktop_pb;
	HIOParam		param_pb;
	GetVolParmsInfoBuffer	volinfo;
	Boolean			retCode = false;
	Str255			applName;

	param_pb.ioNamePtr = NULL;
	param_pb.ioBuffer = (Ptr) &volinfo;
	param_pb.ioReqCount = sizeof(volinfo);

	vol_pb.ioNamePtr = NULL;
	oe = fnfErr;

	for (vol_pb.ioVolIndex = 1;
		PBHGetVInfoSync((HParmBlkPtr) &vol_pb) == noErr;
		vol_pb.ioVolIndex++) {
		param_pb.ioVRefNum = vol_pb.ioVRefNum;
		if (PBHGetVolParmsSync( (HParmBlkPtr) &param_pb) == noErr &&
			volinfo.vMServerAdr == 0) {
			desktop_pb.ioCompletion = NULL;
			desktop_pb.ioVRefNum = vol_pb.ioVRefNum;
			desktop_pb.ioNamePtr= NULL;
			desktop_pb.ioIndex = 0;
			if (PBDTGetPath(&desktop_pb) == noErr) {
				desktop_pb.ioFileCreator = sig;
				desktop_pb.ioNamePtr = applName;
				if (PBDTGetAPPLSync(&desktop_pb) == noErr) {
					retCode = true;
					if ((unsigned long) desktop_pb.ioTagInfo > (unsigned long) maxCreationDate || maxCreationDate == 0) {
						maxCreationDate = desktop_pb.ioTagInfo;
						applSpec->vRefNum = vol_pb.ioVRefNum;
						applSpec->parID = desktop_pb.ioAPPLParID;
						BlockMove(applName,applSpec->name,applName[0]+1);
					}
				}
			}
		}
	}
	if (retCode == false) {	// Search Shared Volumes as well
		for (vol_pb.ioVolIndex = 1;
			PBHGetVInfoSync((HParmBlkPtr) &vol_pb) == noErr;
			vol_pb.ioVolIndex++) {
			param_pb.ioVRefNum = vol_pb.ioVRefNum;
			if (PBHGetVolParmsSync( (HParmBlkPtr) &param_pb) == noErr &&
				volinfo.vMServerAdr != 0) {
				desktop_pb.ioCompletion = NULL;
				desktop_pb.ioVRefNum = vol_pb.ioVRefNum;
				desktop_pb.ioNamePtr= NULL;
				desktop_pb.ioIndex = 0;
				if (PBDTGetPath(&desktop_pb) == noErr) {
					desktop_pb.ioFileCreator = sig;
					desktop_pb.ioNamePtr = applName;
					if (PBDTGetAPPLSync(&desktop_pb) == noErr) {
						retCode = true;
						if ((unsigned long) desktop_pb.ioTagInfo > (unsigned long) maxCreationDate || maxCreationDate == 0) {
							maxCreationDate = desktop_pb.ioTagInfo;
							applSpec->vRefNum = vol_pb.ioVRefNum;
							applSpec->parID = desktop_pb.ioAPPLParID;
							BlockMove(applName,applSpec->name,applName[0]+1);
						}
					}
				}
			}
		}
	}
	return retCode;
}


void LaunchAppBySig(OSType	sig)
{
	FSSpec				fSpec;

	StatusMessage("Searching for Netscape...",0);

	if (!FindAppBySig(sig, &fSpec))
		return;
	LaunchAppByFSpec(&fSpec);
}

// Netscape Suite
#define NS_NetscapeSuite		'WWW!'
#define NS_OpenURL				'OURL'
#define NS_WebActivate			'ACTV'
#define NS_URLSpec				'----'
#define NS_RegisterProtocol		'RGPR'
#define NS_ApplSignatureSpec	'----'
#define NS_ProtocolPrefix		'PROT'

// StdURL Suite
#define SURL_SuiteID			'GURL'
#define SURL_GetURL				'GURL'
#define SURL_Text				'TEXT'

#define kNetscapeCreator		'MOSS'
#define kNetscapeType			'APPL'

// #define kAutoGenerateReturnID
OSErr FindAProcess(OSType sig,
							OSType processType,
							 ProcessSerialNumber *psn)
{
	OSErr err;
	FSSpec			procSpec;
	ProcessInfoRec info;

	psn->highLongOfPSN = 0;
	psn->lowLongOfPSN  = kNoProcess;
	do
	{
		err= GetNextProcess(psn);
		if( err == noErr )
		{

			info.processInfoLength = sizeof(ProcessInfoRec);
			info.processName = NULL;
			info.processAppSpec = &procSpec;

			err= GetProcessInformation(psn, &info);
		}
	} while( (err == noErr) && 
			((info.processSignature != sig) || 
			(info.processType != processType)));

	if( err == noErr )
		*psn = info.processNumber;
	return err;
} // FindProcessBySignature 


void GotoURL(char *url)
{
	if (gPrefs.userPrefsFlags & UPF_ClubMode) {
		StatusMessage("URLs Disabled",0);
		return;
	}

	if (strincmp(url,"palace:",7) == 0) {
		// !!! Extract Room Name and use it when signing on...
		char *str,*roomName,*portStr;
		short	portNo = 9998;
		str= url+7;
		if (*((short *) str) == '//')
			str += 2;
		if ((roomName = strchr(str,'/')) != NULL) {
			*roomName = 0;
			++roomName;	// Use it later
			SetDesiredRoom(roomName);	// 1/14/97 JAB
		}
		else
			SetDesiredRoom(NULL);
		if ((portStr = strchr(str,':')) != NULL) {
			*portStr = 0;
			++portStr;
			portNo = atoi(portStr);
		}
		BeginTCPConnection(str,portNo);
	}
	else {	// Pass to Internet Config or Netscape
		long		theSig = kNetscapeCreator;
		AppleEvent	aeEvent,aeEvent2;
		AEDesc		myAddressDesc;
		OSErr		oe;
		ProcessSerialNumber	pSerial;

		if (FindAProcess(kNetscapeCreator, kNetscapeType, &pSerial)) {
			// LaunchApp("Netscape");
			LaunchAppBySig('MOSS');
			if (FindAProcess(kNetscapeCreator, kNetscapeType, &pSerial))
				oe = AECreateDesc(typeApplSignature, &theSig, sizeof(theSig), &myAddressDesc);
			else
				oe = AECreateDesc(typeProcessSerialNumber, (Ptr) &pSerial, sizeof(pSerial), &myAddressDesc);
		}
		else
				oe = AECreateDesc(typeProcessSerialNumber, (Ptr) &pSerial, sizeof(pSerial), &myAddressDesc);
		if (oe) {
			StatusMessage("Can't Find Netscape",oe);
			return;
		}
		oe = AECreateAppleEvent(NS_NetscapeSuite,	// was SURL_SuiteID which didn't work
								NS_OpenURL,			// was SURL_GetURL which didn't works
								&myAddressDesc,			
								kAutoGenerateReturnID,
								kAnyTransactionID,
								&aeEvent);
		if (oe) {
			StatusMessage("Unable to create Apple Event",oe);
			return;
		}
		oe = AECreateAppleEvent(NS_NetscapeSuite,		// OK?
								NS_WebActivate,				// OK
								&myAddressDesc,			// OK
								kAutoGenerateReturnID,
								kAnyTransactionID,
								&aeEvent2);
		if (oe) {
			StatusMessage("Unable to create Apple Event",oe);
			return;
		}
		oe = AEPutParamPtr(&aeEvent,NS_URLSpec,typeChar,url,strlen(url));	// Was SURL_Text which didn't work
		if (oe) {
			StatusMessage("Can't add URL to AEvent",oe);
			return;
		}
		oe = AESend(&aeEvent2,NULL,kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,
					kAENormalPriority,kAEDefaultTimeout,NULL,NULL);
		if (oe) {
			StatusMessage("AEvent Send Error",oe);
			return;
		}
		oe = AESend(&aeEvent,NULL,kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,
					kAENormalPriority,kAEDefaultTimeout,NULL,NULL);
		if (oe) {
			StatusMessage("AEvent Send Error",oe);
			return;
		}
		AEDisposeDesc(&aeEvent);
	}
}


OSErr ConfigureNetscape(void)
{
	// If netscapeis loaded into memory, send the appropriate appleevents, return noErr
	ProcessSerialNumber	pSerial;
	if (FindAProcess(kNetscapeCreator, kNetscapeType, &pSerial) == noErr) {
		long		theSig = kNetscapeCreator;
		AppleEvent	aeEvent;
		AEDesc		myAddressDesc;
		OSErr		oe;
		long		applSig;
		if ((oe = AECreateDesc(typeProcessSerialNumber, (Ptr) &pSerial, sizeof(pSerial), &myAddressDesc)) != noErr)
			return oe;

		if ((oe = AECreateAppleEvent(NS_NetscapeSuite,
								NS_RegisterProtocol,
								&myAddressDesc,			
								kAutoGenerateReturnID,
								kAnyTransactionID,
								&aeEvent)) != noErr)
			return oe;

		applSig = 'mUsr'; // Palace application signature
		if ((oe = AEPutParamPtr(&aeEvent,NS_ApplSignatureSpec,typeApplSignature,&applSig,sizeof(long))) != noErr)
			return oe;
		oe = AEPutParamPtr(&aeEvent,NS_ProtocolPrefix,typeChar,"palace",6);

		if ((oe = AESend(&aeEvent,NULL,kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,
					kAENormalPriority,kAEDefaultTimeout,NULL,NULL)) != noErr)
			return oe;

		AEDisposeDesc(&aeEvent);
		return noErr;
	}
	// else, look for prefs file, if not found, return -1
	//   if found, modify appropriate STR# resource... return noErr
	else {
		short		volNum = 0,fRefNum = 0;
		long		dirID = 0L;
		Handle		res;
		StringPtr	p;
		short		i,n;
		long		len;
		OSErr		oe;

		FindFolder(kOnSystemDisk,kPreferencesFolderType,kCreateFolder,&volNum,&dirID);
		fRefNum = HOpenResFile(volNum,dirID,"\p:Netscape Ä:Netscape Preferences",fsRdWrPerm);
		if (fRefNum == -1) {
			return -1;
		}
		res = Get1Resource('STR#',2003);
		if (res == NULL) {
			CloseResFile(fRefNum);
			return -1;
		}
		// Check is already setup, if so, close file and return noErr
		p = (StringPtr) *res;
		n = *((short *) p);  p += 2;
		for (i = 0; i < n; ++i) {
			if ((i & 1) && EqualPString(p,"\ppalace",false)) {
				CloseResFile(fRefNum);
				return noErr;
			}
			p += p[0] + 1;
		}
		// Didn't find it add the strings
		len = GetHandleSize(res);
		SetHandleSize(res, len + 12);
		if ((oe = MemError()) != noErr)
			return oe;
		*((short *) *res) += 2;
		BlockMove("\x04mUsr\x06palace",*res+len,12);
		ChangedResource(res);
		CloseResFile(fRefNum);
		return noErr;
	}
	return -1;
}

void NetscapeConfigureCheck(void)
{
	// if preferences say we haven't configured netscape yet
//	if ( !(gPrefs.userPrefsFlags & UPF_NetscapeConfigured) ) {
		if (ConfigureNetscape() == noErr) {
			gPrefs.userPrefsFlags |= UPF_NetscapeConfigured;
			StorePreferences();
		}
//	}
}

// Copied from Think Referenence
#define kFinderSig			'MACS'
#define kSystemType			'FNDR'
#define kAEFinderEvents		'FNDR'
#define kAEOpenSelection	'sope'


OSErr LaunchDocument(FSSpecPtr	theDoc);

OSErr LaunchDocument(FSSpecPtr	theDoc)
{
	AppleEvent	aeEvent;
	AEDesc		myAddressDesc;
	AEDesc		aeDirDesc;
	AEDesc		listElem;
	AEDesc		fileList;
	FSSpec		dirSpec;
	AliasHandle	dirAlias;
	AliasHandle	fileAlias;
	ProcessSerialNumber	process;
	OSErr		myErr;


	if (FindAProcess(kFinderSig,kSystemType,&process))
		return procNotFound;
	myErr = AECreateDesc(typeProcessSerialNumber,(Ptr) &process,sizeof(process),&myAddressDesc);
	if (myErr)
		return myErr;
	// Create an Empty AE
	myErr = AECreateAppleEvent(kAEFinderEvents,kAEOpenSelection,&myAddressDesc,
								kAutoGenerateReturnID,kAnyTransactionID,&aeEvent);
	if (myErr)
		return myErr;
	// Make an FSSpec and alias for the parent folder
	FSMakeFSSpec(theDoc->vRefNum,theDoc->parID,nil,&dirSpec);
	NewAlias(nil,&dirSpec,&dirAlias);
	NewAlias(nil,theDoc,&fileAlias);
			
	// Create the File List
	if ((myErr= AECreateList(nil,0,false,&fileList)) != noErr)
		return myErr;
	
	HLock((Handle) dirAlias);
	AECreateDesc(typeAlias,(Ptr) *dirAlias,GetHandleSize((Handle) dirAlias),&aeDirDesc);
	HUnlock((Handle) dirAlias);
	DisposeHandle((Handle) dirAlias);

	myErr = AEPutParamDesc(&aeEvent,keyDirectObject,&aeDirDesc);

	if (myErr == noErr) {
		long	size;
		AEDisposeDesc(&aeDirDesc);
		HLock((Handle) fileAlias);
		size = GetHandleSize((Handle) fileAlias);
		AECreateDesc(typeAlias,(Ptr) *fileAlias, size, &listElem);
		HUnlock((Handle) fileAlias);
		DisposeHandle((Handle) fileAlias);
		myErr = AEPutDesc(&fileList,0,&listElem);
	}

	if (myErr)
		return myErr;
	AEDisposeDesc(&listElem);
	
	if ((myErr = AEPutParamDesc(&aeEvent,keySelection,&fileList)) != noErr)
		return myErr;
	myErr = AEDisposeDesc(&fileList);
	myErr = AESend(&aeEvent,nil,kAENoReply+kAEAlwaysInteract+kAECanSwitchLayer,kAENormalPriority,
					kAEDefaultTimeout,nil,nil);
	AEDisposeDesc(&aeEvent);	
	
	return myErr;
}
