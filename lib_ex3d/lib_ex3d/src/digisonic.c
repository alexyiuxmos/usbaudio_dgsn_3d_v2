#include "digisonic.h"

#include <ctype.h>
#include <string.h>

#include <time.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EXTERN_C_BEGIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef	int (*PFDSToCaseProc)(int ch);

/* --------------------------------------------------------------------------------
	기    능: 메모리 중첩 영역이 있다면, memmove() 함수를 사용하여 메모리를 복사한다.
			  < 주의 >
			  pDest, pSrc에 __restrict 키워드를 사용하면 안된다.
			  __restrict 키워드는 같은 메모리 공간이 아닐 경우(프로그래머가 미리 알고 있어야 함),
			  메모리 번지를 컴파일러가 최적화하여 사용하도록 한다.
	매개변수: pDest	-> 복사 메모리 VOID 포인터
			  pSrc	-> 원본 메모리 VOID 포인터
			  Dsize	-> 복사 데이터 바이트 크기
	되돌림값: NULL 외	-> 메모리 복사 성공, pDest 포인터
			  NULL		-> 메모리 복사 실패
	비    고:
	--------------------------------------------------------------------------------- */
void* DSWinAPI_memcpy(void* pDest, const void* pSrc, size_t Dsize)
{
	PBYTE pEnd;
	if(pSrc > pDest){
		pEnd = (PBYTE)pDest + Dsize + sizeof(PVOID);
		return pEnd < (PBYTE)pSrc ? memcpy(pDest, pSrc, Dsize) : memmove(pDest, pSrc, Dsize);
	}
	else if(pSrc < pDest){
		pEnd = (PBYTE)pSrc + Dsize + sizeof(PVOID);
		return pEnd < (PBYTE)pDest ? memcpy(pDest, pSrc, Dsize) : memmove(pDest, pSrc, Dsize);
	}
	// 복사 메모리와 원본 메모리 번지가 같을 경우는 복사하지 않고 복사 메모리만 되돌린다.
	return pDest;
}

/* --------------------------------------------------------------------------------
    기    능: Tick 시간을 구한다.
	매개변수: 없음
	되돌림값: Tick 시간, 단위: 100ns
    비    고: 
    --------------------------------------------------------------------------------- */
QWORD DS_GetTickTime(void)
{
	return hwtimer_get_time(SysTimer); // / 100;
}

/* ------------------------------------------------------------------------
    기    능: CHAR 문자열을 복사한다.
	매개변수: pDest			-> 사본 문자열 버퍼 포인터
			  dwDestSize	-> 사본 문자열 버퍼 크기
			  pSrc	-> 원본 문자열 버퍼 포인터
	되돌림값: > -1	-> 복사된 문자열 길이
	          < 0	-> 복사 실패, 오류값	
    비    고: 
  ------------------------------------------------------------------------ */
LONG DS_CopyString(PCHAR pDest, DWORD dwLength, const char* pSrc)
{
	if( pDest ){
		if( pSrc ){
			PCHAR pOffset = pDest;
			PCHAR pEOffset = dwLength ? pOffset + (dwLength - 1) : pOffset;

			if(pSrc == pDest) pOffset = pEOffset;
			else while(*pSrc && (pOffset < pEOffset)) *pOffset++ = *pSrc++;

			*pOffset = 0;
			return (LONG)(pOffset - pDest);
		}
		*pDest = 0;		
	}
	DSTRACE(("[Error - DS_CopyString()] Invalid pointer!! pDest: 0x%8p, pSrc: 0x%8p, Exiting...\n", pDest, pSrc));
	return ERR_PTRS_NULL;
}

#if defined(MEASURE_INTER_ELAPSED_TIME)
const char * ElapsedLogStr[ELAPSED_TIME_LOG_STR_NUM] = {
	"Start",
	"EnQ InQ",
	"DeQ InQ",
	"Normalize",
	"Upmix",
	"SoundPanning",

	"ConvBufSet",
	"ConvResultChk",
	"ConvScale",

	"ConvBufReadStart",
	"ConvBufReadEnd",
	"ConvBufNextSetStart",
	"ConvBufNextSetEnd",
	"ConvResultSetStart",
	"ConvResultSetEnd",

	"Unit",
	"LFE Unit",
	"Limiter",
	"Denormalize",
	"EnQ OutQ",
	"DeQ OutQ",
};

unsigned int g_ElapsedLogCnt;
#define TICK_1US    100
#define ELAPSED_TIME_STAMP_NUM	ELAPSED_TIME_LOG_STR_NUM + 27	// Convolution Unit이 최대 8번 표시
ELAPSED_TIME_STAMP ElapsedStamp[ELAPSED_TIME_STAMP_NUM];

void DS_ElapsedTimeStamp(DWORD strIdx)
{
	if(strIdx == 0) g_ElapsedLogCnt = 0;
	if(
#if defined(MEASURE_INTER_ELAPSED_TIME_CONV)
		(strIdx == 0) || ( (ELAPSED_TIME_LOG_STR_CONV_BUF_READ_START <= strIdx) && ( strIdx <= ELAPSED_TIME_LOG_STR_CONVOLUTION_UNIT_END) )
#else
		(strIdx < ELAPSED_TIME_LOG_STR_CONV_BUF_READ_START) || ( strIdx > ELAPSED_TIME_LOG_STR_CONVOLUTION_UNIT_END)
#endif
 	)
	{
		if(g_ElapsedLogCnt < ELAPSED_TIME_STAMP_NUM) {
			ElapsedStamp[g_ElapsedLogCnt].strIdx = strIdx;
			ElapsedStamp[g_ElapsedLogCnt].tick = DS_GetTickTime();
			g_ElapsedLogCnt++;
		}
	}
}

void DS_ElapsedTimeStampLog(DWORD dwTileNum)
{
	DWORD CurrIdxTick, PrevIdxTick;
	DWORD ElapsedTick;
	DWORD TotalElapsedTick = 0;

	// ElapsedStamp[0]는 표시하지 않음
	if(g_ElapsedLogCnt > 4) {
		// DS_Print("0Tick:%u, ", ElapsedStamp[0].tick / TICK_1US);
		for (int i=1; i<g_ElapsedLogCnt; i++) {
			PrevIdxTick = ElapsedStamp[i-1].tick;
			CurrIdxTick = ElapsedStamp[i].tick;

			if(CurrIdxTick > PrevIdxTick) {
				ElapsedTick = CurrIdxTick - PrevIdxTick;
			} else {
				ElapsedTick = (0xffffffff - PrevIdxTick) + CurrIdxTick + 1;
			}
			debug_printf("%s%u:%u, ", ElapsedLogStr[ElapsedStamp[i].strIdx], dwTileNum, ElapsedTick / TICK_1US);
			TotalElapsedTick += ElapsedTick;
		}
		debug_printf("Total%u:%u\r\n", dwTileNum, TotalElapsedTick / TICK_1US);
	}

	g_ElapsedLogCnt = 0;
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EXTERN_C_END
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
