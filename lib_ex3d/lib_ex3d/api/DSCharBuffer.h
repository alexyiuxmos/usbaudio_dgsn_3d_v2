#ifndef __DCHARSBUFFER_H__
#define __DCHARSBUFFER_H__

#include "digisonic.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSCHARBUFFER_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef struct _CDSCHARBUFFER_{
	PCHAR m_pszMem;
	DWORD m_dwMemSize;
} CDSCHARBUFFER, *PCDSCHARBUFFER;

EXTERN int CDSCharBuffer(PCDSCHARBUFFER pBuf, DWORD dwLength);
EXTERN int CDSCharBufferDestroyer(PCDSCHARBUFFER pBuf);

EXTERN int CDSCharBufferSet(PCDSCHARBUFFER pBuf, PCHAR pszData);
	
EXTERN int CDSCharBufferEmpty(PCDSCHARBUFFER pBuf);

#endif	// __DCHARSBUFFER_H__
