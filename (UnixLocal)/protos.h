// protos.h - header file for after U-USER.H
typedef struct {
	TEWindowRec			win;
	short				logFile;
	RGBColor			curColor;	// 6/21/95
} LogWindowRec, *LogWindowPtr;

#ifndef THINK_C
#define OK		1
#define Cancel	2
#endif

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
				gDebugFlag,gEnteringRoom,g12InchMode,gShowNames, gExpert, 
				gIconized, gSublaunch;
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
void OffscreenTextEnable();
void OffscreenTextRestore();
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
void SetConnectionType(short mode);

// Preferences.c
void LoadPreferences(void);
OSErr CreatePrefsFile(void);
void StorePreferences(void);
void RecordMacro(short n);
void PlayMacro(short n);
void PreferencesDialog(void);
short PasswordDialog(StringPtr str, StringPtr prompt);
void ChildLockDialog(void);
Boolean ChildLock(void);

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
void RefreshPropToolPalette();
void MakeCompositePicture();
void RefreshPropRect(Rect *r);
void RefreshPropFrame();
Boolean PointToPropPoint(Point p, Point *tp);
void PropPenMoveTo(Point tp,Boolean cmdFlag);
void PropPenLineTo(Point tp,Boolean cmdFlag);
void PropPenDone();
void PropPencilTool(Point p, Boolean shiftFlag, Boolean cmdFlag);
void MovePropFrame(short hd, short vd);
Boolean PEProcessKey(EventRecord *theEvent);
Handle ConvertPixelsToProp(PixMapHandle pMap, PixMapHandle mMap);
void SaveProp();

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
void DrawPictureRes(GWorldPtr pWorld, Rect *sr, Rect *dr, unsigned char transColor);
void RefreshLProps(Rect *rr);
void AddLooseProp(long *cmd);
void MoveLooseProp(long *cmd);
void DelLooseProp(long *cmd);
void RefreshSpots(Rect *rr);
void RefreshFaces(Rect *r);
void ThisIsYourStat(short expert);
void InitRoomColors(void);

// DIBRender.c
short DIBRender(Handle h, PixMapHandle	pMap, Point offset);
Boolean IsDIB(Handle h);
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
void DeIconize();
void Iconize();
void SetWindowToCLUT(WindowPtr theWindow, short clutID);
void PrepareTextColors(void);
void RestoreWindowColors(void);
void ShowRoomStatus(void);
void DrawRoomStatus(void);
void ClearRoomWindow(void);
Boolean PrivateMsgTarget(Point p);
Boolean HotSpotTarget(Point p);
Boolean SelfPropClick(Point p, EventRecord *er);
Boolean LoosePropClick(Point p, EventRecord *er);
void ProcessUserString(Ptr ht, short len);
void FullScreen(void);
void PartialScreen(Boolean fromFull);
void ComputeMessageRects();
void StatusMessage(char *str, short errNbr);
void RefreshSplash(void);
void RecordCoordsIdle(void);
LPropPtr PtInLooseProp(LPropRec *prop, Point p, short index, short *retIndex);

// RoomSounds.c
void InitSounds(void);
void EndSounds(void);
void PlaySound(short i, short priority);
void AdjustUserVolume(short volNbr);

// RoomListWindow.c
void NewRLWindow(void);
void RLWindowDraw(WindowPtr theWin);
void RLWindowClick(WindowPtr theWin, Point where, EventRecord *theEvent);
void RLAdjustCursor(WindowPtr theWin, Point where, EventRecord *theEvent);
void DisposeRLWin(WindowPtr theWin);
Boolean GotoRoomInRoomList(short theLine);
void RLWindowReceiveRoomList(long nbrRooms, Ptr	p, long len);
void ProcessRoomList(long nbrRooms, Ptr	roomList, long len);

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
void RoomInfoDialog();
void DoorInfoDialog();
void SwitchModes(short mode);
void DrawPolyHandle(Point p);
void DrawSpotFrame(HotspotPtr hs, Boolean showHandles);
void DrawSpotFrameOnscreen(HotspotPtr hs, Boolean showHandles);
void RefreshSpotFrames();
void ReCenterHotspot(HotspotPtr hs);
short KeyAuthoring(short code);
void ClickAuthoring(Point p, Boolean shiftFlag);

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

// UserMain.c
short TimeBomb(void);
Boolean GotRequiredParams(AppleEvent *theEvent);
void DoOpenEvent(AppleEvent *theEvent);
Boolean GiveTime(short sleepTime);
void ProcessSuspend();
void SetIdleTimer(long x);

// UserMenus.c
void MySetUpMenus(void);
void MyAdjustMenus(void);
void ToggleExpertMenu();
void ToggleWizard();
void MyHandleMenu (long mSelect);
void DebugTest();
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
void PalTCPProcessPacket();
void OpenTCPSession(short connectType);
Boolean PalTCPIdle(EventRecord *theEvent);
void DisposeTCPSession();
OSErr BeginTCPConnection(char *name, short remotePort);

// UserUtils.c
PicHandle	GetPictureFromFile(StringPtr fName);
Boolean AssetsAreReadOnly(short fRefNum);

// TextHistory.c
void PushHistoryText(TEHandle	teH);
void HistoryForward();
void HistoryBackward();


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
short DoPalaceCommand(short cmdType, long arg, char *str);

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

// U-RGNS.C
Boolean PtInHotspot(Point p, Hotspot *hs);
HotspotPtr GetNavArea();
HotspotPtr GetDoor(short doorID);
HotspotPtr GetHotspot(short spotID);
char *GetEventHandler(HotspotPtr hs, short type);
PictureRecPtr GetPictureRec(short n);
void PassHotspotEvent(HotspotPtr hs, short type);
void UserClickHotspot(Point np, short n);
void TriggerHotspotEvents(short eventType);
void SetPictureOffset(short *setRec);
void SetSpotLoc(short *setRec);
void SetSpotState(short *setRec);
Boolean DoorIsLocked(short doorID);
void SetDoorLock(short *lockRec, Boolean lockFlag);
short GetSpotState(short spotID);

// U-ROOMS.C
LocalUserRecPtr GetUser(long id);
LocalUserRecPtr GetUserByName(char *str);
void ThisIsYourRec(long id);
void ComputePropsRect(LocalUserRecPtr up, Rect *r);
void MoveUser(short hd, short vd);
void ComputePropRect(short propIdx,LocalUserRecPtr up,Rect *r);
void ComputeNameRect(LocalUserRecPtr up, Rect *r);
void ComputeUserRect(LocalUserRecPtr	up, Rect *r);
void ComputeUserRRect(LocalUserRecPtr up, Rect *r);
void RefreshUser(LocalUserRecPtr cUser);
void ClearRoom(void);
void ModifyRoomInfo(RoomRecPtr room);
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
void ModifyLoosePropHandles();
void ProcessDrawCmd(DrawRecPtr dp);

// U-SCRIPT.C
void InitScriptMgr(void);

// U-SWAP.C
unsigned short SwapShort(unsigned short *n);
unsigned long SwapLong(unsigned long *n);
unsigned long LongSwap(unsigned long n);
void SwapAsset(AssetBlockPtr p);
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
void ClearBalloons();
void KillBalloon(short balloonID);
void BalloonIdle();
void KillUserBalloons(PersonID userID);
