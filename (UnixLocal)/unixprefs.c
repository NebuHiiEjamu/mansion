/* UnixPrefs.c */

#include "s-server.h"

#define ServerPrefsVersion	0x00010013L

#define DefaultPermissions	(PM_AllowGuests | PM_AllowCyborgs | PM_AllowPainting |	\
						     PM_AllowCustomProps | PM_AllowWizards |				\
						     PM_WizardsMayKill | PM_WizardsMayAuthor |				\
						     PM_PurgeInactiveProps | PM_KillFlooders)

#define DefaultPalaceName		"\x0aThe Palace"
#define DefaultDeathPenalty		5
#define DefaultPurgeInactive	21
#define DefaultFloodEvents		200
#define DefaultTCPPort			9998
#define DefaultTextFont			0
#define DefaultTextSize			9
#define DefaultTextCreator		0
#define DebugWizardPassword		"\x04\x96\x55\x8b\x5c"		/* mark */
/* "\x0b\xfd\x5a\x0b\x1b\x7b\x73\x20\x93\x5c\x81\x56" abracadabra */
#define DebugGodPassword		"\x05\x30\x94\x4a\x9c\x4d"   /* spitz */
#define DefaultMaxOccupancy		200
#define DefaultRoomOccupancy	16
#define DefaultPicturesFolder	"\x09pictures/"
#define DefaultRecycleLimit		10000

#define NOPREFS	0
#if NOPREFS
ServerPrefs gPrefs;
#else
ServerPrefs	gPrefs = {ServerPrefsVersion, DefaultPalaceName, 
					  DefaultTextFont, DefaultTextSize, DefaultTextCreator, 
/* Wizard Password (Superuser Status) */
/* abracadabra password */
						DebugWizardPassword,
						DebugGodPassword,
/* Owner Password (to unlock prefs) */
						"",
						false,true,DefaultTCPPort,
						DefaultPermissions,DefaultDeathPenalty,
						DefaultPurgeInactive, DefaultFloodEvents,
						DefaultMaxOccupancy, DefaultRoomOccupancy,
						DefaultPicturesFolder,		/* old pictures folder */
						"Joe Sysop",
						"",
						"Unix",
						"Generic Description",
						"",		/* Announcement */
						"mansion.thePalace.com",	/* Yellow Pages Server */
						true,						/* auto register */
						DefaultPicturesFolder,
						DefaultRecycleLimit
						};
#endif
Str31		prefsName = "\08.pserver";


void LoadPreferences()
{
#if NOPREFS
	gPrefs.versID = ServerPrefsVersion;
	strcpy(gPrefs.serverName,DefaultPalaceName);
#if !RELEASE
	strcpy(gPrefs.wizardPassword,DebugWizardPassword);
	strcpy(gPrefs.godPassword,DebugGodPassword);
#endif
	gPrefs.allowTCP = true;
	gPrefs.localPort = DefaultTCPPort;
	gPrefs.permissions = DefaultPermissions;
	gPrefs.deathPenaltyMinutes = DefaultDeathPenalty;
	gPrefs.purgePropDays = DefaultPurgeInactive; 
	gPrefs.minFloodEvents = DefaultFloodEvents;
	gPrefs.maxOccupancy = DefaultMaxOccupancy;
	gPrefs.roomOccupancy = DefaultRoomOccupancy;
	strcpy(gPrefs.picFolder, DefaultPicturesFolder);
	gPrefs.recycleLimit = DefaultRecycleLimit;
#endif
}


void StorePreferences()
{
}
