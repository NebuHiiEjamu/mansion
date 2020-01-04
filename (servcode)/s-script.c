/* esr - Need to redo from original with all the new
 *                       Mac2Win stuff.
 *
 * Added const char ScriptNameName
 */
/******************************************************************************************/

/******************************************************************************************/

#include "s-server.h"
#include "m-cmds.h"	/* 6/26/96 JAB */
#include <ctype.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#define LONGALIGN(x)	x += (4 - (x & 3)) & 3

#ifdef macintosh
#define ScriptCreator	'KAHL'
#define ScriptFileType	'TEXT'
const unsigned char *ScriptFileName = "\pMansion Script";
#else
#ifdef unix
#define ScriptCreator	0
#define ScriptFileType	0
unsigned char *ScriptFileName = (unsigned char *) "\x0dmansionscript";
#else
#define ScriptCreator	0
#define ScriptFileType	0
const char *ScriptFileName = "\x0bPServer.dat";
#endif
#endif

/******************************************************************************************/

extern short gNbrRooms;

HugePtr 		 gBigBuffer;  /* 200k - used for temporary stuff */
RoomRecPtr       gRoom;
HotspotPtr       gHotspots;
StateRecPtr      gSpotStates;
PictureRecPtr    gPictureRecs;
DrawRecPtr       gDrawCmds;
Point            *gPoints;
HugePtr          gScript; /* esr - moved up here and changed Ptr to HugePtr */
HugePtr          gSP;
LPropPtr         gLProps;
ServerUserPtr	 *gUserArray;
/******************************************************************************************/

static HugePtr  gScriptBuffer;
/* not used static Str63        gPictDir;   */
char              *gToken;
static Boolean  ungetFlag;

/******************************************************************************************/

/* 12/11/95 */
/* this is done one time only now, was moved up from BeginRoomCreation */

Boolean AllocateServerBuffers()
{
	/* 7/20/96 Not a great place to stick this, but it needs to be initialized somewhere... */
	void InitializeEncryption();
	InitializeEncryption();

#if unix
    gBigBuffer = (HugePtr) NewPtrClear(400000L);
#else
    gBigBuffer = (HugePtr) NewPtrClear(200000L);
#endif
    if (gBigBuffer == NULL)
            return true;

    gRoom = (RoomRecPtr) NewPtrClear(sizeof(RoomRec)+32767L);
    if (gRoom == NULL)
            return true;
    gRoom->lenVars = 2;
    gHotspots = (HotspotPtr) NewPtrClear(sizeof(Hotspot) * MaxHotspots);
    if (gHotspots == NULL)
            return true;
    gDrawCmds = (DrawRecPtr) NewPtrClear(sizeof(DrawRecord) * MaxDrawCmds);
    if (gDrawCmds == NULL)
            return true;
    gPictureRecs = (PictureRecPtr) NewPtrClear(sizeof(PictureRec) * MaxPictureRecs);
    if (gPictureRecs == NULL)
            return true;
    gSpotStates = (StateRecPtr) NewPtrClear(sizeof(StateRec) * MaxSpotStates);
    if (gSpotStates == NULL)
            return true;
    gPoints = (Point *) NewPtrClear(sizeof(Point) * MaxHotPts);
    if (gPoints == NULL)
            return true;
    gLProps = (LPropPtr) NewPtrClear(sizeof(LPropRec) * MaxLProps);
    if (gLProps == NULL)
            return true;
	/* 7/26/96 Allocate an array for speeding up user lookups */
	gUserArray = (ServerUserPtr *) NewPtrClear(sizeof(ServerUserPtr)*(MaxUserID+1));
	if (gUserArray == NULL)
			return true;
	return false;
}

void DisposeServerBuffers()
{
  DisposePtr((Ptr) gPoints);
  DisposePtr((Ptr) gSpotStates);
  DisposePtr((Ptr) gPictureRecs);
  DisposePtr((Ptr) gDrawCmds);
  DisposePtr((Ptr) gLProps);
  DisposePtr((Ptr) gHotspots);
  DisposePtr((Ptr) gRoom);
  DisposePtr((Ptr)gBigBuffer);
}

void UngetToken(void)
{
        ungetFlag = true;
}

/******************************************************************************************/

Boolean GetToken(void)
{
        char    *dp;

        if (ungetFlag) {
                ungetFlag = false;
                return TRUE;
        }
reget:
        if (*gSP == 0)
                return FALSE;
        else if (isspace(*gSP)) {
                ++gSP;
                goto reget;
        }
        else if (*gSP == '#' || *gSP == ';') {
                while (*gSP && *gSP != '\r' && *gSP != '\n')
                        ++gSP;
                goto reget;
        }
        else if (isalnum(*gSP) || *gSP == '_' || *gSP == '.' || (*gSP == '-' && isdigit(gSP[1])))
        {
                dp = gToken;
                if (*gSP == '-')
                        *(dp++)  = *(gSP++);
                while (isalnum(*gSP) || *gSP == '_' || *gSP == '.')
                        *(dp++)  = *(gSP++);
                *dp = 0;
        }
        else if (*gSP == '\"') {
                dp = gToken;
                *(dp++) = *(gSP++);
                while (*gSP && *gSP != '\"') {
                        if (*gSP == '\\') {
                                ++gSP;
                                if (*gSP == '\\' || *gSP == '\"')
                                        *(dp++)  = *(gSP++);
                                else  {
                                        int     t;
                                        sscanf((char *)gSP,"%2X",&t);
                                        *(dp++) = t;
                                        gSP += 2;
                                }
                        }
                        else
                                *(dp++)  = *(gSP++);
                }
                if (*gSP == '\"')
                        *(dp++) = *(gSP++);
                *dp = 0;
        }
        else if (strchr("{}[](),",*gSP)) {
                gToken[0] = *(gSP++);
                gToken[1] = 0;
        }
        else if (ispunct(*gSP)) {
                dp = gToken;
                while (ispunct(*gSP) || *gSP == '_')
                        *(dp++)  = *(gSP++);
                *dp = 0;
        }
        else {
                LogMessage("Script Error\r");
                return FALSE;
        }
        return TRUE;
}

/******************************************************************************************/

