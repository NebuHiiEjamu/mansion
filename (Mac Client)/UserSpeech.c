// Palace Speech

/*o

OK	  _2OW=kEY or _1OW_k1EY
shh   _SSSSSSS
shhh  _SSSSSSS
brb   ~b1IY_r1AYt_b1AEk
bbl   ~b1IY_b1AEk_l1EY=dUXr
hmm   _h&>m
hmmm  _h&>mm

_1OW_k1EY
*/

#include "U-USER.H"
#include <Speech.h>
#include "AppMenus.h"

Boolean 		gSpeechInited,gSpeechAvailable;
SpeechChannel	gWhisperChan;
VoiceSpec		*gVoiceSpecs;
short			gNumVoices;

#define kMaxSpeechChannels	2
#define kNormalChan			0
#define kWhisperChan		1

struct {
	SpeechChannel	chan;
	char			tBuffer[256];
} gSpeechChannel[kMaxSpeechChannels];

void InitSpeech();
void SpeakChat(short flag, char *str);

void InitSpeech()
{
	long				theResult;
	short				n;
	VoiceDescription	vInfo;
	VoiceSpec			*specPtr,*whisperSpec = NULL;
	Handle				userDict;
	if (Gestalt(gestaltSpeechAttr, &theResult) == noErr	&&	
		(theResult & (1 << gestaltSpeechMgrPresent)))
	{
		SetItem(gVoicesMenu, 1, "\pNone");
		AppendMenu(gVoicesMenu, "\p(-");
		CountVoices(&gNumVoices);
		gVoiceSpecs = (VoiceSpec *) NewPtrClear(sizeof(VoiceSpec)*gNumVoices);
		for (n = 0; n < gNumVoices; ++n) {
			if (GetIndVoice(gNumVoices-n,&gVoiceSpecs[n]) == noErr) {
				GetVoiceDescription(&gVoiceSpecs[n],&vInfo,sizeof(VoiceDescription));
				AppendMenu(gVoicesMenu, vInfo.name);
				if (EqualPString(vInfo.name,"\pWhisper",false))
					whisperSpec = &gVoiceSpecs[n];
			}
		}
		if (gPrefs.voiceNumber && gPrefs.voiceNumber <= gNumVoices) {
			specPtr = &gVoiceSpecs[gPrefs.voiceNumber-1];
			CheckItem(gVoicesMenu, gPrefs.voiceNumber+2,true);
		}
		else {
			CheckItem(gVoicesMenu, 1,true);
			specPtr = NULL;
		}
		if (NewSpeechChannel (specPtr, &gSpeechChannel[kNormalChan].chan) != noErr)
			gSpeechChannel[kNormalChan].chan = NULL;
		if (whisperSpec) {
			if (NewSpeechChannel (whisperSpec, &gSpeechChannel[kWhisperChan].chan) != noErr)
				gSpeechChannel[kWhisperChan].chan = NULL;
		}
		userDict = GetResource('dict',128);
		if (userDict) {
			OSErr	oe;
			if (gSpeechChannel[kNormalChan].chan) {
				oe = UseDictionary(gSpeechChannel[kNormalChan].chan,userDict);
				if (oe)
					LogMessage("Dictionary Error: %d",oe);
			}
			if (gSpeechChannel[kWhisperChan].chan) {
				oe = UseDictionary(gSpeechChannel[kWhisperChan].chan,userDict);
				if (oe)
					LogMessage("Dictionary Error: %d",oe);
			}
			ReleaseResource(userDict);
		}
    	gSpeechAvailable = true;
	}
	gSpeechInited = true;
}

void SelectVoice(short voiceNumber)
{
	VoiceSpec			*specPtr;
	Handle				userDict;
	if (gSpeechChannel[kNormalChan].chan) {
		DisposeSpeechChannel(gSpeechChannel[kNormalChan].chan);
		if (voiceNumber && voiceNumber <= gNumVoices) {
			specPtr = &gVoiceSpecs[voiceNumber-1];
		}
		else
			specPtr = NULL;
		NewSpeechChannel(specPtr, &gSpeechChannel[kNormalChan].chan);
		if (gSpeechChannel[kNormalChan].chan) {
			userDict = GetResource('dict',128);
			if (userDict) {
				OSErr	oe;
				oe = UseDictionary(gSpeechChannel[kNormalChan].chan,userDict);
				if (oe)
					LogMessage("Dictionary Error: %d",oe);
				ReleaseResource(userDict);
			}
		}

	}
}
/**
void GetPhonemes(char *str);
void GetPhonemes(char *str)
{
	Handle	ph;
	long	pCount=512;

	ph = NewHandleClear(512);
	TextToPhonemes(gSpeechChannel[kNormalChan].chan,str,strlen(str),ph,&pCount);
	HLock(ph);
	(*ph)[pCount] = 0;
	LogMessage("%s\r",*ph);
	DisposeHandle(ph);
}
**/
void SpeakChat(short flag, char *str)
{
	short	len,chanNbr=0;
	if (flag == 4)	// Don't speak thought balloons
		return;
	if (gPrefs.userPrefsFlags & UPF_TextToSpeech) {
		if (!gSpeechInited)
			InitSpeech();
		if (gSpeechAvailable) {
			if (flag == 1 && gSpeechChannel[kWhisperChan].chan)
				chanNbr = kWhisperChan;
			else
				chanNbr = kNormalChan;
			len = strlen(str);
			if (len > 255)
				len = 255;
			StopSpeech(gSpeechChannel[chanNbr].chan);
			BlockMove(str,gSpeechChannel[chanNbr].tBuffer,len);
			gSpeechChannel[chanNbr].tBuffer[len] = 0;
			SpeakText(gSpeechChannel[chanNbr].chan, gSpeechChannel[chanNbr].tBuffer,len);
		}
	}
}
