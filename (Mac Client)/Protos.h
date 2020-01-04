// protos.h - local header file for after U-USER.H
typedef struct {
	TEWindowRec			win;
	short				logFile;
	RGBColor			curColor;	// 6/21/95
	short				inMiddle;
} LogWindowRec, *LogWindowPtr;

#ifndef THINK_C
#define OK		1
#define Cancel	2
#endif

extern EventRecord	gTheEvent;

extern WindowPtr		gPEWin,gDrawWin,gPropWin,gULWin,gRLWin,gFPWin,gABWin;	// 6/9/95
extern WindowPtr		gIRCWindow;


extern RoomWindowRec	*gRoomWin;
extern LogWindowPtr		gLogWin;
extern IconSetup		*gIconSetup;
extern UserPrefs		gPrefs;
extern MacroRec			gMacros;

extern long 	gTimeSlice;
extern Rect		gOffscreenRect,gVideoRoomRect;
extern Point	gCenterP;
extern Boolean 	gFullScreen,gSuspended,gLogMessages,gRecordCoords,gDragDoors,
				gDebugFlag,gEnteringRoom,g12InchMode,gShowNames,gExpert,
				gIconized, gSublaunch, gShowFrames;
extern short	gMode;
extern short	gConnectionType;
extern short	gCurrentDrawTool;
extern Boolean	gFillShape;
extern RGBColor	gForeColor,gBackColor;

extern RGBColor	gBlackColor;
extern RGBColor	gWhiteColor;
extern RGBColor	gRedColor,gCyanColor,gPurpleColor;
extern RGBColor	gGrayColor;
extern RGBColor	gGrayCC;
extern RGBColor	gGrayAA;
extern RGBColor	gGray66;
extern RGBColor	gGray44;
extern RGBColor	gGray22;

extern short		gCurPenSize;
extern short		gCurLayer;	// 0 = background, 1=foreground
extern short		gCurPattern;
extern AssetSpec	gSelectProp;
extern CursHandle	gCursHandles[NbrCursors];

extern CTabHandle	gCurCLUT;

//
// FUNCTION PROTOTYPES by File
//

// BalloonText.c
void InitBalloons(void);
RgnHandle	ExcitedRegion(Rect *r);
void DrawBalloons(Rect *rr);
void ChangeBalloonFont(short fontID, short fontSize);
void KillPermBalloons(PersonID userID);
void OffscreenTextEnable(void);
void OffscreenTextRestore(void);
void GetBalloonRect(Rect *br, Ptr text, long len, Boolean privateFlag);

// DrawWindow.c
void NewDrawWindow(void);
void DrawWindowDraw(WindowPtr theWin);
void DrawWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void DrawAdjustCursor(WindowPtr theWin, Point p, EventRecord *er);
void DisposeDrawWin(WindowPtr theWin);
void GetPalPixel(short x, short y, RGBColor *rgb);
void GetPalSelect(Rect *r);
void InitDrawTools(void);
void ToggleDrawPalette(void);
void UpdatePaletteGraphics(void);
void RefreshDrawPalette(void);
void StartDrawPalette(void);
void ClearDrawPalette(void);
Boolean EqualDelta(Point p1, Point p2);
void BeginPaintClipping(void);
void EndPaintClipping(void);
void LineTool(Point p, Boolean shiftFlag);
void ShapeTool(Point p, Boolean shiftFlag, short shapeType);
Boolean PencilTool(Point p, Boolean shiftFlag);
void UseDetonateTool(short type);
void ChooseDrawColor(RGBColor *rgb, short background);
void RefreshDrawObjects(Rect *rr, short layer);
Boolean DrawInRoom(Point p, EventRecord *theEvent);

// FacePicker.c
void NewFPWindow(void);
void FPWindowDraw(WindowPtr theWin);
void FPWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposeFPWin(WindowPtr theWin);
void ToggleFacePicker(void);

