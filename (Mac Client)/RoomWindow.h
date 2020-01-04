// RoomWindow.h

#include "Mansion.h"
#include <QDOffscreen.h>
#include <Picker.h>

#define NbrShades	16
#define MaxPictures	128
#define RoomWidth	512
#define RoomHeight	384
#define FaceWidth	44
#define FaceHeight	44

typedef struct {
	ObjectWindowRecord	objWin;
	GWorldPtr			offWorld;		// Offscreen Rendering
	GWorldPtr			offPicture;		// Background Picture
	GWorldPtr			offFaces;		// Grayscale faces (4-bit) & Props
	GWorldPtr			offMask;
	PixMapHandle		offPixMap,offPictureMap,offFacesMap,offMaskMap;

	// 4-bit gray -> 8-bit system color translations for colorizing faces
	unsigned char		cTrans[NbrColors][NbrShades];
	short				nbrFaces;

	RoomRec				curRoom;
	UserRec				userList[MaxPeoplePerRoom+4];
	UserRecPtr			mePtr;
	long				meID;
	long				targetID;
	Rect				msgRect;
	TEHandle			msgTEH;
	Boolean				msgActive;
} RoomWindowRec, *RoomWindowPtr;

extern RoomWindowRec	*gRoomWin;

void NewRoomWindow();