Boolean GetPString(StringPtr str)
{
        unsigned char   *dp;
        unsigned char   *len;
reget:
        if (*gSP == 0)
                return FALSE;
        else if (isspace(*gSP)) {
                ++gSP;
                goto reget;
        }
        else if (*gSP == '#' || *gSP == ';') {
                while (*gSP && *gSP != '\r' && *gSP != '\n')
                        ++gSP;
                goto reget;
        }
        else if (isalnum(*gSP) || *gSP == '_')
        {
                len = (unsigned char *)str++;
                dp = (unsigned char     *)str;
                len[0] = 0;
                while (isalnum(*gSP) || *gSP == '_') {
                        *(dp++)  = *(gSP++);
                        len[0]++;
                }
                /* *dp = 0; */
        }
        else if (*gSP == '\"') {
                len = (unsigned char    *)str++;
                dp = (unsigned char     *)str;
                len[0] = 0;
                ++gSP;
                while (*gSP && *gSP != '\"') {
                        if (*gSP == '\\') {
                                ++gSP;
                                if (*gSP == '\\' || *gSP == '\"')
                                        *(dp++)  = *(gSP++);
                                else  {
                                        int     t;
                                        sscanf((char *)gSP,"%2X",&t);
                                        *(dp++) = t;
                                        gSP += 2;
                                }
                        }
                        else
                                *(dp++)  = *(gSP++);
                        len[0]++;
                }
                if (*gSP == '\"')
                        ++gSP;
                /* *dp = 0; */
        }
        else {
                LogMessage("Script Error\r");
                return FALSE;
        }
        return TRUE;
}

Boolean GetCString(char *str)
{
	if (GetPString((StringPtr) str)) {
		PtoCstr((StringPtr) str);
		return true;
	}
	return false;
}

/******************************************************************************************/

void BooleanParse(Boolean *data)
{
        GetToken();
        if (strcmp(gToken,"ON") == 0 || strcmp(gToken,"TRUE") == 0 || atoi(gToken) > 0)
                *data = true;
        else
                *data = false;
}

/******************************************************************************************/

void ShortParse(short *data)
{
        GetToken();
        if (isdigit(gToken[0]) || gToken[0] == '-')
                *data = atoi(gToken);
        else
                *data = 0;
}

/******************************************************************************************/
unsigned LONG StringToHex(char *str)
{
	char	*p;
	unsigned LONG	val = 0;
	for (p = str; *p; ++p) {
		val <<= 4;
		switch (*p) {
		case '0':	val += 0;	break;
		case '1':	val += 1;	break;
		case '2':	val += 2;	break;
		case '3':	val += 3;	break;
		case '4':	val += 4;	break;
		case '5':	val += 5;	break;
		case '6':	val += 6;	break;
		case '7':	val += 7;	break;
		case '8':	val += 8;	break;
		case '9':	val += 9;	break;
		case 'A':
		case 'a':	val += 10;	break;
		case 'B':
		case 'b':	val += 11;	break;
		case 'C':
		case 'c':	val += 12;	break;
		case 'D':
		case 'd':	val += 13;	break;
		case 'E':
		case 'e':	val += 14;	break;
		case 'F':
		case 'f':	val += 15;	break;
		}
	}
	return val;
}

void LongParse(LONG *data)
{
        GetToken();
		if (gToken[0] == '0' && gToken[1] == 'x') {
				*data = (LONG) StringToHex(gToken+2);
		}
		else if (isdigit(gToken[0]) || gToken[0] == '-')
                *data = atol(gToken);
        else
                *data = 0;
}

/******************************************************************************************/

void BeginRoomCreation(void)
{

/** removed 12/11/95
        static char     *fName = "BeginRoomCreation";
        gRoom = (RoomRecPtr) NewPtrClear(sizeof(RoomRec)+32767L);
        if (gRoom == NULL)
                ReportError(memFullErr,fName);
        gHotspots = (HotspotPtr) NewPtrClear(sizeof(Hotspot) * MaxHotspots);
        if (gHotspots == NULL)
                ReportError(memFullErr,fName);
        gDrawCmds = (DrawRecPtr) NewPtrClear(sizeof(DrawRecord) * MaxDrawCmds);
        if (gDrawCmds == NULL)
                ReportError(memFullErr,fName);
        gPictureRecs = (PictureRecPtr) NewPtrClear(sizeof(PictureRec) * MaxPictureRecs);
        if (gPictureRecs == NULL)
                ReportError(memFullErr,fName);
        gSpotStates = (StateRecPtr) NewPtrClear(sizeof(StateRec) * MaxSpotStates);
        if (gSpotStates == NULL)
                ReportError(memFullErr,fName);
        gPoints = (Point *) NewPtrClear(sizeof(Point) * MaxHotPts);
        if (gPoints == NULL)
                ReportError(memFullErr,fName);
        gLProps = (LPropPtr) NewPtrClear(sizeof(LPropRec) * MaxLProps);
        if (gLProps == NULL)
                ReportError(memFullErr,fName);
**/
	memset(gRoom, 0, sizeof(RoomRec)+32767L);
	memset(gHotspots,0,sizeof(Hotspot) * MaxHotspots);
	memset(gDrawCmds, 0, sizeof(DrawRecord) * MaxDrawCmds);
	memset(gPictureRecs, 0, sizeof(PictureRec) * MaxPictureRecs);
	memset(gSpotStates, 0, sizeof(StateRec) * MaxSpotStates);
	memset(gPoints, 0, sizeof(Point) * MaxHotPts);
	memset(gLProps, 0, sizeof(LPropRec) * MaxLProps);
    gRoom->lenVars = 2;		/* 5/8/96 - moved out of commented out source */
}

/******************************************************************************************/

void EndRoomCreation(void)
{
/** 12/11/95
        DisposePtr((Ptr) gPoints);
        DisposePtr((Ptr) gSpotStates);
        DisposePtr((Ptr) gPictureRecs);
        DisposePtr((Ptr) gDrawCmds);
        DisposePtr((Ptr) gHotspots);
        DisposePtr((Ptr) gRoom);
        DisposePtr((Ptr) gLProps);
 **/
}

/******************************************************************************************/
Ptr SuckTextFile(StringPtr fileName);

