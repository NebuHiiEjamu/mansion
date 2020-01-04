// Elizabot.c
//
// from Wheel of Cheese
//
// Basic Eliza functionality for responding to conversation
//

#include "WOFBot.h"

enum {
	canYou, canI, youAre, iDont, iFeel, whyDontYou, whyCantI, areYou, iCant, 
	iRemember, doYouRemember, whatIf, iDreamt, dreamAbout, myMother, myFather, iAmGlad,
	iAmSad, areLike, isLike, iWas, wasI, amI, wereYou, iYou, someone, everyone, are,
	iAm, iWant, computer, question, name, because, sorry, dream, hello, maybe, always,
	think, alike, you, your, yes, no, friends, wildcard};

struct rules {
	short	tNbr;
	short	nbrResponses;
	char	*resp[10];
};

struct Trigger {
	char	*key;
	short	trigNbr;
};

struct Trigger triggers[] = {
	"* can you *", canYou,
	"* can i *", canI,
	"* you are *", youAre,
	"* you're *", youAre,
	"* i don't *", iDont,
	"* i feel *", iFeel,
	"* i felt *", iFeel,
	"* why don't you *", whyDontYou,
	"* why can't i *", whyCantI,
	"* are you *", areYou,
	"* i can't *",iCant,
	"* i remember *",iRemember,
	"* do you remember *", doYouRemember,
	"* if *", whatIf,
	"* my mother *", myMother,
	"* my father *", myFather,
	"* i am glad *", iAmGlad,
	"* i am happy *", iAmGlad,
	"* i'm glad *", iAmGlad,
	"* i'm happy *", iAmGlad,
	"* i am sad *", iAmSad,
	"* are like *", areLike,
	"* is like *", isLike,
	"* i was *", iWas,
	"* was i *", wasI,
	"* am i *", amI,
	"* were you *", wereYou,
	"* i * you *", iYou,
	"* everyone *", everyone,
	"* because *", because,
	"* cause *", because,
	"* i am *", iAm,
	"* i'm *", iAm,
	"* i want *",iWant,
	"* computer *", computer,
	"* computers *", computer,
	"* mac *", computer,
	"* what *", question,
	"* how *", question,
	"* who *", question,
	"* where *", question,
	"* when *", question,
	"* why *", question,
	"* name *", name,
	"* sorry *", sorry,
	"* dream *", dream,
	"* dreams *", dream,
	"* i dreamt *", iDreamt,
	"* dream about *", dreamAbout,
	"* hello *", hello,
	"* hi *", hello,
	"* maybe *", maybe,
	"* perhaps *", maybe,
	"* always *", always,
	"* think *", think,
	"* alike *", alike,
	"* same *", alike,
	"* friend *", friends,
	"* friends *", friends,
	// Cheezier Responses
	"* someone *", someone,
	"* you *", you,
	"* your *", your,
	"* yes *", yes,
	"* no *", no,
	"* are *", are,
	"*", wildcard,
	NULL, 0,
};
	

