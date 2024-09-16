#define _DSCRUNIT_C_

#include "DSCRUnit.h"
#include "DSWaveFile.h"
#include "DSmath.h"

static int _Open(PDSCRUNIT pUnit, PCSTR pszPath, PDSCRDATA pDscrBuf, DS_BOOL bAllocateCRUD);
static int _ComputeFHT_Filter(PDSCRUNIT pUnit, PDSCRDATA pDscrBuf);
#if defined(XMOS_FHT_FUNC)
static void _ComputeFHT_xmos(PPOINT_T restrict pData, uint32_t dwDataNum);
#else
static void _ComputeFHT(PPOINT_T restrict pSinTable, PPOINT_T restrict pData, DWORD dwDataNum);
#endif

static POINT_T m_fInvSqrt2;

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSCRUnit) 클래스 생성자			 
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSCRUnit(PDSCRUNIT pUnit)
{
	if( !pUnit ){
		DSTRACE(("[Error - CDSCRUnit( NULL )]!!\n\r"));
		return;
	}

	m_fInvSqrt2 = FIXED_CONV(DS_INVSQRT2);

	memset(pUnit, 0, sizeof(DSCRUNIT));

#if DBG
	CDSCharBuffer( &(pUnit->m_szName), 64 );
	CDSCharBuffer( &(pUnit->m_szPath), KB );
#endif
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSCRUnit) 클래스 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSCRUnitDestroyer(PDSCRUNIT pUnit)
{
	if( !pUnit ){
		DSTRACE(("[Error - CDSCRUnitDestroyer( NULL )]!!\n\r"));
		return;
	}

	if( pUnit->m_bOpened ) CDSCRUnitClose(pUnit);

#if DBG
	CDSCharBufferDestroyer( &(pUnit->m_szName) );
	CDSCharBufferDestroyer( &(pUnit->m_szPath) );
#endif
}

#if DBG
int CDSCRUnitSetName(PDSCRUNIT pUnit, PCHAR pszName)
{ 
	return CDSCharBufferSet( &(pUnit->m_szName), pszName ); 
}
#endif

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSCRUnit)를 연다.
	매개변수: pszPath		-> IR 데이터 파일 경로 문자열 CHAR 포인터
			  bAllocateCRUD	-> true: 컨볼루션 리버브단 데이터 메모리를 할당받는다. 기본값
							  false: 컨볼루션 리버브단 데이터 메모리하지 않는다.
	되돌림값: NO_ERR	-> 컨볼루션 리버브 열기 성공
			  NO_ERR 외	-> 컨볼루션 리버브 열기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSCRUnitOpen(PDSCRUNIT pUnit, PCSTR pszPath, PDSCRDATA pDscrBuf, DS_BOOL bAllocateCRUD)
{
	int RetCode;
	
	DSCHECK_PTR("CDSCRUnit::Open()", pUnit, ERR_PARAMS_NULL);
	DSCHECK_PTR("CDSCRUnit::Open()", pszPath, ERR_PARAMS_NULL);
	DSCHECK_PTR("CDSCRUnit::Open()", pDscrBuf, ERR_PARAMS_NULL);

	RetCode = _Open(pUnit, pszPath, pDscrBuf, bAllocateCRUD);
	if(RetCode != NO_ERR) CDSCRUnitClose(pUnit);
	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSCRUnit)를 연다.
	매개변수: pszPath		-> IR 데이터 파일 경로 문자열 CHAR 포인터
			  bAllocateCRUD	-> true: 컨볼루션 리버브단 데이터 메모리를 할당받는다. 
							  false: 컨볼루션 리버브단 데이터 메모리하지 않는다.
	되돌림값: NO_ERR	-> 컨볼루션 리버브 열기 성공
			  NO_ERR 외	-> 컨볼루션 리버브 열기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
