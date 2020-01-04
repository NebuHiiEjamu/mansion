/* frontendcmds.h */
/* structures shared by front end and server */

typedef struct {
	short	cmd;	/* upper 8 bits used for flags */
	unsigned short	dport;
	LONG	dip;
	LONG	length;
	unsigned char	data[1];
} BiHeader;

/* bi commands */

enum {
	bi_packet,		/* 0 server <--> front end  */
	bi_global,		/* 1 server -> front endS */
	bi_room,    	/* 2 server -> front endS to all users in room (possibly excepting 1) */
    bi_serverdown,	/* 3 server -> front endS */
    bi_serverfull,	/* 4 server -> front endS */
    bi_serveravail,	/* 5 server -> front endS */
    bi_begingroup,	/* 6 server -> front end */
    bi_endgroup,	/* 7 server -> front end */
    bi_assoc,		/* 8 server -> front end associate user with room */
	bi_userflags,	/* 9 server -> front end */
	bi_addaction,	/* 10 server -> front end */
	bi_delaction,	/* 11 server -> front end */
    bi_newuser,		/* 12 server <-- front end */
    bi_kill,		/* 13 server <--> front end kill user */
	bi_frontendup,	/* 14 server <-- front end */
	bi_nbrcmds
};
