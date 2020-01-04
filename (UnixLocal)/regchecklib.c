/************************************************************************************
 * Unix Serial Number Verifier
 ************************************************************************************/
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define false	0
#define true	1

#define LONG	int
#include "m-magic.h"
 
#define OLD_MAGIC_LONG	0x21E8E72CL
#define MAGIC_LONG		0x9602c9bfL
#define CRC_MAGIC		0xa95ade76L
#define OLD_CRC_MAGIC	0xd9216290L

unsigned LONG SwapLong(unsigned LONG *n)
{
        *n =  ((*n >> 24) & 0x000000FFL) |
                        ((*n >> 8)  & 0x0000FF00L) |
                        ((*n << 8)  & 0x00FF0000L) |
                        ((*n << 24) & 0xFF000000L);
        return *n;
}

int LittleEndian()
{
    LONG n = 1;
    char *x;
    x = (char *) &n;
    return (x[0] == 1);
}

unsigned LONG ComputeLicenseCRC(unsigned LONG counter, int len)
{
	unsigned LONG crc = CRC_MAGIC;
	unsigned char *p = (unsigned char *) &counter;
	if (LittleEndian())
		SwapLong(&counter);
	while (len--)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[*(p++)];
	return crc;
}

unsigned LONG ComputeOldLicenseCRC(unsigned LONG counter, int len)
{
	unsigned LONG crc = OLD_CRC_MAGIC;
	unsigned char *p = (unsigned char *) &counter;
	if (LittleEndian()) {
		SwapLong(&counter);
	}
	while (len--)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gCRCMask[*(p++)];
	return crc;
}

/* Convert ascii character to 5-bit code. */
int ConvertAsciiToCode(short asc)
{
	static char	codeAsc[] = {"ABCDEFGHJKLMNPQRSTUVWXYZ23456789"};
	short	i;
	for (i = 0; codeAsc[i]; ++i) {
		if (codeAsc[i] == asc || (isalpha(asc) && toupper(asc) == codeAsc[i]))
			return i;
	}
	return -1;		
}

void ConvertStringToCode(char *sa, unsigned char *s, short nbrBits)
{
	short	sn = 0,oCnt=0,mask=0x0080;
	unsigned LONG	*lp = (unsigned LONG *) s;
	*s = 0;
	while (nbrBits--) {
		if (oCnt == 0) {
			sn = ConvertAsciiToCode(*(sa++));
			oCnt = 5;
		}
		if (sn & 0x10) {
			*s |= mask;
		}
		sn <<= 1;
		--oCnt;
		mask >>= 1;
		if (mask == 0) {
			mask = 0x80;
			++s;
			*s = 0;
		}
	}
	if (LittleEndian()) {
		SwapLong((unsigned LONG *) &lp[0]);
		SwapLong((unsigned LONG *) &lp[1]);
	}
}

int ValidSerialString(char *str)
{
	short	i;
	if (strlen(str) != 15)
		return false;
	for (i = 0; i < 15; ++i) {
		switch (i) {
		case 4:
		case 9:
			if (str[i] != '-')
				return false;
			break;
		default:
			if (!isalnum(str[i]))
				return false;
			break;
		}
	}
	return true;
}


enum { R_Bad, R_Newbie, R_Member, R_OldStyle, 
		R_HackedSerialNumber, 
		R_BetaFreebie, R_VIPFreebie};

int CheckRegNumber(char *str)
{
	unsigned char	tempSerial[256],tbuf[32],tCode[16];
	unsigned LONG crc,counter=0;
	int	result = R_Bad;

	strcpy(tempSerial, str);

	if (ValidSerialString(tempSerial)) {
		bcopy(&tempSerial[0],&tbuf[0],4);
		bcopy(&tempSerial[5],&tbuf[4],4);
		bcopy(&tempSerial[10],&tbuf[8],5);
		tbuf[13] = 0;
		ConvertStringToCode((char *) tbuf,tCode,64);
		crc = *((unsigned LONG *) &tCode[0]);
		counter = *((unsigned LONG *) &tCode[4]);
		counter ^= (MAGIC_LONG ^ crc);
		if (crc != ComputeLicenseCRC(counter, 4)) {
			crc = *((unsigned LONG *) &tCode[0]);
			counter = *((unsigned LONG *) &tCode[4]);
			counter ^= OLD_MAGIC_LONG;
			if (crc == ComputeOldLicenseCRC(counter, 4))
				result = R_OldStyle;
			else
				result = R_Bad;
		}
		else {
			if (counter == 0)
				result = R_Newbie;
			else if (counter >= 500 && counter < 1000)
				result = R_BetaFreebie;
			else if (counter >= 1000 && counter < 1500)
				result = R_VIPFreebie;
			else if (counter >= 100000 && counter <= 110000)
				result = R_Member;
            else if (counter >= 500000000L && counter <= 500100000L)
                result = R_Member;
			else
				result = R_HackedSerialNumber;
		}
	}
	return result;
}

