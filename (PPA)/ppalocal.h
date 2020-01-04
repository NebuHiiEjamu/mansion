// PPALocal.h
//
// Machine specific data-types
#ifndef _H_PPALocal
#define _H_PPALocal	1

#ifndef LONG
#define LONG				long
#endif

typedef Rect				PPARect;

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr	PPAProcPtr;
typedef UniversalProcPtr	PPACallbackPtr;
#else
typedef void (*PPAProcPtr)(long cmd, void *ppaRec, long *retCode);
typedef LONG (*PPACallbackPtr)(LONG cmd, LONG arg, char *str);
#endif

typedef EventRecord			PPAEventRecord;
typedef	void				*PPARootObjectPtr;
typedef PixMapHandle		PPAPixMapH;
typedef	Handle				PPAHandle;
typedef Ptr					PPAPtr;

#if USESROUTINEDESCRIPTORS

enum {
	uppPPACallbackInfo	= kThinkCStackBased |
				RESULT_SIZE(SIZE_CODE(sizeof(LONG))) |
				STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(LONG))) |
				STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(LONG))) |
				STACK_ROUTINE_PARAMETER(3, SIZE_CODE(sizeof(char *)))
};

#define CallPPACallback(userRoutine, cmd, arg, str)	\
		CallUniversalProc((UniversalProcPtr) (userRoutine), uppPPACallbackInfo,	\
							cmd, arg, str)
#else
#define CallPPACallback(userRoutine, cmd, arg, str)	\
		(*(userRoutine))(cmd,arg,str)
#endif


#endif