struct rules rulez[] = {

{canYou,0, "Don't you believe that I can *2?",
		"Perhaps you would like to be able to *2?",
		"You want me to be able to *2?" },
{canI,0, "Perhaps you don't want to *2?",
		"Do you want to be able to *2?" },
{youAre,0, "What makes you think I am *2?",
		"Does it please you to believe I am *2?",
		"Perhaps you would like to be *2?",
		"Do you sometimes wish you were *2?" },
{iDont,0, "Don't you really *2?",
		"Why don't you *2?",
		"Do you wish to be able to *2?",
		"Does that trouble you?" },
{iFeel,0, "Tell me more about such feelings.",
		"Do you often feel *2?",
		"Do you enjoy feeling *2?",
		"What other feelings do you have?" },
{whyDontYou ,0, "Do you really believe I don't *2?",
		"Perhaps I will *2 in good time.",
		"Do you want me to *2?",
		"Should you *2 yourself?" },
{whyCantI,0, "Do you think you should be able to *2?",
		"Why can't you *2?" },
{areYou,0, "Why are you interested in whether I am *2 or not?",
		"Would you prefer if I were not *2?",
		"Perhaps I am *2 in your fantasies?" },
{iCant,0, "How do you know you can't *2?",
		"Have you tried?",
		"Perhaps you could *2 now?",
		"What if you could *2?" },
{iRemember, 0, "Do you often think of *2?",
		"Does thinking of *2 bring anything else to mind?",
		"What else do you remember?",
		"Why do you recall *2 right now?",
		"What in the present situation reminds you of *2?",
		"What is the connection between and *2?"},
{doYouRemember, 0, "Did you think I would forget *2?",
		"What about *2?",
		"Why do you think I should recall *2 now?",
		"You mentioned *2?"},
{whatIf, 0, "Do you really think it's likely that *2?",
		"Do you wish that *2?",
		"What do you think about *2?",
		"Really... if *2"},
{iDreamt, 0, "Really... *2",
			"Have you ever fantasized *2 while you were awake?",
			"Have you dreamt about *2 before?"},
{dreamAbout, 0, "How do you feel about *2 in reality?"},

{myMother, 0, "Who else in your family *2?",
			"Tell me more about your family"},
{myFather, 0, "Your father.",
			"Does he influence you strongly?",
			"What else comes to mind when you think of your father?"},
{iAmGlad, 0, "How have I helped you to be *2?",
			"What makes you happy just now?",
			"Can you explain?"},
{iAmSad, 0, "I am sorry to hear that",
			"I'm sure it's not pleasant"},
{areLike, 0, "What resemblance do you see between *1 and *2"},
{isLike, 0, "In what way is *1 like *2?",
			"What resemblance do you see?",
			"Could there really be some connection?",
			"How?"},
{iWas,0,	"Were you really?",
			"Perhaps I already knew you were *2",
			"Why do you tell me this now?"},
{wasI,0,	"What if you were *2?",
			"Do you think you were *2?",
			"What would it mean if you were *2?"},
{amI,0,		"Do you believe you are *2?",
			"Would you want to be *2?",
			"You wish I would tell you you are *2",
			"What would it mean if you were *2?"},
{wereYou,0,	"Perhaps I was *2",
			"What do you think?",
			"What if I had been *2?"},
{iYou,0,	"Perhaps in your fantasy we *2 each other"},
{someone,0,	"Could you be more specific?"},
{everyone,0,"Surely not everyone",
			"Can you think of anyone in particular?",
			"Who for example?",
			"Who specifically?"},
{are,0,		"Did you think they might not be *2?",
			"Possibly they are *2"},
{iAm,0, "Did you come to me because you are *2?",
		"How long have you been *2?",
		"Do you believe it is normal to be *2?",
		"Do you enjoy being *2?" },
{iWant,0, "Why do you want *2?",
		"What would it mean if you got *2?",
		"Suppose you got *2 soon?",
		"What if you never got *2?",
		"I sometimes also want *2" },
{computer ,0, "Do computers worry you?",
		"Are you talking about me in particular?",
		"Are you frightened by machines?",
		"Why do you mention computers?",
		"What do you think machines have to do with your problems?",
		"Don't you think computers can help people?",
		"What is it about machines that worries you?" },
{question,0, "Why do you ask?",
		"Does that question interest you?",
		"What answer would please you the most?",
		"What do you think?",
		"Are such questions on your mind often?",
		"What is it that you really want to know?",
		"Have you asked anyone else?",
		"Have you asked such questions before?",
		"What else comes to mind when you ask that?" },
{name,0, "Names don't interest me.",
		"I don't care about names --please go on." },
{because,0, "Is that the real reason?",
		"Don't any other reasons come to mind?",
		"Does that reason explain anything else?",
		"What other reasons might there be?",
		"Why is that?" },
{sorry,0, "Please don't apologize!",
		"Apologies are not necessary.",
		"What feelings do you have when you apologise?",
		"Don't be so defensive!" },
{dream,0, "What does that dream suggest to you?",
		"Do you dream often?",
		"What persons appear in your dreams?",
		"Are you disturbed by your dreams?" },
{hello,0, "How do you...please state your problem." },
{maybe,0, "You don't seem quite certain.",
		"Why the uncertain tone?",
		"Can't you be more positive?",
		"You aren't sure?",
		"Don't you know?" },
{always,0, "Can you think of a specific example?",
		"When?",
		"What are you thinking of?",
		"Really, always?" },
{think,0, "Do you really think so?",
		"But you are not sure you *2?",
		"Do you doubt you *2?" },
{alike,0, "In what way?",
		"What resemblence do you see?",
		"What does the similarity suggest to you?",
		"What other connections do you see?",
		"Cound there really be some connection?",
		"How?" },
{you,0, "We were discussing you --not me.",
		"Oh... *2",
		"You're not really talking about me, are you?" },
{your,0, "Why are you concerned about my *2?",
		"What about your own *2?" },
{yes,0, "You seem quite positive.",
		"Are you Sure?",
		"I see.",
		"I understand.",
		"Why?" },
{no,0, "Are you saying no just to be negative?",
		"You are being a bit negative.",
		"Why not?",
		"Are you sure?",
		"Why?"},
{friends,0, "Why do you bring up the topic of friends?",
		"Do your friends worry you?",
		"Do your friends pick on you?",
		"Are you sure you have any friends?",
		"Do you impose on your friends?",
		"Perhaps your love for friends worries you." },

{wildcard,8, ""},
 {-1}
};

