#ifndef __DSBUFFER_H__
#define __DSBUFFER_H__

#include "digisonic.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSBUFFER_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef struct _CDSBUFFER_ {
	PBYTE m_pBuf;
	DWORD m_dwSize;
	DWORD m_dwLength;
} CDSBUFFER, *PCDSBUFFER;

EXTERN int CDSBuffer(PCDSBUFFER pBuf, DWORD dwSize);
EXTERN int CDSBufferDestroyer(PCDSBUFFER pBuf);

EXTERN int CDSBufferEnQ(PCDSBUFFER pBuf, PBYTE pData, DWORD dwSize, DWORD dwMinSize);
EXTERN int CDSBufferIncreaseEnQPointer(PCDSBUFFER pBuf, DWORD dwSize, DWORD dwMinSize);

EXTERN int CDSBufferDeQ(PCDSBUFFER pBuf, PBYTE pData, DWORD dwSize, DWORD dwMinSize);
EXTERN int CDSBufferDeQ2(PCDSBUFFER pBuf, DWORD dwLength, PFDSProc pfProc, PVOID pContext);
EXTERN int CDSBufferDeQ3(PCDSBUFFER pBuf, DWORD dwLength, PFDSProc pfProc, PVOID pContext);
EXTERN int CDSBufferClear(PCDSBUFFER pBuf);
EXTERN int CDSBufferNew(PCDSBUFFER pBuf, DWORD dwSize);
EXTERN int CDSBufferExpansion(PCDSBUFFER pBuf, DWORD dwSize);

EXTERN int CDSBufferAvailableSize(PCDSBUFFER pBuf);

#endif	// __DSBUFFER_H__
