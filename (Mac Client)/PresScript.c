#include "PCast.h"
#include "PresScript.h"
#include "MainWin.h"
#include <ctype.h>

PresScript	*ps;

void PresScript::AddImageNode(StringPtr imageName, long effect)
{
	Node	*node;
	Point	center = {-1,-1};

	node = NewNode();
	AddNodeEvent(node, NT_Clear, NULL, 0L);
	AddNodeEvent(node, NT_LoadImage, imageName, *((long *) &center));
	AddNodeEvent(node, NT_Effect, NULL, effect);
}

void InitializePCastScript()
{
	// Point	center = {-1,-1};
	// Node	*node;

	firstNode = NULL;
	lastNode = NULL;
	curNode = NULL;
	gSP = gScriptBuffer = NULL;
	ungetFlag = false;
	
	// Load Script, building list of nodes
	// Sample
	ReadScriptFile("\pPresScript");
}

Boolean PresScript::GetToken(void)
{
        char    *dp;

        if (ungetFlag) {
                ungetFlag = false;
                return TRUE;
        }
reget:
        if (*gSP == 0)
                return FALSE;
        else if (isspace(*gSP)) {
                ++gSP;
                goto reget;
        }
        else if (*gSP == '#' || *gSP == ';') {
                while (*gSP && *gSP != '\r' && *gSP != '\n')
                        ++gSP;
                goto reget;
        }
        else if (isalnum(*gSP) || *gSP == '_' || *gSP == '.' || (*gSP == '-' && isdigit(gSP[1])))
        {
                dp = gToken;
                if (*gSP == '-')
                        *(dp++)  = *(gSP++);
                while (isalnum(*gSP) || *gSP == '_' || *gSP == '.')
                        *(dp++)  = *(gSP++);
                *dp = 0;
        }
        else if (*gSP == '\"') {
                dp = gToken;
                *(dp++) = *(gSP++);
                while (*gSP && *gSP != '\"') {
                        if (*gSP == '\\') {
                                ++gSP;
                                if (*gSP == '\\' || *gSP == '\"')
                                        *(dp++)  = *(gSP++);
                                else  {
                                        int     t;
                                        sscanf((char *)gSP,"%2X",&t);
                                        *(dp++) = t;
                                        gSP += 2;
                                }
                        }
                        else
                                *(dp++)  = *(gSP++);
                }
                if (*gSP == '\"')
                        *(dp++) = *(gSP++);
                *dp = 0;
        }
        else if (strchr("{}[](),",*gSP)) {
                gToken[0] = *(gSP++);
                gToken[1] = 0;
        }
        else if (ispunct(*gSP)) {
                dp = gToken;
                while (ispunct(*gSP) || *gSP == '_')
                        *(dp++)  = *(gSP++);
                *dp = 0;
        }
        else {
			SysBeep(1);
            return false;
        }
        return true;
}

int PresScript::GetInteger()
{
	int	i;
    GetToken();
    if (isdigit(gToken[0]) || gToken[0] == '-')
           i = atoi(gToken);
    else
            i = 0;
	return i;
}

Boolean PresScript::GetPString(StringPtr str)
{
        unsigned char   *dp;
        unsigned char   *len;
reget:
        if (*gSP == 0)
                return FALSE;
        else if (isspace(*gSP)) {
                ++gSP;
                goto reget;
        }
        else if (*gSP == '#' || *gSP == ';') {
                while (*gSP && *gSP != '\r' && *gSP != '\n')
                        ++gSP;
                goto reget;
        }
        else if (isalnum(*gSP) || *gSP == '_')
        {
                len = (unsigned char *)str++;
                dp = (unsigned char     *)str;
                len[0] = 0;
                while (isalnum(*gSP) || *gSP == '_') {
                        *(dp++)  = *(gSP++);
                        len[0]++;
                }
                /* *dp = 0; */
        }
        else if (*gSP == '\"') {
                len = (unsigned char    *)str++;
                dp = (unsigned char     *)str;
                len[0] = 0;
                ++gSP;
                while (*gSP && *gSP != '\"') {
                        if (*gSP == '\\') {
                                ++gSP;
                                if (*gSP == '\\' || *gSP == '\"')
                                        *(dp++)  = *(gSP++);
                                else  {
                                        int     t;
                                        sscanf((char *)gSP,"%2X",&t);
                                        *(dp++) = t;
                                        gSP += 2;
                                }
                        }
                        else
                                *(dp++)  = *(gSP++);
                        len[0]++;
                }
                if (*gSP == '\"')
                        ++gSP;
                /* *dp = 0; */
        }
        else {
             SysBeep(1);
             return FALSE;
        }
        return TRUE;
}