OSErr ReadScriptFile(StringPtr   scriptName)
{
        /* OSErr   oe; */
        /* short   iFile; */
        /* LONG    length; */

        BeginRoomCreation();

		gScriptBuffer = SuckTextFile((StringPtr) ScriptFileName);
        if (gScriptBuffer == NULL) {
            return 1;
        }

        gToken = NewPtrClear(2048);

        /* Dispose of Old Rooms !!! */
        gNbrRooms = 0;


        gSP = gScriptBuffer;
        
        while (GetToken()) {
                if (strcmp(gToken,"ROOM") == 0)
                        RoomParse();
                else if (strcmp(gToken,"ENTRANCE") == 0)
                        ShortParse(&gEntrance);
				else if (strcmp(gToken,"SERVERNAME") == 0) {
			        GetPString(gPrefs.serverName);
				}
				else if (strcmp(gToken,"WIZARDPASSWORD") == 0) {
			        GetPString(gPrefs.wizardPassword);
				}
				else if (strcmp(gToken,"GODPASSWORD") == 0) {
			        GetPString(gPrefs.godPassword);
				}
				else if (strcmp(gToken,"PICFOLDER") == 0) {
			        GetPString(gPrefs.picFolder);
				}
				else if (strcmp(gToken,"MAXSESSIONID") == 0) {
			       LongParse(&gPrefs.recycleLimit);
				}
				else if (strcmp(gToken,"SERVEROPTIONS") == 0) {
					LongParse(&gPrefs.serverOptions);
				}
				else if (strcmp(gToken,"SAVESESSIONKEYS") == 0) {	
					/* 4/26 OLD provided for backward compat - covered by serveroptions */
#if unix
					gPrefs.serverOptions |= SO_SaveSessionKeys;
#endif
				}
				else if (strcmp(gToken,"PERMISSIONS") == 0) {
					LongParse(&gPrefs.permissions);
				}
				else if (strcmp(gToken,"DEATHPENALTY") == 0) {
					ShortParse(&gPrefs.deathPenaltyMinutes);
				}
				else if (strcmp(gToken,"MAXOCCUPANCY") == 0) {
					ShortParse(&gPrefs.maxOccupancy);
					if (gPrefs.maxOccupancy > gMaxPeoplePerServer)
						gPrefs.maxOccupancy = gMaxPeoplePerServer;
				}
				else if (strcmp(gToken,"ROOMOCCUPANCY") == 0) {
					ShortParse(&gPrefs.roomOccupancy);
					if (gPrefs.roomOccupancy > MaxPeoplePerRoom)
						gPrefs.roomOccupancy = MaxPeoplePerRoom;
				}
				else if (strcmp(gToken,"MINFLOODEVENTS") == 0) {
					ShortParse(&gPrefs.minFloodEvents);
				}
				else if (strcmp(gToken,"PURGEPROPDAYS") == 0) {
					ShortParse(&gPrefs.purgePropDays);
				}
				else if (strcmp(gToken,"SYSOP") == 0) {
					GetPString((StringPtr) gPrefs.sysop);
					PtoCstr((StringPtr) gPrefs.sysop);
				}
				else if (strcmp(gToken,"URL") == 0) {
					GetPString((StringPtr) gPrefs.url);
					PtoCstr((StringPtr) gPrefs.url);
				}
				else if (strcmp(gToken,"MACHINETYPE") == 0) {
					GetPString((StringPtr) gPrefs.machineType);
					PtoCstr((StringPtr) gPrefs.machineType);
				}
				else if (strcmp(gToken,"BLURB") == 0) {
					GetPString((StringPtr) gPrefs.description);
					PtoCstr((StringPtr) gPrefs.description);
				}
				else if (strcmp(gToken,"ANNOUNCEMENT") == 0) {
					GetPString((StringPtr) gPrefs.announcement);
					PtoCstr((StringPtr) gPrefs.announcement);
				}
				else if (strcmp(gToken,"AUTOANNOUNCE") == 0) {
					GetPString((StringPtr) gPrefs.autoAnnounce);
					PtoCstr((StringPtr) gPrefs.autoAnnounce);
				}
				else if (strcmp(gToken,"YPADDR") == 0) {
					GetPString((StringPtr) gPrefs.ypIPAddr);
					PtoCstr((StringPtr) gPrefs.ypIPAddr);
				}
				else if (strcmp(gToken,"AUTOREGISTER") == 0) {
					gPrefs.autoRegister = true;
				}
                else if (strcmp(gToken,"END") == 0)
                        break;
				else if (strcmp(gToken, "BANREC") == 0) {
					ParseBanRec();
				}
                else {
                        LogMessage("Syntax Error: %s\r",gToken);
                        break;
                }
        }
        LogMessage("%d Rooms\r",gNbrRooms);
        DisposePtr(gScriptBuffer);
        DisposePtr((Ptr) gToken);
        EndRoomCreation();
        if (gNbrRooms <= 0) {
			LogMessage("No Rooms in Script File (probably an error) - exiting\r");
        	return -1;
		}
        else
			return noErr;
}

/******************************************************************************************/

void IdxStringParse(short *idx)
{
        GetPString((StringPtr) &gRoom->varBuf[gRoom->lenVars]);
        *idx = gRoom->lenVars;
        gRoom->lenVars += gRoom->varBuf[gRoom->lenVars] + 1;
}


/******************************************************************************************/