static int _Open(PDSCRUNIT pUnit, PCSTR pszPath, PDSCRDATA pDscrBuf, DS_BOOL bAllocateCRUD)
{
	int RetCode;

	DSTRACE(("[CDSCRUnit::_Open(), %s] Entering...\n\r", pUnit->m_szName.m_pszMem));

	RetCode = CDSWaveFile(pszPath, false);
#if DBG
	if(RetCode != NO_ERR){
		DSTRACE(("[Error - CDSCRUnit::_Open(), %d] 'CDSWaveFile()' failed!! Exiting...\n\r", RetCode));
	}
	CDSCharBufferSet( &(pUnit->m_szPath), (PCHAR)pszPath );
#endif

	if(RetCode == NO_ERR){
		DWORD dwCR_SegNum;
		DWORD dwIR_SampleNum = g_WaveHeader.wBlockAlign ? g_WaveHeader.dwDataSize / g_WaveHeader.wBlockAlign : 8192;
		DWORD dwCR_SampleNum = pUnit->m_dwCR_SampleNum;
		DWORD dwCR_DataNum = pUnit->m_dwCR_DataNum;
		PIRFILTER pIrFilter = &(pUnit->m_IR_Filter);

		if(dwIR_SampleNum >= dwCR_SampleNum) {
			dwCR_SegNum = dwIR_SampleNum / dwCR_SampleNum;
			if(dwIR_SampleNum % dwCR_SampleNum) {
				dwCR_SegNum += 1;
			}
		} else {
			dwCR_SegNum = 1;
		}
		pUnit->m_dwCR_SegNum = dwCR_SegNum;
		pUnit->m_bOpened = true;

#if 0
		DSTRACE(("[CDSCRUnit::_Open, %s] m_pdwBitRevT: 0x%08X, m_dwCR_DataNum: %lu, m_pSinTable: 0x%08X, m_dwCR_SegNum: %u\n\r", 
				pUnit->m_szName.m_pszMem, pUnit->m_pdwBitRevT, pUnit->m_dwCR_DataNum, pUnit->m_pSinTable, pUnit->m_dwCR_SegNum));
#endif

		if( bAllocateCRUD ){
			DWORD dwCR_InBuf_DataNum = dwCR_DataNum * dwCR_SegNum;
			DWORD dwCR_Filter_SampleNum = dwCR_SampleNum * dwCR_SegNum;

			NEW_DSBUFFER("CDSCRUnit::_Open()", pUnit->m_pCR_InBuf, POINT_T, dwCR_InBuf_DataNum, ERR_MEM_NULL);
			ZeroMemory(pUnit->m_pCR_InBuf, sizeof(POINT_T) * dwCR_InBuf_DataNum);
			NEW_DSBUFFER("CDSCRUnit::_Open()", pIrFilter->m_pLaddFilter, POINT_T, dwCR_Filter_SampleNum, ERR_MEM_NULL);
			NEW_DSBUFFER("CDSCRUnit::_Open()", pIrFilter->m_pLsubFilter, POINT_T, dwCR_Filter_SampleNum, ERR_MEM_NULL);
			NEW_DSBUFFER("CDSCRUnit::_Open()", pIrFilter->m_pRaddFilter, POINT_T, dwCR_Filter_SampleNum, ERR_MEM_NULL);
			NEW_DSBUFFER("CDSCRUnit::_Open()", pIrFilter->m_pRsubFilter, POINT_T, dwCR_Filter_SampleNum, ERR_MEM_NULL);

#if 0
			DSTRACE(("[CDSCRUnit::_Open, %s] m_pLaddFilter: 0x%08X, m_pLsubFilter: 0x%08X, m_pRaddFilter: 0x%08X, m_pRsubFilter: 0x%08X\n\r",
					pUnit->m_szName.m_pszMem, pIrFilter->m_pLaddFilter, pIrFilter->m_pLsubFilter, pIrFilter->m_pRaddFilter, pIrFilter->m_pRsubFilter));
#endif

			// IR 데이터를 읽어 온다.
			// IR L/R 데이터 Read 용도로 m_pLaddFilter/m_pRaddFilter 버퍼 사용
			// _ComputeFHT_Filter() 함수에서 m_pLaddFilter/m_pRaddFilter 버퍼의 Data를 DSCRData 버퍼에 재배치
			DSCRUCONTEXT Context;
			// ZeroMemory(pIrFilter->m_pLaddFilter + (sizeof(POINT_T) * dwCR_SampleNum * (dwCR_SegNum - 1)), sizeof(POINT_T) * dwCR_SampleNum);
			// ZeroMemory(pIrFilter->m_pRaddFilter + (sizeof(POINT_T) * dwCR_SampleNum * (dwCR_SegNum - 1)), sizeof(POINT_T) * dwCR_SampleNum);
			Context.pL = pIrFilter->m_pLaddFilter;
			Context.pR = pIrFilter->m_pRaddFilter;
			Context.dwSampleNum = dwCR_Filter_SampleNum;

			RetCode = CDSWaveFileReadFrameSamples(&Context);
			if (RetCode < 1) {
				DSTRACE(("[Error - CDSCRUnit::_Open(), %d] 'CDSWaveFileReadFrameSamples()' failed!! Exiting...\n\r", RetCode));
				RetCode = ERR_NO_DATA;
			} else {
				RetCode = NO_ERR;

				_ComputeFHT_Filter(pUnit, pDscrBuf);
			}
		}else{
			pUnit->m_pCR_InBuf = NULL;

			pIrFilter->m_pLaddFilter = NULL;
			pIrFilter->m_pLsubFilter = NULL;
			pIrFilter->m_pRaddFilter = NULL;
			pIrFilter->m_pRsubFilter = NULL;
		}
	}

	DSTRACE(("[CDSCRUnit::_Open(), %s, %d] Leaving...\n\r", pUnit->m_szName.m_pszMem, RetCode));
	CDSWaveFileDestroyer();
	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSCRUnit)를 닫는다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 컨볼루션 리버브 닫기 성공
			  NO_ERR 외	-> 컨볼루션 리버브 닫기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSCRUnitClose(PDSCRUNIT pUnit)
{
	int RetCode = NO_ERR;
	
	DSCHECK_PTR("CDSCRUnit::Close()", pUnit, ERR_PARAMS_NULL);

	DSTRACE(("[CDSCRUnit::Close(), %s] Entering...\n\r", pUnit->m_szName.m_pszMem));

	DeleteDSArrayMem( pUnit->m_pCR_InBuf );

	PIRFILTER pIrFilter = &(pUnit->m_IR_Filter);
	DeleteDSArrayMem( pIrFilter->m_pLaddFilter );
	DeleteDSArrayMem( pIrFilter->m_pLsubFilter );
	DeleteDSArrayMem( pIrFilter->m_pRaddFilter );
	DeleteDSArrayMem( pIrFilter->m_pRsubFilter );

#if DBG
	CDSCharBufferEmpty(&(pUnit->m_szPath));
#endif
	pUnit->m_bOpened = false;

	DSTRACE(("[CDSCRUnit::Close(), %s, %d] Leaving...\n\r", pUnit->m_szName.m_pszMem, RetCode));
	return RetCode;
}

