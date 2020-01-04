// MacServ.h

// MacLocalUtils
void    ReportError(short code, char *name);
void    ErrorExit  (char *str,...);
void 	  ErrorMessage(char *str,...);
void    ProcessMansionEvent(ServerUserPtr cUser, long cmd, long msgRefCon, char *buffer, long len);
OSErr OpenFileReadOnly(StringPtr name, short volNum, FileHandle *refNum);

// Main
void CloseAllWindows();
Boolean GotRequiredParams(AppleEvent *theEvent);
void DoOpenEvent(AppleEvent *theEvent);
pascal OSErr AppleEventHandler(AppleEvent *theEvent,AppleEvent *reply, long refCon);
void SetIdleTime(long idle);

// Menus
void MySetUpMenus(void);
void AppAdjustMenus(void);
void MyEnableMenuItem (MenuHandle menu, short item, short ok);
void AppProcessCommand (short menuID, short menuItem);
void MyAdjustMenus();
void MyHandleMenu(long mSelect);

// Log Window
void NewLogWindow(void);
void Iconize();
void DeIconize();
void ToggleLogFile();

// Prefs
void LoadPreferences();
OSErr CreatePrefsFile();
void StorePreferences();
pascal void DrawLockItem(DialogPtr dp, short itemNbr);
pascal void PrefsButtonState(DialogPtr dp, short itemNbr);
pascal Boolean PrefsFilter(DialogPtr dp, EventRecord *ep, short *itemHit);
void PreferencesDialog();
pascal Boolean WizDialogFilter(DialogPtr dp, EventRecord *ep, short *itemHit);
short PasswordDialog(StringPtr str, StringPtr prompt);

// Yellow Pages
void YPDialog();
void RegisterOnYellowPages(Boolean force, Boolean downFlag);


// LocalServUtils
void PostUserEvent(ServerUserPtr user,unsigned long eventType,unsigned long refNum,Ptr bufferContents,long bufferLength);
short DisconnectUser(ServerUserPtr user);
void BeginSendGroup(ServerUserPtr cUser);
void EndSendGroup(ServerUserPtr cUser);
void SendGroupBuffer(ServerUserPtr cUser);
void FlushGroupBuffer(ServerUserPtr cUser);
Boolean GiveTime(short sleepTime);
void ConvertNetAddressToString(ServerUserPtr cUser, char *dbuf);
void ConvertNetAddressToNumericString(ServerUserPtr cUser, char *dbuf);
void ConvertIPToString(unsigned LONG ip, char *dbuf);
long GetIPAddress(ServerUserPtr cUser);
Boolean IsPendingSend(ServerUserPtr curUser);
long PendingSendLength(ServerUserPtr curUser);
StringPtr	BuildPictureName(StringPtr name);
Ptr SuckTextFile(StringPtr fileName);

// ServUserList.c
void ToggleUserList();
void RebuildUserDisplay();
void UpdateUserDisplay(long userID);


// ServerLocalTalk.c
Boolean PostServerLTBuffer(ServerUserPtr user, Ptr buffer, long len);
void InitPPCStuff();
void ClosePPCStuff();
void ProcessLocalTalkEvent(EventRecord *er);

// ServerTCP.c
Boolean PostServerTCPBuffer(ServerUserPtr cUser, Ptr buffer, long len);
unsigned long GetNewStream();
void DisposeStream(unsigned long stream);
void InitServerTCP();
void CleanupServerTCP();
void SetupNewTCPUser();
void DisconnectTCPUser(ServerUserPtr cUser);
Boolean IdleTCPUser(ServerUserPtr cu);
void ServerTCPIdle();

// ServNBP.c
void pcatdec(StringPtr str, short num);
OSErr	AddPPCNBPAlias(NamesTableEntry *theNTE, Str32 newNBPType, EntityName *newEntity);
OSErr	RemoveNBPAlias(EntityPtr theEntity);

// DNR
OSErr OpenResolver(char *fileName);
OSErr AddrToStr(unsigned long addr, char *addrStr);
OSErr CloseResolver(void);

// ServUtils.c
StringPtr	BuildMediaFolderName(StringPtr pictureName, int catalogNumber, int indexNumber);
