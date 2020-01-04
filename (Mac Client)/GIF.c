

// ----- A GIF reading routine - by François Pottier <pottier@dmi.ens.fr>>
// ----- April 16th, 1994
// ----- Best viewed in Geneva 9, with 4 character tabs

// Modified by Jim Bumgardner to not use File Handling - GIF is assumed to already have been
// loaded into memory...


// ----- Originally based on code by Patrick J. Naughton, greatly modified since then.

// ----- Input : a file specification and a flag longword to be passed to NewGWorld
// ----- The file size is also given as input because in my case the caller happens to have it at hand.
// ----- Output : an 8 bit deep GWorld with the picture. Pixels locked.

// ----- The routine is rather fast, and uses a minimum amount of memory: only about 15K (plus the GWorld size, of course),
// ----- whatever the GIF file size. It uses a buffer to read chunks of the file - you can change the buffer size by modifying the
// ----- BufferSize #define.

// ----- Note on reliability: I have been using this code in a shareware product for several months and it works fine.
// ----- IMHO this is good quality code.

// ----- You can use this code freely. I only have two requests: 1. that you give me proper credit in your About box, and
// ----- 2. that you tell me if you improve this code. Have fun!

#include <QDOffscreen.h>	// JAB

// extern	OSErr	gError;											/* Imported in order to */
short gMessage;
extern	short	gMessage;											/* be able to report errors */
extern	CTabHandle	gCurCLUT;

#define	strCantOpenData		128									/* Constants for gMessage */
#define	strGIFNoMem			129									/* You can change them, and you ought to move */
#define	strCantRead			130									/* them to a global #include file */
#define	strInvalidGIFSig		131									/* so that the caller sees them too */
#define	strCantSetFPos			132
#define	strUnknownGIFBlock		133
#define	strNoColorMap			134

// JAB
void DrawGIF (Ptr gifPtr, long FileSize, long worldFlags, GWorldPtr *world, short *transIndex);	/* Main routine prototype */

// ----- A small note about programming style : I would have liked to use
// ----- nested subroutines, but C doesn't have them. I could have allocated all my vars in a struct and passed them along to every
// ----- subroutine, but it seemed painful, so I finally chose to use macros. What's more, this code is optimized for speed (or at
// ----- least it tries to be), so it isn't very readable. My apologies.

#define 		ColorMapMask		0x80
#define		ImageSeparator		0x2C
#define		ExtensionIntroducer	0x21
#define		NullBlock			0x00
#define		InterlaceMask		0x40
// #define		BufferSize			8192								/* May be freely modified */

#define		abort(x)			{ gMessage = x; goto end; }
#define		Check(x)			if (e) abort(x)							/* this means return e */

// JAB Optimized to read from pointer
#define		Read(c, buf)	BlockMove(gifPtr, buf, c);			\
							gifPtr += c;					\
							FileSize -= c
//							e = FSRead (refNum, &count, buf);			/* Read the requested number of bytes */	\
//							Check(strCantRead)																	\
//							FileSize -= count						/* And note that we have read them */
						
// JAB Optimized to read from pointer
#define		NextByte(b)		Check(FileSize < 1);				\
							b = *(gifPtr++);				\
							FileSize--

#define		Skip(n)			Check(FileSize < n);				\
							gifPtr += n;					\
							FileSize -= n
//							e = SetFPos (refNum, fsFromMark, n);		/* Skip n bytes */						\
//							Check(strCantSetFPos)																	\
//							FileSize -= n							/* And note that we have read them */
							
// JAB Modified to not use additional buffering, since GIF is already loaded into Memory
//
#define		BufferGetOne	{				\
	if (BlockCount == 0) {					\
		++BufferPointer;					\
		BlockCount = *(BufferPointer);			\
	}									\
	++BufferPointer;						\
	BlockCount--;							\
}

// ----- Since the length bytes are taken out by BufferGetOne, ReadCode can consider the buffer as a continuous flow of raw bytes.
// ----- Here is how it works : BufferPointer points to the byte we're reading, and BitOffset points to the bit inside that byte.
// ----- The convention is that if BitOffset = 0, then a new byte needs to be read.
// ----- We have to read exactly CodeSize bits. So we read as many bytes as are necessary (by comparing BitOffset + CodeSize
// ----- with 8 and 16), then all we have to do is shift right to kill the bits that were part of the last code, and mask the left side
// ----- to mask off the bits that will be part of the next code.

