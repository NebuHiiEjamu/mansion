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
#define DebugWizardPassword		"\x04\x96\x55\x8b\x5c"
/* "\x0b\xfd\x5a\x0b\x1b\x7b\x73\x20\x93\x5c\x81\x56" abracadabra */
#define DebugGodPassword		"\x05\x30\x94\x4a\x9c\x4d"
#define DefaultMaxOccupancy		128
#define DefaultRoomOccupancy	24
#define DefaultPicturesFolder	"\x09pictures/"

#define NOPREFS	0
#if NOPREFS
ServerPrefs gPrefs;
#else
ServerPrefs	gPrefs = {ServerPrefsVersion, DefaultPalaceName, 
					  DefaultTextFont, DefaultTextSize, DefaultTextCreator, 
/* Wizard Password (Superuser Status) */
#if !RELEASE		/* abracadabra password */
						DebugWizardPassword,
						DebugGodPassword,
#else
						"",
						"",
#endif
/* Owner Password (to unlock prefs) */
						"",
						false,true,DefaultTCPPort,
						DefaultPermissions,DefaultDeathPenalty,
						DefaultPurgeInactive, DefaultFloodEvents,
						DefaultMaxOccupancy, DefaultRoomOccupancy,
						DefaultPicturesFolder};
#endif
Str31		prefsName = "\08.pserver";


void LoadPreferences()
{
#if NOPREFS
	gPrefs.versID = ServerPrefsVersion;
	strcpy(gPrefs.serverName,DefaultPalaceName);
#if !RELEASE
	strcpy(gPrefs.wizardPassword,DebugWizardPassword);
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
#endif
}


void StorePreferences()
{
}
