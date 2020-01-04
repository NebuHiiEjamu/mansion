/* Yellow Pages Daemon
 *
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ipc.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

#define DEFAULT_YPD_PORT		9991

int 	gSockfd;
struct 	sockaddr_in serv_addr;
int		gDebugFlag;

void MainLoop();

static char gTBuf[512];
char		*gLogFileName = "ypd.log";

void LogMessage(char *str,...)
{
	FILE	*lFile;
	char	*timeStr;
	time_t	tim;
	struct tm *tm;

	va_list args;
	va_start(args,str);
	vsprintf(gTBuf,str,args);
	va_end(args);

	time(&tim);
	tm = localtime(&tim);
	timeStr = asctime(tm);

	if ((lFile = fopen(gLogFileName,"a")) != NULL) {
		fputs(timeStr,lFile);
		fputs(gTBuf,lFile);
		fclose(lFile);
	}
}

void ErrorExit(char *str,...)
{
	va_list args;
	va_start(args,str);
	vsprintf(gTBuf,str,args);
	va_end(args);

	fputs(gTBuf,stderr);

	exit(1);
}


void InitializeYPD(int thePort)
{
	int	fd;
	struct servent *pse;
	int		err;
/**
	fd = open("/dev/tty", O_RDWR);
	(void) ioctl(fd, TIOCNOTTY, 0);
	(void) close(fd);
 **/
	umask(027);
	setpgrp(0, getpid());
	signal(SIGHUP,SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	if (thePort == 0) {
		pse = getservbyname("ypd","tcp");
		if (pse)
			thePort = pse->s_port;
		else
			thePort = htons(DEFAULT_YPD_PORT);
	}
	else
		thePort= htons(thePort);
	LogMessage("Start up: Using Port %d\n",ntohs(thePort));
    if ((gSockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
       ErrorExit("server: can't open stream socket");
	{
		int			reuseFlag = 1;
		setsockopt(gSockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseFlag, sizeof(reuseFlag));
	}
    bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port        = thePort;

    if ((err = bind(gSockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0){
		perror("bind failed");
    	ErrorExit("server: can't bind local address");
	}

    listen(gSockfd, 5);
    LogMessage("Listening\n");
}


int main(int argc, char **argv) 
{
	int		i,	port=0;
	char	*p;
	/* Read command line arguments, set debug flag */
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'd':
			case 'D':
				gDebugFlag = 1;
				LogMessage("Debug On\n");
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
			default:
				ErrorExit("Syntax: %s [-p port#] [-d]\n",argv[0]);
				exit(0);
				break;
			}
		}
		else {
			if (isdigit(argv[i][0]))
				port = atoi(argv[i]);
			else {
				ErrorExit("Syntax: %s [-p port#] [-d]\n",argv[0]);
				exit(0);
			}
		}
	}

	InitializeYPD(port);

	LoadListing();
	
	while (1)
		MainLoop();
}

void SetNonBlocking(int fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) == -1)
	    return;
	fcntl(fd, F_SETFL, flags | FNDELAY);
}

int readline(int sockFD, char *ptr, int maxLen)
{
	int 	n,rc;
	char	c;
	int	starttime;

	SetNonBlocking(sockFD);

	starttime = time(NULL);
	n = 0;
	while (n < maxLen) {
		if ( ( rc = read(sockFD, &c, 1) ) == 1) {
			++n;
			*(ptr++) = c;
			if (c == '\n' || c == '\r')
				break;
		} else if ( rc == 0) {
			if (time(NULL) - starttime < 20)
				continue;
			if (n == 0)
				return 0;
			else
				break;
		}
		else {
			if (errno == 11 || errno == 35) {
				if (time(NULL) - starttime < 20)
					continue;
				return -1;
			}
			return -1;
		}		
	}
	*ptr = 0;
	return n;
}

void ConvertNetAddressToNumericString(unsigned int ipSrce, char *dbuf);
void ConvertNetAddressToNumericString(unsigned int ipSrce, char *dbuf)
{
	unsigned int	ip = ipSrce;

	if (LittleEndian())
		ip = SwapLong(&ip);
	sprintf(dbuf,"%d.%d.%d.%d",
		(int) ((ip >> 24) & 0x00FF),
		(int) ((ip >> 16) & 0x00FF),
		(int) ((ip >> 8) & 0x00FF),
		(int) (ip & 0x00FF));
}

void ProcessConnect(int sockFD, int ipAddr)
{
	char	tbuf[1024]="";
	char	ipBuf[32];
	int n;

	ConvertNetAddressToNumericString(ipAddr,ipBuf);

    n = readline(sockFD, tbuf, 1023);

	LogMessage("Accepted Connection (%s) %d chars read\n",ipBuf,n);

	if (n <= 0) {
		LogMessage("Read Error (%s)\n",ipBuf);
		close(sockFD);
	}
	else {
		close(sockFD);
		UpdateListing(tbuf,ipAddr);
	}
}

void MainLoop()
{
	struct sockaddr_in cli_addr;
    int cliLen, childpid;
    int newsockfd;

    cliLen = sizeof(cli_addr);
    newsockfd = accept(gSockfd, (struct sockaddr *) &cli_addr, &cliLen);
	if (newsockfd < 0) {
		perror("connection error");
		errno = 0;
		return;
	}
    if (newsockfd < 0)
    	ErrorExit("server: accept error");
	ProcessConnect(newsockfd, cli_addr.sin_addr.s_addr);
}
