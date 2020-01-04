// PalaceMusic.c
/*

Todo:
	Fix parser to parse note events better.
	For example F#3e
	

*/

#include <OSUtils.h>
#include <Script.h>
#include <QuickTimeComponents.h>
#include "U-USER.H"
#include "U-Snds.h"

#define MUSIC_ON	0

Boolean		gMusicInited;
TunePlayer	tMusicPlayer;
unsigned long *BuildTuneHeader();
void PlayTune(char *s);
void EndMusic();

// Copied from Q&A
Boolean AreQuickTimeMusicInstrumentsPresent(void);

// Note this is failing!
Boolean AreQuickTimeMusicInstrumentsPresent(void)
{
        ComponentDescription aCD;
        
        aCD.componentType = 'inst';
        aCD.componentSubType = 'ss  ';
        aCD.componentManufacturer = 'appl';
        
        if(FindNextComponent((Component)0, &aCD) != NULL)
                return true;
        else
                return false;
}


void InitMusic()
{
#if MUSIC_ON
	long	theResult;
	if (Gestalt(gestaltQuickTimeVersion, &theResult) == noErr && theResult >= 0x210) {
		// if (!AreQuickTimeMusicInstrumentsPresent())
		//	return;
		tMusicPlayer = OpenDefaultComponent(kTunePlayerType,0);
		if (tMusicPlayer != NULL)
			gMusicInited = true;
	}
	else
		LogMessage("No Music\r");
#endif
}


#define kNoteRequestHeaderEventLength (sizeof(NoteRequest)/sizeof(long) + 2) /* longwords */
unsigned long *BuildTuneHeader()
	{
	unsigned long *header;
	unsigned long *w,*w2;
	NoteRequest *nr;
	NoteAllocator na;	/* just for the NAStuffToneDescription call */
	ComponentResult thisError;

	header = 0;
	na = 0;

	/*
	 * Open up the Note Allocator
	 */
	na = OpenDefaultComponent('nota',0);
	if(!na)
		goto goHome;

	/*
	 * Allocate space for the tune header,
	 * rather inflexibly.
	 */
	header = (unsigned long *)
			NewPtrClear((2 * kNoteRequestHeaderEventLength + 1) * sizeof(long));
	if(!header)
		goto goHome;

	w = header;

	/*
	 * Stuff request for piano polyphony 4
	 */
	w2 = w + kNoteRequestHeaderEventLength - 1; /* last longword of general event */
	_StuffGeneralEvent(*w,*w2, 1, kGeneralEventNoteRequest, kNoteRequestHeaderEventLength);
	nr = (NoteRequest *)(w + 1);
	nr->polyphony = 4;		/* simultaneous tones */
	nr->typicalPolyphony = 0x00010000;
	thisError = NAStuffToneDescription(na,1,&nr->tone);	/* 1 is Piano */
	w += kNoteRequestHeaderEventLength;

	/*
	 * Stuff request for violin polyphony 3
	 */
	w2 = w + kNoteRequestHeaderEventLength - 1; /* last longword of general event */
	_StuffGeneralEvent(*w,*w2, 2, kGeneralEventNoteRequest, kNoteRequestHeaderEventLength);
	nr = (NoteRequest *)(w + 1);
	nr->polyphony = 3;		/* simultaneous tones */
	nr->typicalPolyphony = 0x00010000;
	thisError = NAStuffToneDescription(na,41,&nr->tone);	/* ???!!! what is violin? */
	w += kNoteRequestHeaderEventLength;

	*w++ = 0x60000000;		/* end of sequence marker */


goHome:
	if(na)
		CloseComponent(na);

	return header;
}
// Note 60 = Middle C
// note 

enum { MS_Init, MS_NoteMods, MS_Tempo };