void RoomParse(void)
{
	short	maxMembers=0,maxGuests=0;
    memset(gRoom,0,sizeof(RoomRec));
    gRoom->lenVars = 2;
    ++gNbrRooms;
    while (GetToken()) {
            if (strcmp(gToken,"ENDROOM") == 0) {
                    break;
            }
            else if (strcmp(gToken,"ID") == 0) {
                    ShortParse(&gRoom->roomID);
					/* 8/28/96 JAB added DropZone Flag */
                    if (gRoom->roomID == gEntrance)
	                    gRoom->roomFlags |= RF_DropZone;
            }
			else if (strcmp(gToken,"MAXMEMBERS") == 0) {
					ShortParse(&maxMembers);
			}
			else if (strcmp(gToken,"MAXGUESTS") == 0) {
					ShortParse(&maxGuests);
			}
            else if (strcmp(gToken,"FACES") == 0) {
                    LongParse(&gRoom->facesID);
            }
            else if (strcmp(gToken,"NAME") == 0) {
                    IdxStringParse(&gRoom->roomNameOfst);
            }
            else if (strcmp(gToken,"ARTIST") == 0) {
                    IdxStringParse(&gRoom->artistNameOfst);
            }
            else if (strcmp(gToken,"LOCKED") == 0) {
                    gRoom->roomFlags |= RF_AuthorLocked;
                    IdxStringParse(&gRoom->passwordOfst);
            }
            else if (strcmp(gToken,"PRIVATE") == 0) {		/* 6/14/95 */
                    gRoom->roomFlags |= RF_Private;
            }
            else if (strcmp(gToken,"NOPAINTING") == 0) {	/* 7/25/95 */
                    gRoom->roomFlags |= RF_NoPainting;
            }
            else if (strcmp(gToken,"NOCYBORGS") == 0) {	/* 7/25/95 */
                    gRoom->roomFlags |= RF_CyborgFreeZone;
            }
            else if (strcmp(gToken,"HIDDEN") == 0) {	/* 11/7/95 */
                    gRoom->roomFlags |= RF_Hidden;
            }
            else if (strcmp(gToken,"NOGUESTS") == 0) {	/* 11/7/95 */
                    gRoom->roomFlags |= RF_NoGuests;
            }
            else if (strcmp(gToken,"WIZARDSONLY") == 0) {	/* 1/27/96 */
                    gRoom->roomFlags |= RF_WizardsOnly;
            }
            else if (strcmp(gToken,"DROPZONE") == 0) {	/* 8/28/96 */
                    gRoom->roomFlags |= RF_DropZone;
            }
            else if (strcmp(gToken,"PICT") == 0) {
                    IdxStringParse(&gRoom->pictNameOfst);
            }
            else if (strcmp(gToken,"BOLT") == 0) {
                    HotspotParse(HS_Bolt);
            }
            else if (strcmp(gToken,"NAVAREA") == 0) {
                    HotspotParse(HS_NavArea);
            }
            else if (strcmp(gToken,"DOOR") == 0) {
                    HotspotParse(HS_Door);
            }
            else if (strcmp(gToken,"HOTSPOT") == 0 || strcmp(gToken,"SPOT") == 0) {
                    HotspotParse(HS_Normal);
            }
            else if (strcmp(gToken,"PICTURE") == 0) {
                    PictResParse();
            }
            else if (strcmp(gToken,"PROP") == 0) {
                    PropParse();
            }
    }
    AttachCurrentRoom(maxMembers,maxGuests);
}

/******************************************************************************************/

void PointParse(Point *p)
{
        GetToken();
        if (isdigit(gToken[0]) || gToken[0] == '-') {
                p->h = atoi(gToken);
                GetToken();
                if (gToken[0] == ',')
                        GetToken();
                if (isdigit(gToken[0]) || gToken[0] == '-')
                        p->v = atoi(gToken);
    else
      UngetToken();
        }
}

/******************************************************************************************/

void ParseOutline(HotspotPtr hs)
{
        char    xFlag = false;
        short   i;
        Point   p;

        while (GetToken()) {
                if (isdigit(gToken[0])) {
                        if (xFlag) {
                                gPoints[hs->nbrPts].v = atoi(gToken);
                                xFlag = false;
                                ++hs->nbrPts;
                        }
                        else {
                                gPoints[hs->nbrPts].h = atoi(gToken);
                                xFlag = true;
                        }
                }
                else if (gToken[0] != ',') {
                        UngetToken();
                        break;
                }
        }
        if (hs->nbrPts == 0) {
                LogMessage("Invalid Outline Room %d\r",gRoom->roomID);
        }
        else {
                p.h = p.v = 0;
                for (i = 0; i < hs->nbrPts; ++i) {
                        p.h += gPoints[i].h;
                        p.v += gPoints[i].v;
                }
                p.h /= hs->nbrPts;
                p.v /= hs->nbrPts;
                hs->loc = p;
                for (i = 0; i < hs->nbrPts; ++i) {
                        gPoints[i].h -= p.h;
                        gPoints[i].v -= p.v;
                }
        }
}

/******************************************************************************************/

void InitHotspot(HotspotPtr hp)
{
        memset((Ptr) hp, 0, sizeof(Hotspot));
        hp->nbrStates =1;
        gSpotStates[0].pictID = 0;
        gSpotStates[1].pictID = 0;	/* 6/30/95 */
}

/******************************************************************************************/

#if 0	/* 7/24/95 No More Script Parsing on Server Side */
void AddEventHandler(HotspotPtr hp, short type, char *str,...)
{
        EventHandlerPtr ehp;
        char                    *dp;
        va_list args;

        ehp = &gSpotEvents[hp->nbrScripts];
        ehp->eventType = type;
        ehp->scriptTextOfst = gRoom->lenVars;
        dp = &gRoom->varBuf[gRoom->lenVars];

        va_start(args,str);
        vsprintf(dp,str,args);
        va_end(args);

        gRoom->lenVars += strlen(&gRoom->varBuf[gRoom->lenVars])+1;
        ++hp->nbrScripts;
        hp->scriptEventMask |= (1L << ehp->eventType);
}
#endif

/******************************************************************************************/
void ParseSpotScript(HotspotPtr hp)
{
	Ptr	begScript,endScript;
	/* Skip past next CR/LF */
	while (*gSP && isspace(*gSP) && !iscntrl(*gSP))
		++gSP;
	if (*gSP == '\r')
		++gSP;
	if (*gSP == '\n')
		++gSP;
	begScript = gSP;
	while (GetToken())	{
		if (strcmp(gToken,"ENDSCRIPT") == 0)
			break;
	}
	endScript = gSP - 10;
	if (endScript > begScript) {
		while (isspace(*(endScript-1)))
			--endScript;
		if (endScript > begScript) {
			*endScript = 0;
			hp->scriptTextOfst =  gRoom->lenVars;
			strcpy(&gRoom->varBuf[gRoom->lenVars], begScript);
			gRoom->lenVars += strlen(begScript) + 1;
			LONGALIGN(gRoom->lenVars);
		}
	}
}

#if 0	/* 7/24/95 No More Event Handler Parsing... */

