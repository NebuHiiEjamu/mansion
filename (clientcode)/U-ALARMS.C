// U-ALARMS.C
//
// 7/21/95 Added different alarm types - spot alarms, and script alarms
//
// Script alarms include the actual code to be executed in the future.
//
#include "U-USER.H"
#include "m-cmds.H"
#include "U-SCRIPT.H"
#include "PPAMgr.h"

#define MaxAlarmEvents	32

enum {AT_SpotAlarm = 0, AT_ScriptAlarm, AT_CyborgAlarm};

typedef struct {
	short	alarmType;
	short	spotID;
	unsigned long alarmTime;
	Ptr 	alarmData;
} AlarmRecord;

short		gRoomID=-1;
short		gNbrAlarms;
AlarmRecord gAlarms[MaxAlarmEvents];

void ClearAlarms(void);
void AlarmsIdle(void);
void SetSpotAlarm(short spotID, long futureTime);
void InsertAlarm(AlarmRecord *temp);

void ClearAlarms(void)
{
	short	i;
	for (i = 0; i < gNbrAlarms; ++i)
		if (gAlarms[i].alarmData)
			DisposePtr(gAlarms[i].alarmData);
	gNbrAlarms = 0;
	gRoomID = gRoomWin->curRoom.roomID;
}

void AlarmsIdle(void)
{
#if BOTCODE
	BotEntryPoint(gRoomWin, 0L, 0L, 0L, "");
#endif
#if PPASUPPORT
	PPAMgrIdle();
#endif
	if (gNbrAlarms == 0)
		return;
    // 9/12/95 don't use alarms when navigation is in progress
	if (gRoomID != gRoomWin->curRoom.roomID || gRoomWin->navInProgress) {
		ClearAlarms();
		return;
	}
	if (TickCount() >= gAlarms[0].alarmTime) {
		// Remove Alarm
		HotspotPtr	hs;
		AlarmRecord	temp;
		temp = gAlarms[0];
		BlockMove((Ptr)&gAlarms[1],(Ptr)&gAlarms[0],(gNbrAlarms-1)*sizeof(AlarmRecord));
		--gNbrAlarms;
		// Generate Alarm Event
		switch (temp.alarmType) {
		case AT_CyborgAlarm:
			if (gRoomWin->mePtr != NULL)
				GenerateUserEvent(PE_Alarm);
			break;
		case AT_SpotAlarm:
			hs = GetHotspot(temp.spotID);
			if (hs) {
				PassHotspotEvent(hs, PE_Alarm);
			}
			break;
		case AT_ScriptAlarm:
			if (temp.spotID)
				gPV.curSpot = GetHotspot(temp.spotID);
			else
				gPV.curSpot = NULL;
			RunMessageScript(temp.alarmData);
			break;
		}
		if (temp.alarmData)
			DisposePtr(temp.alarmData);
	}
}


void InsertAlarm(AlarmRecord *temp)
{
	int	i;
	if (gNbrAlarms >= MaxAlarmEvents)
		return;
	for (i = 0; i < gNbrAlarms; ++i) {
		if (temp->alarmTime < gAlarms[i].alarmTime) {
			BlockMove((Ptr)&gAlarms[i],(Ptr)&gAlarms[i+1],(gNbrAlarms-(i+1))*sizeof(AlarmRecord));
			break;
		}
	}
	gAlarms[i] = *temp;
	++gNbrAlarms;
}

// Set Spot Alarm
void SetSpotAlarm(short spotID, long futureTime)
{
	AlarmRecord	temp;
	if (gRoomID != gRoomWin->curRoom.roomID)
		ClearAlarms();
	temp.alarmType = spotID? AT_SpotAlarm : AT_CyborgAlarm;
	temp.spotID = spotID;
	temp.alarmTime = TickCount() + futureTime;	
	temp.alarmData = NULL;
	InsertAlarm(&temp);
}

void SetScriptAlarm(char *script, long futureTime)
{
	AlarmRecord	temp;
	if (gRoomID != gRoomWin->curRoom.roomID)
		ClearAlarms();
	temp.alarmType = AT_ScriptAlarm;
	if (gPV.curSpot)
		temp.spotID = gPV.curSpot->id;
	else
		temp.spotID = 0;
	temp.alarmTime = TickCount() + futureTime;	
	/* 7/1/96 JAB Changed to NewPtrClear */
	temp.alarmData = NewPtrClear(strlen(script)+1);
	if (temp.alarmData) {
		strcpy(temp.alarmData,script);
		InsertAlarm(&temp);
	}
}

