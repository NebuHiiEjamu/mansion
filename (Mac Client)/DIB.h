/* DIB.h */

#define WORD	short
#define DWORD	long

typedef struct tagBITMAPFILEHEADER {
	WORD	bfType;
	DWORD	bfSize;
	WORD	bfReserved1;	
	WORD	bfReserved2;	
	DWORD	bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagRGBQUAD {
	unsigned char b,g,r,a;
} RGBQUAD;

typedef struct tagRGBTRIPLET {
	unsigned char b,g,r;
} RGBTRIPLET;

typedef struct tagBITMAPINFO {
	DWORD	biSize;
	DWORD	biWidth;
	DWORD	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
	DWORD	biCompression;
	DWORD	biSizeImage;
	DWORD	biXPelsPerMeter;
	DWORD	biYPelsPerMeter;
	DWORD	biClrUsed;
	DWORD	biClrImportant;
} BITMAPINFOHEADER;

typedef struct tagSMALLINFO {
	DWORD	biSize;
	WORD	biWidth;
	WORD	biHeight;
	WORD	biPlanes;
	WORD	biBitCount;
} SMALLINFOHEADER;

enum CompressionModes {BI_RGB,BI_RLE8,BI_RLE4};
