/******************************************************************************************
 * esr - Done!
 * esr - Changed:	* Removed ErrorMessage, ReportError, ErrorExit,
 *
 ******************************************************************************************/

#include "mansion.h"

void EncryptString(StringPtr inStr, StringPtr outStr);
LONG GetSRand(void);
void MySRand(LONG s);
void Randomize(void);
LONG LongRandom(void);
double DoubleRandom(void);
short MyRandom(short max);
void EncryptString(StringPtr inStr, StringPtr outStr);
void DecryptString(StringPtr inStr, StringPtr outStr);
void EncryptCString(unsigned char *inStr, unsigned char *outStr, int len);
void DecryptCString(unsigned char *inStr, unsigned char *outStr, int len);
Boolean EqualPString(StringPtr inStr, StringPtr outStr, Boolean caseSens);
int stricmp(const char *str1,const char *str2);
int strincmp(const char *str1,const char *str2, int len);	/* 6/15/95 */
char *CvtToCString(StringPtr str);
unsigned LONG SwapLong(unsigned LONG *n);
unsigned short SwapShort(unsigned short *n);
unsigned LONG LongSwap(unsigned LONG n);
Boolean LittleEndian();

/******************************************************************************************/
/*  Random number generator
 *       Source:  Stephen K. Park and Keith W. Miller, 
 *                "Random Number Generators:  Good Ones Are Hard to Find", 
 *                Communications of the ACM,
 *                vol. 31, p. 1192  (October 1988).
 */
#define	R_A	16807L
#define	R_M	2147483647L
#define R_Q	127773L
#define R_R	2836L

LONG 	gSeed = 1;

/******************************************************************************************/

LONG GetSRand(void)
{
	return gSeed;
}

/******************************************************************************************/

void MySRand(LONG s)
{
	gSeed = s;
	if (gSeed == 0)
		gSeed = 1;
}

/******************************************************************************************/

void Randomize(void)
{
	unsigned LONG t;

	GetDateTime(&t);
	MySRand(t);
}

/******************************************************************************************/

LONG LongRandom(void)
{
	LONG	hi,lo,test;

	hi   = gSeed / R_Q;
	lo   = gSeed % R_Q;
	test = R_A * lo - R_R * hi;
	if (test > 0)
    gSeed = test;
	else
	  gSeed = test + R_M;
	return gSeed;
}

/******************************************************************************************/

double DoubleRandom(void)
{
	return LongRandom() / (double) R_M;
}

/******************************************************************************************/

short MyRandom(short max)
{
	return (short) (DoubleRandom() * max);
}

/******************************************************************************************/

/* JAB 7/20/96 */
/* Implemented Random Number Table to speed encryptions */
/* short	gEncryptTable[512]; */
short	far *gEncryptTable;
void 	InitializeEncryption();

void InitializeEncryption()
{
	int		i;
	LONG	saveSeed;

	saveSeed = GetSRand();
	MySRand(666666L);

	gEncryptTable = (short far *) NewPtrClear(sizeof(short)*512);

	for (i = 0; i < 512; ++i)
		gEncryptTable[i] = MyRandom(256);

	MySRand(saveSeed);
}

void EncryptString(StringPtr inStr, StringPtr outStr)
{
	int			    i,rc=0;
	/* LONG	saveSeed;
	 */
	unsigned char	lastChar=0;

	/* saveSeed = GetSRand();	7/20/96 changed to use table JAB
	 * MySRand(666666L);
	 */
	outStr[0] = inStr[0];
	for (i = inStr[0]; i > 0; --i)
    {
		/* outStr[i] =  inStr[i] ^ MyRandom(256) ^ lastChar;    7/20/96 removed
		 * lastChar = outStr[i] ^ MyRandom(256);
		 */
		outStr[i] =  inStr[i] ^ gEncryptTable[rc++] ^ lastChar;
		lastChar = outStr[i] ^ gEncryptTable[rc++];
	}
	/* MySRand(saveSeed); 7/20/96 */
}

/******************************************************************************************/

void DecryptString(StringPtr inStr, StringPtr outStr)
{
	int		i,rc=0;
	/* LONG	saveSeed;
	 */
	unsigned char	lastChar=0,tmp;

	/* saveSeed = GetSRand();
	 * MySRand(666666L);
	 */
	outStr[0] = inStr[0];
	for (i = inStr[0]; i > 0; --i) {
		tmp = inStr[i];
		outStr[i] = inStr[i] ^ gEncryptTable[rc++] ^ lastChar;
		lastChar = tmp ^ gEncryptTable[rc++];
	}
	/* MySRand(saveSeed); */
}

