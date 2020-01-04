/****************************************************************************
 *
 * s-serial.c
 *
 * Code for validating, converting and parsing commercial server serial numbers.
 *
 * Jim Bumgardner - TWI
 *
 * Use one the following (valid) serial numbers to test with:
 *
 *		U77RM-HABRA-8CHTU-JZ5BA	(seed 0, unix, 100 user capacity)
 *		TFSRY-KJ9NJ-3KXYC-UX37S (seed 0, unix, 200 user capacity)
 *
 *      KEZLZ-FQ9FT-TCYGA-66DFA (seed 1, unix, 100 user capacity)
 *		L64LL-MGBG3-YKEBS-G8B3S (seed 1, unix, 200 user capacity
 *
 *		CVGRM-Z6C32-8GATR-8Z4AA (seed 0, NT, 100 user capacity)
 *		BMMRY-3W64S-3PUY9-EX26S (seed 0, NT, 200 user capacity)
 *
 *		3LNLZ-XU6VK-TGTG7-L6CEA	(seed 1, NT, 100 user capacity)
 *		4UBLL-54CSB-YPDBP-S8A2S (seed 1, NT, 200 user capcity)
 ****************************************************************************/

#include "s-server.h"
#include "s-magicss.h"

#include "s-magicss.c"    /* include the table used for generating CRCs */

/*
A portable library for server reg numbers.


	
	A serial number consists of 3 numbers, a,b,c.

	It is computed from the following information:	
		    seed           (32-bits)  - A unique user ID seed number
			options        (32-bits) consisting of:
				flags		   (16-bits)  - currently undefined
				capacity   	   (8-bits    - stored as multiples of 10 users e.g. 2 = 20 users)	
										  - 0 = minimum # of users (3?)
										  - 255 = maximum # of users (2550+)
				platform type  (8-bits)   - 0=mac,1=windows,2=NT,3=unix

	The three numbers are computed as follows:

		a = ComputeCRC(seed,options);    This is how we check for validity.
		b = seed XOR MAGIC_SEED_NUMBER XOR a
		c = options XOR MAGIC_OPTIONS_NUMBER XOR b

   These three numbers (a,b,c) are converted to a 20-digit alpha-numeric representation
   by converting groups of 5 bits into a letter.

*/

/* To check if the server's serial number is valid, you will need to 

	1) read the server's serial number (from a config file?),

		it should look like abcde-abcde-abcde-abcde

	2) remove the hyphens, resulting in a 20 byte string.
							abcdeabcdeabcdeabcde

	3) convert it to 12-byte binary form using SSAsciiToBinary()

       You can skip steps 2 and 3 if you store the serial number in binary form.

	4) Call SSIsValidSerialNumber() on the binary-encoded serial number - it will
	   return TRUE if the serial number is valid.
*/

/* To determine the server's  maximum user capacity and other options, use 
   SSParseSerialNumber() on the serial number in it's binary (12 byte) form.
 */

/* The code that sends the server info to the palace directory (ypd)
   should send an ascii-encoded serial number.  If the serial number is encoded
   in binary form, use SSBinaryToAscii() to convert it back.  To add hyphens,
   use something like:

   		sprintf(prettyStr,"%.5s-%.5s-%.5s-%.5s\n",
			&ascSer[0],
			&ascSer[5],
			&ascSer[10],
			&ascSer[15]);

*/

/* This is an OBFUSCATED version of the validity check algorithm, it
   is harder to hack, because the correct CRC value is never
   generated.  
 */

int SSIsValidSerialNumber(unsigned char *binBuf)
{
	unsigned LONG	testNum[3];
	testNum[0] = *((LONG *) &binBuf[0]);							/* CRC */
	testNum[1] = *((LONG *) &binBuf[4]);							/* SEED */
	testNum[2] = *((LONG *) &binBuf[8]);							/* OPTIONS */
	testNum[0] ^= OBFUSCATE_LONG;
	testNum[2] ^= testNum[1] ^ FLAGS_MAGIC_LONG;					/* DECRYPT OPTIONS */
	testNum[1] ^= testNum[0] ^ (SEED_MAGIC_LONG ^ OBFUSCATE_LONG);	/* DECRYPT SEED */

	/* check if CRC is correct, return TRUE if so */
	return ((testNum[0] ^ SSDecodeCRC(&testNum[1])) == DECODE_OBFUSCATE);

	/* unobfuscated method would be:
	 *	return testNum[0] == SSComputeCRC(&testNum[1]);
	 */
}

/* parses a serial number into it's component parts */

void SSParseSerialNumber(unsigned char *binBuf,
				  int *platform,int *capacity,int *flags, unsigned LONG *seed)
{
	unsigned long a,b,c;
	a = *((LONG *) &binBuf[0]);
	b = *((LONG *) &binBuf[4]);
	c = *((LONG *) &binBuf[8]);
	c ^= b ^ FLAGS_MAGIC_LONG;
	b ^= a ^ SEED_MAGIC_LONG;

	*platform = c & 0xFF;
			/* 0 = mac, 1 = windows, 2 = NT, 3 = Unix */

	*capacity = ((c >> 8) & 0x0FF) * 10;
            /* capacity is encoded as multiples of 10, so we convert back here */
			/* if 0, use some MINIMUM capacity (no less than 3) */
			/* if 2550, don't limit the capacity, or use some hard coded
			   maximum */

	*flags = (c >> 16) & 0x0FFFF;
			/* Flags are currently undefined */
			/* they'll be used to provide additional-cost server features */

	*seed = b;
			/* We may at some later date want to do range checking on the
			   seed, for now you can ignore it */
}


