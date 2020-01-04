/******************************************************************************************/
/* Asset.h - an implementation of something like Apples Resource Manager
 */
/******************************************************************************************/
/******************************************************************************************/

#ifndef _H_Asset
#define _H_Asset        1

#include "local.h"
#include <string.h>

#ifdef macintosh
  #define ASSET_CREATOR    'mPrp'
  #define ASSET_FILETYPE   'mAst'
  #define ASSET_FILETYPEPC 'tsAm'

  #define ASSET_FAVE       'Fave'
  #define RT_PROP          'Prop'
  #define RT_USERBASE      'User'
  #define RT_IPUSERBASE    'IUsr'
#else
#define ASSET_CREATOR    0x00000000L    /* 'mPrp' */
#define ASSET_FILETYPE   0x00000000L    /* 'mAst' */

#define ASSET_FAVE       0x46617665L     /* 'Fave' */

#define RT_PROP          0x50726f70L    /* 'Prop' */
#define RT_USERBASE      0x55736572L    /* 'User' */
#define RT_IPUSERBASE    0x49557372L    /* 'IUsr' */
#endif

#if unix /* 3/4/96 */
typedef LONG	AssetHandle;
typedef short	FileHandle;
#else
typedef Handle	AssetHandle;
#endif

/******************************************************************************************/

typedef short   ErrNbr;
typedef LONG    AssetType;

typedef struct {
        LONG    dataOffset;         /* Swap */
        LONG    dataSize;           /* Swap */
        LONG    assetMapOffset;     /* Swap */
        LONG    assetMapSize;       /* Swap */
} AssetFileHeader;

typedef struct {
        LONG    nbrTypes;
        LONG    nbrAssets;
        LONG    lenNames;
        LONG    typesOffset;    /* Offset into AssetMap */
        LONG    recsOffset;             /* offset into AssetMap */
        LONG    namesOffset;
} AssetMapHeader;

typedef struct {
        AssetType       assetType;
        LONG            nbrAssets;
        LONG            firstAsset;     /* Index # in Asset Recs List */
} AssetTypeRec;

typedef struct {
        LONG      idNbr;        /* Unique ID# */
        AssetHandle	rHandle;		/* (used for Handle) */
        LONG      dataOffset;
        LONG      dataSize;
        LONG      lastUseTime;  /* seconds since 1/1/04 */
        LONG      nameOffset;   /* offset into StringList or -1 */
        LONG      flags;
        unsigned LONG  crc;		/* This is a refnum 6/9/95 */
} AssetRec;

typedef struct AssetFileVars {
        struct AssetFileVars *nextAssetFile;
        FileHandle            aRefNum;
        AssetFileHeader afHeader;
        AssetMapHeader  afMap;
        Handle                  typeList;
        Handle                  assetList;
        Handle                  nameList;
#if unix
		Handle					handleList;	/* 9/18/95 */
		int						nbrLoadedAssets;  /* 9/18/95 */
		int						nbrAllocHandles;
#endif
        FSSpec                  fsSpec;
        Boolean                 fileNeedsUpdate;
		LONG					gLastAssetType;
		LONG					gLastAssetNbr;

		/* 7/2/95 New - support temp asset file to allow recovery of props
			after crash */
		FileHandle		tempRefNum;
		FSSpec			tempFSSpec;
		LONG			tempDataSize;
		Boolean			isClosing;

} AssetFileVars;

#define AssetModified   0x01
#define AssetLoaded     0x02
#define AssetPurgeable  0x04
#define AssetProtected  0x08
#define AssetInTempFile	0x10

typedef struct {
	LONG	id;
	unsigned LONG crc;
} AssetSpec, *AssetSpecPtr;	/* 6/7/95 JAB */

typedef struct {
        AssetType       type;
		AssetSpec		spec;	/* 6/7/95 JAB */
        LONG            blockSize;
        LONG            blockOffset;
        short           blockNbr;
        short           nbrBlocks;
        union {
                struct {
                        LONG    flags;
                        LONG    size;
                        char    name[32];
                        char    data[1];
                } firstBlockRec;
                struct {
                        char    data[1];
                } nextBlockRec;
        } varBlock;
} AssetBlockHeader, *AssetBlockPtr;     /* Used to send assets over net */