/******************************************************************************************/

void EncryptCString(unsigned char *inStr, unsigned char *outStr, int len)
{
	int		i,rc=0;
	/* LONG	saveSeed;
	 */
	 unsigned char	lastChar=0;
	 
	/* saveSeed = GetSRand();
	 * MySRand(666666L);
	 */
	for (i = len-1; i >= 0; --i) {
		outStr[i] =  inStr[i] ^ gEncryptTable[rc++] ^ lastChar;
		lastChar = outStr[i] ^ gEncryptTable[rc++];
	}
	outStr[len] = 0;
	/* MySRand(saveSeed); */
}

/******************************************************************************************/

void DecryptCString(unsigned char *inStr, unsigned char *outStr, int len)
{
	int		i,rc=0;
	/* LONG	saveSeed;
	 */
	unsigned char	lastChar=0,tmp;

	/* saveSeed = GetSRand();
	 * MySRand(666666L);
	 */
	for (i = len-1; i >= 0; --i) {
		tmp = inStr[i];
		outStr[i] = inStr[i] ^ gEncryptTable[rc++] ^ lastChar;
		lastChar = tmp ^ gEncryptTable[rc++];
	}
	outStr[len] = 0;
	/* MySRand(saveSeed); */
}

/******************************************************************************************/

Boolean EqualPString(StringPtr inStr, StringPtr outStr, Boolean caseSens)
{
	short i;
	if (inStr[0] != outStr[0])
		return false;
	for (i = 1; i <= inStr[0]; ++i) {
		if (caseSens || !isalpha(inStr[i])) {
			if (inStr[i] != outStr[i])
				return false;
		}
		else {
			if (tolower(inStr[i]) != tolower(outStr[i]))
				return false;
		}
	}
	return true;
}

/******************************************************************************************/

int stricmp(const char *str1,const char *str2)
{
	while (*str1) {
		if (isalpha(*str1)) {
			if (toupper(*str1) != toupper(*str2))
				return 1;
		}
		else if (*str1 != *str2)
			return 1;
		++str1;
		++str2;
	}
	if (*str1 == 0 && *str2 == 0)
		return 0;
	else
		return 1;
}

int strincmp(const char *str1,const char *str2, int len)	/* 6/15/95 */
{
	while (*str1 && len--) {
		if (isalpha(*str1)) {
			if (toupper(*str1) != toupper(*str2))
				return 1;
		}
		else if (*str1 != *str2)
			return 1;
		++str1;
		++str2;
	}
	return 0;
}

/******************************************************************************************/


/* Conversion function for printing up to 4 pascal strings with printf */

#define MaxCCStrings	4

char *CvtToCString(StringPtr str)
{
	static	int	ctr=0;
	static	char	buf[MaxCCStrings][256], *p;

	ctr += 1;
	ctr %= MaxCCStrings;
	p = &buf[ctr][0];
	BlockMove(&str[1],p,str[0]);
	p[str[0]] = 0;
	return p;
}

/* 9/8/95 - moved Swap routines to M-UTILS.c */
unsigned LONG SwapLong(unsigned LONG *n);
unsigned LONG SwapLong(unsigned LONG *n)
{
	*n =  ((*n >> 24) & 0x000000FFL) |
			((*n >> 8)  & 0x0000FF00L) |
			((*n << 8)  & 0x00FF0000L) |
			((*n << 24) & 0xFF000000L);
	return *n;
}

unsigned short SwapShort(unsigned short *n)
{
	*n = (unsigned short)( ((*n >> 8) & 0x00FFL) |	((*n << 8)  & 0xFF00L));
	return *n;
}

/* this (portable) function returns TRUE if the machine is
   LittleEndian (PC Intel etc.)
 */
Boolean LittleEndian()
{
    LONG n = 1;
    char *x;
    x = (char *) &n;
    return (x[0] == 1);
}

/* same as SwapLong, but doesn't use address */
unsigned LONG LongSwap(unsigned LONG n)
{
	n =  ((n >> 24) & 0x000000FFL) |
			((n >> 8)  & 0x0000FF00L) |
			((n << 8)  & 0x00FF0000L) |
			((n << 24) & 0xFF000000L);
	return n;
}