// HotRegions.c
void CollapsePicture(PictureRecPtr ps);
void ExpandPicture(PictureRecPtr ps);
Boolean DragDoors(Point p);
void CollapseHotspot(Hotspot *hs);
void ExpandHotspot(Hotspot *hs);
StringPtr BuildPictureName(StringPtr pictureName);

// LaunchApp.c
void LaunchApp(char *aName);
void GotoURL(char *url);
void NetscapeConfigureCheck(void);
OSErr ConfigureNetscape(void);

// LogWindow.c
void NewLogWindow(void);
void DisposeLogWin(WindowPtr theWin);
void ToggleLogFile(void);
void CloseLogWin(WindowPtr theWin);
void LogMessage(char *str,...);
void LogString(char *str);
void SetLogColor(RGBColor *color);

// MacLocalUtils.c
void ErrorMessage(char *str,...);
void ReportError(short code, char *name);
void ErrorExit(char *str,...);
int MyFGets(short refNum, char *buf);
void ReportError(short code, char *name);
Boolean MembersOnly(Boolean flag);
void ReportMessage(short strCode);
void ReportVerboseMessage(short strCode);
OSErr OpenFileReadOnly(StringPtr name, short volNum, FileHandle *refNum);

// MAppleTalk.c
void InitAppleTalkBuffers(void);
void ProcessAppleTalkEvent(EventRecord *er);
void SignOnAppleTalk(void);
void PostUserAppleTalkEvent(unsigned long eventType, unsigned long refNum,
					Ptr bufferContents, long bufferLength);


// NetworkMain.c
void DisconnectUser(void);
Boolean ConnectUser(long sysStuff,short type);
void PostServerEvent(unsigned long eventType, unsigned long refNum,
					Ptr bufferContents, long bufferLength);
void SetConnectionType(short mode, Boolean verbose);

// Preferences.c
void LoadPreferences(void);
OSErr CreatePrefsFile(void);
void StorePreferences(void);
short ComputeMacroNumber(short seed, short modifiers);
void RecordMacro(short n);
void PlayMacro(short n);
void PreferencesDialog(void);
short PasswordDialog(StringPtr str, StringPtr prompt);
void ChildLockDialog(void);
Boolean ChildLock(void);
void RestoreWindowPos(WindowPtr theWin, Point *p);
void SaveWindowPos(WindowPtr theWin, Point *p);
void RestoreWindowSize(WindowPtr theWin, Point *p);
void SaveWindowSize(WindowPtr theWin, Point *p);

// PropEditor.c
void NewPEWindow(AssetSpec *propSpec);
void PEWindowDraw(WindowPtr theWin);
void PEWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void PEAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposePEWin(WindowPtr theWin);
void GetPEPalPixel(short x, short y, RGBColor *rgb);
void GetPEPalSelect(Rect *r);
void DrawMansionPropMask(Handle	propH, PixMapHandle pMap);
void LoadPropIntoEditor(AssetSpec *propSpec);
void RefreshPropToolPalette(void);
void RefreshPropRect(Rect *r);
void RefreshPropFrame(void);
Boolean PointToPropPoint(Point p, Point *tp);
void PropPenMoveTo(Point tp,Boolean cmdFlag);
void PropPenLineTo(Point tp,Boolean cmdFlag);
void PropPenDone(void);
void PropPencilTool(Point p, Boolean shiftFlag, Boolean cmdFlag);
void MovePropFrame(short hd, short vd);
Boolean PEProcessKey(EventRecord *theEvent);
Handle ConvertPixelsToProp(PixMapHandle pMap, PixMapHandle mMap);
void SaveProp(void);
void PasteLarge(void);

// PropPickers.c
void NewPropWindow(void);
void PropWindowDraw(WindowPtr theWin);
void PropWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposePropWin(WindowPtr theWin);
void StartPropPicker(void);
void TogglePropPicker(void);
void RefreshPropPicker(void);
void ClearPropPicker(void);

// RoomGraphics.c
void RefreshRoom(Rect *r);
void HiliteRect(Rect *r);
void DkHiliteRect(Rect *r);
void RevHiliteRect(Rect *r);
void DkRevHiliteRect(Rect *r);
void WhiteRect(Rect *r);
void ColorPaintRect(Rect *r, RGBColor *fc);
void RefreshNewRoom(void);
void DrawFacePixmap(short faceNbr, short faceColor, 
					short h, short v, 
					short fadePass,
					PixMapHandle pixMap);