void ParseEventHandler(HotspotPtr hp)
{
        short                   type;
        short                   braceCnt;
        char                    *dp;
        EventHandlerPtr ehp;
        if (!GetToken())
                return;
        if (strcmp(gToken,"SELECT") == 0) {     /* 4/6/95 JBUM */
                type = PE_Select;
        }
        else if (strcmp(gToken,"LOCK") == 0) {
                type = PE_Lock;
        }
        else if (strcmp(gToken,"UNLOCK") == 0) {
                type = PE_Unlock;
        }
        else if (strcmp(gToken,"HIDE") == 0) {
                type = PE_Hide;
        }
        else if (strcmp(gToken,"SHOW") == 0) {
                type = PE_Show;
        }
        else if (strcmp(gToken,"STARTUP") == 0) {
                type = PE_Startup;
        }
        else if (strcmp(gToken,"ALARM") == 0) {	/* 6/28 */
                type = PE_Alarm;
        }
        else if (strcmp(gToken,"CUSTOM") == 0) {
                type = PE_Custom;
        }
        else if (strcmp(gToken,"CHAT") == 0) {        /* 6/6/95 JAB */
                type = PE_InChat;
        }
        else if (strcmp(gToken,"INCHAT") == 0) {      /* 6/6/95 JAB */
                type = PE_InChat;
        }
        else if (strcmp(gToken,"PROPCHANGE") == 0) {    /* 4/6/95 JBUM */
                type = PE_PropChange;
        }
        else if (strcmp(gToken,"ENTER") == 0) {         /* 4/6/95 JBUM */
                type = PE_Enter;
        }
        else if (strcmp(gToken,"LEAVE") == 0) {        /* 4/6/95 JBUM */
                type = PE_Leave;
        }
        else if (strcmp(gToken,"OUTCHAT") == 0) {      /* 6/6/95 JAB */
                type = PE_OutChat;
        }
        else {
                LogMessage("Invalid Event Handler: ON %s\r",gToken);
                type = PE_NbrEvents;
        }
        GetToken();     /* Skip { */
        braceCnt = 1;
        ehp = &gSpotEvents[hp->nbrScripts];
        memset((char *) ehp, 0, sizeof(EventHandlerRec));
        ehp->eventType = type;
        ehp->scriptTextOfst = gRoom->lenVars;
        dp = &gRoom->varBuf[gRoom->lenVars];
        while (GetToken()) {
                switch (gToken[0]) {
                case '}':
                        --braceCnt;
                        if (braceCnt == 0)
                                goto DoneParse;
                        else {
                                *(dp++) = '}';
                                *(dp++) = ' ';
                        }
                        break;
                case '{':
                        ++braceCnt;
                        *(dp++) = '{';
                        *(dp++) = ' ';
                        break;
                case '(':
                case ')':
                        break;
                default:
                        strcpy(dp,gToken);
                        dp += strlen(dp);
                        *(dp++) = ' ';
                        break;
                }
                if (*gSP == '\r')               /* 4/6/95 JBUM - trying to preserve CRs */
                        *(dp++) = '\r';         /* in iptscrays (doesn't work) */
        }
DoneParse:
        *dp = 0;
        gRoom->lenVars += strlen(&gRoom->varBuf[gRoom->lenVars])+1;
        ++hp->nbrScripts;
        hp->scriptEventMask |= (1L << ehp->eventType);
}
#endif

/******************************************************************************************/
#if 0	/* 7/24/95 no more script parsing on server side */
void AddDefaultEventHandlers(HotspotPtr  hp)
{
        switch (hp->type) {
        case HS_Door:
                if (hp->dest) {
                        if (!(hp->scriptEventMask & (1L << PE_Select))) /* 4/6/95 JBUM */
                                AddEventHandler(hp,PE_Select,"DEST GOTOROOM");
                }
                break;
        case HS_LockableDoor:
        case HS_ShutableDoor:
                if (hp->dest) {
                        if (!(hp->scriptEventMask & (1L << PE_Select))) /* 4/6/95 JBUM */
                                AddEventHandler(hp,PE_Select,"{ \"Sorry, the Door is locked\" LOCALMSG } {  DEST GOTOROOM } ME ISLOCKED IFELSE");
                }
                break;
        case HS_Bolt:
                if (hp->dest) {
                        if (!(hp->scriptEventMask & (1L << PE_Select))) /* 4/6/95 JBUM */
                                AddEventHandler(hp,PE_Select,"DEST { UNLOCK } { LOCK } DEST ISLOCKED IFELSE");
                }
                break;
        }
}
#endif
/******************************************************************************************/

void PropParse(void)			/* 6/8/95 modified to support CRCs */
{
        LPropPtr        pp;
        pp = &gLProps[gRoom->nbrLProps];
        memset(pp,0,sizeof(LPropRec));
        while (GetToken()) {
                if (strcmp(gToken,"ENDPROP") == 0) {
                        break;
                }
                else if (strcmp(gToken,"PROPID") == 0)
                        LongParse(&pp->propSpec.id);
                else if (strcmp(gToken,"CRC") == 0)			/* 6/8/95 */
                        LongParse((LONG *) &pp->propSpec.crc);	/* 6/8/95 */
                else if (strcmp(gToken,"LOC") == 0)
                        PointParse(&pp->loc);
        }
        ++gRoom->nbrLProps;
}

/******************************************************************************************/