#define		ReadCode(code)		{																		\
	if (BitOffset == 0) BufferGetOne																		\
	data = *BufferPointer;											/* Read the current byte */				\
	newBitOffset = BitOffset + CodeSize;																	\
	if (newBitOffset > 8) {											/* If we need more, read the next byte */	\
		BufferGetOne																					\
		data += ((long) (*BufferPointer)) << 8;																\
	}																								\
	if (newBitOffset > 16) {											/* If we still need more, read a third byte */	\
		BufferGetOne																					\
		data += ((long) (*BufferPointer)) << 16;																\
	}																								\
	data >>= BitOffset;												/* Skip the bits we already processed */		\
	BitOffset = newBitOffset & 7;										/* Compute the new bit offset */			\
	code = data & ReadMask;											/* Mask off the bits we don't want yet */	\
}

// ----- Since QuickDraw is much too slow for pixel drawing, we blast bits ourselves to the offscreen pixmap. We are able to
// ----- do things very fast by using a pointer, 'curAddr', walking the pixmap. The macro below needs to be in 32 bit addressing
// ----- mode in order to run.

#define		DrawPixel(index)		{																	\
	*(curAddr++) = index;																				\
	if (++xc == Width) {												/* Update the x-coordinate */				\
		xc = 0;													/* If it overflows, update the y-coordinate */	\
		if (!Interlaced)												/* In a non-interlaced picture, just */		\
			yc++;												/* increment yc to the next scan line */		\
		else {																						\
			switch (Pass) {											/* Otherwise deal with the interlace as */	\
			case 0:												/* described in the GIF spec */				\
				yc += 8;																				\
				if (yc >= Height) {	Pass++;	yc = 4;	}													\
				break;																				\
			case 1:																					\
				yc += 8;																				\
				if (yc >= Height) {	Pass++;	yc = 2;	}													\
				break;																				\
			case 2:																					\
				yc += 4;																				\
				if (yc >= Height) {	Pass++;	yc = 1;	}													\
				break;																				\
			case 3:																					\
				yc += 2;																				\
				break;																				\
			default:																					\
				break;																				\
			}																						\
		}																							\
		curAddr = (unsigned char *) ((long) srcBaseAddr + (long) yc * (long) rowBytes);	/* update current address in bitmap */		\
	}																								\
}

// ----- ReadColorMap processes the flag byte 'flags' and reads the color map if one is present, and creates a CTabHandle.
// ----- This code is used for the global and local color maps. If both maps are present, the local one overrides the global one.
// ----- The resulting CTabHandle is used when creating the GWorld - this way, color codes in the file and in the pixmap have
// ----- the same meaning.

#define		ReadColorMap			{																	\
	Boolean		mapPresent;																			\
	CTabPtr		tablePtr;																				\
	short		i;																					\
																									\
	mapPresent = (flags & ColorMapMask) ? true : false;					/* Does the file have a global color map ? */	\
	if (mapPresent) {																					\
		BitsPerPixel = (flags & 7) + 1;									/* Image bit depth */					\
		ColorMapSize = 1 << BitsPerPixel;								/* Number of entries in color table */		\
		BitMask = ColorMapSize - 1;																		\
																									\
		ColorTable = (CTabHandle) NewHandleClear (8*ColorMapSize+8);			/* Allocate memory for the table */		\
		if (ColorTable == NULL) { e = memFullErr; abort(strGIFNoMem) }															\
		HLock((Handle) ColorTable);																				\
		tablePtr = *ColorTable;																			\
		tablePtr->ctSeed = GetCTSeed(); /* 8/10/96 was 0, may have been causing crashes when >256 colors were used */ 	\
		tablePtr->ctFlags = 0;																			\
		tablePtr->ctSize = ColorMapSize - 1;																\
		for (i = 0; i < ColorMapSize; i++) {																	\
			tablePtr->ctTable[i].value = i;																	\
			NextByte(b);																				\
			tablePtr->ctTable[i].rgb.red = ((short) b << 8) | b;				/* Determine RGB value */				\
			NextByte(b);																				\
			tablePtr->ctTable[i].rgb.green =((short) b << 8) | b;													\
			NextByte(b);																				\
			tablePtr->ctTable[i].rgb.blue = ((short) b << 8) | b;													\
		}																							\
		HUnlock((Handle) ColorTable);																				\
	}																								\
	HasColorMap = HasColorMap || mapPresent;																\
}

// ----- Here comes the main routine.

