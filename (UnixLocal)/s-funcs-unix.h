/******************************************************************************************/
/* S-Rooms.c */
ServerRoomPtr	GetRoom               (short id);
void DeleteRoom(short roomNumber);
HotspotPtr    GetHotspot            (ServerRoomPtr room, short id);
void          SendRoomToUser        (ServerRoomPtr cRoom, ServerUserPtr cUser);
void          AddUserToRoom         (ServerRoomPtr cRoom, ServerUserPtr cUser, Point where);
void 		EnterMansion(LogonInfoPtr lPtr, ServerUserPtr	user);
void          LockDoor              (ServerUserPtr cUser, short *lockRec, Boolean lockFlag);
void          UpdatePictureLocation (ServerUserPtr cUser, short *uRec);
void          UpdateSpotLocation    (ServerUserPtr cUser, short *uRec);
void          UpdateSpotState       (ServerUserPtr cUser, short *uRec);
void          RemovePersonFromRoom  (ServerRoomPtr cRoom, PersonID userID);
void GenerateListOfAllUsers(ServerUserPtr cUser);
void GenerateListOfAllRooms(ServerUserPtr cUser);
void SendServerInfo(ServerUserPtr user);
void SendServerInfoToAll(void);
short CountRoomGuests(ServerRoomPtr cRoom);

/* Security Functions */
Boolean UserSecurityCheck(LogonInfoPtr lPtr, ServerUserPtr	user);
void UpdateUserDatabase(ServerUserPtr user, short whyKilled);

/******************************************************************************************/
/* s-Author.c */
short         AddRoomBuffer         (void *buf, LONG len);
short         AddRoomPString        (StringPtr name);
ServerRoomPtr EditRoomToNewRoom     (void);
void          AttachCurrentRoom     (short maxMembers, short maxGuests); /* 1/27/95 */
void          MakeNewRoom           (ServerUserPtr cUser);
void          CopyHotspot           (RoomRecPtr	rp, HotspotPtr hhS);
void          CopyRoom              (RoomRecPtr	rp);
void		  DuplicateRoom(ServerUserPtr cUser);

void          ReplaceRoom           (ServerRoomPtr oldRoom, ServerRoomPtr newRoom, Boolean notify);
void          SetRoomInfo           (ServerUserPtr cUser, RoomRecPtr room);
void          MakeNewSpot           (ServerUserPtr cUser);
void          DeleteSpot            (ServerUserPtr cUser, short spotID);
void          AddDrawCommand        (ServerUserPtr cUser, DrawRecPtr drawCmd);
void          AddNewProp            (ServerUserPtr cUser, LONG *cmd);
void          MoveLooseProp         (ServerUserPtr cUser, LONG *cmd);
void          DeleteLooseProp       (ServerUserPtr cUser, LONG *cmd);
short         AddRoomCtoPString(char *name);

/******************************************************************************************/
/* s-Users.c */
LONG	UserRank(ServerUserPtr cUser);
ServerUserPtr NewUser               (void /* PersonID userID,StringPtr name */);
void 		RecycleSessionIDs(void);
void          LogoffUser            (PersonID	userID, short why);
ServerUserPtr	GetServerUser         (LONG id);
void          ChangeUserFace        (ServerUserPtr cUser, short faceNbr);
void          ChangeUserProp        (ServerUserPtr cUser, LONG *propNbr);
void          ChangeUserColor       (ServerUserPtr cUser, short colorNbr);
void          ChangeUserName        (ServerUserPtr cUser, StringPtr uName);
void          ChangeUserRoom        (ServerUserPtr cUser, short newRoom);
void          UpdateUserPosition    (ServerUserPtr cUser, Point *newpos);
void LogoffAllUsers(void);
void ChangeUserDesc(ServerUserPtr	rUser, Ptr p);
void ServerTextToUser(ServerUserPtr cUser, char *str);
ServerUserPtr	GetServerUserByName(StringPtr name);
Boolean IsLegalName(ServerUserPtr	rUser, StringPtr uName);

/******************************************************************************************/
/* s-Assets.c */
void          InitServerAssets      (void);
void          CloseServerAssets     (void);
void          CheckForProp          (ServerUserPtr cUser, LONG propID, unsigned LONG crc);
void          AddAssetTrans         (LONG type, LONG oldID, LONG newID);
LONG          GetAssetTrans         (LONG type, LONG oldID);
void          DisposeAssetTrans     (LONG type, LONG oldID, LONG newID);
void          ReceiveAsset          (ServerUserPtr cUser, Ptr assetMsg);
void          UserAssetQuery        (ServerUserPtr cUser, LONG *assetMsg);

