/************************************************************************************
 * User.c
 *
 ************************************************************************************/

#include "U-User.h"
#include <Sound.h>
#include "U-Snds.h"

static Handle			sounds[S_NbrSounds];
static SndChannelPtr	scp;
static SndCommand		tCmd;						/* Sound Command */
static long				savedVolumeLong;
SndCallBackUPP			mySndCallBack;
Ptr	gWaveSound;

pascal void MyCallBack(SndChannelPtr chan, SndCommand *cmd);

pascal void MyCallBack(SndChannelPtr chan, SndCommand *cmd)
{
	chan->userInfo = 0;
}

OSErr ChangeVolume(SndChannelPtr chan, long myVolume);
OSErr ChangeVolume(SndChannelPtr chan, long myVolume)
{
    OSErr           err;
    SndCommand      cmd;

    cmd.cmd = volumeCmd;
    cmd.param2 = myVolume;
    err = SndDoImmediate(chan, &cmd);
    return (err);
}

void InitSounds(void)
{
	short	i;
	OSErr	oe;
	for (i = 0; i < S_NbrSounds; ++i) {
		sounds[i] = Get1Resource('snd ',128+i);
		if (sounds[i] != NULL) {
			HNoPurge(sounds[i]);
			MoveHHi(sounds[i]);
			HLock(sounds[i]);
		}
	}
	scp = (SndChannelPtr) NewPtrClear((long) sizeof(SndChannel));
	scp->qLength = stdQLength;
	mySndCallBack = NewSndCallBackProc((ProcPtr) MyCallBack);
	if ((oe = SndNewChannel(&scp,sampledSynth,initMono,mySndCallBack)) != noErr) {
		DisposePtr((Ptr) scp);
		scp = NULL;
		LogMessage("Sound problem\r");
		return;
	}

	// GetSoundVol(&savedVolume);	// address of short
	// SetSoundVol(gPrefs.soundLevel);
	// GetDefaultOutputVolume(&savedVolumeLong);
	ChangeVolume(scp,((long) gPrefs.soundLevel << 5) | ((long) gPrefs.soundLevel << 21));
	// SetDefaultOutputVolume(((long) gPrefs.soundLevel << 8) | ((long) gPrefs.soundLevel << 24));
	InitMusic();
}

void EndSounds(void)
{
	if (scp) {
		SndDisposeChannel(scp,true);
		DisposPtr((Ptr) scp);
		scp = 0L;
	}
	EndMusic();
	// SetSoundVol(savedVolume);
	// SetDefaultOutputVolume(savedVolumeLong);
}

void PlaySnd(short i, short priority)
{
	if (scp == NULL)
		return;
	if (gPrefs.soundLevel > 0 && i < S_NbrSounds && sounds[i] &&
		priority > scp->userInfo && (!gSuspended || priority >= 10)) {
		tCmd.cmd = quietCmd;
		SndDoImmediate(scp,&tCmd);
		tCmd.cmd = flushCmd;
		SndDoImmediate(scp,&tCmd);
		SndPlay(scp, (SndListHandle) sounds[i], true);
		scp->userInfo = priority;
		tCmd.cmd = callBackCmd;
		SndDoCommand(scp,&tCmd,true);

		if (gWaveSound) {
			DisposePtr(gWaveSound);
			gWaveSound = NULL;
		}

	}
}

void AdjustUserVolume(short volNbr)
{
	if (scp == NULL)
		return;
	gPrefs.soundLevel = volNbr;
	StorePreferences();
	ChangeVolume(scp,((long) gPrefs.soundLevel << 5) | ((long) gPrefs.soundLevel << 21));
	// SetDefaultOutputVolume(((long) gPrefs.soundLevel << 8) | ((long) gPrefs.soundLevel << 24));
	PlaySnd(S_Fader, 4);
	// SetSoundVol(gPrefs.soundLevel);
	// SysBeep(1);
}

typedef struct {
		short	wFormatTag;
		short	wChannels;
		long	dwSamplesPerSec;
		long	dwAvgBytesPerSec;
		short	wBlockAlign;
		short	wSampleSize;
} WaveHeader;

void PlayWaveFile(char *fName, Boolean requestForDownload)
{
	Str63		fileName;
	StringPtr	fullName;
	FileHandle	refNum;
	long		length;
	OSErr		oe;
	WaveHeader	wHdr;
	SoundHeader	*sndH;
	int			n;
	FInfo		fInfo;

	if (scp == NULL)
		return;

	if (scp->userInfo)
		return;

	if (gWaveSound) {
		DisposePtr(gWaveSound);
		gWaveSound = NULL;
	}
	

	strcpy((char *) fileName, fName);
	CtoPstr((char *) fileName);

	// 8/6/96
	// Search for sound in a number of locations, including
	// server's sounds folder
	n = 0;
	do {
		++n;
		fullName = BuildMediaFolderName(fileName,SoundsPathList, n);
		if (fullName) {
			oe = GetFInfo(fullName, 0, &fInfo);
		}
	} while (fullName && oe != noErr);

	// 8/6/96 If we can't find sound, request for downloading
	// if this routine was called from a script
	if (fullName == NULL || oe != noErr) {
		if (requestForDownload && 
			!FileIsBeingDownloaded(fileName) &&
			(gPrefs.userPrefsFlags & UPF_DownloadGraphics) > 0)
		{
			RequestFile(fileName, MT_Sound);
		}
		return;
	}
	if ((oe = OpenFileReadOnly((StringPtr) fullName,0,&refNum)) != noErr) {
		return;
	}
	GetEOF(refNum,&length);
	if (length < 0x23) {
		FSClose(refNum);
		return;
	}
	gWaveSound = NewPtrClear(length);	/* 7/1/96 JAB Changed to NewPtrClear */
	if (gWaveSound == NULL) {
		FSClose(refNum);
		return;
	}
	FSRead(refNum, &length, gWaveSound);
	FSClose(refNum);

	wHdr = *((WaveHeader *)  &gWaveSound[0x14]);
	SwapLong((unsigned long *) &wHdr.dwSamplesPerSec);
	if (wHdr.dwSamplesPerSec == 0 ||
		wHdr.dwSamplesPerSec > 50000L)
		wHdr.dwSamplesPerSec = 22050;

	sndH = (SoundHeader *) &gWaveSound[22];	// 0x2C - 22
	memset(sndH,0,22);	
	sndH->length = length - 0x2C;
	sndH->sampleRate = (Fixed) (wHdr.dwSamplesPerSec << 16);
	sndH->baseFrequency = 0x003c;
//	sndH->samplePtr = NULL;
//	sndH->length = length - 0x2C;
//	sndH->sampleRate = (Fixed) (wHdr.dwSamplesPerSec << 16);
//	sndH->loopStart = sndH->loopEnd = 0;
//	sndH->baseFrequency = 0x003c;

	tCmd.cmd = bufferCmd;
	tCmd.param1 = 0;
	tCmd.param2 = (long) sndH;
	SndDoCommand(scp,&tCmd,true);
	scp->userInfo = 1;
	tCmd.cmd = callBackCmd;
	tCmd.param1 = 0;
	tCmd.param2 = 0L;
	SndDoCommand(scp,&tCmd,true);
}