void DrawFace(short faceNbr, short faceColor, short h, short v, short fadePass);
void DrawMansionPropPixMap(Handle	propH, PixMapHandle pMap, short h, short v, short fadePass);
void DrawMansionProp(Handle propH, Rect *dr);
void DrawProp(Handle propH, short h, short v, short fadePass);
void DrawPictureRes(GWorldPtr pWorld, Rect *sr, Rect *dr, short transColor);
void RefreshLProps(Rect *rr);
void AddLooseProp(long *cmd);
void MoveLooseProp(long *cmd);
void DelLooseProp(long *cmd);
void RefreshSpots(Rect *rr);
void RefreshFaces(Rect *r);
void ThisIsYourStat(short expert);
void InitRoomColors(void);

// DIBRender.c
short DIBRender(Handle h, GWorldPtr	pWorld, Point offset);
Boolean IsDIB(Handle h);
// GIFRender.c
short GIFRender(Handle h, GWorldPtr	pWorld, Point offset, short *transIndex);
Boolean IsGIF(Handle h);
// PalJPEGEntry.c
Boolean IsJPEG(Handle h);
short JPEGRender(Handle h, GWorldPtr	pWorld, Point offset);
void GetJPEGFrame(Handle h, Rect *r);

void GetPictureFrame(Handle h, Rect *r);

// RoomWindow.c
void NewRoomWindow(void);
void RoomWindowDraw(WindowPtr theWin);
void RoomWindowIdle(WindowPtr theWin, EventRecord *theEvent);
void RoomProcessKey(WindowPtr theWin, EventRecord *theEvent);
void RoomProcessKeyUp(WindowPtr theWin, EventRecord *theEvent);
void RoomWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void RoomWindowAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent);
void RoomWindowActivate(WindowPtr theWindow, Boolean activeFlag);
void DeIconize(void);
void Iconize(void);
void SetWindowToCLUT(WindowPtr theWindow, short clutID);
void PrepareTextColors(void);
void RestoreWindowColors(void);
void ShowRoomStatus(void);
void DrawRoomStatus(void);
void ClearRoomWindow(void);
void PrivateChatSelect(LONG userID, StringPtr name);
Boolean PrivateMsgTarget(Point p);
Boolean HotSpotTarget(Point p);
Boolean SelfPropClick(Point p, EventRecord *er);
Boolean LoosePropClick(Point p, EventRecord *er);
void ProcessUserString(Ptr ht, short len);
void FullScreen(void);
void PartialScreen(Boolean fromFull);
void ComputeMessageRects(void);
void StatusMessage(char *str, short errNbr);
void RefreshSplash(void);
void RecordCoordsIdle(void);
LPropPtr PtInLooseProp(LPropRec *prop, Point p, short index, short *retIndex);
void StdStatusMessage(short strCode);
void ErrStatusMessage(short strCode, short errCode);

// RoomSounds.c
void InitSounds(void);
void EndSounds(void);
void PlaySnd(short i, short priority);
void AdjustUserVolume(short volNbr);
void PlayWaveFile(char *fileName, Boolean requestForDownload);

// RoomListWindow.c
void NewRLWindow(void);
void RLWindowDraw(WindowPtr theWin);
void RLWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void RLAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposeRLWin(WindowPtr theWin);
Boolean GotoRoomInRoomList(short theLine);
void RLWindowReceiveRoomList(long nbrRooms, Ptr	p, long len);
void ProcessRoomList(long nbrRooms, Ptr	roomList, long len);
void RefreshRoomList(void);

// ScriptEditor.c
void NewScriptEditor(void);
void DisposeSEWin(WindowPtr theWin);
short AddRoomBuffer(Ptr str, long len);	// 4/6/95 JBUM
void UngetToken(void);
Boolean GetToken(void);
void LoadScriptIntoEditor(void);
void ParseEventHandler(void);
void SaveScriptFromEditor(void);

