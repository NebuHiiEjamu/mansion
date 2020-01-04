/* God Commands */


enum {
/* Guests OK */
GC_help=1, 

/* Members */
GC_hide, GC_unhide, GC_hidefrom, GC_unhidefrom, GC_page, GC_respond, 
GC_mute, GC_unmute, GC_rejectesp, GC_rejectprivate, GC_newroom,
GC_rname, GC_rdelete, GC_rpicture, GC_rowner, GC_rkick, GC_runkick,
GC_rpassword, GC_rclose, GC_ropen, GC_rscripts, GC_on, GC_off,
GC_rpainting, GC_rguests, GC_rhide, GC_runhide, GC_password,

/* Wizards */
GC_repage, GC_er, GC_list, GC_glist, GC_painting, GC_bots, GC_deathpenalty, GC_floodlimit,
GC_duplicate, GC_deleteroom, GC_banlist, GC_purgebanlist, GC_unban,
GC_untrack, GC_banip, GC_trackip, GC_banuser, GC_kill, GC_track,
GC_comment, GC_extend, GC_gag, GC_ungag, GC_pin, GC_unpin, GC_propgag,
GC_unpropgag, GC_sortprops, GC_purgeprops,

/* Gods */
GC_reset, GC_ban, GC_recycle, GC_shutdown, GC_wizpassword, GC_godpassword,
GC_servername, GC_maxoccupancy, GC_defaultroomocc, GC_roommaxocc,
GC_roommaxguests, GC_picdir, GC_tcp, GC_guestaccess, GC_customprops,
GC_allowwizards, GC_wizardkill, GC_playerkill, GC_spoof, GC_memberrooms,
GC_wizardsonly, GC_uplist, GC_downlist, GC_dropzone, GC_botkill,
GC_authoring, GC_purgelimit, GC_passwordsecurity, GC_autoannounce, 
GC_killprop, GC_savesessionkeys, GC_allowdemomembers,

GC_NbrCommands};

typedef struct GC_Parse {
	char 	*keyword;
	unsigned char	cmd;
	unsigned char	userArg;
	unsigned char	rank;
} GC_Parse;

