#define _DSEVENT_C_

#include "DSEvent.h"

/* ------------------------------------------------------------------------
    기    능: 이벤트(CDSEvent 클래스) 생성자
	매개변수: bManualReset	-> true: 수동 리셋 방식
					          false: 자동 리셋 방식, 기본값
		      bInitialState	-> true: 이벤트 설정으로 초기화한다.
							  false: 이벤트 리셋으로 초기화한다. 기본값
		      lpName		-> NULL: "CDSEvent"+ID
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSEvent(PDSEVENT pEvent, DS_BOOL bManualReset, DS_BOOL bInitialState, LPCSTR lpName)
{
	DSTRACE(("[CDSEvent()] Entering... pEvent:0x%08x\n\r", pEvent));
    pEvent->m_bWaiting = FALSE;
	swlock_init(&(pEvent->m_hEvent));
	DSTRACE(("[CDSEvent()] Leaving... m_hEvent:0x%08x, m_bWaiting:%d\n\r", pEvent->m_hEvent, pEvent->m_bWaiting));
}

/* ------------------------------------------------------------------------
    기    능: 이벤트(CDSEvent 클래스) 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSEventDestroyer(PDSEVENT pEvent)
{
	DSTRACE(("[CDSEventDestroyer()] Entering... pEvent:0x%08x\n\r", pEvent));
    if( pEvent->m_bWaiting ){
		DWORD dwT, dwBT;

        EventSet(pEvent);

		dwBT = GetTickCount();
		while( pEvent->m_bWaiting )
		{
			Sleep( 1 );

			DS_ElapsedTimeDW(dwBT, dwT);
			if(dwT > 1000){
				DS_Print("[Error - CDSEventDestroyer()] Timeout!! Elapsed time: %u ms\n\r", dwT);
				break;
			}
		}
    }
	DSTRACE(("[CDSEventDestroyer()] Leaving...\n\r"));
}

/* ------------------------------------------------------------------------
    기    능: 이벤트를 설정한다.
	매개변수: 없음
	되돌림값: TRUE	-> 설정 성공
	          FALSE	-> 설정 실패
    비    고:
  ------------------------------------------------------------------------ */
DS_BOOL EventSet(PDSEVENT pEvent)
{
	//DSTRACE(("[EventSet()] pEvent:0x%08x, m_hEvent:0x%08x, m_bWaiting:%d\n\r", pEvent, pEvent->m_hEvent, pEvent->m_bWaiting));
	swlock_release(&(pEvent->m_hEvent));
	return TRUE;
}

/* ------------------------------------------------------------------------
    기    능: 이벤트를 리셋한다.
	매개변수: 없음
	되돌림값: TRUE	-> 리셋 성공
	          FALSE	-> 리셋 실패
    비    고:
  ------------------------------------------------------------------------ */
DS_BOOL EventReset(PDSEVENT pEvent)
{
	//DSTRACE(("[EventReset()] pEvent:0x%08x, m_hEvent:0x%08x, m_bWaiting:%d\n\r", pEvent, pEvent->m_hEvent, pEvent->m_bWaiting));
	swlock_release(&(pEvent->m_hEvent));
	return TRUE;
}

/* ------------------------------------------------------------------------
    기    능: 단일 이벤트를 최대 대기 시간까지 기다린다.
	매개변수: dwMilliseconds    -> 최대 대기 시간, 단위: ms
	되돌림값: WAIT_OBJECT_0     -> 이벤트 받음
	          WAIT_OBJECT_0 외  -> 이벤트 기다리기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
DWORD EventWaitForSingle(PDSEVENT pEvent, DWORD dwMilliseconds)
{
	//DSTRACE(("[EventWaitForSingle()] Entering... pEvent:0x%08x, dwMilliseconds:%d\n\r", pEvent, dwMilliseconds));
    DWORD dwWait = WAIT_FAILED;

	swlock_acquire(&(pEvent->m_hEvent));

	dwWait = WAIT_OBJECT_0;

	pEvent->m_bWaiting = FALSE;
	//DSTRACE(("[EventWaitForSingle()] Leaving... m_bWaiting:%d, dwWait:%d\n\r", pEvent->m_bWaiting, dwWait));
    return dwWait;
}
