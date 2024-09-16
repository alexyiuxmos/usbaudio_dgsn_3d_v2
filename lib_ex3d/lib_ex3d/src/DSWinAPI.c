#define _DSWINAPI_C_

#include "DSWinAPI.h"

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fcntl.h>
#include <unistd.h>

#include <limits.h>

#include <errno.h>
#include <ctype.h>

#include "DSEvent.h"

/* ------------------------------------------------------------------------
    기    능: 디버그 글을 출력한다.
	매개변수: lpOutputString -> 디버그 글
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void Sleep(DWORD dwMilliseconds)
{
	delay_milliseconds(dwMilliseconds);
}

/* ------------------------------------------------------------------------
    기    능: 현재 틱카운트를 구한다.
	매개변수: 없음
	되돌림값: 틱카운트
    비    고:
  ------------------------------------------------------------------------ */
DWORD GetTickCount()
{
	return hwtimer_get_time(SysTimer) / 100000;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
