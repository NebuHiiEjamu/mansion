/* UnixServMain.c */
#include "s-server.h"

Boolean	gQuitFlag, gDebugFlag, gModified, gNoForkFlag;
extern unsigned char *ScriptFileName;

int main(int argc, char **argv) 
{
	int		i,port=0;
	char	*p;
	char	logFileName[512];
	LONG	nbrMembers;
	char	*syntaxMsg = "Syntax: %s [-p port#] [-d] [-l logfile] [-m scriptfile]\n";

	/* Read command line arguments, set debug flag */
	sprintf(logFileName,"%s.log",argv[0]);

	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'l':
			case 'L':
				if (argv[i][2])
					p = &argv[i][2];
				else {
					++i;
					p = argv[i];
				}
				strcpy(logFileName,p);
				break;
			case 'd':
			case 'D':
				gDebugFlag = 1;
				LogMessage("Debug On");
				break;
			case 'x':
			case 'X':
				gNoForkFlag = 1;
				LogMessage("No Forking");
				break;
			case 'p':
			case 'P':
				if (argv[i][2])
					p = &argv[i][2];
				else {
					++i;
					p = argv[i];
				}
				port = atoi(p);
				break;
			case 'm':
			case 'M':
				if (argv[i][2])
					p = &argv[i][2];
				else {
					++i;
					p = argv[i];
				}
				ScriptFileName = p;
				CtoPstr(ScriptFileName);
				break;
			default:
				fprintf(stderr,syntaxMsg,argv[0]);
				exit(0);
				break;
			}
		}
		else {
			if (isdigit(argv[i][0]))
				port = atoi(argv[i]);
			else {
				fprintf(stderr,syntaxMsg,argv[0]);
				exit(0);
			}
		}
	}
	InitDaemon();
	OpenLog(logFileName);
	if (AllocateServerBuffers())
		ErrorExit("Not enough memory...");
	LoadPreferences();
	InitServerTCP(port);
	LogMessage("Reading Mansion Script...\n");
	if (ReadScriptFile((unsigned char *) "mansion.scr") != noErr)
		ErrorExit("Can't continue...");
	LogMessage("Opening Assets...\n");
	InitServerAssets();
	nbrMembers= CountAssets(RT_USERBASE);
	if (nbrMembers > 2)
		LogMessage("Over %ld members served!\n", nbrMembers-1);

	LogMessage("Server Active\n");
	/* Main Loop */
	while (!gQuitFlag) {
		ServerIdle();
		ServerTCPIdle();
	}
	LogMessage("Shutting down\n");
	/* Done with Main Loop */

	LogoffAllUsers();
	SaveScript();
	CleanupServerTCP();
	CloseServerAssets();
	CloseLog();
	return 0;
}
