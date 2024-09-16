#ifndef __DSLOCK_H__
#define __DSLOCK_H__

#include "digisonic.h"

#include "swlock.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSLOCK_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef struct _DSLOCK_{
	swlock_t m_Lock;
} DSLOCK, *PDSLOCK;

EXTERN int CDSLock(PDSLOCK pLock);
EXTERN int CDSLockDestroyer(PDSLOCK pLock);

EXTERN int SetName(PDSLOCK pLock, const char *pName);

EXTERN int Lock(PDSLOCK pLock, DWORD dwTimeout_ms);
EXTERN int Unlock(PDSLOCK pLock);

#endif // __DSLOCK_H__