/* Shared Routines */
/* */

/******************************************************************************************/

/* ErrorHandling */
ErrNbr  AssetError(void);   /* returns last error */

/* Asset File Management */
ErrNbr  OpenAssetFile(FSSpec *sfFile, FileHandle *refNum);
ErrNbr  NewAssetFile(FSSpec *sfFile, FileHandle *refNum);
ErrNbr  CloseAssetFile(FileHandle refNum);
ErrNbr  WriteAssetFile(void);
ErrNbr  UpdateAssetFile(void);
FileHandle   CurAssetFile(void);
ErrNbr  UseAssetFile(FileHandle refNum);
ErrNbr  UpdateAssetFile(void);

/* Asset Maintainence */
Handle  GetAsset(AssetType type, LONG id);
ErrNbr  ChangedAsset(Handle h);
ErrNbr  AddAsset(Handle h, AssetType type, LONG id, StringPtr name);
ErrNbr  GetAssetInfo(Handle h, AssetType *type, LONG *id, StringPtr name);
ErrNbr  SetAssetInfo(Handle h, LONG id, StringPtr name);

/* Asset Memory Management */
ErrNbr  DetachAsset(Handle h);
ErrNbr  ReleaseAsset(Handle h);

/* Asset Queries */
LONG    CountAssets(AssetType type);
Handle  GetIndAsset(AssetType type, LONG nbr);
LONG    CountAssetTypes(void);
AssetType GetIndAssetType(LONG nbr);
Boolean AssetExists(AssetType type, LONG id);
LONG    UniqueAssetID(AssetType type, LONG minID);

/* Internal Asset Manager functions */
AssetRec        *FindAssetRec(AssetType type, LONG id);
AssetRec        *FindAssetRecByIndex(AssetType type, LONG idx);
Boolean         FindAssetByHandle(Handle rHandle, AssetTypeRec **att, AssetRec **app);

/* CRC Functions 6/7/95 */
unsigned LONG 	ComputeCRC(Ptr p, LONG len); /* 6/7/95 */
AssetRec		*FindAssetRecWithCRC(AssetType type, LONG id, unsigned LONG crc);	/* 6/7/95 */
Handle			GetAssetWithCRC(AssetType type, LONG id, unsigned LONG crc);	/* 6/7/95 */
unsigned LONG	GetAssetCRC(Handle h);
Boolean 		AssetExistsWithCRC(AssetType type, LONG id, unsigned LONG crc);
ErrNbr 			SetAssetCRC(Handle h, unsigned LONG crc);

/* Asset by Name functions 6/7/95 */
AssetRec	*FindAssetRecByName(AssetType type, StringPtr name);
Handle	GetAssetByName(AssetType type, StringPtr name);

/******************************************************************************************/
ErrNbr NewAssetFileRec(FileHandle refNum);
ErrNbr LoadAssetMap(FileHandle refNum);
ErrNbr WriteAsset(AssetRec *ap);
ErrNbr WriteAssetMap(FileHandle oRefNum);
ErrNbr RmveAsset(Handle rHandle);
void CompressAssetNames(void);
Boolean	AssetIsDuplicate(Handle h);

/* 10/9 release all assets of a particular tgype
 * used to release 'Props' during room->room navigation
 */
ErrNbr ReleaseAssets(AssetType assetType);

ErrNbr SortAssetsByUsage(void);


/* new internal routines for dealing with 64 bit pointers 9/18/95 */
#if unix
Handle GetAssetHandle(LONG handleID);
LONG AddAssetHandle(Handle h);
void ClearAssetHandle(LONG handleID);
#define NullAssetHandle		0
#else
#define GetAssetHandle(x)	x
#define AddAssetHandle(x)	x
#define ClearAssetHandle(x)	DisposeHandle(x)
#define NullAssetHandle		NULL
#endif

/* 7/2/96 Request to use temporary asset file */
ErrNbr UseTempAssetFile(void);
FileHandle CreateAndOpenTempFile(FSSpec *seed);
Handle	LoadAsset(AssetRec huge *ap);

#endif