/******************************************************************************************/
/* s-Script.c */
void          UngetToken            (void);
Boolean       GetToken              (void);
Boolean       GetCString            (char *str);
Boolean       GetPString            (StringPtr str);
void          BooleanParse          (Boolean *data);
void          ShortParse            (short *data);
void          LongParse             (LONG *data);
void          BeginRoomCreation     (void);
void          EndRoomCreation       (void);
OSErr          ReadScriptFile        (StringPtr	scriptName);
void          IdxStringParse        (short *idx);
void          PictNameParse         (short *idx);
void          RoomParse             (void);
void          PointParse            (Point *p);
void          FavePropsParse        (void);
void          PropParse             (void);
void          ParseOutline          (HotspotPtr hs);
void          InitHotspot           (HotspotPtr hp);
void          AddEventHandler       (HotspotPtr hp, short type, char *str,...);
void          ParseEventHandler     (HotspotPtr hp);
void          AddDefaultEventHandlers(HotspotPtr hp);
void          HotspotParse          (short defType);
void          PictResParse          (void);
void          AddScriptLine         (char *str,...);
void          SaveScript            (void);
unsigned LONG StringToHex(char *str);
void          AddScriptPString(StringPtr str);
void          AddScriptCString(char *str);
void          ParseSpotScript(HotspotPtr hp);
Boolean 	  AllocateServerBuffers(void);
void 			DisposeServerBuffers();

/******************************************************************************************/
/* s-Events.c */
void ServerIdle           (void);
void ProcessMacro         (ServerUserPtr cUser, PersonID targetID, char *str);
void ProcessMansionEvent  (ServerUserPtr cUser, LONG cmd, LONG msgRefCon, char *buffer, LONG len);
void PostRoomEvent        (short roomID,unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength);
void PostNeighborEvent    (ServerUserPtr cUser,unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength);
void PostGlobalEvent      (unsigned LONG eventType,unsigned LONG refNum,Ptr bufferContents,LONG bufferLength);
void PostGlobalEventShort (unsigned LONG eventType);
void ProcessSuperUser(ServerUserPtr	cUser, StringPtr str);
void KillUser(ServerUserPtr cUser, PersonID targetID);
void FloodControl(ServerUserPtr cUser);
void ScheduleUserKill(ServerUserPtr cUser, short why, unsigned LONG penalty);
void BeginSendGroup(ServerUserPtr cUser);
void EndSendGroup(ServerUserPtr cUser);
void BeginSendGroup(ServerUserPtr cUser);
Boolean UserBadAssert(ServerUserPtr cUser, Boolean condition);
void ProcessBlowthrough(ServerUserPtr	cUser, LONG refCon, char *buffer, LONG length);

/******************************************************************************************/
/* s-Utils.h */
LONG    GetSRand      (void);
void    MySRand       (LONG s);
void    Randomize     (void);
LONG    LongRandom    (void);
double  DoubleRandom  (void);
short   MyRandom      (short max);
void    EncryptString (StringPtr inStr, StringPtr outStr);
void    DecryptString (StringPtr inStr, StringPtr outStr);
void    EncryptCString(StringPtr inStr, StringPtr outStr, int len);
void    DecryptCString(StringPtr inStr, StringPtr outStr, int len);
Boolean EqualPString  (StringPtr inStr, StringPtr outStr, Boolean caseSens);
int     stricmp       (const char *str1,const char *str2);
char 	*CvtToCString(StringPtr str);
unsigned LONG SwapLong(unsigned LONG *n);
unsigned short SwapShort(unsigned short *n);
Boolean LittleEndian();

/* S-File.c */
void ProcessFileSend(ServerUserPtr cUser, StringPtr fName);
void NewFileSend(LONG userID, StringPtr fName, short fRefNum);
void FileSendIdle(void);

/* s-gmacro.c */
Boolean GodMacro(ServerUserPtr cUser, char *buffer, Boolean encrypted);
void ProcessGodCommand(ServerUserPtr cUser, char *p);
char *GetGodToken(char *tok, char *p);
void UserMessage(ServerUserPtr cUser, char *tmp,...);
void UserPrivateMessage(ServerUserPtr cUser, char *tmp,...);
void RoomMessage(RoomID roomID, char *tmp,...);
void WizRoomMessage(RoomID roomID, char *tmp,...);
void WizGlobalMessage(char *tmp,...);
void GodGlobalMessage(char *tmp,...);
void YellowPagesIdle(void);

/* s-banrec.c */
void SaveBanRecs(void);
void ParseBanRec(void);

/* s-actions.c */
UserActionPtr AddUserActionRecord(ServerUserPtr cUser, ServerUserPtr tUser);
UserActionPtr FindUserActionRecord(ServerUserPtr cUser, ServerUserPtr tUser);
void SetUserAction(ServerUserPtr cUser, char *arg, LONG action, Boolean setFlag);
Boolean ActionExists(ServerUserPtr cUser, ServerUserPtr tUser, LONG action);
void DeleteUserActionRecord(ServerUserPtr cUser, ServerUserPtr tUser);
void DeleteAllUserActionRecords(ServerUserPtr cUser);

#if TAP
void TAPOn(ServerUserPtr cUser, char *arg);
void TAPOff(ServerUserPtr cUser, char *arg);
void TAPMessage(ServerUserPtr cUser, ServerUserPtr tUser, char *buffer,
Boolean encrypted);
#endif

/******************************************************************************************/
/* Platform specific */

#if macintosh
#include "MacServ.h"
#else
#if unix
#include "unixserv.h"
#else
#include "WinServ.h"
#endif
#endif



/******************************************************************************************/


