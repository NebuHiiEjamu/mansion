// C-Local.h for before U-USER.H
//
// Mac Specific Functions for Client
#ifndef _C_LOCAL
#define _C_LOCAL	1

#define PPASUPPORT	1		// 6/27/96 JAB Supporting PPA Plug-in Interface
#define KIDS_CLIENT 0		// 2/19/97 JAB Kids Only Sites...

#define PicturesPathList	150
#define SoundsPathList		151
#define MoviesPathList		152

#define IsAppIconic()	gIconized
#define IsAppSuspended()	gSuspended
#define LPUSER			LocalUserRecPtr
#define FontInfoMac		FontInfo
#define GetFontInfoMac	GetFontInfo

PicHandle	GetPictureFromFile(StringPtr fName);

extern CTabHandle	gCurCLUT;	// 4/4/95 JBUM

// Tools Icons
enum {
      PencilIcon,
      LineIcon,
	  DetonatorIcon,
      BackgroundIcon,
      ForegroundIcon,
      SelectIcon,
      EraserIcon,
      ResizeIcon,
      FreePolyIcon,
      PPaletteIcon,
      SatchelIcon,
      TrashIcon,
      FaceIcon,
      PPaletteHiIcon,
      SatchelHiIcon,
      TrashHiIcon,
      FaceHiIcon,
	  KillIcon,     KillHiIcon,
      GoIcon,       GoHiIcon,
	  CancelIcon,   CancelHiIcon,
      SaveIcon,	    SaveHiIcon,
      NewIcon,	    NewHiIcon,
      EditIcon,     EditHiIcon,
      CopyIcon,		CopyHiIcon,
      DeleteIcon,	DeleteHiIcon,
      NakedIcon,	NakedHiIcon,
      AddIcon,		AddHiIcon
      };

enum {WT_Face,
      WT_Satchel,
	  WT_PPalette,
      WT_Trash,
      NbrWindowTools};

enum {HandCursor,
      EyedropperCursor,
      CrosshairsCursor,
      PencilCursor,
      BrushCursor,
	  IBeamCursor,
      EraserCursor,
      PaintBucketCursor,
      OpenHandCursor,
      CloseHandCursor,
      WatchCursor1,		// jab 6/9/95
      WatchCursor2,		// jab 6/9/95
      WatchCursor3,		// jab 6/9/95
      WatchCursor4,		// jab 6/9/95
      NbrCursors};

#define NbrWatchCursors	4
#endif