void DrawGIF (Ptr gifPtr, long FileSize, long worldFlags, GWorldPtr *world, short *transIndex)
{
	OSErr		e = 0;												/* Error code */
	// short		refNum;											/* File reference number */
	long			sig;
	// long			count;
	Byte			b, flags, c, extLabel;
	
	short		BitsPerPixel, ColorMapSize, BitMask;
	short		Width, Height;
	short		ClearCode, FreeCode, EOFCode, FirstFree;
	Byte			CodeSize, InitCodeSize;
	short		MaxCode;
	
	// Byte			*BufferBase = NULL;									/* Read buffer */
	Byte			*BufferPointer /* , *BufferTop */;							/* Offset in buffer */
	Byte			BlockCount;										/* Remaining bytes in current block */
	Byte			BitOffset, newBitOffset;
	long 			data;
	short		ReadMask;										/* Mask with exactly CodeSize bits set to 1 */
	
	short		CurCode, InCode, OldCode, Code;						/* Decompressor variables */
	short		FinChar;
	short		*OutCodeBase = NULL;								/* Output array used by the decompressor */
	short		*OutCode;											/* Current pointer into this array */
	short		*Prefix = NULL;									/* The hash table used by the decompressor */
	short		*Suffix = NULL;
	
	short		xc, yc;											/* Pen position */
	short		Pass;											/* Used by the output routines for interlaced pics */
		
	Boolean		HasColorMap=false, Interlaced;
	
	PixMapHandle	srcPixMap;										/* These are associated with 'world' */
	long			*srcBaseAddr;									/* Pixmap base address and current address */
	unsigned char	*curAddr;
	short		rowBytes;
	Rect			bounds;
	CTabHandle	ColorTable = NULL;									/* Custom color table */
	
	GWorldPtr		oldWorld;											/* Some temporary variables */
	GDHandle		oldDevice;
	
	// if (e = FSpOpenDF (spec, fsRdPerm, &refNum))							/* Open the file */
	//	abort(strCantOpenData)
	
// ----- Allocate temporary buffer and arrays.

	*transIndex = -1;

	if (!(OutCodeBase = (short*) NewPtrClear(2050)))		{ e = MemError(); abort(strGIFNoMem) }
	if (!(Prefix = (short*) NewPtrClear(8192)))			{ e = MemError(); abort(strGIFNoMem) }
	if (!(Suffix = (short*) NewPtrClear(8192)))			{ e = MemError(); abort(strGIFNoMem) }
	// if (!(BufferBase = (Byte*) NewPtr(BufferSize)))	{ e = MemError(); abort(strGIFNoMem) }
	
// ----- Read variables from the global GIF descriptor
	
	Read (4, &sig);													/* Read the GIF signature */
	if (sig != 'GIF8') { e = paramErr; abort(strInvalidGIFSig) }					/* and make sure it's correct */
	Skip(6);														/* Skip screen dimensions */
	NextByte(flags);												/* This flag byte is processed below */
	Skip(2);														/* Skip background color and aspect ratio */
	
	ReadColorMap													/* Read the global color map */
	
// ----- In an attempt to support GIF89 pictures, we recognize extension blocks and skip them.
// ----- An extension block starts with ExtensionIntroducer. It is followed by any number of data blocks, the last of them being
// ----- a trailer block.

	while (true) {
		NextByte(b);												/* Read the first byte of the block */
		
		switch (b) {
		case ImageSeparator:										/* Image separator, let's go read the image */
			goto ReadImage;
		case ExtensionIntroducer:										/* Extension block */
			NextByte(extLabel);		/* was Skip(1) */					/* Skip the extension label */

			NextByte(b);											/* Get the block size */

			if (extLabel == 0xF9 && (gifPtr[0] & 1)) 						/* graphics extension, get transparency index# */
				*transIndex = (unsigned char) gifPtr[3];

			Skip(b);												/* Skip the block */
			
			NextByte(b);											/* Read the subsequent data blocks */
			while (b != NullBlock) {									/* until a trailer block is reached */
				Skip(b);											/* Skip this block */
				NextByte(b);										/* Get length of the next block */
			}
			
			break;
		default:													/* Unknown file format */
			e = paramErr;
			abort(strUnknownGIFBlock);
		}
	}
	
// ----- Read the image descriptor

ReadImage:
	Skip(4);														/* Skip left and top offsets */
	NextByte(b); NextByte(c);										/* Read width and height */
	Width = (short) c * 0x100 + b;										/* Byte per byte because they are written */
	NextByte(b); NextByte(c);										/* in stupid PC-like big-endian order */
	Height = (short) c * 0x100 + b;

	NextByte(flags);
	Interlaced = (flags & InterlaceMask) ? true : false;						/* See if the image is interlaced */

	ReadColorMap													/* If there is a local color map, read it */
	
	if (!HasColorMap) { e = paramErr; abort(strNoColorMap) }					/* Make sure we have at least one color map */
	
// ----- Now that we know the picture size, we can create the GWorld. We make it 8 bit deep because most GIF files are 8 bit deep,
// ----- and also because it makes life much easier for DrawPixel : one byte = one pixel. Afterwards, a simple CopyBits call will
// ----- enable the caller to rescale/dither the image.
// ----- The right way would be to switch to 32 bit mode before accessing the pixmap. But calling SwapMMUMode everytime in the
// ----- loop is *very* time consuming. So I check to see if we are in 24 bit mode, and if that is the case, then I force the pixmap
// ----- to be stored in main memory by using the keepLocal flag. This way I make certain that the pixmap can be accessed
// ----- in 24 bit mode.
	// (*ColorTable)->ctSeed = (*gCurCLUT)->ctSeed;
	
	GetGWorld (&oldWorld, &oldDevice);									/* save the current world */
	SetRect (&bounds, 0, 0, Width, Height);								/* and create our own */
	// SysBreakStr("\pNGW"); // DEBUG !!!!
	if ((e = NewGWorld(world, 8, &bounds, ColorTable, NULL, worldFlags | (GetMMUMode() == false32b ? keepLocal : 0))) != noErr)
		abort(strGIFNoMem);
	LockPixels(GetGWorldPixMap(*world));								/* Lock the pixel map */
	SetGWorld (*world, NULL);										/* activate it */
		
// ----- Start reading the raster data. First initialize the decoder.

	NextByte(CodeSize);
	ClearCode = (1 << CodeSize);
	EOFCode = ClearCode + 1;
	FreeCode = FirstFree = ClearCode + 2;
	
// ----- The GIF spec has it that the code size used to compute the above values is the code size given in the file, but the
// ----- code size used in compression/decompression is the code size given in the file plus one. (thus the ++).

	CodeSize++;
	InitCodeSize = CodeSize;
	MaxCode = (1 << CodeSize);
	ReadMask = MaxCode - 1;

	BlockCount = 0;													/* We start at the beginning of a block */
	BitOffset = 0;													/* and at the beginning of a byte */
	// BufferPointer  = BufferTop = BufferBase + BufferSize;					/* Force the buffer to be filled immediately */
	BufferPointer = (unsigned char *) gifPtr-1;
	
	xc = yc = 0;
	Pass = 0;
			
// ----- Decompress the file, continuing until a GIF EOF code is reached.

	srcPixMap = GetGWorldPixMap(*world);								/* Get the pixmap */
	rowBytes = (*srcPixMap)->rowBytes & 0x7FFF;						/* get rowBytes */
	srcBaseAddr = (long*) GetPixBaseAddr (srcPixMap);			/* and the base address of the pixmap */
	curAddr=  (unsigned char *) srcBaseAddr;
	
	ReadCode(Code);
	while (Code != EOFCode) {							
		if (Code == ClearCode) {										/* Clear code sets everything back to its */
			CodeSize = InitCodeSize;									/* initial value, then reads the subsequent code */
			MaxCode = (1 << CodeSize);								/* as uncompressed data. */
			ReadMask = MaxCode - 1;
			FreeCode = FirstFree;
			ReadCode(Code);
			CurCode = OldCode = Code;
			FinChar = CurCode & BitMask;
			DrawPixel(FinChar);
		}
		else {													/* If not Clear code, then must be data. */
			CurCode = InCode = Code;									/* Save same as CurCode and InCode */

			OutCode = OutCodeBase;
			if (CurCode >= FreeCode) {								/* If >= FreeCode, not in the hash table yet */
				CurCode = OldCode;									/* repeat the last character decoded */
				*(OutCode++) = FinChar;
			}
			
			while (CurCode > BitMask) {								/* Pursue the chain pointed to by CurCode */
				*(OutCode++) = Suffix[CurCode];						/* through the hash table to its end */
				CurCode = Prefix[CurCode];							/* the output queue */
			}

			FinChar = CurCode & BitMask;								/* The last code in the chain is treated as raw data. */
			*OutCode = FinChar;
						
			while (OutCode >= OutCodeBase)							/* Now we put the data out */
				DrawPixel(*(OutCode--));							/* It's been stacked LIFO, so deal with it that way */

			Prefix[FreeCode] = OldCode;								/* Build the hash table on-the-fly */
			Suffix[FreeCode] = FinChar;								/* No table is stored in the file */
			OldCode = InCode;
			
			if (++FreeCode >= MaxCode)								/* Point to the next slot in the table */
				if (CodeSize < 12) {									/* If we exceed the current MaxCode value */
					CodeSize++;									/* increment the code size unless it's already 12 */
					MaxCode *= 2;									/* If it is, do nothing; the next code better be */
					ReadMask = MaxCode - 1;							/* Clear */
				}												
		}

		ReadCode(Code);											/* Read the next code */
	}
	
	SetGWorld (oldWorld, oldDevice);									/* restore the previous graphics world */
	
// ----- Release temporary buffer and arrays, close file.

	e = noErr;

end:
	if (ColorTable)													/* We can destroy the CTable, NewGWorld */
		DisposeHandle((Handle) ColorTable);								/* copied it */
	// if (BufferBase)
	//	DisposePtr((Ptr) BufferBase);
	if (Suffix)
		DisposePtr((Ptr) Suffix);
	if (Prefix)
		DisposePtr((Ptr) Prefix);
	if (OutCodeBase)
		DisposePtr((Ptr) OutCodeBase);

	// gError = e;
	return;
}