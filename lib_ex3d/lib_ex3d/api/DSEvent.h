#ifndef __CDSEVENT_H__
#define __CDSEVENT_H__

#include "digisonic.h"

#include "swlock.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSEVENT_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef swlock_t    Event_t;

typedef struct _DSEVENT_{
    DS_BOOL m_bWaiting;

    Event_t m_hEvent;
} DSEVENT, *PDSEVENT;

EXTERN void CDSEvent(PDSEVENT pEvent,
             DS_BOOL bManualReset,
		     DS_BOOL bInitialState,
		     LPCSTR lpName);
EXTERN void CDSEventDestroyer(PDSEVENT pEvent);

EXTERN DWORD EventWaitForSingle(PDSEVENT pEvent, DWORD dwMilliseconds);
EXTERN DS_BOOL EventSet(PDSEVENT pEvent);
EXTERN DS_BOOL EventReset(PDSEVENT pEvent);

#endif //__CDSEVENT_H__