Handle BuildTuneSequence(char *str);
Handle BuildTuneSequence(char *str)
{
	unsigned long *sequence;
	unsigned long *w;
	Handle h=NULL;
	short	curOctave = 5,curNote=0,noteVal=4;
	short	state = MS_Init;
	Boolean	dotFlag=false;
	long	ticksPerWhole = 1200,curDuration;
	long	tempo = 120;
							//   a b c d e f g
	static  char noteOffset[] = {9,11,0,2,4,5,7};
	/*
	 * Allocate space for the tune sequence,
	 * rather inflexibly.
	 */
#define kNoteDuration 240 /* in 600ths of a second */
#define kRestDuration 300 /* in 600ths - tempo will be 120bpm */

	h = NewHandleClear((strlen(str)*2+1) * sizeof(long));
	if(!h)
		goto goHome;
	HLock(h);
	sequence = (unsigned long *) *h;

	w = sequence;

	// w = whole
	// h = half
	// q = quarter
	// e = eighth
	// s = sixteenth
	// t = 32nd
	// x = 64th
	// . = 1/2
	// [0-9] octave (4 = middle c octave)
	// b = flat
	// # = sharp
	// Tempo (optional # at start) - sets tempo and ticksPerWhole
	// Voice# (optional # at start - general midi)
	while (1) {
		if (*str == ' ' || *str == 0) {

			// let's say tempo is 120
			// wholes per minute = tempo/4 = 30
			// seconds per whole = 60 / 30 = 2
			// ticks per whole = 2 * 600 = 1200
			// ticks per half = 1200 / 2 = 600
			// ticks per quarter = 1200 / 4 = 300
			// ticks per dotted quarter = 300 + 300/2 = 450
			curDuration = ticksPerWhole / noteVal;
			if (dotFlag) {
				curDuration += curDuration >> 1;
				dotFlag = false;
			}
			_StuffNoteEvent(*w++,1,12*curOctave+curNote,100,(curDuration*5)/6);	/* piano C */
			_StuffRestEvent(*w++,curDuration);
			state = MS_Init;
			if (*str == 0)
				break;
		}
		else if (state == MS_Init && (*str >= 'A' && *str <= 'G') ||
			(*str >= 'a' && *str <= 'g')) 
		{
			curNote = noteOffset[tolower(*str) - 'a'];
			state = MS_NoteMods;
		}
		else if (*str >= '0' && *str <= '9') {
			curOctave = (*str -= '0') + 1;
		}
		else if (*str == '#') {
			curNote++;
		}
		else if (*str == 'b') {
			curNote++;
		}
		else if (*str == 'w') { // 	whole
			noteVal = 1;
		}
		else if (*str == 'h') { // 	half
			noteVal = 2;
		}
		else if (*str == 'q') { // 	quarter.
			noteVal = 4;
		}
		else if (*str == 'e') { // 	8th.
			noteVal = 8;
		}
		else if (*str == 's') { // 	16th.
			noteVal = 16;
		}
		else if (*str == 't') { // 	32nd.
			noteVal = 32;
		}
		else if (*str == 'x') { // 	64th.
			noteVal = 64;
		}
		else if (*str == '.') {
			dotFlag = true;
		}
		
		++str;
	}
	*w++ = 0x60000000;	/* end marker */

goHome:
	return h;
}

void PlayTune(char *s)
{
#if MUSIC_ON
	unsigned long	*header,*sequence;
	Handle			sequenceH;
	OSErr			oe;
	if (!gMusicInited)
		return;
	header = BuildTuneHeader();
	sequenceH = BuildTuneSequence(s);
	if (!header || !sequenceH)
		return;
	sequence = (unsigned long *)*sequenceH;
	oe = TuneSetHeader(tMusicPlayer, header);
	oe= TuneQueue(tMusicPlayer, sequence, 0x00010000, 0, 0x7FFFFFFF, 0, 0, 0);
	// ! memory leak!
#endif
}

void EndMusic()
{
#if MUSIC_ON
	if (tMusicPlayer) {
		CloseComponent(tMusicPlayer);
		tMusicPlayer = NULL;
	}
#endif
}