char *PresScript::SuckTextFile(StringPtr name)
{
	OSErr		oe;
	short	iFile;
	char		*fileBuffer;
	long		length;

    if ((oe = FSOpen(name, 0, &iFile)) != noErr) {
         return NULL;
    }

    GetEOF(iFile,&length);
    fileBuffer = (char *)NewPtrClear(length+1);
    FSRead(iFile,&length,(char *)fileBuffer);
    FSClose(iFile);
    fileBuffer[length] = 0;
    return fileBuffer;
}

Node *PresScript::NewNode()
{
	Node	*node;
	node = (Node *) NewPtrClear(sizeof(Node));
	if (lastNode) {
		lastNode->nextNode = node;
		node->prevNode = lastNode;
		lastNode = node;
	}
	else {
		firstNode = lastNode = node;
	}
	return node;
}

NodeEvent *PresScript::NewNodeEvent(Node *node)
{
	NodeEvent	*nEvent;
	nEvent = (NodeEvent *) NewPtrClear(sizeof(NodeEvent));
	if (node->lastNEvent) {
		node->lastNEvent->nextNEvent = nEvent;
		nEvent->prevNEvent = node->lastNEvent;
		node->lastNEvent = nEvent;
	}
	else {
		node->firstNEvent = node->lastNEvent = nEvent;
	}
	return nEvent;
}


void PresScript::AddNodeEvent(Node *node, int eventType, StringPtr text, long arg1)
{
	NodeEvent	*nEvent;
	nEvent = NewNodeEvent(node);
	if (text && text[0]) {
		nEvent->text = (StringPtr) NewPtrClear(text[0]+1);
		BlockMove(text,nEvent->text,text[0]+1);
	}
	nEvent->eventType = eventType;
	nEvent->arg1 = arg1;
}

void PresScript::FirstScreen()
{
	RunNode(firstNode);
}

void PresScript::NextScreen(Boolean keyFlag)
{
	Node	*node;
	if (curNode == NULL)
		RunNode(firstNode);
	else if (curNode->nextNode) {
		if (!keyFlag)
			node = curNode->nextNode;
		else {
			node = curNode;
			do {
				node = node->nextNode;
			} while (!(node->flags & NF_KeyPoint) && node->nextNode);
		}
		RunNode(node);
	}
	else
		RunNode(firstNode);
}

void PresScript::PrevScreen(Boolean keyFlag)
{
	Node	*node;
	if (curNode == NULL)
		RunNode(lastNode);
	else if (curNode->prevNode) {
		if (!keyFlag)
			node = curNode->prevNode;
		else {
			node = curNode;
			do {
				node = node->prevNode;
			} while (!(node->flags & NF_KeyPoint) && node->prevNode);
		}
		RunNode(node);
	}
	else
		RunNode(lastNode);
}

void PresScript::RunNode(Node *node)
{
	NodeEvent *nEvent;
	Boolean	display=false;
	curNode = node;
	
	for (nEvent = curNode->firstNEvent; nEvent; nEvent = nEvent->nextNEvent)
	{
		switch (nEvent->eventType) {
		case NT_Clear:
			mw->EraseOffscreen();
			display=false;
			break;
		case NT_LoadImage: 
			{
				mw->LoadImage(nEvent->text,*((Point *) &nEvent->arg1));
				display=false;
			}
			break;
		case NT_Effect:
			mw->DisplayWithEffect(nEvent->arg1);
			display = true;
			break;
		case NT_Delay:
			long	startTicks;
			startTicks = TickCount();
			while (TickCount() - startTicks < nEvent->arg1 && !Button())
				;
			break;
		case NT_PlayMovie:
			// mw->PlayMove(nEvent->fileName, nEvent->arg1);
			break;
		case NT_Text:
			mw->DisplayText(nEvent->text,*((Point *) &nEvent->arg1));
			break;
		case NT_SetFont:
			mw->SetDefaultFont(nEvent->text);
			break;
		case NT_SetSize:
			mw->SetDefaultSize(nEvent->arg1);
			break;
		case NT_SetMode:
			mw->SetDefaultMode(nEvent->arg1);
			break;
		case NT_SetColor:
			mw->SetDefaultColor(nEvent->text);
			break;
		}
	}
	if (!display) {
		mw->DisplayWithEffect(ET_Normal);
	}
}