// TEWindow.c
void DisposeTEWin(WindowPtr theWin);

// UserAuthoring.c
void RoomInfoDialog(void);
void DoorInfoDialog(void);
void SwitchModes(short mode);
void DrawPolyHandle(Point p);
void DrawSpotFrame(HotspotPtr hs, short mode, Boolean showHandles);
void DrawSpotFrameOnscreen(HotspotPtr hs, Boolean showHandles);
void RefreshSpotFrames(void);
void ReCenterHotspot(HotspotPtr hs);
short KeyAuthoring(short code);
void ClickAuthoring(Point p, Boolean shiftFlag);
void LayerSpot(short cmd);
Boolean	HandleDialogEvent(EventRecord *theEvent);
void SpotDialogHit(EventRecord *theEvent, short itemHit);
void RoomDialogHit(EventRecord *theEvent, short itemHit);
void KillAuthoring();

// UserCursor.c
void InitCursors(void);
void SpinCursor(void);

// UserListWindow.c
void NewULWindow(void);
void ULWindowDraw(WindowPtr theWin);
void ULWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void ULAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposeULWin(WindowPtr theWin);
Boolean GotoRoomInUserList(short theLine);
void ULWindowReceiveUserList(long nbrUsers, Ptr	p, long len);
void ULWindowReceiveRoomList(long nbrRooms, Ptr	p, long len);
void ProcessUserList(long nbrUsers, Ptr	userList, long len);
void RefreshUserList(void);

// UserMain.c
short TimeBomb(void);
Boolean GotRequiredParams(AppleEvent *theEvent);
void DoOpenEvent(AppleEvent *theEvent);
Boolean GiveTime(short sleepTime);
void ProcessSuspend(void);
void SetIdleTimer(long x);

// UserMenus.c
void MySetUpMenus(void);
void MyAdjustMenus(void);
void ToggleExpertMenu(Boolean isExpert);
void ToggleWizard(void);
void MyHandleMenu (long mSelect);
void DebugTest(void);
void MyEnableMenuItem(MenuHandle menu, short item, short ok);
void NewABWindow(void);


// UserTCP.c
void PalTCPAbort(void);
Boolean PalTCPPollPacket(void);
void PostUserPalTCPEvent(unsigned long eventType, unsigned long refNum,
					Ptr buffer, long bufferLength);
void ClosePalTCP(void);
void SwapOutgoingClientTCPPacket(Ptr	buf);
void SwapIncomingClientTCPPacket(Ptr	buf);
short HostnameDialog(short connectType);
void LogMessage(char *str,...);
void PalTCPProcessPacket(void);
void OpenTCPSession(short connectType);
Boolean PalTCPIdle(EventRecord *theEvent);
void DisposeTCPSession(void);
OSErr BeginTCPConnection(char *name, short remotePort);

// UserUtils.c
PicHandle	GetPictureFromFile(StringPtr fName);
Boolean AssetsAreReadOnly(short fRefNum);
Boolean UserIsTryingToInterrupt(void);

// TextHistory.c
void PushHistoryText(TEHandle	teH);
void HistoryForward(void);
void HistoryBackward(void);


// M-UTILS.C
void EncryptString(StringPtr inStr, StringPtr outStr);
long GetSRand(void);
void MySRand(long s);
void Randomize(void);
long LongRandom(void);
double DoubleRandom(void);
short MyRandom(short max);
void EncryptString(StringPtr inStr, StringPtr outStr);
void DecryptString(StringPtr inStr, StringPtr outStr);
void EncryptCString(unsigned char *inStr, unsigned char *outStr, int len);
void DecryptCString(unsigned char *inStr, unsigned char *outStr, int len);
Boolean EqualPString(StringPtr inStr, StringPtr outStr, Boolean caseSens);
int stricmp(const char *str1,const char *str2);
int strincmp(const char *str1,const char *str2, int len);
char *CvtToCString(StringPtr str);
unsigned long SwapLong(unsigned long *n);
unsigned short SwapShort(unsigned short *n);
unsigned long LongSwap(unsigned long n);
Boolean LittleEndian();