static int _ComputeFHT_Filter(PDSCRUNIT pUnit, PDSCRDATA pDscrBuf)
{
	int RetCode = NO_ERR;

	DSTRACE(("[CDSCRUnit _ComputeFHT_Filter()] Entering...\n\r"));

	DWORD dwCR_SampleNum = pUnit->m_dwCR_SampleNum;
	DWORD dwCR_DataNum = pUnit->m_dwCR_DataNum;
	DWORD dwCR_SegNum = pUnit->m_dwCR_SegNum;
	PIRFILTER pIrFilter = &(pUnit->m_IR_Filter);

	for(DWORD dwSegIdx = 0; dwSegIdx < dwCR_SegNum; dwSegIdx++) {
		PPOINT_T pL1 = pDscrBuf->m_pL;
		PPOINT_T pR1 = pDscrBuf->m_pR;
		PPOINT_T pEOffset = pL1 + pDscrBuf->m_dwLength;

		PPOINT_T pIR_L = pIrFilter->m_pLaddFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T pIR_R = pIrFilter->m_pRaddFilter + (dwSegIdx * dwCR_SampleNum);

		PDWORD pdwBitRT = pUnit->m_pdwBitRevT;

		do {
			DWORD dwBitRT = *pdwBitRT;

			*pL1++ = pIR_L[dwBitRT];
			*pL1++ = 0;
			*pR1++ = pIR_R[dwBitRT];
			*pR1++ = 0;

			pdwBitRT += 2;
		} while(pL1 < pEOffset);

#if defined(XMOS_FHT_FUNC)
		_ComputeFHT_xmos(pDscrBuf->m_pL, dwCR_SampleNum);
		_ComputeFHT_xmos(pDscrBuf->m_pR, dwCR_SampleNum);
#else
		_ComputeFHT(pUnit->m_pSinTable, pDscrBuf->m_pL, dwCR_DataNum);
		_ComputeFHT(pUnit->m_pSinTable, pDscrBuf->m_pR, dwCR_DataNum);
#endif

#if 0
		DS_PrintDoubleData("[CDSCRUnit::_ComputeFHT_Filter()] pDscrBuf->m_pL", pDscrBuf->m_pL, 0, dwCR_DataNum, 100);
		DS_PrintDoubleData("[CDSCRUnit::_ComputeFHT_Filter()] pDscrBuf->m_pR", pDscrBuf->m_pR, 0, dwCR_DataNum, 100);
#endif

	// Ls, Ld, Rs, Rd 필터값들을 초기화한다.
		PPOINT_T pLadd = pIrFilter->m_pLaddFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T pLsub = pIrFilter->m_pLsubFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T pRadd = pIrFilter->m_pRaddFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T pRsub = pIrFilter->m_pRsubFilter + (dwSegIdx * dwCR_SampleNum);

		pL1 = pDscrBuf->m_pL;
		pR1 = pDscrBuf->m_pR;
		pEOffset = pL1 + dwCR_SampleNum;

		PPOINT_T pL2 = pL1 + dwCR_DataNum;
		PPOINT_T pR2 = pR1 + dwCR_DataNum;

		*pLadd = 2 * *pL1;
		*pLsub = 0;

		*pRadd = 2 * *pR1;
		*pRsub = 0;

		pL1++;
		do {
			pR1++;
			pL2--, pR2--;
			pLadd++, pRadd++, pLsub++, pRsub++;

			POINT_T L1 = *pL1;
			POINT_T L2 = *pL2;
			POINT_T R1 = *pR1;
			POINT_T R2 = *pR2;

			*pLadd = L1 + L2;
			*pLsub = L1 - L2;

			*pRadd = R1 + R2;
			*pRsub = R1 - R2;

			pL1++;
		} while(pL1 < pEOffset);

#if 0
		DS_PrintDoubleData("[CDSCRUnit::_ComputeFHT_Filter()] m_pLaddFilter", pIrFilter->m_pLaddFilter, 0, dwCR_SampleNum, 100);
		DS_PrintDoubleData("[CDSCRUnit::_ComputeFHT_Filter()] m_pLsubFilter", pIrFilter->m_pLsubFilter, 0, dwCR_SampleNum, 100);
		DS_PrintDoubleData("[CDSCRUnit::_ComputeFHT_Filter()] m_pRaddFilter", pIrFilter->m_pRaddFilter, 0, dwCR_SampleNum, 100);
		DS_PrintDoubleData("[CDSCRUnit::_ComputeFHT_Filter()] m_pRsubFilter", pIrFilter->m_pRsubFilter, 0, dwCR_SampleNum, 100);
#endif
	}

	DSTRACE(("[CDSCRUnit::_ComputeFHT_Filter() %d] Leaving...\n\r", RetCode));
	return RetCode;
}