// Note: Need to fix 1/way and 2/way conjugations.
char	*conjP[][2] = {
	"i",	"you",
	"i'm",	"you're",
	"i've",	"you've",
	"i'll",	"you'll",
	"my",	"your",
	"me",	"you",
	"am",	"are",
	"were",	"were",		// I were  --> You were
	"was",	"were",
	NULL,		NULL};


void Eliza(char *q, char *a)
{
	short	i,trigNbr,nw;
	Boolean	trigFlag = false;
	char	*sp,*dp,*pp,*ssp,*ppp;
	char	*fillin[4];
	char	tBuf[255];
	static	Boolean	initFlag;
	a[0] = 0;

	// Initialize Rules Table
	if (!initFlag) {
		initFlag = true;
		for (i = 0; rulez[i].tNbr != -1; ++i) {
			for (nw = 0; rulez[i].resp[nw]; ++nw)
				;
			rulez[i].nbrResponses = nw;
		}
	}
	// strip punctuation, convert double spaces to single, convert to lower
	dp = q;
	for (sp = q; *sp; ++sp) {
		if (*sp == '\r')
			break;
		if (isspace(*sp)) {
			*(dp++) = ' ';
			++sp;
			while (isspace(*sp))
				++sp;
			--sp;
			continue;
		}
		if (*sp == '.' || *sp == ',' || *sp == '?' || *sp == '!')
			continue;
		*(dp++) = tolower(*sp);
	}
	*dp = 0;


	// Try Rule Matching
	trigFlag = false;
	for (i = 0; triggers[i].key; ++i) {
		pp = triggers[i].key;
		strcpy(tBuf,q);
		sp = tBuf;
		ppp = pp;
		nw = 0;
		ssp = NULL;
		while (*pp && *sp) {
			if (*pp == '*') {
				if (nw > 0 && ssp && ssp > tBuf)	// Terminate previous wild phrase
					*(ssp - 1) = 0;
				++pp;
				fillin[nw] = sp;
				++nw;
				ppp = pp;
				ssp = sp;
				if (sp == tBuf)	// Skip initial space in pattern at beg of line
					++pp;
			}
			else if (*sp != *pp) {
				pp = ppp;
				sp = ssp+1;
				ssp = sp;
			}
			else {
				++sp;
				++pp;
				if (*sp == 0 && *((short *) pp) == ' *') {
					if (nw > 0 && ssp && ssp > tBuf && *ssp == ' ')	// Terminate previous wild phrase
						*ssp = 0;
					pp += 2;
					break;
				}
			}
		}
		if (*pp == 0) {
			trigFlag = true;
			trigNbr = triggers[i].trigNbr;
			fillin[nw] = sp + strlen(sp);
			break;
		}
	}
	if (trigFlag == false)
		return;

	// Output reponse, doing conjugations as needed for fill-ins
	pp = rulez[trigNbr].resp[((unsigned) Random()) % rulez[trigNbr].nbrResponses];
	dp = a;
	for (; *pp; ++pp) {
		if (*pp == '*') {
			short	n;
			++pp;
			n = *pp - '1';
			if (n >= nw)
				continue;

			// Conjugate...
			sp = fillin[n];
			while (*sp && sp < fillin[n+1]) {
				if (sp == fillin[n] || *(sp-1) == ' ') {
					// Check if conjugate, if so substitute word
					short	len;
					Boolean	conjFlag = false;
					for (i = 0; conjP[i][0]; ++i) {
						if (strncmp(sp, conjP[i][0], (len = strlen(conjP[i][0]))) == 0 &&
							(sp[len] == ' ' || sp[len] == 0)) {
							strcpy(dp, conjP[i][1]);
							dp += strlen(dp);
							sp += len;
							conjFlag = true;
							break;
						}
						else if (strncmp(sp, conjP[i][1], (len = strlen(conjP[i][1]))) == 0 &&
							(sp[len] == ' ' || sp[len] == 0)) {
							strcpy(dp, conjP[i][0]);
							dp += strlen(dp);
							sp += len;
							conjFlag= true;
							break;
						}
					}
					if (conjFlag == false)
						*(dp++) = *(sp++);
				}
				else
					*(dp++) = *(sp++);
			}
		}
		else
			*(dp++) = *pp;
	}
	*dp = 0;
}