// U-ALARMS.C
void AlarmsIdle(void);
void SetSpotAlarm(short spotID, long futureTime);

// U-ASSETS.C
void InitUserAssets(void);
void CloseUserAssets(void);
void RequestAsset(AssetType type, AssetSpec *spec);
void ReceiveAsset(Ptr assetMsg);
void RegisterAsset(AssetType type, AssetSpec *spec);
void ServerAssetQuery(long *assetMsg);	
void RefreshForAsset(AssetType type, long id, long crc);
Handle GetUserAsset(AssetType type, AssetSpec *spec);

// U-CMDS.C
void ProcessMacro(char *str);
LONG DoPalaceCommand(LONG cmdType, LONG arg, char *str);
Boolean BackStackAvailable();	// Used to enable/disable menu
void AddRoomToBackStack();
void GoBack();
void ClearBackStack();

// U-CYBORG.C
OSErr LoadUserScript(void);
Boolean GenerateUserEvent(short eventType);


// U-EVENTS.C
void SignOn(long sysStuff,short type);
void SignOff(void);
void ProcessMansionEvent(long cmd, long msgRefCon, char *buffer, long len);
void PostServerEventShort(unsigned long eventType);

// U-PROPS.C
Boolean PropInUse(long id, long crc);
void SavePropFavorites(void);
void ReplacePropFavorite(AssetSpec *oldspec,AssetSpec *newspec);
void LoadPropFavorites(void);
void DuplicatePropFavorite(AssetSpec *spec);
void AddPropFavorite(AssetSpec *spec);
void DeletePropFavorite(AssetSpec *spec);	// 6/7/95 
void ToggleProp(AssetSpec *spec);	// 6/7/95 rewritten to use AssetSpecs JAB
Boolean PtInProp(Point p, Point op, AssetSpec *propSpec);	// 6/7/95
Boolean IsRareProp(AssetSpec *propID);
Boolean IsPropFavorite(AssetSpec *spec);   // 6/9/95
void PurgeOldProps(void);

// U-RGNS.C
Boolean PtInHotspot(Point p, Hotspot *hs);
HotspotPtr GetNavArea(void);
HotspotPtr GetDoor(short doorID);
HotspotPtr GetHotspot(short spotID);
char *GetEventHandler(HotspotPtr hs, short type);
PictureRecPtr GetPictureRec(short n);
PictureRecPtr GetPictureRecByName(StringPtr name);
void PassHotspotEvent(HotspotPtr hs, short type);
void UserClickHotspot(Point np, short n);
void TriggerHotspotEvents(short eventType);
void SetPictureOffset(short *setRec);
void SetSpotLoc(short *setRec);
void SetSpotState(short *setRec);
Boolean DoorIsLocked(short doorID);
void SetDoorLock(short *lockRec, Boolean lockFlag);
short GetSpotState(short spotID);
void GetSpotName(HotspotPtr hs, StringPtr name);	// 8/24/95