/* input (12 byte (96-bit) number
 * output: abcde-abcde-abcde-abcde (with NO hyphens)
 */
 
void SSBinaryToAscii(unsigned char *s, char *sa)
{
	int		sn = 0,oCnt=0,mask = 0x0080;
	int		nbrBits;
	unsigned LONG	tempAry[3];
	unsigned char	*binPtr;

	tempAry[0] = *((LONG *) &s[0]);
	tempAry[1] = *((LONG *) &s[4]);
	tempAry[2] = *((LONG *) &s[8]);

	/* Serial numbers are stored as 3 longs using local byte order.
	   When converted to ascii, they need to be converted to MSB,
	   so that the letters are in the same order for all platforms. */

	if (LittleEndian()) {
		SwapLong( &tempAry[0]);
		SwapLong( &tempAry[1]);
		SwapLong( &tempAry[2]);
	}

	binPtr = (unsigned char *) &tempAry[0];

	for (nbrBits = 0; nbrBits < SS_LENGTH; ++nbrBits) 
	{
		sn <<= 1;
		if (*binPtr & mask)
			sn |= 0x01;
		if (++oCnt == 5) {
			*(sa++) = SSConvertCodeToAscii(sn);
			sn = oCnt = 0;
		}
		mask >>= 1;
		if (mask == 0) {
			mask = 0x80;
			++binPtr;
		}
	}
	if (oCnt) {
		do {
			sn <<= 1;
		} while (++oCnt < 5);
		*(sa++) = SSConvertCodeToAscii(sn);
	}
	*sa = 0;
}

/*input: absde-abcde-abcde-abcde  (20 bytes, no hyphens)
 * output: 12 byte (96-bit) number
 */
void SSAsciiToBinary(char *sa, unsigned char *s)
{
	int		sn = 0,oCnt=0,mask=0x0080,nbrBits;
	LONG	*lp = (LONG *) s;

	*s = 0;
	for (nbrBits = 0; nbrBits < SS_LENGTH; ++nbrBits)
	{
		if (oCnt == 0) {
			sn = SSConvertAsciiToCode(*(sa++));
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

	/* When the serial number was converted to Ascii, it was in MSB, so 
	   we need to convert it to local byte order so we can parse the components
	   correctly. */
	if (LittleEndian()) {
		SwapLong((unsigned LONG *) &lp[0]);
		SwapLong((unsigned LONG *) &lp[1]);
		SwapLong((unsigned LONG *) &lp[2]);
	}
}

/* This function is almost identical to the simpler SSComputeCRC, except
   that it intentionally obfuscates the result by XORing the initial CRC with
   a rotated magic number and inverting the result.  We are taking advantage of
   the fact that (A XOR B XOR A) = B
  
  This way, the correct CRC value is never actually generated in memory, and 
  isn't returned to the function that is performing the validity check.

   This makes it harder to generate a valid CRC by reverse engineering
   the compiled code or examining the validation algorithm in a debugger.
 */

unsigned LONG SSDecodeCRC(unsigned long *v)
{
	unsigned LONG	crc = CRC_ENCODE;
	unsigned LONG	temp[3];
	unsigned char 	*p = (unsigned char *) &temp[0];
	int				i;	

	temp[0] = v[0];
	temp[1] = v[1];

	/* The CRC algorithm is byte-order dependent, so we  */
	/* convert the input from local byte order to MSB to guarantee the same */
	/* result on all platforms */
    if (LittleEndian()) {
		SwapLong(&temp[0]);
		SwapLong(&temp[1]);
	}

	/* For each byte of the input, rotate the CRC by one position, and then
	 * XOR with a magic number from the CRC Table
	 */

	for (i = 0; i < 8; ++i)
		crc = ((crc << 1L) | ((crc & 0x80000000L)? 1 : 0)) ^ gSSCRCMask[*(p++)];

	return ~crc;
}


/* INTERNAL ROUTINES */

/* A table for representing a 5-bit number (0-31) as a
  single ascii character
 
  Note: missing are the letters  I,O and the numbers 1 and 0,
  these were chosen to reduce typos.
 */
char	codeAsc[] = {"ABCDEFGHJKLMNPQRSTUVWXYZ23456789"};

/* Convert a 5-bit code to an ascii character */
int SSConvertCodeToAscii(short code)
{
	if (code >= 0 && code < 32)
		return codeAsc[code];
	else
		return -1;
}

/* Convert ascii character to 5-bit code. */
int SSConvertAsciiToCode(short asc)
{
	short	i;
	for (i = 0; codeAsc[i]; ++i) {
		if (codeAsc[i] == asc || (isalpha(asc) && toupper(asc) == codeAsc[i]))
			return i;
	}
	return -1;		
}
