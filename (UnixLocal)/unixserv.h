/* unixserv.h */
/* Function Prototypes for Unix-Specific Functions */

/* UnixLocalUtils.c */
void ErrorMessage(char *str,...);
void ReportError(short code, char *name);
void ErrorExit(char *str,...);
void LogMessage(char *str,...);
void err_dump(char *str);
void err_sys(char *str);
void OpenLog(char *name);
void CloseLog(void);


/* LocalServUtils.c */
Boolean IsPendingSend(ServerUserPtr curUser);
LONG PendingSendLength(ServerUserPtr curUser);
short DisconnectUser(ServerUserPtr curUser);
void FlushGroupBuffer(ServerUserPtr cUser, LONG bytesneeded);	/* Wait till Group Buffer is Sent */
void SendGroupBuffer(ServerUserPtr cUser);
void PostUserEvent(ServerUserPtr cUser, unsigned LONG eventType, unsigned LONG refNum,
					Ptr bufferContents, LONG bufferLength);
Boolean GiveTime(short sleepTime);	
void ConvertNetAddressToString(ServerUserPtr cUser, char *dbuf);
LONG GetIPAddress(ServerUserPtr cUser);
StringPtr BuildPictureName(StringPtr pictureName);
Ptr SuckTextFile(StringPtr fileName);	/* filename is p-string, returns entire file in buffer */
void ConvertNetAddressToNumericString(ServerUserPtr cUser, char *dbuf);
void ConvertIPToString(unsigned LONG ipSrce, char *dbuf);

/* UnixPrefs.c */
void LoadPreferences(void);
void StorePreferences(void);

/* Unwritten */
void UpdateUserDisplay(LONG userID);
void RebuildUserDisplay(void);
OSErr OpenFileReadOnly(StringPtr name, short volNum, short *refNum);

/* ServerTCP.c */
unsigned LONG GetNewStream(void);
void DisposeStream(unsigned LONG stream);
void InitServerTCP(int port);
void InitDaemon();
void ServerTCPIdle(void);
void CleanupServerTCP(void);
Boolean PostServerTCPBuffer(ServerUserPtr cUser, Ptr buffer, LONG len, LONG *bytesWritten);
void DisconnectTCPUser(ServerUserPtr cUser);

