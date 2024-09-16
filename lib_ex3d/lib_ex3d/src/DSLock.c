#define _DSLOCK_C_

#include "DSLock.h"

/* --------------------------------------------------------------------------------
    기    능: 잠금 CDSLock 클래스 생성자
	매개변수: 없음
	되돌림값: 없음
    비    고:
    --------------------------------------------------------------------------------- */
int CDSLock(PDSLOCK pLock)
{
	DSCHECK_PTR("CDSLock()", pLock, ERR_PARAMS_NULL);

	swlock_init(&(pLock->m_Lock));

	return NO_ERR;
}

/* --------------------------------------------------------------------------------
    기    능: 잠금 CDSLock 클래스 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
    --------------------------------------------------------------------------------- */
int CDSLockDestroyer(PDSLOCK pLock)
{
	DSCHECK_PTR("CDSLockDestroyer()", pLock, ERR_PARAMS_NULL);

	return NO_ERR;
}

/* --------------------------------------------------------------------------------
    기    능: 잠근다.
	매개변수: dwTimeout_ms	-> 잠금 최대 대기 시간, 단위: ms, 기본값: 5000(=5초)
	되돌림값: NO_ERR	-> 잠금 성공
			  NO_ERR 외	-> 잠금 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int Lock(PDSLOCK pLock, DWORD dwTimeout_ms)
{
	DSCHECK_PTR("Lock()", pLock, ERR_PARAMS_NULL);

	swlock_acquire(&(pLock->m_Lock));

	return NO_ERR;
}

/* --------------------------------------------------------------------------------
    기    능: 잠금을 푼다.
	매개변수: 없음
	되돌림값: 없음
    비    고:
    --------------------------------------------------------------------------------- */
int Unlock(PDSLOCK pLock)
{
	DSCHECK_PTR("Unlock()", pLock, ERR_PARAMS_NULL);

	swlock_release(&(pLock->m_Lock));

	return NO_ERR;
}
