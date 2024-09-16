#define _DSCRDATA_C_

#include "DSCRData.h"

static int New(PDSCRDATA pData, DWORD wLength, DS_BOOL bDeleteOldData);
static int _New(PDSCRDATA pData, DWORD dwLength);
static void Delete(PDSCRDATA pData);
static void _InitParams(PDSCRDATA pData);

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브 데이터(=IR 데이터) 클래스 생성자
			  보안을 위해 이름 IR Data 대신 CR(=Convolution Reverb) Data으로 사용한다.
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
int CDSCRData(PDSCRDATA pData, DWORD dwLength)
{
	int RetCode = NO_ERR;

	DSCHECK_PTR("CDSCRData()", pData, ERR_PARAMS_NULL);

	_InitParams(pData);
	if( dwLength ) {
		RetCode = New(pData, dwLength, false);
	}

	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브 데이터(=IR 데이터) 클래스 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
int CDSCRDataDestroyer(PDSCRDATA pData)
{
	DSCHECK_PTR("CDSCRData()", pData, ERR_PARAMS_NULL);

	if( pData->m_bAllocated ) Delete(pData);

	return NO_ERR;
}

static int New(PDSCRDATA pData, DWORD dwLength, DS_BOOL bDeleteOldData)
{
	int RetCode;
	
	if( pData->m_bAllocated ){
		if( bDeleteOldData ){
			DSTRACE(("[Warning - CDSCRData::New( %u )] Old data was deleted!!\n", dwLength));
			Delete(pData);
		}else{
			DSTRACE(("[Error - CDSCRData::New( %u )] Data is in use!! Exiting...\n", dwLength));
			return ERR_DATA_IS_IN_USE;
		}
	}

	RetCode = _New(pData, dwLength );
	if(RetCode != NO_ERR) Delete(pData);
	return RetCode;
}

static int _New(PDSCRDATA pData, DWORD dwLength)
{	
	if(dwLength < 8){
		DSTRACE(("[Error - CDSCRData::_New( %u )] Invalid length!! Minimum length: 8, Exiting...\n", dwLength));
		return ERR_INVALID_LENGTH;
	}

	NEW_DSBUFFER("CDSCRData::_New( L )", pData->m_pL, POINT_T, (dwLength + 1), ERR_MEM_NULL);
	ZeroMemory(pData->m_pL, sizeof(POINT_T) * (dwLength + 1));

	NEW_DSBUFFER("CDSCRData::_New( R )", pData->m_pR, POINT_T, (dwLength + 1), ERR_MEM_NULL);
	ZeroMemory(pData->m_pR, sizeof(POINT_T) * (dwLength + 1));

	pData->m_dwLength = dwLength;
	pData->m_bAllocated = true;

	//DSTRACE(("[CDSCRData::_New( %lu )] m_pL: 0x%08X, m_pR: 0x%08X\n", dwLength, m_pL, m_pR));
	return NO_ERR;
}

static void Delete(PDSCRDATA pData)
{
	//DSTRACE(("[CDSCRData::Delete()] m_pL: 0x%08X, m_pR: 0x%08X\n", m_pL, m_pR));

	DeleteDSArrayMem( pData->m_pL );
	DeleteDSArrayMem( pData->m_pR );
	pData->m_dwLength = 0;

	pData->m_bAllocated = false;
}

/* ------------------------------------------------------------------------
    기    능: 변수들을 초기화한다.
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
static void _InitParams(PDSCRDATA pData)
{
	pData->m_pL = NULL;
	pData->m_pR = NULL;
	pData->m_dwLength = 0;
	pData->m_bAllocated = false;
}

/* ------------------------------------------------------------------------
    기    능: 초기화한다.
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSCRDataInit(PDSCRDATA pData)
{
	if( pData->m_bAllocated ){
		DWORD dwSize = sizeof( POINT_T ) * pData->m_dwLength;

		//DSTRACE(("[CDSCRData::Init()] m_pL: 0x%08X, m_pR: 0x%08X, dwSize: %lu\n", m_pL, m_pR, dwSize));

		if( pData->m_pL ) ZeroMemory(pData->m_pL, dwSize);
		if( pData->m_pR ) ZeroMemory(pData->m_pR, dwSize);
	}
}