void PresScript::ReadScriptFile(StringPtr name)
{
	Node	*node;
	Str255	imageName;
	char	effectName[63];
	long	effectNbr;
	Point	center = {-1,-1};

	gScriptBuffer = SuckTextFile(name);
	if (gScriptBuffer == NULL) {
         return;
    }
    gSP = gScriptBuffer;
    while (GetToken()) {
		if (strcmp(gToken,"EIMAGE") == 0) {
			GetToken();	
			BlockMove(gToken,effectName,strlen(gToken)+1);
			GetPString(imageName);
			if (strcmp(effectName,"FADE") == 0)
				effectNbr = ET_BlackFade;
			else if (strcmp(effectName,"DISSOLVE") == 0)
				effectNbr = ET_PixDissolve;
			else if (strcmp(effectName,"NORMAL") == 0)
				effectNbr = ET_Normal;
			else
				effectNbr = ET_Normal;
			node = NewNode();
			AddNodeEvent(node, NT_Clear, NULL, 0L);
			AddNodeEvent(node, NT_LoadImage, imageName, *((long *) &center));
			AddNodeEvent(node, NT_Effect, NULL, effectNbr);
		}
		else if (strcmp(gToken,"NODE") == 0)
			node = NewNode();
		else if (strcmp(gToken,"CLEAR") == 0)
			AddNodeEvent(node, NT_Clear, NULL, 0);
		else if (strcmp(gToken,"DELAY") == 0)
			AddNodeEvent(node, NT_Delay, NULL, GetInteger());
		else if (strcmp(gToken,"IMAGE") == 0) {
			Point	pos;
			pos.h = GetInteger();
			pos.v = GetInteger();
			GetPString(imageName);
			AddNodeEvent(node, NT_LoadImage, imageName, *((long *) &pos));
		}
		else if (strcmp(gToken,"EFFECT") == 0) {
			GetToken();	
			BlockMove(gToken,effectName,strlen(gToken)+1);
			if (strcmp(effectName,"FADE") == 0)
				effectNbr = ET_BlackFade;
			else if (strcmp(effectName,"DISSOLVE") == 0)
				effectNbr = ET_PixDissolve;
			else if (strcmp(effectName,"NORMAL") == 0)
				effectNbr = ET_Normal;
			else
				effectNbr = ET_Normal;
			AddNodeEvent(node, NT_Effect, NULL, effectNbr);
		}
		else if (strcmp(gToken,"TEXT") == 0) {
			Point	pos;
			pos.h = GetInteger();
			pos.v = GetInteger();
			GetPString(imageName);
			AddNodeEvent(node, NT_Text, imageName, *((long *) &pos));
		}
		else if (strcmp(gToken,"FONT") == 0 ||
				strcmp(gToken,"TEXTFONT") == 0) {
			GetPString(imageName);
			AddNodeEvent(node, NT_SetFont, imageName, 0L);
		}
		else if (strcmp(gToken,"COLOR") == 0 ||
				strcmp(gToken,"TEXTCOLOR") == 0) {
			GetPString(imageName);
			AddNodeEvent(node, NT_SetColor, imageName, 0L);
		}
		else if (strcmp(gToken,"SIZE") == 0 ||
				strcmp(gToken,"TEXTSIZE") == 0) {
			AddNodeEvent(node, NT_SetSize, NULL, GetInteger());
		}
		else if (strcmp(gToken,"MODE") == 0 ||
				strcmp(gToken,"TEXTMODE") == 0) {
			AddNodeEvent(node, NT_SetMode, NULL, GetInteger());
		}
		else if (strcmp(gToken,"KEYPOINT") == 0) {
			node->flags |= NF_KeyPoint;
		}
		else if (strcmp(gToken,"PAINT") == 0) {
			node->flags |= NF_Paint;
		}
		else if  (strcmp(gToken,"END") == 0) {
			break;
		}
    }
    DisposePtr(gScriptBuffer);
}

