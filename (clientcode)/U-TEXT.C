////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "u-User.h"
#include "m-cmds.H"
#include "u-Snds.h"
#include "u-Script.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is a copyright notice to be compiled into the source code.
static char	copyRightMessage[] = "Copyright 1996 The Palace, Inc";


BalloonRec balloons[MaxBalloonRecs];
short		   nbrBalloons;
#ifdef macintosh
long		   readingSpeedTicks[] = {11,9,8};
#else
long		   readingSpeedTicks[] = {183,150,133};
#endif

void SpeakBalloon(BalloonRec *balloon);

// Process String for Output
//
void ProcessUserString(Ptr ht, short len)
{
	char	temp[256],temp2[256],*p;
	Boolean	privateFlag;

	len = len > 255? 255 : len;

	if (ht[0] == '/') {	// 9/21/95
		if (!(gRoomWin->serverInfo.serverPermissions & PM_AllowCyborgs) && !(gRoomWin->userFlags & U_SuperUser)) {
			StdStatusMessage(SM_DeniedByServer);
		}
		else if ((gRoomWin->curRoom.roomFlags & RF_CyborgFreeZone) && !(gRoomWin->userFlags & U_SuperUser))
			StdStatusMessage(SM_CyborgFreeZone);
		else {
			p = temp;
			BlockMove(ht,p,len);
			p[len] = 0;
			DoUserScript(&p[1]);
		}
	}
	else if (ht[0] == '~') {
		p = temp;
		BlockMove(ht,p,len);
		p[len] = 0;
		ProcessMacro(&p[1]);
	}
	else {
		BlockMove(ht,temp2,len);

		temp2[len] = 0;		// 7/21/95
		gWhoChat = gRoomWin->mePtr->user.userID;
		SetExternStringGlobal(temp2,GS_ChatString);	// 6/6/95 JAB - allow script to intercept
		TriggerHotspotEvents(PE_OutChat);			// 6/6/95 JAB
		GetExternStringGlobal(temp2,GS_ChatString);	// 6/6/95 JAB

		len = strlen(temp2);		// 7/21/95

		if (len <= 0)				// 7/21/95
			return;

		privateFlag = (gRoomWin->targetID != 0); /* 3/20/95 bug fix - wasn't assigning bool properly */
		// 11/8/95 god commands aren't private
		//if ((gRoomWin->userFlags & U_God) && temp2[0] == '`')
		//	privateFlag = false;
		
		if (privateFlag) {
			*((long *) &temp[0]) = gRoomWin->targetID;
			p = &temp[4];
		}
		else {
			p = &temp[0];
		}
		len = len > 247? 247 : len;
		*((short *) p) = len+3;	// Flag to indicate Encryption
		p += 2;
		EncryptCString((unsigned char *)temp2,(unsigned char *) p,len);
		p[len] = 0;
		if (privateFlag)
			PostServerEvent(MSG_XWHISPER,gRoomWin->meID,temp,len+7);
		else
			PostServerEvent(MSG_XTALK,gRoomWin->meID,temp,len+3);
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void DeleteBalloon(short n)
{
	short	i;

	for (i = n; i < nbrBalloons-1; ++i) {
		balloons[i] = balloons[i+1];
	}
	nbrBalloons--;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClearBalloons()
{
	nbrBalloons = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void KillBalloon(short balloonID)
{
	Rect	ur,tr;
	short	i,j;
	long	t;
	Boolean	sectFlag;

	ur = balloons[balloonID].rr;
	DeleteBalloon(balloonID);
	for (i = 0; i < nbrBalloons && nbrBalloons < MaxBalloonRecs; ++i) {
		if (!balloons[i].active) {
			sectFlag = false;
			for (j = 0; j < nbrBalloons; ++j) {
				if (balloons[j].active && SectRectMac(&balloons[i].r,&balloons[j].r,&tr)) {
					sectFlag= true;
					break;
				}
			}
			if (!sectFlag) {
				UnionRectMac(&ur,&balloons[i].rr,&ur);
				balloons[i].active = true;
				SpeakBalloon(&balloons[i]);
				t =  balloons[i].len*readingSpeedTicks[gPrefs.readingSpeed];
				if (t < BalloonMinTicks)
					t = BalloonMinTicks;
				balloons[i].exitTime = GetTicks() + t;
			}
		}
	}
	RefreshRoom(&ur);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void BalloonIdle()
{
	short	i;
	unsigned long	t;
	static unsigned long lastUpdate;

	if ((t = GetTicks()) - lastUpdate > 5L*TICK_SECONDS) {
		UpdateAssetFile();	// 2/28/95 Good time to update the asset file
		lastUpdate = t;
	}

	for (i = 0; i < nbrBalloons; ++i) {
		if (balloons[i].active && balloons[i].exitTime && GetTicks() >= (unsigned long)balloons[i].exitTime) {
			KillBalloon(i);
			return;
		}
	}
	AlarmsIdle();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void KillUserBalloons(PersonID userID)
{
	short	i;

	for (i = 0; i < nbrBalloons; ++i) {
		if (balloons[i].active && balloons[i].speakerID == userID) {
			KillBalloon(i);
			--i;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kill Permanant balloons only
//
void KillPermBalloons(PersonID userID)
{
	short	i;

	for (i = 0; i < nbrBalloons; ++i) {
		if (balloons[i].active && balloons[i].speakerID == userID && balloons[i].perm) {
			KillBalloon(i);
			--i;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserText(PersonID userID,LPSTR text,Boolean privateFlag)
{
	LPUSER	cUser = GetUser(userID);

	if (cUser)
  {
	
		if (gLogMessages)
    {
#if macintosh
			SetLogColor(&gGray44);
#endif

			if (privateFlag)
				LogMessage("* %.*s * %*s",cUser->user.name[0],&cUser->user.name[1],
										cUser->user.name[0] > 13? 0 : 13-cUser->user.name[0],
										"");
			else
				LogMessage("%.*s: %*s",cUser->user.name[0],&cUser->user.name[1],
										cUser->user.name[0] > 16? 0 : 16-cUser->user.name[0],
										"");
#if macintosh
			SetLogColor(&gBlackColor);
#endif
			LogString(text);
			LogString("\r");
		}
		if (cUser)			// 6/23/95
        {
			gWhoChat = cUser->user.userID;
			SetExternStringGlobal(text,GS_ChatString);
			TriggerHotspotEvents(PE_InChat);			// 6/6/95 JAB
			GetExternStringGlobal(text,GS_ChatString);	// 6/6/95 JAB
        }
		if (!(text[0] == ';' && text[1] != ')'))		// 9/22/95
		  	AddBalloon(cUser,text,privateFlag);
	}
	else if (userID == 0)
  {
		if (gLogMessages)
    	{
#if macintosh
			SetLogColor(&gRedColor);
#endif
			LogString("*** ");
			LogString(text);
			LogString("\r");
#if macintosh
			SetLogColor(&gBlackColor);
#endif
   		}
		if (text[0] != ';')		// 9/22/95
			AddBalloon(NULL, text, privateFlag);
	}
	else {
		// User is in another room - will eventually need to get the user's name...
		if (gLogMessages)
    	{
#if macintosh
			SetLogColor(&gGray44);
#endif
			LogString("<unknown user> ");
			LogString(text);
			LogString("\r");
#if macintosh
			SetLogColor(&gBlackColor);
#endif
   		}
		if (text[0] != ';')		// 9/22/95
			AddBalloon(NULL, text, true);
	}
#ifdef WIN32
if (text[0] == ';')	
	LogUserText(cUser,text,privateFlag);
#endif
	if (privateFlag && (IsAppIconic() || IsAppSuspended()))
		PlaySnd(S_DoorChime, 10); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddBalloon(LocalUserRecPtr cUser, char *text, Boolean privateFlag)
{
	short		len,i;
	Rect		br;
	long		t;
	Boolean		waitFlag=false;	
	Point		target;
	BalloonRec	*bPtr;
	// CGrafPtr	curPort;
	// GDHandle	curDevice;
								// 6/1/95
	Boolean		thoughtFlag=false,exciteFlag=false,permFlag=false;

#ifdef WIN32
gTalkingUser = cUser;
#endif

	//
	// 6/1/95 set up default target first
	//
	if (cUser){
		target = cUser->user.roomPos;
		KillPermBalloons(cUser->user.userID);	// 6/1/95 Kill Perm Balloons
	}
	else {
		target.h = -10;
		target.v = 16;
	}

	// 6/1/95
	//
	// Wrote a more flexible parser (doesn't care about order of prefixes)
	//
	while (*text) {
		switch (*text) {
		case '!':	
			exciteFlag = true;
			// if (cUser == NULL)
			//	soundFlag = true;
			++text;
			continue;
		case '|':
			++text;
			if (cUser) {
				cUser->user.faceNbr = FACE_Normal;
				RefreshUser(cUser);
			}
			continue;
		case ';':
			++text;
			switch (*text) {
			case ')':
				++text;
			default:
				if (cUser)	{
					cUser->user.faceNbr = FACE_WinkLeft;
					RefreshUser(cUser);
				}
				break;
			}
			continue;
		case ':':
			++text;
			switch (*text) {
			case '(':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_Sad;
					RefreshUser(cUser);
				}
				break;
			case '[':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_Angry;
					RefreshUser(cUser);
				}
				break;
			case '|':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_Normal;
					RefreshUser(cUser);
				}
				break;
			case ')':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_Smile;
					RefreshUser(cUser);
				}
				break;
			case '0':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_Talk;
					RefreshUser(cUser);
				}
				break;
			case '/':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_TiltRight;
					RefreshUser(cUser);
				}
				break;
			case '\\':
				++text;
				if (cUser)	{
					cUser->user.faceNbr = FACE_TiltLeft;
					RefreshUser(cUser);
				}
				break;
			default:
				thoughtFlag = true;
				break;
			}
			continue;
		case '^':			// 6/1/95 parse for Perm Balloon
			if (cUser)		//    don't allow usage on system messages
				permFlag = true;
			++text;
			continue;
		case '@':
			++text;
			target.h = 0;
			target.v = 0;
			if (isdigit(*text)) {
				target.h = atoi(text);
				while (*text && *text == '-' || isdigit(*text))
					++text;
				if (*text == ' ')
					++text;
				if (*text == ',')
					++text;
				target.v = atoi(text);
				while (*text == '-' || isdigit(*text))
					++text;
				if (*text == ' ')
					++text;
				if (*text == ',')
					++text;
				if (*text == ' ')
					++text;
			}
			continue;
		case ')':
			++text;
			{
				char	*sp,soundFile[65];
				short	i = 0;
				sp = soundFile;
				while (*text && i < 63 && *text != ' ' && (isalnum(*text) || *text == '_'
															|| *text == '.')) 
				{
					*(sp++) = *(text++);
					++i;
				}
				*sp= 0;
				if (soundFile[0])
					PlayWaveFile(soundFile, false);	// 8/6/95 - added flag
				if (*text == ' ')
					++text;
			}
			continue;
#if M5SUPPORT
		case '}':
			++text;
			{
				char	*sp,movieName[65];
				short	i = 0;
				void	PlayM5Movie(char *name);
				sp = movieName;
				while (*text && i < 63 && *text != ' ' && (isalnum(*text) || *text == '_'
															|| *text == '.')) 
				{
					*(sp++) = *(text++);
					++i;
				}
				*sp= 0;
				if (movieName[0])
					PlayM5Movie(movieName);	// 9/27/95
				if (*text == ' ')
					++text;
			}
			continue;
#endif
		case '%':
			++text;
			PlayTune(text);
			return;
		default:
			break;	// If not a known prefix character, break out of loop
		}
		break;	// break out of loop if we haven't hit a continue statement
	}

	if (*text == 0)
		return;


#ifdef WIN32
LogUserText(cUser,text,privateFlag);
gTalkingUser = NULL;
#endif

	len = strlen(text);

	GetBalloonRect(&br,text,len,privateFlag);
	// GetBalloonRect(Rect *br, Ptr text, long length, Boolean privateFlag)


	//
	// 6/1/95 - if PermFlag rectangle is positioned with left corner in users face
	//
	if (permFlag) {
		OffsetRectMac(&br,target.h,target.v);
	}
	else {
		OffsetRectMac(&br,target.h+FaceWidth/2+BalloonGap,target.v);
		if (br.right > gOffscreenRect.right)
			OffsetRectMac(&br,-((br.right-br.left)+FaceWidth+BalloonGap*2),0); // 6/13/95
		// 9/11/95 Attempt to not overlap other user's faces...
		else {
				LocalUserRecPtr	up;
				Rect			br2,ur;
				Boolean			overLap = false;
				up = gRoomWin->userList;
				for (i = 0; i < gRoomWin->curRoom.nbrPeople; ++i,++up) {
					if (up != cUser) {
						ComputeUserRect(up, &ur);
						if (SectRectMac(&br,&ur,&ur)) {
							overLap = true;
							break;
						}
					}
				}
				if (overLap) {
					br2 = br;
					OffsetRectMac(&br2,-((br.right-br.left)+FaceWidth+BalloonGap*2),0); // 6/13/95
					if (br2.left >= gOffscreenRect.left-24)
						br = br2;
				}
		}
	}
	if ( br.right > gOffscreenRect.right)
		OffsetRectMac(&br,-(br.right - gOffscreenRect.right),0); // 6/13/95
	if ( br.bottom > gOffscreenRect.bottom)
		OffsetRectMac(&br,0,-(br.bottom - gOffscreenRect.bottom)); // 6/13/95
	if (br.left < gOffscreenRect.left)
		OffsetRectMac(&br,gOffscreenRect.left - br.left,0);
	if (br.top < gOffscreenRect.top)
		OffsetRectMac(&br,0,gOffscreenRect.top - br.top);

	for (i = 0; i < nbrBalloons; ++i) {
		if (balloons[i].active && !balloons[i].perm)	// 6/13/95
		{
			Rect	ttr,tr;
			ttr = balloons[i].r;
			InsetRectMac(&ttr,BalloonMargin,BalloonMargin);
			if (SectRectMac(&br,&ttr,&tr)) {
				waitFlag = true;
				break;
			}
		}
	}
	if (nbrBalloons >= MaxBalloonRecs)	// 7/22 changed to >=
		return;

	bPtr = &balloons[nbrBalloons];

	bPtr->active = !waitFlag;
	bPtr->r = br;
	i = BalloonGap+FaceWidth/2;
	i /= 2;
	InsetRectMac(&br,-i,-i);
	if (thoughtFlag)
		InsetRectMac(&br,-4,-4);
	if (exciteFlag)					// 8/22/95 compensate for shadow on excite balloons
		InsetRectMac(&br,-1,-1);
	bPtr->rr = br;
	bPtr->target = target;
	if (cUser) {
		bPtr->speakerID = cUser->user.userID;
		bPtr->system = false;
		bPtr->tint = cUser->user.colorNbr;
	}
	else {
		bPtr->speakerID = 0;
		bPtr->system = true;
		bPtr->tint = -1;
	}
	bPtr->len = len;
	t =  bPtr->len*readingSpeedTicks[gPrefs.readingSpeed];
	if (t < BalloonMinTicks)
		t = BalloonMinTicks;
	if (permFlag)				// 6/1/95 - no exittime for Perm balloons
		bPtr->exitTime = 0;
	else
		bPtr->exitTime = GetTicks() + t;
	bPtr->private = privateFlag;
	bPtr->thought = thoughtFlag;
	bPtr->excited = exciteFlag;
	bPtr->perm = permFlag;		// 6/1/95
	BlockMove(text,bPtr->str,len+1);
	++nbrBalloons;

	if (!waitFlag) {
		SpeakBalloon(bPtr);
		//  10/6/95 disabled automatic choir sound to save memory use )Amen
		// if (soundFlag)
		// 	PlaySound(S_Choir, 3);
		RefreshRoom(&br);
	}
}

void SpeakBalloon(BalloonRec *balloon)
{
	short	flag = 0;
	if (gPrefs.userPrefsFlags & UPF_TextToSpeech) {
		if (balloon->private)
			flag = 1;
		else if (balloon->system)
			flag = 2;
		else if (balloon->excited)
			flag = 3;
		else if (balloon->thought)
			flag = 4;
		SpeakChat(flag,balloon->str); // Local Function
	}
}