// U-ROOMS.C
LocalUserRecPtr GetUser(long id);
LocalUserRecPtr GetUserByName(char *str);
void ThisIsYourRec(LONG id);
void ComputePropsRect(LocalUserRecPtr up, Rect *r);
void MoveUser(short hd, short vd);
void ComputePropRect(short propIdx,LocalUserRecPtr up,Rect *r);
void ComputeNameRect(LocalUserRecPtr up, Rect *r);
void ComputeUserRect(LocalUserRecPtr	up, Rect *r);
void ComputeUserRRect(LocalUserRecPtr up, Rect *r);
void RefreshUser(LocalUserRecPtr cUser);
void ClearRoom(void);
void ModifyRoomInfo(RoomRecPtr room, Boolean reloadFlag);	// 7/24/96 JAB bug fix
void NewRoom(RoomRecPtr room);
void EndRoomDescription(void);
void RoomPerson(UserRecPtr person);
void RoomPeople(short n, UserRecPtr ary);
void UpdateUserPosition(PersonID userID, Point *newpos);
void UpdateUserFace(PersonID userID, short faceNbr);
void UpdateUserProp(PersonID userID, long *p);
void UpdateUserColor(PersonID userID, short colorNbr);
void UpdateUserName(PersonID userID, StringPtr name);
void LogonPerson(PersonID userID);
void LogOff(void);
void LogoffPerson(PersonID userID);
void ChangeFace(short faceNbr);
void ChangeColor(short colorNbr);
void ChangeProp(short nbrProps, AssetSpec *specList);
void DonPropByName(char *cString);
void DoffPropByName(char *cString);
void NewPerson(UserRecPtr person);
void ChangeColorLocal(short colorNbr);
void MoveUserLocal(short hd, short vd);
short AddRoomString(StringPtr str);
short AddRoomBuffer(Ptr str, long len);
void FadeOutPerson(PersonID userID);
void ExitPerson(PersonID userID);
void DeletePeople(void);
void UpdateUserDesc(PersonID userID, short *p);	// 6/7/95 New Function
void ChangeUserDesc(short faceNbr, short colorNbr, short nbrProps, AssetSpec *specList);
void DonProp(long id, unsigned long crc);	// Put on prop with id#
void DoffProp(long id, unsigned long crc);	// Put on prop with id#
void ClearProps(void);		// take off all props#
void CheckUserFace(LocalUserRecPtr cUser);
void ModifyLoosePropHandles(void);
void ProcessDrawCmd(DrawRecPtr dp);
void DropProp(void);

// U-SCRIPT.C
void InitScriptMgr(void);

// U-SECURE.C
void GetServerInfo(ServerInfoPtr buf, Boolean newStyle);
Boolean DeniedByServer(long flag);

// U-SWAP.C
unsigned short SwapShort(unsigned short *n);
unsigned long SwapLong(unsigned long *n);
unsigned long LongSwap(unsigned long n);
void SwapAsset(AssetBlockPtr p);
void SwapFile(FileBlockPtr p);
void SwapIncomingUser(UserRecPtr p);
void SwapDrawCmdOutBuffer(short cmd, Ptr cp);
void SwapDrawCmdInBuffer(short cmd, Ptr cp);
void SwapDrawCmdOut(DrawRecPtr p, Ptr cp);
void SwapDrawCmdIn(DrawRecPtr p, Ptr cp);
void SwapIncomingRoom(RoomRecPtr rp);
void SwapOutgoingRoom(RoomRecPtr rp);
void SwapOutgoingClientTCPPacket(Ptr buf);
void SwapIncomingClientTCPPacket(Ptr buf);


// U-TEXT.C
void UserText(PersonID userID, char *text, Boolean privateFlag);
void AddBalloon(LocalUserRecPtr cUser, char *text, Boolean privateFlag);
void DeleteBalloon(short n);
void ClearBalloons(void);
void KillBalloon(short balloonID);
void BalloonIdle(void);
void KillUserBalloons(PersonID userID);

// U-FILE.C
void ReceiveFile(Ptr fileMsg);	// Server is sending file block
void RequestFile(StringPtr name, int mediaType);
Boolean FileIsBeingDownloaded(StringPtr name);
void AbortDownloads(void);
void ShowDownloadStatus(StringPtr name, unsigned long timeStamp, short blockNbr, short maxBlock);
void CreateServerDirectory(int cat, int idx);							// 9/22/95
StringPtr	BuildMediaFolderName(StringPtr pictureName, int cat, int idx);	// 9/22/95

// PalaceSpeech.c
void SpeakChat(short flag, char *str);
void SelectVoice(short voiceNumber);


// Midi Stuff
void MidiPlay(char *fileName);				// note filename is a C-String!!
void MidiLoop(char *fileName, short iters);	// -1 means infinite
void MidiStop(void);

// Registration Stuff
void RegInfo(void);
void RegComplete2Dialog(void);
void RegComplete1Dialog(void);
void RegCompleteDialog(short dialogNbr);