void HotspotParse(short defType)
{
        HotspotPtr      hp;

        hp = &gHotspots[gRoom->nbrHotspots];
        InitHotspot(hp);
        hp->id = gRoom->nbrHotspots+1;
        hp->type = defType;

        while (GetToken()) {
                if (strcmp(gToken,"ENDDOOR") == 0 ||
                        strcmp(gToken,"ENDBOLT") == 0 ||
                        strcmp(gToken,"ENDSPOT") == 0 ||
                        strcmp(gToken,"ENDHOTSPOT") == 0)
                        break;
                else if (strcmp(gToken,"NAME") == 0) {				/* 8/23 */
						IdxStringParse(&hp->nameOfst);
		        		LONGALIGN(gRoom->lenVars);
				}
                else if (strcmp(gToken,"ID") == 0)
                        ShortParse(&hp->id);
                else if (strcmp(gToken,"PICT") == 0 ||
                                 strcmp(gToken,"PICTID") == 0)
                {
                        ShortParse(&gSpotStates[0].pictID);
                        hp->nbrStates = 1;
                }
                else if (strcmp(gToken,"PICTS") == 0 ||
                                 strcmp(gToken,"PICTIDS") == 0)
                {
                        short   n=0;
                        while (GetToken()) {
                                if (strcmp(gToken,"ENDPICTS") == 0 ||
                                        strcmp(gToken,"ENDPICTIDS") == 0)
                                        break;
                                else {
                                        gSpotStates[n].pictID = atoi(gToken);
                                        GetToken();
                                        if (gToken[0] != ',')
                                                UngetToken();
                                        PointParse(&gSpotStates[n].picLoc);
                                        ++n;
                                }
                                hp->nbrStates = n;
                        }
                }
                else if (strcmp(gToken,"DEST") == 0)
                        ShortParse(&hp->dest);
                else if (strcmp(gToken,"DOOR") == 0 && defType == HS_Bolt) {
                        ShortParse(&hp->dest);
                }
                else if (strcmp(gToken,"LOC") == 0)
                        PointParse(&hp->loc);
                else if (strcmp(gToken,"LOCKABLE") == 0) {
                        defType = HS_LockableDoor;
                        hp->type = defType;
                        hp->nbrStates = 2;
                }
                else if (strcmp(gToken,"DONTMOVEHERE") == 0) {
                        hp->flags |= HS_DontMoveHere;
                }
                else if (strcmp(gToken,"DRAGGABLE") == 0) {
                        hp->flags |= HS_Draggable;
                }
                else if (strcmp(gToken,"SHOWNAME") == 0) {
                        hp->flags |= HS_ShowName;
                }
                else if (strcmp(gToken,"INVISIBLE") == 0) {
                        hp->flags |= HS_Invisible;
                }
                else if (strcmp(gToken,"SHOWFRAME") == 0) {
                        hp->flags |= HS_ShowFrame;
                }
                else if (strcmp(gToken,"SHADOW") == 0) {
                        hp->flags |= HS_Shadow;
                }
                else if (strcmp(gToken,"FILL") == 0) {
                        hp->flags |= HS_Fill;
                }
                else if (strcmp(gToken,"SHUTABLE") == 0) {
                        defType = HS_ShutableDoor;
                        hp->type = defType;
                        hp->nbrStates = 2;
                }
                else if (strcmp(gToken,"OUTLINE") == 0)
                        ParseOutline(hp);
                else if (strcmp(gToken,"SCRIPT") == 0)
                		ParseSpotScript(hp);
        }
        if (hp->nbrStates) {
        		LONGALIGN(gRoom->lenVars);
                hp->stateRecOfst = AddRoomBuffer(gSpotStates,hp->nbrStates*sizeof(StateRec));
        }
        if (hp->nbrPts) {
        		LONGALIGN(gRoom->lenVars);
                hp->ptsOfst = AddRoomBuffer(gPoints,hp->nbrPts*sizeof(Point));
        }
        ++gRoom->nbrHotspots;
}

/******************************************************************************************/

void PictResParse(void)
{
        PictureRecPtr   pp;
        pp = &gPictureRecs[gRoom->nbrPictures];
        memset((char *) pp, 0, sizeof(PictureRec));
        pp->transColor = -1;	/* 8/29/95 */
        while (GetToken()) {
                if (strcmp(gToken,"ENDPICTURE") == 0)
                        break;
                else if (strcmp(gToken,"ID") == 0)
                        ShortParse(&pp->picID);
                else if (strcmp(gToken,"NAME") == 0)
                        IdxStringParse(&pp->picNameOfst);
                else if (strcmp(gToken,"TRANSCOLOR") == 0)
                        ShortParse(&pp->transColor);
                else
                        LogMessage("Invalid Picture Statement in Room %d\r",gRoom->roomID);
        }
        if (pp->picID == 0)
                pp->picID = gRoom->nbrPictures+1;
        ++gRoom->nbrPictures;
}

/******************************************************************************************/

void AddScriptLine(char *str,...)
{
        va_list args;
        va_start(args,str);
        vsprintf((char *)gSP,str,args);
        va_end(args);
#if unix
		if (strlen(gSP) && gSP[strlen(gSP)-1] == '\r')
			gSP[strlen(gSP)-1] = '\n';
#endif
        gSP += strlen((char *)gSP);
}

/******************************************************************************************/

void AddScriptPString(StringPtr str)
{
        short   i;
        *(gSP++) = '\"';
        i = str[0];
        ++str;
        while (i--) {
                if (*str >= ' ' && *str <= '~' && *str != '\\' && *str != '\"') {
                        *(gSP++) = *(str++);
                }
                else {
                        *(gSP++) = '\\';
                        if (*str == '\\' || *str == '\"')
                                *(gSP++) = *(str++);
                        else {
                                sprintf((char *)gSP,"%02.2X",*((unsigned char *) str));
                                gSP += 2;
                                ++str;
                        }
                }
        }
        *(gSP++) = '\"';
}

void AddScriptCString(char *str)
{
        short   i;
        *(gSP++) = '\"';
        i = strlen(str);
        while (i--) {
                if (*str >= ' ' && *str <= '~' && *str != '\\' && *str != '\"') {
                        *(gSP++) = *(str++);
                }
                else {
                        *(gSP++) = '\\';
                        if (*str == '\\' || *str == '\"')
                                *(gSP++) = *(str++);
                        else {
                                sprintf((char *)gSP,"%02.2X",*((unsigned char *) str));
                                gSP += 2;
                                ++str;
                        }
                }
        }
        *(gSP++) = '\"';
}

/******************************************************************************************/

