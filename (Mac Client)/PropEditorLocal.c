#include "PropEditor.h"

#include "U-USER.H"
#include "AppMenus.h"


// PropEditorLocal.c		PALACE VERSION
void InitPropEditor()
{
	gCurrentDrawTool = PT_Pencil;
	gCurLayer = 1;
	gCurPenSize = 1;
}

Boolean PEProcessKey(EventRecord *theEvent)
{
	char	c,theCode;
	c = theEvent->message & charCodeMask;
	theCode = (theEvent->message & keyCodeMask) >> 8;

	if (gPEPtr->nameActive && c != '\r') {
		PreparePETextColors();


		switch (theCode) {
		case 0x73:		// Home
				TESetSelect(0,0,gPEPtr->nameTEH);
				break;
		case 0x77:		// End
				TESetSelect(32767,32767,gPEPtr->nameTEH);
				break;
		case 0x75:		// KP Del
			if ((*gPEPtr->nameTEH)->selEnd > (*gPEPtr->nameTEH)->selStart) {
				TEKey('\b',gPEPtr->nameTEH);
			}
			else {
				TESetSelect((*gPEPtr->nameTEH)->selStart,(*gPEPtr->nameTEH)->selStart+1,gPEPtr->nameTEH);
				if ((*gPEPtr->nameTEH)->selEnd > (*gPEPtr->nameTEH)->selStart)
					TEKey('\b',gPEPtr->nameTEH);
			}
			break;
		default:
			TEKey(c,gPEPtr->nameTEH);
		}
		RestorePEColors();
		return true;
	}

	switch (theCode) {
	case 0x33:
	case 0x75:
		if (EmptyRect(&gPEPtr->selectR) || gPEPtr->offSel == NULL)
			return true;
		ClearPropSelection();
		break;
	case 0x7e:	// up
		MovePropFrame(0,-1);
		break;
	case 0x7d:	// down
		MovePropFrame(0,1);
		break;
	case 0x7b:	// left
		MovePropFrame(-1,0);
		break;
	case 0x7c:	// right
		MovePropFrame(1,0);
		break;
	default:
		return false;
	}
	return true;
}