GC_Parse gPTable[] = {

/* Guest OK */
{"help",			GC_help,			0,		0},

/* Member OK	 */
{"hide",			GC_hide,			0,		1},
{"unhide",		GC_unhide,			0,		1},
{"hidefrom",		GC_hidefrom,		1,		1},
{"unhidefrom",	GC_unhidefrom,		1,		1},
{"page",			GC_page,			0,		1},
{"respond",		GC_respond,			0,		1},
{"re",			GC_respond,			0,		1},
{"mute",			GC_mute,			1,		1},
{"unmute",		GC_unmute,			1,		1},
{"rejectesp",	GC_rejectesp,		0,		1},
{"*rejectpri",	GC_rejectprivate,	0,		1},
{"newroom",		GC_newroom,			0,		1},
{"rname",		GC_rname,			0,		1},
{"rdelete",		GC_rdelete,			0,		1},
{"rpicture",		GC_rpicture,		0,		1},
{"rowner",		GC_rowner,			1,		1},
{"rkick",		GC_rkick,			1,		1},
{"runkick",		GC_runkick,			1,		1},
{"rpassword",	GC_rpassword,	0, 		1},
{"rclose",		GC_rclose,			0, 		1},
{"ropen",		GC_ropen,			0, 		1},
{"rscripts",		GC_rscripts,		0, 		1},
{"on",			GC_on,				0, 		1},
{"off",			GC_off,				0,		1},
{"rpainting",		GC_rpainting,	0, 		1},
{"rguests",		GC_rguests,			0, 		1},
{"rhide",		GC_rhide,			0, 		1},
{"runhide",		GC_runhide,			0, 		1},
{"password",		GC_password,		0,		1},

/* Wizards OK */

{"repage",		GC_repage,				0, 		2},
{"er",			GC_er,					0, 		2}, 
{"list",			GC_list,			1,  	2},
{"glist",		GC_glist,				0,		2},
{"*paint",		GC_painting,			0, 		2},
{"bots",			GC_bots,			0, 		2},
{"*death",		GC_deathpenalty,		0, 		2},
{"*flood",		GC_floodlimit,			0, 		2},
{"duplicate",	GC_duplicate,			0, 		2},
{"*delete",		GC_deleteroom,			0, 		2},
{"banlist",		GC_banlist,				1, 		2},
{"purgebanlist",	GC_purgebanlist,	0, 		2},
{"unban",		GC_unban,				1, 		2},
{"untrack",		GC_untrack,				1, 		2},
{"banip",		GC_banip,				0, 		2},
{"trackip",		GC_trackip,				0, 		2},
{"banuser",		GC_banuser,				1, 		2},
{"kill",			GC_kill,			1, 		2},
{"track",		GC_track,				1, 		2},
{"comment",		GC_comment,				0,		2},
{"extend",		GC_extend,				0, 		2}, 
{"gag",			GC_gag,					1, 		2},
{"ungag",		GC_ungag,				1, 		2},
{"pin",			GC_pin,					1, 		2},
{"unpin",		GC_unpin,				1, 		2},
{"propgag",		GC_propgag,				1, 		2},
{"unpropgag",	GC_unpropgag,			1, 		2},
{"sortprops",	GC_sortprops,			0, 		2},
{"purgeprops",	GC_purgeprops,			0, 		2},
{"autoannounce",	GC_autoannounce,	0, 		2},
{"*roommaxocc",	GC_roommaxocc,			0,		2},
{"roommaxguests",GC_roommaxguests,		0, 		2},
/* GODS */

{"reset",		GC_reset,				0, 		3},
{"ban",			GC_ban,					1, 		3}, 
{"recycle",		GC_recycle,				0,		3},
{"shutdown",		GC_shutdown,		0,		3}, 
{"*wizpass",		GC_wizpassword,		0,		3},
{"*godpass",		GC_godpassword,		0,		3},
{"servername",	GC_servername,			0,		3},
{"*maxocc",		GC_maxoccupancy,		0,		3},
{"*maxservocc",	GC_maxoccupancy,		0,		3},
{"*maxserverocc",GC_maxoccupancy,		0,		3},
{"*maxroomocc",	GC_defaultroomocc,		0,		3},
{"*defaultroomocc",	GC_defaultroomocc,	0,		3},
{"picdir",		GC_picdir,				0, 		3},
{"tcp",			GC_tcp,					0,		3},
{"guestaccess",	GC_guestaccess,			0, 		3},
{"guests",		GC_guestaccess,			0, 		3},
{"customprops",	GC_customprops,			0,		3},
{"custom",		GC_customprops,			0,		3},
{"allowwizards",	GC_allowwizards,	0, 		3},
{"wizards",		GC_allowwizards,		0, 		3},
{"wizardkill",	GC_wizardkill,			0, 		3},
{"playerkill",	GC_playerkill,			0, 		3},
{"spoof",		GC_spoof,				0, 		3},
{"memberrooms",	GC_memberrooms,			0,		3},
{"wizardsonly",	GC_wizardsonly,			0, 		3},
{"uplist",		GC_uplist,				0, 		3},
{"downlist",		GC_downlist,		0, 		3},
{"dropzone",		GC_dropzone,		0, 		3},
{"botkill",		GC_botkill,				0,		3},
{"authoring",	GC_authoring,			0, 		3},
{"author",		GC_authoring,			0, 		3},
{"purge",		GC_purgelimit,			0, 		3},
{"purgelimit",	GC_purgelimit,			0, 		3},
{"*passwordsec", GC_passwordsecurity, 	0, 		3},
{"passwordsecurity",GC_passwordsecurity, 0, 	3},
{"killprop",		GC_killprop,		0, 		3},
{"savesessionkeys", GC_savesessionkeys,	0, 		3},
{"allowdemomembers", GC_allowdemomembers, 0,	3},	/* 1/14/97 JAB */
{NULL,			0,0,0}};