void SaveScript(void)
{
        ServerRoomPtr  			roomPtr;
        LONG                    lenScript;
        OSErr                   oe;
        short                   refNum;
        RoomRecPtr              room;
        StringPtr               str;
        PictureRecPtr   prl;
        HotspotPtr              hsl;
        LPropPtr                lpl;
        short                   i,j;

        gScript = (HugePtr) gBigBuffer; /* 12/11/95 was calling NewPtr */
        if (gScript == NULL) {
                ReportError(memFullErr,"Save Script");
                return;
        }
        gSP = gScript;

		/* New: save some server prefs here (eventually we will put all of them here...)
		 */
		AddScriptLine("; Server Prefs\r");
        AddScriptLine(";\r");
		if (gPrefs.serverName[0]) {
			AddScriptLine("SERVERNAME ");
			AddScriptPString(gPrefs.serverName);
			AddScriptLine("\r");
		}
		if (gPrefs.wizardPassword[0]) {
			AddScriptLine("WIZARDPASSWORD ");
			AddScriptPString(gPrefs.wizardPassword);
			AddScriptLine("\r");
		}
		if (gPrefs.godPassword[0]) {
			AddScriptLine("GODPASSWORD ");
			AddScriptPString(gPrefs.godPassword);
			AddScriptLine("\r");
		}
		AddScriptLine("PERMISSIONS 0x%-8lX\r",(long) gPrefs.permissions);

		if (gPrefs.deathPenaltyMinutes)
			AddScriptLine("DEATHPENALTY %d\r",gPrefs.deathPenaltyMinutes);
		if (gPrefs.maxOccupancy)
			AddScriptLine("MAXOCCUPANCY %d\r",gPrefs.maxOccupancy);
		if (gPrefs.roomOccupancy)
			AddScriptLine("ROOMOCCUPANCY %d\r",gPrefs.roomOccupancy);
		if (gPrefs.minFloodEvents)
			AddScriptLine("MINFLOODEVENTS %d\r",gPrefs.minFloodEvents);
		if (gPrefs.purgePropDays)
			AddScriptLine("PURGEPROPDAYS %d\r",gPrefs.purgePropDays);
		if (gPrefs.recycleLimit)
			AddScriptLine("MAXSESSIONID %ld\r",(long) gPrefs.recycleLimit);
		if (gPrefs.serverOptions)
			AddScriptLine("SERVEROPTIONS 0x%-8lX\r",(long) gPrefs.serverOptions);
		if (gPrefs.picFolder[0]) {
			AddScriptLine("PICFOLDER ");
			AddScriptPString(gPrefs.picFolder);
			AddScriptLine("\r");
		}
/* Yellow Pages Stuff */
		/* 10/10/96 JAB - fixed to deal with embedded quotes in strings properly */
		if (gPrefs.sysop[0]) {
			AddScriptLine("SYSOP ");
			AddScriptCString(gPrefs.sysop);
			AddScriptLine("\r");
		}
		if (gPrefs.url[0]) {
			AddScriptLine("URL ");
			AddScriptCString(gPrefs.url);
			AddScriptLine("\r");
		}
		if (gPrefs.machineType[0]) {
			AddScriptLine("MACHINETYPE ");
			AddScriptCString(gPrefs.machineType);
			AddScriptLine("\r");
		}
		if (gPrefs.description[0]) {
			AddScriptLine("BLURB ");
			AddScriptCString(gPrefs.description);
			AddScriptLine("\r");
		}
		if (gPrefs.announcement[0]) {
			AddScriptLine("ANNOUNCEMENT ");
			AddScriptCString(gPrefs.announcement);
			AddScriptLine("\r");
		}
		if (gPrefs.ypIPAddr[0]) {
			AddScriptLine("YPADDR ");
			AddScriptCString(gPrefs.ypIPAddr);
			AddScriptLine("\r");
		}
		if (gPrefs.autoRegister)
			AddScriptLine("AUTOREGISTER\r");

/* 6/21/96 Automatic announcement string */
		/* 10/10/96 JAB - fixed to deal with embedded quotes in strings properly */
		if (gPrefs.autoAnnounce[0]) {
			AddScriptLine("AUTOANNOUNCE ");
			AddScriptCString(gPrefs.autoAnnounce);
			AddScriptLine("\r");
		}
		SaveBanRecs();

        AddScriptLine(";\r");
        AddScriptLine("; Mansion Layout\r");
        AddScriptLine(";\r");
        if (gEntrance)
                AddScriptLine("ENTRANCE %d\r",gEntrance);
        for (roomPtr = gRoomList; roomPtr; roomPtr = roomPtr->nextRoom)
        {
				if (roomPtr->memberOwner)	/* 5/8/95 member owned rooms are not saved */
					continue;

                room = &roomPtr->room;


                AddScriptLine("\r");
                AddScriptLine("\r");
                AddScriptLine("ROOM\r");
                AddScriptLine("\tID %d\r",room->roomID);
                if ((room->roomFlags & RF_AuthorLocked) > 0 && room->passwordOfst) {
                        str = (StringPtr) &room->varBuf[room->passwordOfst];
                        AddScriptLine("\tLOCKED ");
                        AddScriptPString(str);
                        AddScriptLine("\r");
                }
                if ((room->roomFlags & RF_DropZone) > 0)	/* 1/27/96 */
                		AddScriptLine("\tDROPZONE\r");
                if ((room->roomFlags & RF_Private) > 0)	/* 6/14/95 */
                		AddScriptLine("\tPRIVATE\r");
                if ((room->roomFlags & RF_NoPainting) > 0)	/* 7/25/95 */
                		AddScriptLine("\tNOPAINTING\r");
                if ((room->roomFlags & RF_CyborgFreeZone) > 0)	/* 7/25/95 */
                		AddScriptLine("\tNOCYBORGS\r");
                if ((room->roomFlags & RF_Hidden) > 0)	/* 11/7/95 */
                		AddScriptLine("\tHIDDEN\r");
                if ((room->roomFlags & RF_NoGuests) > 0)	/* 11/7/95 */
                		AddScriptLine("\tNOGUESTS\r");
                if ((room->roomFlags & RF_WizardsOnly) > 0)	/* 1/27/96 */
                		AddScriptLine("\tWIZARDSONLY\r");
				
				if (roomPtr->maxOccupancy)	/* 1/27/96 */
						AddScriptLine("\tMAXMEMBERS %d\r",(int) roomPtr->maxOccupancy);
				if (roomPtr->maxGuests)	    /* 1/27/96 */
						AddScriptLine("\tMAXGUESTS %d\r",(int) roomPtr->maxGuests);

				if (room->roomNameOfst) {
	                str = (StringPtr) &room->varBuf[room->roomNameOfst];
	                AddScriptLine("\tNAME ");
	                AddScriptPString(str);
	                AddScriptLine("\r");
				}
				if (room->pictNameOfst) {
	                str = (StringPtr) &room->varBuf[room->pictNameOfst];
	                AddScriptLine("\tPICT ");
	                AddScriptPString(str);
	                AddScriptLine("\r");
				}
                if (room->artistNameOfst) {
                        str = (StringPtr) &room->varBuf[room->artistNameOfst];
                        AddScriptLine("\tARTIST ");
                        AddScriptPString(str);
                        AddScriptLine("\r");
                }
                if (room->facesID)
                        AddScriptLine("\tFACES %ld\r",room->facesID);
                if (room->nbrPictures) {
                        prl = (PictureRecPtr) &room->varBuf[room->pictureOfst];
                        for (i = 0; i < room->nbrPictures; ++i,++prl) {
                                str = (StringPtr) &room->varBuf[prl->picNameOfst];
                                AddScriptLine("\tPICTURE ID %d",prl->picID);
                                AddScriptLine(" NAME ");
                                AddScriptPString(str);
								if (prl->transColor >= 0)
	                                AddScriptLine(" TRANSCOLOR %d",
	                                                prl->transColor);
								AddScriptLine(" ENDPICTURE\r");						
		               }
                }
                if (room->nbrLProps) {
                        lpl = (LPropPtr) &room->varBuf[room->firstLProp];
                        for (i = 0; i < room->nbrLProps; ++i) {
                        		if (lpl->propSpec.crc)			/* 6/8/95 */
#if unix
	                                AddScriptLine("\tPROP PROPID 0x%-8X CRC 0x%-8X LOC %-3d,%-3d ENDPROP\r",
	                                				lpl->propSpec.id,lpl->propSpec.crc,lpl->loc.h,lpl->loc.v);
#else
	                                AddScriptLine("\tPROP PROPID 0x%-8lX CRC 0x%-8lX LOC %-3d,%-3d ENDPROP\r",
	                                				lpl->propSpec.id,lpl->propSpec.crc,lpl->loc.h,lpl->loc.v);
#endif
                        		else
#if unix
	                                AddScriptLine("\tPROP PROPID %-6d LOC %-3d,%-3d ENDPROP\r",
	                                				lpl->propSpec.id,lpl->loc.h,lpl->loc.v);
#else
	                                AddScriptLine("\tPROP PROPID %-6ld LOC %-3d,%-3d ENDPROP\r",
	                                				lpl->propSpec.id,lpl->loc.h,lpl->loc.v);
#endif
								if (lpl->link.nextOfst)
									lpl = (LPropPtr) &room->varBuf[lpl->link.nextOfst];
								else
									break;
                        }
                }
                if (room->nbrHotspots) {
                        hsl = (HotspotPtr) &room->varBuf[room->hotspotOfst];
                        for (i = 0; i < room->nbrHotspots; ++i,++hsl) {
                                switch (hsl->type) {
                                case HS_Door:   
                                        AddScriptLine("\tDOOR\r");      
                                        break;
                                case HS_ShutableDoor:
                                        AddScriptLine("\tDOOR\r");      
                                        AddScriptLine("\t\tSHUTABLE\r");
                                        break;
                                case HS_LockableDoor:
                                        AddScriptLine("\tDOOR\r");      
                                        AddScriptLine("\t\tLOCKABLE\r");
                                        break;
                                case HS_Bolt:   
                                        AddScriptLine("\tBOLT\r");      
                                        break;
                                default:                
                                        AddScriptLine("\tSPOT\r");      
                                        break;
                                }
                                if (hsl->id)
                                        AddScriptLine("\t\tID %d\r",hsl->id);
								if (hsl->nameOfst) {
					                str = (StringPtr) &room->varBuf[hsl->nameOfst];
					                AddScriptLine("\t\tNAME ");
					                AddScriptPString(str);
    						        AddScriptLine("\r");
								}
                                if (hsl->flags & HS_Draggable)
                                        AddScriptLine("\t\tDRAGGABLE\r");
                                if (hsl->flags & HS_DontMoveHere)
                                        AddScriptLine("\t\tDONTMOVEHERE\r");
								if (hsl->flags & HS_Invisible)
										AddScriptLine("\t\tINVISIBLE\r");
								if (hsl->flags & HS_ShowName)
										AddScriptLine("\t\tSHOWNAME\r");
								if (hsl->flags & HS_ShowFrame)
										AddScriptLine("\t\tSHOWFRAME\r");
								if (hsl->flags & HS_Shadow)
										AddScriptLine("\t\tSHADOW\r");
								if (hsl->flags & HS_Fill)
										AddScriptLine("\t\tFILL\r");
                                if (hsl->dest)
                                        AddScriptLine("\t\tDEST %d\r",hsl->dest);
                                if (hsl->nbrPts) {
                                        Point   *pts;
                                        AddScriptLine("\t\tOUTLINE ");
                                        pts = (Point *) &room->varBuf[hsl->ptsOfst];
                                        for (j = 0; j < hsl->nbrPts; ++j,++pts)
                                                AddScriptLine("%d,%d%s",pts->h+hsl->loc.h,pts->v+hsl->loc.v,
                                                        j < hsl->nbrPts-1? "  ":"\r");
                                }
                                if (hsl->nbrStates && hsl->stateRecOfst) {
                                        StateRecPtr     sr;
                                        Boolean         nonZero = false;
                                        sr = (StateRecPtr) &room->varBuf[hsl->stateRecOfst];
                                        for (j = 0; j < hsl->nbrStates; ++j,++sr) {
                                                if (sr->pictID) {
                                                        nonZero = true;
                                                        break;
                                                }
                                        }
                                        if (nonZero) {
                                                sr = (StateRecPtr) &room->varBuf[hsl->stateRecOfst];
                                                AddScriptLine("\t\tPICTS ");
                                                for (j = 0; j < hsl->nbrStates; ++j,++sr) {
                                                        AddScriptLine("%d,%d,%d ",sr->pictID,sr->picLoc.h,sr->picLoc.v);
                                                }
                                                AddScriptLine("ENDPICTS\r");
                                        }
                                }
                                if (hsl->scriptTextOfst) {
									AddScriptLine("\t\tSCRIPT\r");
									AddScriptLine("%s",&room->varBuf[hsl->scriptTextOfst]);
									AddScriptLine("\r");
									AddScriptLine("\t\tENDSCRIPT\r");
                                }
                                switch (hsl->type) {
                                case HS_Door:   AddScriptLine("\t\tENDDOOR\r"); break;
                                case HS_Bolt:   AddScriptLine("\t\tENDBOLT\r"); break;
                                default:                AddScriptLine("\t\tENDSPOT\r"); break;
                                }
                        }
                }

                AddScriptLine("\tENDROOM\r");
        }       
        AddScriptLine("\rEND\r");

        lenScript = (long) gSP - (long) gScript;
        oe = FSDelete(ScriptFileName,0);
        oe = Create(ScriptFileName,0,ScriptCreator,ScriptFileType);
        if (oe != noErr)
                return;
        oe = FSOpen(ScriptFileName,0,&refNum);
        if (oe != noErr)
                return;
        FSWrite(refNum,&lenScript,(char *)gScript);
        FSClose(refNum);
        FlushVol(NULL,0);
        /* DisposePtr(gScript); */
}

/******************************************************************************************/
