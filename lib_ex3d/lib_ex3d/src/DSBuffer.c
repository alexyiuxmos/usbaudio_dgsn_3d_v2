#define _DSBUFFER_C_

#include "DSBuffer.h"

/* ------------------------------------------------------------------------
    기    능: 바이트 버퍼 생성자
	매개변수: dwSize	-> 버퍼 바이트 크기
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
int CDSBuffer(PCDSBUFFER pBuf, DWORD dwSize)
{
	DSCHECK_PTR("CDSBuffer()", pBuf, ERR_PARAMS_NULL);

	pBuf->m_pBuf = (PBYTE)DS_malloc(dwSize);
	DSCHECK_MEM("CDSBuffer()", pBuf->m_pBuf, ERR_MEM_NULL);

	pBuf->m_dwSize = dwSize;
	pBuf->m_dwLength = 0;

	return NO_ERR;
}

/* ------------------------------------------------------------------------
    기    능: 바이트 버퍼 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferDestroyer(PCDSBUFFER pBuf)
{
	DSCHECK_PTR("CDSBufferDestroyer()", pBuf, ERR_PARAMS_NULL);

	if(pBuf->m_pBuf){
		DS_free(pBuf->m_pBuf);
		pBuf->m_pBuf = NULL;
	}
	pBuf->m_dwSize = 0;
	pBuf->m_dwLength = 0;

	return NO_ERR;
}

/* ------------------------------------------------------------------------
    기    능: 빈 버퍼 크기가 최소 바이트 크기보다 클 경우 버퍼에 데이터를 넣는다.
	매개변수: pData		-> 데이터 포인터
			  dwSize	-> 추가할 데이터 (바이트)크기
			  dwMinSize	-> 최소 바이트 크기
	되돌림값: > -1	-> 넣은 데이터 길이
			  < 0	-> 데이터 넣기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferEnQ(PCDSBUFFER pBuf, PBYTE pData, DWORD dwSize, DWORD dwMinSize)
{
	DWORD dwWLength;
	DSCHECK_PTR("CDSBuffer::EnQ()", pBuf, ERR_PARAMS_NULL);
	DSCHECK_2PTRS("CDSBuffer::::EnQ()", pBuf->m_pBuf, pData, ERR_PARAMS_NULL);

	dwWLength = pBuf->m_dwSize - pBuf->m_dwLength;
	if(dwWLength < dwMinSize) return 0;

	if(dwWLength >= dwSize) dwWLength = dwSize;
	if( dwWLength ){
		CopyMemory(pBuf->m_pBuf + pBuf->m_dwLength, pData, dwWLength);
		pBuf->m_dwLength += dwWLength;
	}

	return (int)dwWLength;
}

/* ------------------------------------------------------------------------
    기    능: 빈 버퍼 크기가 최소 바이트 크기보다 클 경우,
	          EnQ 버퍼 포인터를 데이터 바이크 크기만큼 증가시킨다.
	매개변수: dwSize	-> 데이터 (바이트)크기
			  dwMinSize	-> 최소 바이트 크기
	되돌림값: > -1	-> 증가시킨 데이터 바이트 크기
			  < 0	-> EnQ 버퍼 포인터 증가 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferIncreaseEnQPointer(PCDSBUFFER pBuf, DWORD dwSize, DWORD dwMinSize)
{
	DWORD dwWLength;
	DSCHECK_PTR("CDSBuffer::IncreaseEnQPointer()", pBuf, ERR_PARAMS_NULL);
	DSCHECK_PTR("CDSBuffer::IncreaseEnQPointer()", pBuf->m_pBuf, ERR_PARAMS_NULL);

	dwWLength = pBuf->m_dwSize - pBuf->m_dwLength;
	if(dwWLength < dwMinSize) return 0;

	if(dwWLength >= dwSize) dwWLength = dwSize;
	if( dwWLength ) pBuf->m_dwLength += dwWLength;

	return (int)dwWLength;
}

/* ------------------------------------------------------------------------
    기    능: 버퍼에서 데이터를 빼낸다.
	매개변수: pBuf		-> 빼낸 데이터를 저장할 데이터 버퍼 포인터
			  dwSize	-> 빼낼 데이터 길이(=바이트 크기)
	되돌림값: 뺀 데이터 길이(=바이트 크기)
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferDeQ(PCDSBUFFER pBuf, PBYTE pData, DWORD dwSize, DWORD dwMinSize)
{
	DWORD dwRLength;
	DSCHECK_PTR("CDSBuffer::DeQ()", pBuf, ERR_PARAMS_NULL);
	DSCHECK_2PTRS("CDSBuffer::DeQ()", pBuf->m_pBuf, pData, ERR_PARAMS_NULL);
	if(pBuf->m_dwLength < dwMinSize) return 0;

	dwRLength = pBuf->m_dwLength < dwSize ? pBuf->m_dwLength : dwSize;
	if( dwRLength ){
		CopyMemory(pData, pBuf->m_pBuf, dwRLength);

		pBuf->m_dwLength -= dwRLength;
		if( pBuf->m_dwLength ) DSWinAPI_memcpy(pBuf->m_pBuf, pBuf->m_pBuf + dwRLength, pBuf->m_dwLength);
	}

	return (int)dwRLength;
}

/* ------------------------------------------------------------------------
    기    능: 버퍼에서 데이터를 빼낸다.
	매개변수: dwLength	-> 빼낼 데이터 길이(=바이트 크기)
			  pfProc	-> 빼낸 데이터 처리 프로시저
						   NULL: 데이터만 빼낸다. 제거 처리와 같다.
			  pContext	-> 프로시저 콘텍스트
	되돌림값: 뺀 데이터 길이(=바이트 크기)
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferDeQ2(PCDSBUFFER pBuf, DWORD dwLength, PFDSProc pfProc, PVOID pContext)
{
	DWORD dwRLength;
	DSCHECK_PTR("CDSBuffer::DeQ2()", pBuf, ERR_PARAMS_NULL);
	DSCHECK_PTR("CDSBuffer::DeQ2()", pBuf->m_pBuf, ERR_PARAMS_NULL);

	dwRLength = pBuf->m_dwLength < dwLength ? pBuf->m_dwLength : dwLength;
	if( dwRLength ){
		if( pfProc ) pfProc(pContext, pBuf->m_pBuf, dwRLength);
		pBuf->m_dwLength -= dwRLength;
		if( pBuf->m_dwLength ) DSWinAPI_memcpy(pBuf->m_pBuf, pBuf->m_pBuf + dwRLength, pBuf->m_dwLength);
	}

	return (int)dwRLength;
}

/* ------------------------------------------------------------------------
    기    능: 버퍼에서 데이터를 빼낸다.
			  빼낼 데이터 길이 이상 버퍼에 데이터가 있을 경우만 빼낸다.
	매개변수: dwLength	-> 빼낼 데이터 길이(=바이트 크기)
			  pfProc	-> 빼낸 데이터 처리 프로시저
						   NULL: 데이터만 빼낸다. 제거 처리와 같다.
			  pContext	-> 프로시저 콘텍스트
	되돌림값: 뺀 데이터 길이(=바이트 크기)
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferDeQ3(PCDSBUFFER pBuf, DWORD dwLength, PFDSProc pfProc, PVOID pContext)
{
	if(pBuf->m_dwLength >= dwLength){
		DSCHECK_PTR("CDSBuffer::DeQ3()", pBuf, ERR_PARAMS_NULL);
		DSCHECK_PTR("CDSBuffer::DeQ3()", pBuf->m_pBuf, ERR_PARAMS_NULL);

		if( pfProc ) pfProc(pContext, pBuf->m_pBuf, dwLength);
		pBuf->m_dwLength -= dwLength;
		if( pBuf->m_dwLength ) DSWinAPI_memcpy(pBuf->m_pBuf, pBuf->m_pBuf + dwLength, pBuf->m_dwLength);
		
		return (int)dwLength;
	} 
	return 0;
}

/* ------------------------------------------------------------------------
    기    능: 버퍼를 초기화한다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 비퍼 초기화 설공
			  NO_ERR 외	-> 비퍼 초기화 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferClear(PCDSBUFFER pBuf)
{
	DSCHECK_PTR("CDSBuffer::Clear()", pBuf, ERR_PARAMS_NULL);
	DSCHECK_PTR("CDSBuffer::Clear()", pBuf->m_pBuf, ERR_PARAMS_NULL);

	pBuf->m_dwLength = 0;

	return NO_ERR;
}

/* ------------------------------------------------------------------------
    기    능: 사용 가능한 크기를 구한다.
	매개변수: 없음
	되돌림값: 사용 가능한 바이트 크기
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferAvailableSize(PCDSBUFFER pBuf)
{
	DSCHECK_PTR("CDSBuffer::AvailableSize()", pBuf, ERR_PARAMS_NULL);

	return pBuf->m_dwSize - pBuf->m_dwLength;
}

/* ------------------------------------------------------------------------
    기    능: 할당받은 메모리를 먼저 해제 한 후, 새 메모리를 할당받는다.
	매개변수: dwSize	-> 버퍼 바이트 크기
	되돌림값: NO_ERR	-> 버퍼 메모리 생성 성공
			  NO_ERR 외	-> 버퍼 메모리 생성 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferNew(PCDSBUFFER pBuf, DWORD dwSize)
{
	DSCHECK_PTR("CDSBuffer::New()", pBuf, ERR_PARAMS_NULL);

	if(pBuf->m_pBuf) {
		DS_free(pBuf->m_pBuf);
	}
	pBuf->m_pBuf = (PBYTE)DS_malloc(dwSize);
	DSCHECK_MEM("CDSBuffer::New()", pBuf->m_pBuf, ERR_MEM_NULL);

	pBuf->m_dwSize = dwSize;
	pBuf->m_dwLength = 0;

	return NO_ERR;
}

/* ------------------------------------------------------------------------
    기    능: 새 메모리를 할당받아 기존 메모리의 내용을 복사한다.
	매개변수: dwSize	-> 버퍼 바이트 크기
	되돌림값: NO_ERR	-> 버퍼 메모리 생성 성공
			  NO_ERR 외	-> 버퍼 메모리 생성 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSBufferExpansion(PCDSBUFFER pBuf, DWORD dwSize)
{
	DSCHECK_PTR("CDSBuffer::Expansion()", pBuf, ERR_PARAMS_NULL);

	PBYTE pData = (PBYTE)DS_malloc(dwSize);
	DSCHECK_MEM("CDSBuffer::Expansion()", pData, ERR_MEM_NULL);

	CopyMemory(pData, pBuf->m_pBuf, pBuf->m_dwLength);
	DS_free(pBuf->m_pBuf);

	pBuf->m_pBuf = pData;
	pBuf->m_dwSize = dwSize;

	return NO_ERR;
}
