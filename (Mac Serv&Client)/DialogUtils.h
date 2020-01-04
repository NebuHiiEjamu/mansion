/**************************************************************
 * DialogUtils.h						for Class 6
 *
 * Dialog Utility Functions
 **************************************************************/
#ifndef OK
#define OK		1
#define Cancel	2
#endif

void SetControl(DialogPtr dial, int item, int value);
int GetControl(DialogPtr dial,register int item);
void ToggleControl(DialogPtr dial, int item);
void SetText(DialogPtr dial, int item, StringPtr text);
void GetText(DialogPtr dial, int item, StringPtr text);
void OutlineButton(DialogPtr dp, int bid, Pattern *pat);
void DisableButton(DialogPtr dp, int bid);
void EnableButton(DialogPtr dp, int bid);
void SimulateButtonPress(DialogPtr dp, int bid);
void PrintfItem(DialogPtr dp, int item, char *template,...);
int ScanfItem(DialogPtr dp, int item, char *template,...);
int GetIntItem(DialogPtr dp, int item);
void PutIntItem(DialogPtr dp, int item, int n);
int GetIntItem(DialogPtr dp, int item);
Boolean CallFilterProc(DialogPtr dp, EventRecord *ep, short *itemHit);