#if defined(XMOS_FHT_FUNC)
void fft_index_bit_reversal_int32(int32_t* a, const unsigned length)
{
	unsigned int logn = u32_ceil_log2(length);

	for(unsigned i = 0; i < length; i++) {
		unsigned rev = n_bitrev(i, logn);

		if(rev< i) continue;

		int32_t tmp = a[i];

		a[i] = a[rev];
		a[rev] = tmp;
	}
}

void _ComputeFHT_xmos(POINT_T *restrict pData, uint32_t dwLength)
{
	fft_index_bit_reversal_int32(pData, dwLength*2);

    exponent_t exp = 0;
    bfp_s32_t samples;
    bfp_s32_init(&samples, pData, exp, dwLength*2, 1);
	
	bfp_complex_s32_t* real_spectrum = bfp_fft_forward_mono(&samples);

	exponent_t spectrum_exp = real_spectrum->exp;
	headroom_t spectrum_hr = real_spectrum->hr;

	int32_t dc = real_spectrum->data[0].re;
	int32_t nq = real_spectrum->data[0].im;

    int32_t  real_data[DEFAULT_IN_CR_SAMPLE_NUM], imag_data[DEFAULT_IN_CR_SAMPLE_NUM];
    bfp_s32_t real, imag;

    bfp_s32_init(&real, real_data, 0, dwLength, 0);
    bfp_s32_init(&imag, imag_data, 0, dwLength, 0);

	bfp_complex_s32_real_part(&real, real_spectrum);
	bfp_complex_s32_imag_part(&imag, real_spectrum);

    right_shift_t real_shr, imag_shr;
	exponent_t final_exp;

    vect_s32_sub_prepare(&final_exp, &real_shr, &imag_shr, spectrum_exp, spectrum_exp, spectrum_hr, spectrum_hr);

	pData[dwLength]  = 0;

	imag_data[0] = nq; //the nq is in the wrong place so take it out for now
	real_data[0] = 0; //the nq is in the wrong place so take it out for now
    vect_s32_add(pData + dwLength, real_data, imag_data, dwLength, real_shr, imag_shr);


	imag_data[0] = 0; //the nq is in the wrong place so take it out for now
	real_data[0] = dc; //the nq is in the wrong place so take it out for now
    vect_s32_sub(pData,            real_data, imag_data, dwLength, real_shr, imag_shr);
	
	for(int i=0; i<dwLength/2; i++){
		int32_t p = pData[dwLength + i + 1];
		pData[dwLength + i + 1] = pData[dwLength + dwLength - 1 - i];
		pData[dwLength + dwLength - 1 - i] = p;
	}

  	vect_s32_shr(pData, pData, dwLength*2, -samples.exp + spectrum_exp - final_exp);
}
#else
/* ------------------------------------------------------------------------
    기    능: 고속 하틀리 변환(FHT: Fast Hartley transform) 연산을 한다.
	          X = FHT(X)
	매개변수: pSinTable	-> 사인표 포인터
		      pData		-> 데이터 포인터
			  dwDataNum	-> 데이터 수
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
static void _ComputeFHT(PPOINT_T restrict pSinTable, PPOINT_T restrict pData, DWORD dwDataNum)
{
	PPOINT_T restrict pOffset = pData;
	PPOINT_T restrict pEOffset = pData + dwDataNum;

	POINT_T a0, a1, a2, a3, a4, a5, a6, a7;
	POINT_T b0, b1, b2, b3, b4, b5, b6, b7;

	pOffset = pData;
	do {
		a0 = pOffset[ 0 ];
		a1 = pOffset[ 1 ];
		a2 = pOffset[ 2 ];
		a3 = pOffset[ 3 ];
		a4 = pOffset[ 4 ];
		a5 = pOffset[ 5 ];
		a6 = pOffset[ 6 ];
		a7 = pOffset[ 7 ];

		// stage 1(2 points) 4회
		b0 = a0 + a1;
		b1 = a0 - a1;
		b2 = a2 + a3;
		b3 = a2 - a3;
		b4 = a4 + a5;
		b5 = a4 - a5;
		b6 = a6 + a7;
		b7 = a6 - a7;

		// stage 2(4 points) 2회
		a0 = b0 + b2;
		a1 = b1 + b3;
		a2 = b0 - b2;
		a3 = b1 - b3;
		a4 = b4 + b6;
		a5 = b5 + b7;
		a6 = b4 - b6;
		a7 = b5 - b7;

		// stage  3(8 points)
		b0 = POINT_MUL_2(m_fInvSqrt2, (a5 + a7));
		b1 = POINT_MUL_2(m_fInvSqrt2, (a5 - a7));

		pOffset[ 0 ] = a0 + a4;
		pOffset[ 1 ] = a1 + b0;
		pOffset[ 2 ] = a2 + a6;
		pOffset[ 3 ] = a3 + b1;
		pOffset[ 4 ] = a0 - a4;
		pOffset[ 5 ] = a1 - b0;
		pOffset[ 6 ] = a2 - a6;
		pOffset[ 7 ] = a3 - b1;

		pOffset += 8;
	} while(pOffset < pEOffset);

	DWORD dwN = 16;
	DWORD dwN2 = 8;
	DWORD dwTheta_Inc = dwDataNum >> 4;	
	DWORD dwCosOffset = dwDataNum >> 2;		// 사인표에서의 코사인 각 위치

	do {
		pOffset = pData;
		do {
			DWORD dwN4 = dwN2 >> 1;
			DWORD dwN4P2 = dwN4 + dwN2;

			a0 = pOffset[ 0 ];
			a1 = pOffset[ dwN2 ];
			a2 = pOffset[ dwN4 ];
			a3 = pOffset[ dwN4P2 ];

			pOffset[ 0 ] = a0 + a1;
			pOffset[ dwN2 ] = a0 - a1;
			pOffset[ dwN4 ] = a2 + a3;
			pOffset[ dwN4P2 ] = a2 - a3;

			PPOINT_T restrict pSinT = pSinTable;
			DWORD a4Offset = 1;
			DWORD a5Offset = dwN2;
			DWORD a6Offset = dwN2;
			DWORD a7Offset = dwN;
			do {
				pSinT += dwTheta_Inc;
				a5Offset++;
				a7Offset--;
				a6Offset--;

				POINT_T SinVal = pSinT[0];
				POINT_T CosVal = pSinT[dwCosOffset];

				a4 = pOffset[ a4Offset ];
				a5 = pOffset[ a5Offset ];
				a6 = pOffset[ a6Offset ];
				a7 = pOffset[ a7Offset ];

				b0 = POINT_MUL_2(a5, CosVal) + POINT_MUL_2(a7, SinVal);
				b1 = POINT_MUL_2(a5, SinVal) - POINT_MUL_2(a7, CosVal);

				pOffset[ a4Offset ] = a4 + b0;
				pOffset[ a5Offset ] = a4 - b0;
				pOffset[ a6Offset ] = a6 + b1;
				pOffset[ a7Offset ] = a6 - b1;

				a4Offset++;
			} while(a4Offset < dwN4);

			pOffset += dwN;
		} while(pOffset < pEOffset);

		dwN <<= 1;
		dwN2 <<= 1;
		dwTheta_Inc >>= 1;
	} while(dwN <= dwDataNum);
}
#endif

/* ------------------------------------------------------------------------
    기    능: (double 형)오디오 데이터를 CR 처리한다.
			  < 주의 >
			  CR 처리 관련 변수들 및 매개변수들에 대한 유효성 검사는
			  호출단에서 미리 하도록 한다.
	매개변수: pData	-> 오디오 데이터 double 포인터
			  pOutBuf -> 오디오 데이터 출력 버퍼 포인터
			  dwSampleNum -> 오디오 데이터 sample 수
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSCRUnitProcess(PDSCRUNIT pUnit, PPOINT_T pData, PDSCRDATA pOutBuf, DWORD dwSampleNum)
{
	if( !pUnit || !pData || !pOutBuf ) {
		DSTRACE(("[Error - CDSCRUnit::Process( NULL )]!!\n\r"));
		return;
	}

#if 0
	DSTRACE(("[CDSCRUnit::Process(), %s] pData: 0x%08X, pOutBuf->m_pL: 0x%08X, pOutBuf->m_pR: 0x%08X\n\r", 
			pUnit->m_szName.m_pszMem, pData, pOutBuf->m_pL, pOutBuf->m_pR));
#endif
#if 0
	if( pUnit->m_bPrinted ) DS_PrintDoubleData("[CDSCRUnit::Process] pData", pData, 0, dwCR_SampleNum, 100);
#endif

	DWORD dwCR_SampleNum = pUnit->m_dwCR_SampleNum;
	DWORD dwCR_DataNum = pUnit->m_dwCR_DataNum;
	DWORD dwCR_SegNum = pUnit->m_dwCR_SegNum;
	PPOINT_T pCurrCR_InBuf = pUnit->m_pCR_InBuf + (dwCR_DataNum * pUnit->m_dwCR_InSegIdx);


	PPOINT_T pOffset = pCurrCR_InBuf;
	PPOINT_T pEOffset = pOffset + dwCR_DataNum;
	DWORD *pdwBitRev = pUnit->m_pdwBitRevT;

	do {
		*pOffset++ = pData[*pdwBitRev];
		*pOffset++ = 0;	// Bit reverse 된 홀수 번째 데이터(window size 이 후의 zero data 영역)를 0으로 초기화

		pdwBitRev += 2;
	} while(pOffset < pEOffset);

#if defined(XMOS_FHT_FUNC)
	_ComputeFHT_xmos(pCurrCR_InBuf, dwCR_SampleNum);
#else
	_ComputeFHT(pUnit->m_pSinTable, pCurrCR_InBuf, dwCR_DataNum);
#endif

#if 0
	if( pUnit->m_bPrinted ) DS_PrintDoubleData("[CDSCRUnit::Process] pCurrCR_InBuf", pCurrCR_InBuf, 0, dwCR_DataNum, 100);
#endif

	PIRFILTER pIrFilter = &(pUnit->m_IR_Filter);
	PPOINT_T restrict pOutL = pOutBuf->m_pL;
	PPOINT_T restrict pOutR = pOutBuf->m_pR;

	ZeroMemory(pOutL, sizeof(POINT_T) * dwCR_DataNum);
	ZeroMemory(pOutR, sizeof(POINT_T) * dwCR_DataNum);
	for(DWORD dwSegIdx = 0; dwSegIdx < dwCR_SegNum; dwSegIdx++) {
		// 하틀리 영역에서 컨볼루션을 한다.
		DWORD dwInSegIdx = pUnit->m_dwCR_InSegIdx + dwSegIdx;
		if(dwInSegIdx >= dwCR_SegNum) dwInSegIdx -= dwCR_SegNum;

		PPOINT_T restrict pD1 = pUnit->m_pCR_InBuf + (dwCR_DataNum * dwInSegIdx);
		PPOINT_T restrict pD2 = pD1 + dwCR_DataNum;
		pEOffset = pD1 + dwCR_SampleNum;

		PPOINT_T restrict pLadd = pIrFilter->m_pLaddFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T restrict pLsub = pIrFilter->m_pLsubFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T restrict pRadd = pIrFilter->m_pRaddFilter + (dwSegIdx * dwCR_SampleNum);
		PPOINT_T restrict pRsub = pIrFilter->m_pRsubFilter + (dwSegIdx * dwCR_SampleNum);

		DWORD *restrict pdwBitR1 = pUnit->m_pdwBitRevT;
		DWORD *restrict pdwBitR2 = pUnit->m_pdwBitRevT + dwCR_DataNum;

		POINT_T Data1 = *pD1;
		DWORD BitR1 = *pdwBitR1;
		POINT_T Ladd = *pLadd;
		POINT_T Radd = *pRadd;
		POINT_T Data2, Lsub, Rsub;
		DWORD BitR2;

		pOutL[BitR1] += POINT_MUL_2(Data1, Ladd);
		pOutR[BitR1] += POINT_MUL_2(Data1, Radd);

		pD1++;
		do {
			pdwBitR1++;
			pLadd++, pRadd++, pLsub++, pRsub++;
			pD2--,	pdwBitR2--;

			Data1 = *pD1, Data2 = *pD2;
			BitR1 = *pdwBitR1, BitR2 = *pdwBitR2;
			Ladd = *pLadd, Radd = *pRadd, Lsub = *pLsub, Rsub = *pRsub;

			pOutL[BitR1] += ( POINT_MUL_2(Data1, Ladd) + POINT_MUL_2(Data2, Lsub) );
			pOutL[BitR2] += ( POINT_MUL_2(Data2, Ladd) - POINT_MUL_2(Data1, Lsub) );

			pOutR[BitR1] += ( POINT_MUL_2(Data1, Radd) + POINT_MUL_2(Data2, Rsub) );
			pOutR[BitR2] += ( POINT_MUL_2(Data2, Radd) - POINT_MUL_2(Data1, Rsub) );

			pD1++;
		} while (pD1 < pEOffset);
	}

#if defined(XMOS_FHT_FUNC)
	_ComputeFHT_xmos(pOutL, dwCR_SampleNum);
	_ComputeFHT_xmos(pOutR, dwCR_SampleNum);
#else
	_ComputeFHT(pUnit->m_pSinTable, pOutL, dwCR_DataNum);
	_ComputeFHT(pUnit->m_pSinTable, pOutR, dwCR_DataNum);
#endif

#if 0
	if( pUnit->m_bPrinted ){
		DS_PrintDoubleData("[CDSCRUnit::Process, STEP5] pOutL", pOutL, 0, dwCR_DataNum, 100);
		DS_PrintDoubleData("[CDSCRUnit::Process, STEP5] pOutR", pOutR, 0, dwCR_DataNum, 100);
	}
	pUnit->m_bPrinted = false;
#endif

	if(pUnit->m_dwCR_InSegIdx == 0) {
		pUnit->m_dwCR_InSegIdx = dwCR_SegNum - 1;
	} else {
		pUnit->m_dwCR_InSegIdx--;
	}
}
