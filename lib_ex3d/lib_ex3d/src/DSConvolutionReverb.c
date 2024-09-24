#define _DSCONVOLUTIONREVERB_C_

#define LOCAL_DEBUG		0

#include "DSConvolutionReverb.h"
#include "DSWaveFile.h"
#include "DSAudioDef.h"
#include "DSmath.h"

#include "xcore/channel.h"
#include "xcore/chanend.h"

#if defined(_DEBUG) || (LOCAL_DEBUG == 1)
#undef DSTRACE
#define DSTRACE(_x_) debug_printf _x_
#else
#define DSTRACE(_x_)
#endif

// 보안 설정 비트임. 바꾸지 말것.
#define DSCR_SET_DSCRUD_BIT			BIT19

static int _Open(DWORD dwChannels, PCHAR pszName, DS_BOOL bCloseIfOpen);
static int _LoadSoundField(PCHAR szName);

static void UnloadSoundField();
static void _MakeBitReverseTable(PDWORD pdwTable, DWORD dwLength);
#if !defined(XMOS_FHT_FUNC)
static void _MakeHalfSineTable(PPOINT_T pTable, DWORD dwLength);
#endif
static DWORD NextPowerOf2(DWORD Val);

static DWORD m_dwTileNum;
/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSConvolutionReverb) 클래스 생성자			 
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSConvolutionReverb(DWORD dwTileNum)
{
	m_dwTileNum = dwTileNum;
	m_bOpened = false;

	m_pdwBitRevT = NULL;
#if !defined(XMOS_FHT_FUNC)
	m_pSinTable = NULL;
#endif
	m_dwBitRevTSize = 0;
	m_dwSinTableSize = 0;

	m_pBUnits = NULL;
	m_uiUnitCount = 0;
	m_StartHAngle = EX3D_CRUD_START_H_ANGLE;

	m_Scale = 0;
	m_ScaleGain_dB = 0;

	m_pTaskUnitInBuf = NULL;
	m_pTaskUnitOutBuf = NULL;
	m_pTaskResultBuf = NULL;
	m_pTaskOverlapBuf = NULL;
	m_ResultBuf.m_bAllocated = FALSE;

	m_dwIN_CR_SampleNum = 0;

	m_dwSFCount = 0;
	m_pSFs = NULL;

	m_pszSFName = (char *)DS_malloc(EX3D_SOUNDFIELD_NAME_MAX_LENGTH + 4);
	*(PDWORD)m_pszSFName = 0;

#if defined(USE_LFE_UNIT)
	m_pLFEUnit = NULL;
	m_LFEGain = 1.0;				// LFE CR: 없음
	m_uiLFEdB = 0;
	m_bEnableLFE = false;
#endif
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSConvolutionReverb) 클래스 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSConvolutionReverbDestroyer()
{
	if( m_bOpened ) CDSConvolutionReverbClose();

	DeleteDSArrayMem( m_pSFs );
	DeleteDSArrayMem( m_pszSFName );
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSConvolutionReverb)를 연다.
	매개변수: pszHRTFDirectory	-> HRTF 디렉토리 문자열 CHAR 포인터
	          dwChannels		-> 음원 채널 개수
	          dwIN_CR_SampleNum	-> 컨볼루션 처리 샘플 개수, 0이면 HRTF 샘플 수 사용
			  pszName			-> 기본 음장 이름
								   NULL: 기본값
			  bCloseIfOpen		-> true: 이미 열려 있다면 닫는다.
								  false: 이미 열려 있다면 오류처리한다.
	되돌림값: NO_ERR	-> 컨볼루션 리버브 열기 성공
			  NO_ERR 외	-> 컨볼루션 리버브 열기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSConvolutionReverbOpen(DWORD dwChannels, DWORD dwIN_CR_SampleNum, PCHAR pszName, DS_BOOL bCloseIfOpen)
{
	int RetCode = NO_ERR;

	if(dwIN_CR_SampleNum != 0) {
		m_dwIN_CR_SampleNum = NextPowerOf2(dwIN_CR_SampleNum);
	} else {
		m_dwIN_CR_SampleNum = 0;
	}
	DSTRACE(("[CDSConvolutionReverb%u::Open()] dwIN_CR_SampleNum: %u, m_dwIN_CR_SampleNum: %u\n\r", m_dwTileNum, dwIN_CR_SampleNum, m_dwIN_CR_SampleNum));

	RetCode = _Open( (dwChannels / EX3D_TILE_NUM), pszName, bCloseIfOpen);

	if((RetCode != NO_ERR) && (RetCode != ERR_OPENED)) {
		CDSConvolutionReverbClose();
	}

	return RetCode;
}

int CDSConvolutionSetSoundField(PCHAR pszName)
{
	PEX3DSOUNDFIELD pSF = NULL;
	int RetCode = NO_ERR;
	if( pszName ){
		pSF = GetEX3DSoundField(pszName);	// 입력 음장 Load
	}

	// 입력 음장이 없으면 default 음장 load
	if(!pSF) {
		char * pDefaultUnitName = (PCHAR)"exir2k_xmos_game_wm";

		pSF = GetEX3DSoundField(pDefaultUnitName);
		if( !pSF ){
			DSTRACE(("[Error - CDSConvolutionReverb%u::_Open()] %s: Not found!! Exiting...\n\r", m_dwTileNum, pDefaultUnitName));
			RetCode = ERR_NOT_FOUND;
		}
	}

	if(RetCode == NO_ERR) {
		RetCode = _LoadSoundField(pSF->szName);
		if(RetCode != NO_ERR){
			DSTRACE(("[Error - CDSConvolutionReverb%u::_Open(), %d] '_LoadSoundField()' Failed!! Exiting...\n\r", m_dwTileNum, RetCode));
		}
	}
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSConvolutionReverb)를 연다.
	매개변수: pszSFDirectory	-> 음장 디렉토리 경로 문자열 CHAR 포인터
			  dwChannels		-> 음원 채널 개수
			  pszName			-> 기본 음장 이름
			  bCloseIfOpen		-> true: 이미 열려 있다면 닫는다.
								  false: 이미 열려 있다면 오류처리한다.
	되돌림값: NO_ERR	-> 컨볼루션 리버브 열기 성공
			  NO_ERR 외	-> 컨볼루션 리버브 열기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
static int _Open(DWORD dwChannels, PCHAR pszName, DS_BOOL bCloseIfOpen)
{
	PEX3DSOUNDFIELD pSF = NULL;
	int RetCode = NO_ERR;

	DSTRACE(("[CDSConvolutionReverb%u::_Open()] Entering...\n\r", m_dwTileNum));

	if( m_bOpened ){
		if( bCloseIfOpen ){
			DSTRACE(("[Warning - CDSConvolutionReverb%u::_Open()] Opened!!\n\r", m_dwTileNum));
			CDSConvolutionReverb(m_dwTileNum);
		}else{
			DSTRACE(("[Error - CDSConvolutionReverb%u::_Open()] Opened!! Exiting...\n\r", m_dwTileNum));
			RetCode = ERR_OPENED;
		}
	}

	if(RetCode == NO_ERR) {
		// 각 채널에 대한 CR단들을 만들어 연다.
		NEW_DSBUFFER("CDSConvolutionReverb::_Open()", m_pBUnits, DSCRUNIT, dwChannels, ERR_MEM_NULL);
		for(DWORD dwIdx = 0; dwIdx < dwChannels; dwIdx++){
			CDSCRUnit(m_pBUnits + dwIdx);
		}

#if defined(USE_LFE_UNIT)
		// LFE CR단을 만든다.
		NEW_DSBUFFER("CDSConvolutionReverb::_Open()", m_pLFEUnit, DSCRUNIT, 1, ERR_MEM_NULL);
		CDSCRUnit(m_pLFEUnit);
#endif

#if DBG
		char pUnitName[] = "BCRU00";
		for(DWORD dwIdx = 0; dwIdx<m_uiUnitCount; dwIdx++){
			DWORD Num = dwIdx+1;
			*(pUnitName+4) = 0x30 + ((Num / 10) % 10);	// 10의 자리 ASCII
			*(pUnitName+5) = 0x30 + (Num % 10);			// 1의 자리 ASCII
			CDSCRUnitSetName( m_pBUnits + dwIdx, pUnitName );
		}

	#if defined(USE_LFE_UNIT)
		CDSCRUnitSetName( m_pLFEUnit, (PCHAR)"LFECRU" );
	#endif
#endif

		m_uiUnitCount = dwChannels;

		// EX-3D 기본 음장 정보을 불려온다.
		if( !m_dwSFCount ) {
			RetCode = CDSConvolutionReverbPrepareEX3DSoundFields();
		}
	}

	if(RetCode == NO_ERR) {
		if( pszName ){
			pSF = GetEX3DSoundField(pszName);	// 입력 음장 Load
		}

		// 입력 음장이 없으면 default 음장 load
		if(!pSF) {
			char * pDefaultUnitName = (PCHAR)"exir2k_xmos_game_wm";

			pSF = GetEX3DSoundField(pDefaultUnitName);
			if( !pSF ){
				DSTRACE(("[Error - CDSConvolutionReverb%u::_Open()] %s: Not found!! Exiting...\n\r", m_dwTileNum, pDefaultUnitName));

				RetCode = ERR_NOT_FOUND;
			}
		}
	}

	if(RetCode == NO_ERR) {
		RetCode = _LoadSoundField(pSF->szName);
		if(RetCode != NO_ERR){
			DSTRACE(("[Error - CDSConvolutionReverb%u::_Open(), %d] '_LoadSoundField()' Failed!! Exiting...\n\r", m_dwTileNum, RetCode));
		}
	}

	if(RetCode == NO_ERR) {
		m_bOpened = true;
	}

	DSTRACE(("[CDSConvolutionReverb%u::_Open(), %d] Leaving...\n\r", m_dwTileNum, RetCode));
	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSConvolutionReverb)를 닫는다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 컨볼루션 리버브 닫기 성공
			  NO_ERR 외	-> 컨볼루션 리버브 닫기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSConvolutionReverbClose()
{
	int RetCode = NO_ERR;
	DSTRACE(("[CDSConvolutionReverb%u::Close()] Entering...\n\r", m_dwTileNum));

	m_bOpened = false;

	UnloadSoundField();

#if defined(XMOS_FHT_FUNC)
	DSTRACE(("[CDSConvolutionReverb%u::Close()] Delete Table - m_pdwBitRevT: 0x%08X\n\r", m_dwTileNum, m_pdwBitRevT));
#else
	DSTRACE(("[CDSConvolutionReverb%u::Close()] Delete Table - m_pdwBitRevT: 0x%08X, m_pSinTable: 0x%08X\n\r", m_dwTileNum, m_pdwBitRevT, m_pSinTable));
	DeleteDSArrayMem(m_pSinTable);
#endif
	DeleteDSArrayMem(m_pdwBitRevT);

#if defined(USE_LFE_UNIT)
	DeleteDSArrayMem( m_pLFEUnit );
#endif
	DeleteDSArrayMem( m_pBUnits );
	m_uiUnitCount = 0;

	DSTRACE(("[CDSConvolutionReverb%u::Close(), %d] Leaving...\n\r", m_dwTileNum, RetCode));
	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 컨볼루션 리버브(CDSConvolutionReverb)가 열려있는지 여부를 검사한다.
	매개변수: 없음
	되돌림값: true	-> 컨볼루션 리버브 열려 있음.
			  false	-> 컨볼루션 리버브 닫혀 있음.
    비    고:
  ------------------------------------------------------------------------ */
DS_BOOL IsOpened()
{
	return m_bOpened;
}

/* ------------------------------------------------------------------------
    기    능: 오디오 데이터 컨볼루션 리버브(CDSConvolutionReverb) 처리를 한다.
	매개변수: pData			-> 오디오 데이터 포인터, 입출력 겸용(입력 planar 형식, 출력 interleave 형식)
			  dwIN_CR_SampleNum	-> 채널당 입력 오디오 데이터 개수
			  CR_BitField	-> 컨볼루션 처리할 channel bit field(bit0:channel1, ...)
	되돌림값: 0 이상	-> 오디오 데이터 컨볼루션 리버브 처리 완료 sample 수
			  0 미만	-> 오디오 데이터 컨볼루션 리버브 처리 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
void ConvolutionProcess(int taskIdx, PPOINT_T pData)
{
	PDSCRDATA pUnitOutBuf = m_pTaskUnitOutBuf + taskIdx;
	PDSCRDATA pResultBuf = m_pTaskResultBuf + taskIdx;
	PDSCRDATA pOverlapBuf = m_pTaskOverlapBuf + taskIdx;

	POINT_T *pUnitOutBL, *pUnitOutBR;
	POINT_T *pUnitOutOverlapBL, *pUnitOutOverlapBR;

	PPOINT_T pResultBL = pResultBuf->m_pL;
	PPOINT_T pResultBR = pResultBuf->m_pR;
	PPOINT_T pOverlapBL = pOverlapBuf->m_pL;
	PPOINT_T pOverlapBR = pOverlapBuf->m_pR;
	PPOINT_T pResultBL_End = pResultBL + m_dwIN_CR_SampleNum;

	// OverlapBuf에서 PlannerBuf로 복사, OverlapBuf 0으로 초기화
	while (pResultBL < pResultBL_End) {
		*pResultBL++ = *pOverlapBL;
		*pResultBR++ = *pOverlapBR;
		*pOverlapBL++ = 0;
		*pOverlapBR++ = 0;
	}

	UINT uiDivideNum = m_uiUnitCount / EX3D_UNIT_TASK_NUM;
	if (m_uiUnitCount % EX3D_UNIT_TASK_NUM) {
		uiDivideNum += 1;
	}
	UINT uiUnitIdx = taskIdx * uiDivideNum;
	UINT uiUnitEndIdx = uiUnitIdx + uiDivideNum;
	if(uiUnitEndIdx > m_uiUnitCount) {
		uiUnitEndIdx = m_uiUnitCount;
	}
	PDSCRUNIT pBUnit = m_pBUnits + uiUnitIdx;
	// uint32_t BitFieldMask = 0x01 << uiUnitIdx;

	while (uiUnitIdx < uiUnitEndIdx) 
	{
		// if (CR_BitField & BitFieldMask)
		{
			// (double 형)오디오 데이터를 CR 처리한다.
			CDSCRUnitProcess(pBUnit, pData + (m_dwIN_CR_SampleNum * uiUnitIdx), pUnitOutBuf, m_dwIN_CR_SampleNum);

			pResultBL = pResultBuf->m_pL;
			pResultBR = pResultBuf->m_pR;
			pOverlapBL = pOverlapBuf->m_pL;
			pOverlapBR = pOverlapBuf->m_pR;
			pUnitOutBL = pUnitOutBuf->m_pL;
			pUnitOutBR = pUnitOutBuf->m_pR;
			pUnitOutOverlapBL = pUnitOutBL + m_dwIN_CR_SampleNum;
			pUnitOutOverlapBR = pUnitOutBR + m_dwIN_CR_SampleNum;

			// Total L,R OutData 생성
			while(pResultBL < pResultBL_End) {
				*pOverlapBL++ += *pUnitOutOverlapBL++;
				*pOverlapBR++ += *pUnitOutOverlapBR++;

				*pResultBL++ += *pUnitOutBL++;
				*pResultBR++ += *pUnitOutBR++;
			}

			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONVOLUTION_UNIT_END);
		}

		pBUnit++;
		uiUnitIdx++;
		// BitFieldMask <<= 1;
	}

#if defined(USE_LFE_UNIT)
	if ( (m_bEnableLFE == true) && (taskIdx == (EX3D_UNIT_TASK_NUM - 1)) ) {
		//DSTRACE(("[CDSConvolutionReverb%u::Process()] m_pLFEUnit CDSCRUnitProcess, *pData: %f\n\r", m_dwTileNum, *(pData + (m_dwIN_CR_SampleNum * m_uiUnitCount))));
		CDSCRUnitProcess(m_pLFEUnit, pData + (m_dwIN_CR_SampleNum * m_uiUnitCount), pUnitOutBuf, m_dwIN_CR_SampleNum);

		pResultBL = pResultBuf->m_pL;
		pResultBR = pResultBuf->m_pR;
		pOverlapBL = pOverlapBuf->m_pL;
		pOverlapBR = pOverlapBuf->m_pR;
		pUnitOutBL = pUnitOutBuf->m_pL;
		pUnitOutBR = pUnitOutBuf->m_pR;
		pUnitOutOverlapBL = pUnitOutBL + m_dwIN_CR_SampleNum;
		pUnitOutOverlapBR = pUnitOutBR + m_dwIN_CR_SampleNum;

		// Total L,R OutData 생성
		//DSTRACE(("[CDSConvolutionReverb%u::Process()] m_pLFEUnit CDSCRUnitProcess, *pUnitOutBL: %f, *pUnitOutBR: %f, m_LFEGain: %f\n\r", m_dwTileNum, *pUnitOutBL, *pUnitOutBR, m_LFEGain));
#if 0
		dsp_vector_muls_addv(pUnitOutOverlapBL, m_LFEGain, pOverlapBL, pOverlapBL, m_dwIN_CR_SampleNum, QVAL);
		dsp_vector_muls_addv(pUnitOutOverlapBR, m_LFEGain, pOverlapBR, pOverlapBR, m_dwIN_CR_SampleNum, QVAL);
		dsp_vector_muls_addv(pUnitOutBL, m_LFEGain, pResultBL, pResultBL, m_dwIN_CR_SampleNum, QVAL);
		dsp_vector_muls_addv(pUnitOutBR, m_LFEGain, pResultBR, pResultBR, m_dwIN_CR_SampleNum, QVAL);
#else
		while(pResultBL < pResultBL_End) {
			*pOverlapBL += POINT_MUL_2(*pUnitOutOverlapBL, m_LFEGain);
			*pOverlapBR += POINT_MUL_2(*pUnitOutOverlapBR, m_LFEGain);
			pOverlapBL++, pOverlapBR++, pUnitOutOverlapBL++, pUnitOutOverlapBR++;

			*pResultBL += POINT_MUL_2(*pUnitOutBL, m_LFEGain);
			*pResultBR += POINT_MUL_2(*pUnitOutBR, m_LFEGain);
			pResultBL++, pResultBR++, pUnitOutBL++, pUnitOutBR++;
		}
#endif

		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONVOLUTION_LFE_UNIT_END);
	}
#endif
}

void ConvolutionTaskInit()
{
    for (int i = 0; i < EX3D_UNIT_TASK_NUM; i++) {
		m_lockConvInBuf[i] = SWLOCK_INITIAL_VALUE;
		m_ConvInBuf[i] = NULL;

		m_lockConvPause[i] = SWLOCK_INITIAL_VALUE;
		m_bConvPause[i] = TRUE;
	}
	m_lockConvResult = SWLOCK_INITIAL_VALUE;
	m_bConvResult = FALSE;
	m_lockConvPauseComplete = SWLOCK_INITIAL_VALUE;
	m_bConvPauseComplete = FALSE;

 	SysTimer = hwtimer_alloc();
}

void ConvolutionBufSet(int taskIdx, PPOINT_T pData, DS_BOOL bBlock)
{
	DS_BOOL bLoop = TRUE;

	while(bLoop) {
        swlock_acquire(&m_lockConvInBuf[taskIdx]);
		if(bBlock) {
			if (m_ConvInBuf[taskIdx] == NULL) {
				m_ConvInBuf[taskIdx] = pData;
				bLoop = FALSE;
			}
		} else {
			m_ConvInBuf[taskIdx] = pData;
			bLoop = FALSE;
		}
		swlock_release(&m_lockConvInBuf[taskIdx]);
    }
}

PPOINT_T ConvolutionBufGet(int taskIdx, DS_BOOL bBlock)
{
	PPOINT_T pBuf;
	DS_BOOL bLoop = TRUE;

	while(bLoop) {
        swlock_acquire(&m_lockConvInBuf[taskIdx]);
		if(bBlock) {
			if (m_ConvInBuf[taskIdx] != NULL) {
				pBuf = m_ConvInBuf[taskIdx];
				bLoop = FALSE;
			}
		} else {
			pBuf = m_ConvInBuf[taskIdx];
			bLoop = FALSE;
		}
		swlock_release(&m_lockConvInBuf[taskIdx]);
    }

	return pBuf;
}

void ConvolutionResultSet()
{
	swlock_acquire(&m_lockConvResult);
	m_bConvResult = TRUE;
	swlock_release(&m_lockConvResult);
}

void ConvolutionResultChk()
{
	DS_BOOL bLoop = TRUE;
	while(bLoop) {
        swlock_acquire(&m_lockConvResult);

        if (m_bConvResult == TRUE) {
			m_bConvResult = FALSE;
			bLoop = FALSE;
		}

		swlock_release(&m_lockConvResult);
    }
}

void ConvolutionPauseSet(int taskIdx, DS_BOOL bSet)
{
	swlock_acquire(&m_lockConvPause[taskIdx]);
	m_bConvPause[taskIdx] = bSet;
	swlock_release(&m_lockConvPause[taskIdx]);
}

DS_BOOL ConvolutionPauseGet(int taskIdx)
{
	DS_BOOL bSet;

	swlock_acquire(&m_lockConvPause[taskIdx]);
	bSet = m_bConvPause[taskIdx];
	swlock_release(&m_lockConvPause[taskIdx]);

	return bSet;
}

void ConvolutionPauseCompleteSet(DS_BOOL bSet)
{
	swlock_acquire(&m_lockConvPauseComplete);
	m_bConvPauseComplete = bSet;
	swlock_release(&m_lockConvPauseComplete);
}

void ConvolutionPauseCompleteChk()
{
	DS_BOOL bLoop = TRUE;
	while(bLoop) {
        swlock_acquire(&m_lockConvPauseComplete);

        if (m_bConvPauseComplete == TRUE) {
			m_bConvPauseComplete = FALSE;
			bLoop = FALSE;
		}

		swlock_release(&m_lockConvPauseComplete);
    }
}

void ConvolutionTask(int taskIdx, chanend c_main_tile_to_sub_tile1)
{
	PPOINT_T pBuf;
	int nextIdx = taskIdx + 1;

	DSTRACE(("ConvolutionTask%u\n\r", m_dwTileNum));
 	while(1) {	
		if(ConvolutionPauseGet(taskIdx) == TRUE) {
			if(nextIdx < EX3D_UNIT_TASK_NUM) {
				if(ConvolutionPauseGet(nextIdx) == FALSE) {
					ConvolutionPauseSet(nextIdx, TRUE);
					//DSTRACE(("[ConvolutionTask%u::ConvolutionPauseSet(%d, TRUE)]\n\r", m_dwTileNum, nextIdx));
				}
			} else {
				ConvolutionPauseCompleteSet(TRUE);
				//DSTRACE(("[ConvolutionTask%u::ConvolutionPauseCompleteSet(TRUE)]\n\r", m_dwTileNum));
			}

			Sleep(1);
			continue;
		} else {
			if(nextIdx < EX3D_UNIT_TASK_NUM) {
				ConvolutionPauseSet(nextIdx, FALSE);
				// DSTRACE(("[ConvolutionTask%u::ConvolutionPauseSet(%d, FALSE)]\n\r", m_dwTileNum, nextIdx));
			} else {
				ConvolutionPauseCompleteSet(FALSE);
				// DSTRACE(("[ConvolutionTask%u::ConvolutionPauseCompleteSet(FALSE)]\n\r", m_dwTileNum));
			}
		}

#if defined(MEASURE_INTER_ELAPSED_TIME_CONV)
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_AUDIOPROCESS_START);
#endif
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_BUF_READ_START);

		if(m_dwTileNum == EX3D_TILE_MAIN) {
		pBuf = ConvolutionBufGet(taskIdx, TRUE);
		}
        swlock_acquire(&m_lockConvInBuf[taskIdx]);
		if(taskIdx == 0) {
			int end = m_dwIN_CR_SampleNum * (m_uiUnitCount + 1);

			if(m_dwTileNum == EX3D_TILE_MAIN) {
				PPOINT_T pInBuf = pBuf + (m_dwIN_CR_SampleNum * m_uiUnitCount);
				// debug_printf("main tile:%x\n", *(pInBuf + m_dwIN_CR_SampleNum));
				chan_out_word(c_main_tile_to_sub_tile1, 1); // sync
				for (int i = 0; i < end; i++) {
					chanend_out_word(c_main_tile_to_sub_tile1, *pInBuf);
					pInBuf++;
				}
			} else {
				PPOINT_T pInBuf = m_pTaskUnitInBuf;
				uint32_t tmp;		
				tmp = chan_in_word(c_main_tile_to_sub_tile1);
				for (int i = 0; i < end; i++) {
					*pInBuf = chanend_in_word(c_main_tile_to_sub_tile1);
					pInBuf++;
				}
				// debug_printf("sub tile1:%x\n", *(m_pTaskUnitInBuf + m_dwIN_CR_SampleNum));
			}

			if(m_dwTileNum == EX3D_TILE_MAIN) {
#if 1
				PPOINT_T pDst = m_pTaskUnitInBuf; 
				PPOINT_T pDstEnd = pDst + (m_dwIN_CR_SampleNum * m_uiUnitCount); 
				PPOINT_T pSrc = pBuf;
				while(pDst < pDstEnd) {
					*pDst++ = *pSrc++;
				}
#else
				memcpy(m_pTaskUnitInBuf, pBuf, (m_dwIN_CR_SampleNum * m_uiUnitCount * sizeof(POINT_T)));
#endif
			}
		}
		swlock_release(&m_lockConvInBuf[taskIdx]);
		if(m_dwTileNum == EX3D_TILE_MAIN) {
		ConvolutionBufSet(taskIdx, NULL, FALSE);
		}
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_BUF_READ_END);

		PDSCRDATA pResultBuf = m_pTaskResultBuf + taskIdx;
		PPOINT_T pResultBL = pResultBuf->m_pL;
		PPOINT_T pResultBR = pResultBuf->m_pR;
		PPOINT_T pResultBL_End = pResultBL + m_dwIN_CR_SampleNum;
		PPOINT_T pOutDataL = m_ResultBuf.m_pL;
		PPOINT_T pOutDataR = m_ResultBuf.m_pR;

		if(taskIdx == 0) {
			while (pResultBL < pResultBL_End) {
				*pOutDataL++ = *pResultBL++;
				*pOutDataR++ = *pResultBR++;
			}
		} else {
			while (pResultBL < pResultBL_End) {
				*pOutDataL++ += *pResultBL++;
				*pOutDataR++ += *pResultBR++;
			}
		}

		if(taskIdx == (EX3D_UNIT_TASK_NUM - 1)) {
			PPOINT_T pOutDataL = m_ResultBuf.m_pL;
			PPOINT_T pOutDataR = m_ResultBuf.m_pR;
			if(m_dwTileNum == EX3D_TILE_MAIN) {
				uint32_t tmp;
				
				tmp = chan_in_word(c_main_tile_to_sub_tile1);
				for (int i = 0; i < m_dwIN_CR_SampleNum; i++) {
					*pOutDataL += chanend_in_word(c_main_tile_to_sub_tile1);
					*pOutDataR += chanend_in_word(c_main_tile_to_sub_tile1);
					pOutDataL++, pOutDataR++;
				}
			} else {
				chan_out_word(c_main_tile_to_sub_tile1, 1); // sync

				for (int i = 0; i< m_dwIN_CR_SampleNum; i++) {
					chanend_out_word(c_main_tile_to_sub_tile1, *pOutDataL);
					chanend_out_word(c_main_tile_to_sub_tile1, *pOutDataR);
					pOutDataL++, pOutDataR++;
				}
			}
		}

		if(nextIdx < EX3D_UNIT_TASK_NUM) {
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_BUF_NEXT_SET_START);
			ConvolutionBufSet(nextIdx, m_pTaskUnitInBuf, TRUE);
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_BUF_NEXT_SET_END);
		} else {
			if(m_dwTileNum == EX3D_TILE_MAIN) {
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_RESULT_SET_START);
			ConvolutionResultSet();
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_RESULT_SET_END);
		}
		}

		ConvolutionProcess(taskIdx, m_pTaskUnitInBuf);

#if defined(MEASURE_INTER_ELAPSED_TIME_CONV)
		DS_ELAPSEDTIMESTAMPLOG(m_dwTileNum);
#endif
	}
}

int CDSConvolutionReverbProcess(PPOINT_T pData, DWORD dwIN_CR_SampleNum, DWORD CR_BitField)
{
	// 입력 data Size 확인
	if(dwIN_CR_SampleNum != m_dwIN_CR_SampleNum) {
		DSTRACE(("[Error - CDSConvolutionReverb%u::Process()] The length of audio data is different!! dwIN_CR_SampleNum: %u, m_dwIN_CR_SampleNum: %u, Exiting...\n\r", m_dwTileNum, dwIN_CR_SampleNum, m_dwIN_CR_SampleNum));
		return ERR_INVALID_LENGTH;
	}

	// DSTRACE(("[CDSConvolutionReverb%u::Process()] m_uiUnitCount: %u, dwIN_CR_SampleNum: %lu, \n\r", m_dwTileNum, m_uiUnitCount, dwIN_CR_SampleNum));

	DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_BUF_SET);
	ConvolutionBufSet(0, pData, TRUE);
	ConvolutionResultChk();
	DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_RESULT_CHK);

	// 역변환 후 scale
#if 0
	dsp_vector_muls(m_ResultBuf.m_pL, m_Scale, pData, m_dwIN_CR_SampleNum, QVAL);
	dsp_vector_muls(m_ResultBuf.m_pR, m_Scale, pData + m_dwIN_CR_SampleNum, m_dwIN_CR_SampleNum, QVAL);
#else
	PPOINT_T pOutDataL = pData;
	PPOINT_T pOutDataR = pData + m_dwIN_CR_SampleNum;
	PPOINT_T pResultBL = m_ResultBuf.m_pL;
	PPOINT_T pResultBR = m_ResultBuf.m_pR;
	PPOINT_T pResultBL_End = pResultBL + m_dwIN_CR_SampleNum;
	while (pResultBL < pResultBL_End) {
		*pOutDataL++ = POINT_MUL_2(*pResultBL, m_Scale);
		*pOutDataR++ = POINT_MUL_2(*pResultBR, m_Scale);

		pResultBL++, pResultBR++;
	}
#endif
	DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONV_SCALE);

	return (int)dwIN_CR_SampleNum;
}

DWORD IN_CR_SampleNum()
{
	return m_bOpened ? m_dwIN_CR_SampleNum : 0;
}

UINT UnitCount()
{
	return m_bOpened ? (m_uiUnitCount * EX3D_TILE_NUM) : 0;
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장들을 읽는다.
	매개변수: pfProc	-> EX-3D 음장 읽기 처리문
						   되돌림값 
						        0: 성공, 처리 계속
							 0 외: 실패, 처리 중지
			  pContext	-> EX-3D 음장 읽기 처리문 콘텍스트
	되돌림값: NO_ERR	-> EX-3D 음장 읽기 처리 성공
			  NO_ERR 외	-> EX-3D 음장 읽기 처리 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSConvolutionReverbReadEX3DSoundFields(PFDSProc pfProc, PVOID pContext)
{
	PEX3DSOUNDFIELD pSF = m_pSFs;
	DWORD dwIdx = 0;
	int RetCode = NO_ERR;

	for(dwIdx = 0; dwIdx<m_dwSFCount; dwIdx++)
	{
		RetCode = pfProc(pContext, pSF++, dwIdx);
		if( RetCode ) break;
	}

	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장들을 준비한다.
	매개변수: pszDirectory	-> EX-3D 음장 최상위 디렉토리 경로 문자열 CHAR 포인터			  
	되돌림값: NO_ERR	-> EX-3D 음장들 준비 성공
			  NO_ERR 외	-> EX-3D 음장들 준비 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSConvolutionReverbPrepareEX3DSoundFields()
{
	DSSZITEM BRIRs[] = {
						{"exir2k_xmos_game_wm", 0}
						, {"exir2k_xmos_movie_wm", 0}
						, {"exir2k_xmos_music_wm", 0}
	};
	DWORD dwItems = sizeof(BRIRs) / sizeof(DSSZITEM);

	m_dwSFCount = 0;
	if (m_pSFs) DS_free(m_pSFs);
	m_pSFs = (PEX3DSOUNDFIELD)DS_malloc(sizeof(EX3DSOUNDFIELD) * dwItems);

	{
		PEX3DSOUNDFIELD pSF = m_pSFs;
		PDSSZITEM pBRIR = BRIRs;
		DWORD dwIdx;

		for (dwIdx = 0; dwIdx < dwItems; dwIdx++) {
			DSTRACE(("[CDSConvolutionReverb%u::PrepareEX3DSoundFields()] Sound Fields: %s\n\r", m_dwTileNum, pBRIR->pBuffer));
			ZeroMemory(pSF, sizeof(EX3DSOUNDFIELD));

			DS_CopyString(pSF->szName, EX3D_SOUNDFIELD_NAME_MAX_LENGTH, pBRIR->pBuffer);

			m_dwSFCount++;
			pSF++;

			pBRIR++;
		}
	}
	DSTRACE(("[CDSConvolutionReverb%u::PrepareEX3DSoundFields()] dwItems: %u, m_dwSFCount: %u\n\r", m_dwTileNum, dwItems, m_dwSFCount));
	return NO_ERR;
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장 정보를 구한다.
	매개변수: pszName	-> EX-3D 음장 이름 문자열 CHAR 포인터
			  uiHAngle	-> 수평각, 0 ~ 359
			  uiVAngle	-> 수직각, 0 ~ 359
	되돌림값: NULL 외	-> EX-3D 음장 정보, EX3DSOUNDFIELD 구조체 포인터
			  NULL		-> EX-3D 음장 정보 구하기 실패, 음장 없음.
    비    고: 
    --------------------------------------------------------------------------------- */
PEX3DSOUNDFIELD GetEX3DSoundField(PCHAR pszName)
{
	if(pszName && m_pSFs){
		PEX3DSOUNDFIELD pSF = m_pSFs;
		PEX3DSOUNDFIELD pEndSF = pSF + m_dwSFCount;

		while(pSF < pEndSF)
		{
			//DSTRACE(("[CDSConvolutionReverb%u::GetEX3DSoundField(%s), SF:%u] pSF->szName: %s\n\r", m_dwTileNum, pszName, (pSF - m_pSFs) / sizeof(EX3DSOUNDFIELD), pSF->szName));
			if( !_stricmp(pSF->szName, pszName) ) return pSF;
			pSF++;
		}
	}
	return NULL;
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장을 설정한다.
			  < 주의 >
			  설정 요청 음장 정보와 다를 경우, 
			  같은 이름의 음장에서 방위각이 가장 가까운 방위각의 음장을 설정한다.
	매개변수: pszName			-> EX-3D 음장 이름
	되돌림값: NO_ERR	-> EX-3D 음장 설정 성공
			  NO_ERR 외	-> EX-3D 음장 설정 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSConvolutionReverbSetEX3DSoundField(PCHAR pszName)
{
	int RetCode;
	DSCHECK_PTR("CDSConvolutionReverb::SetEX3DSoundField()", pszName, ERR_PTRS_NULL);

	DSTRACE(("[CDSConvolutionReverb%u::SetEX3DSoundField(%s)] Entering...\n\r", m_dwTileNum, pszName));

	// 음장을 불려온다.
	RetCode = _LoadSoundField(pszName);
	if(RetCode != NO_ERR){
		UnloadSoundField();
		DSTRACE(("[Error - CDSConvolutionReverb%u::SetEX3DSoundField(%s)] '_LoadSoundField()' failed!! Exiting...\n\r", m_dwTileNum, pszName));
		return RetCode;
	}

	DSTRACE(("[CDSConvolutionReverb%u::SetEX3DSoundField(%s), %d] Leaving...\n\r", m_dwTileNum, pszName, RetCode));
	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장 컨볼루션 리버브 데이터를 초기화 한다.
	매개변수: 없음
	되돌림값: NO_ERR	-> EX-3D 음장 컨볼루션 리버브 데이터 초기화 성공
			  NO_ERR 외	-> EX-3D 음장 컨볼루션 리버브 데이터 초기화  실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSConvolutionReverbClearEX3DSoundFieldsBuffer()
{
	int RetCode = NO_ERR;

	DSTRACE(("[CDSConvolutionReverb%u::ClearEX3DSoundFieldsBuffer()] Entering...\n\r", m_dwTileNum));

	PDSCRDATA pDscrBuf = m_pTaskUnitOutBuf;
	for(int i = 0; i < EX3D_UNIT_TASK_NUM; i++){
		CDSCRDataInit(pDscrBuf);
		pDscrBuf++;
	}

	DSTRACE(("[CDSConvolutionReverb%u::ClearEX3DSoundFieldsBuffer(), RetCode:%d] Leaving...\n\r", m_dwTileNum, RetCode));

	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장을 불려온다.
			  < 주의 >
			  매개변수 유효성을 검사하지 않기 때문에 호출단에서 검사해야 한다.
	매개변수: szName	-> EX-3D 음장 이름
	되돌림값: NO_ERR	-> EX-3D 음장 불려오기 성공
			  NO_ERR 외	-> EX-3D 음장 불려오기 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
static int _LoadSoundField(PCHAR szName)
{
	int RetCode = NO_ERR;

	DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField(%s), m_uiUnitCount: %u] Entering...\n\r", m_dwTileNum, szName, m_uiUnitCount));
	/* --------------------------------------------------------------
		음장이 같으면 해당 위치의 컨볼루션 리버브단 데이터만 설정한다.
		다르면 전체 위치의 모든 컨볼루션 리버브단 데이터들을 모두 불려온다.
	   -------------------------------------------------------------- */
	if(_stricmp(m_pszSFName, szName)){
		char pszBuf[EX3D_SOUNDFIELD_NAME_MAX_LENGTH * 2];
		DWORD dwBufSize, dwSfNameLen;
		DWORD dwCR_DataNum;
		DWORD dwHAngle, dwVAngle, dwHAIncrement, dwVAIncrement, dwIdx;

		dwBufSize = sizeof(pszBuf);
		
		DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField, Change SoundField: From: %s To:%s]\n\r", m_dwTileNum,m_pszSFName, szName));

		ConvolutionPauseSet(0, TRUE);
		DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField, ConvolutionPauseSet(TRUE)]\n\r", m_dwTileNum));
		ConvolutionPauseCompleteChk();
		DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField, ConvolutionPauseCompleteChk()]\n\r", m_dwTileNum));
		// 이전 EX-3D 음장을 모두 없앤다.
		if( m_pszSFName[0] ) UnloadSoundField();
		DS_CopyString(m_pszSFName, EX3D_SOUNDFIELD_NAME_MAX_LENGTH, szName);
		//alexy
		//DS_CopyString(pszBuf, dwBufSize, szName);
		DS_CopyString(pszBuf, dwBufSize, "exir2k_xmos_game_wm");
		dwSfNameLen = strlen(pszBuf);
		if(dwSfNameLen > EX3D_SOUNDFIELD_NAME_MAX_LENGTH) {
			dwSfNameLen = EX3D_SOUNDFIELD_NAME_MAX_LENGTH;
		}
		if(m_dwIN_CR_SampleNum == 0) {
			DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField()] m_dwIN_CR_SampleNum(%u) is set DEFAULT_IN_CR_SAMPLE_NUM(%u)\n\r", m_dwTileNum, m_dwIN_CR_SampleNum, DEFAULT_IN_CR_SAMPLE_NUM));
			m_dwIN_CR_SampleNum = DEFAULT_IN_CR_SAMPLE_NUM;
			dwCR_DataNum = DEFAULT_IN_CR_SAMPLE_NUM;
		} else {
			if(m_dwIN_CR_SampleNum > DEFAULT_IN_CR_SAMPLE_NUM) {
				dwCR_DataNum = m_dwIN_CR_SampleNum;
			} else {
				dwCR_DataNum = DEFAULT_IN_CR_SAMPLE_NUM;
			}
		}
		dwCR_DataNum <<= 1;
		DS_BOOL bNeedNewDscrBuffer;
		if(m_pTaskUnitOutBuf == NULL) {
			NEW_DSBUFFER("CDSConvolutionReverb::_LoadSoundField()", m_pTaskUnitOutBuf, DSCRDATA, EX3D_UNIT_TASK_NUM, ERR_MEM_NULL);
			bNeedNewDscrBuffer = TRUE;
		} else {
			bNeedNewDscrBuffer = FALSE;
		}
		PDSCRDATA pDscrBuf = m_pTaskUnitOutBuf;
		for(dwIdx = 0; dwIdx < EX3D_UNIT_TASK_NUM; dwIdx++){
			if(bNeedNewDscrBuffer == TRUE) {
				NEW_PDSCRD_BUFFER("CDSConvolutionReverb::_LoadSoundField()", pDscrBuf, dwCR_DataNum, RetCode);
			} else {
				CDSCRDataInit(pDscrBuf);
			}
			pDscrBuf++;
		}

		if (m_dwBitRevTSize != dwCR_DataNum) {
			DeleteDSArrayMem(m_pdwBitRevT);
		}
		if (m_pdwBitRevT == NULL) {
			m_dwBitRevTSize = dwCR_DataNum;
			NEW_DSBUFFER("CDSConvolutionReverb::_LoadSoundField()", m_pdwBitRevT, DWORD, dwCR_DataNum, ERR_MEM_NULL);
			_MakeBitReverseTable(m_pdwBitRevT, dwCR_DataNum);
			DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField()] Create Table - m_pdwBitRevT: 0x%08x\n\r", m_dwTileNum, m_pdwBitRevT));
#if 0
			DS_PrintDWData("[CDSConvolutionReverb::_LoadSoundField()] m_pdwBitRevT", m_pdwBitRevT, dwCR_DataNum - 10, dwCR_DataNum, 10);
#endif
		}

#if !defined(XMOS_FHT_FUNC)
		if (m_dwSinTableSize != dwCR_DataNum) {
			DeleteDSArrayMem(m_pSinTable);
		}
		if (m_pSinTable == NULL) {
			m_dwSinTableSize = dwCR_DataNum;
			NEW_DSBUFFER("CDSConvolutionReverb::_LoadSoundField()", m_pSinTable, POINT_T, dwCR_DataNum, ERR_MEM_NULL);
			_MakeHalfSineTable(m_pSinTable, dwCR_DataNum);
			DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField()] Create Table - m_pSinTable: 0x%08x\n\r", m_dwTileNum, m_pSinTable));
#if 0
			DS_PrintDoubleData("[CDSConvolutionReverb::_LoadSoundField()] m_pSinTable", m_pSinTable, dwCR_DataNum - 10, dwCR_DataNum, 10);
#endif
		}
#endif

		// 음장에 대한 컨볼루션 리버브단 데이터를 불러온다.
		// 위치 초기각, 증가각을 설정한다.
		// 수직각
#if defined(USE_ONLY_HORIZONTALITY)
		// 90 의 수직각 HRTF 사용
		dwVAngle = 90;
#else
		// 0, 90, 180 의 수직각 HRTF 사용
		dwVAngle = 0;
#endif
		dwVAIncrement = 90;

		// 수평각
		// dwHAngle = m_StartHAngle;
		dwHAngle = m_dwTileNum * (EX3D_CRUD_STEP_H_ANGLE * (EX3D_CRUD_H_POSITION_COUNT / EX3D_TILE_NUM));
		dwHAIncrement = EX3D_CRUD_STEP_H_ANGLE;

		for(dwIdx = 0; dwIdx < m_uiUnitCount; dwIdx++){
			PDSCRUNIT pUnit = &m_pBUnits[ dwIdx ];

			DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField()] dwIdx: %u, dwHAngle: %u\n\r", m_dwTileNum, dwIdx, dwHAngle));

			DS_CopyString(pszBuf + dwSfNameLen, (dwBufSize - dwSfNameLen), "/v090h000.wav");
		#if !defined(USE_ONLY_HORIZONTALITY)
			// VAngle
			*(pszBuf + dwSfNameLen + 2) = 0x30 + ((dwVAngle / 100) % 10);	// 100의 자리 ASCII
			*(pszBuf + dwSfNameLen + 3) = 0x30 + ((dwVAngle / 10) % 10);	// 10의 자리 ASCII
			*(pszBuf + dwSfNameLen + 4) = 0x30 + (dwVAngle % 10);			// 1의 자리 ASCII
		#endif

			// HAngle
			*(pszBuf + dwSfNameLen + 6) = 0x30 + ((dwHAngle / 100) % 10);	// 100의 자리 ASCII
			*(pszBuf + dwSfNameLen + 7) = 0x30 + ((dwHAngle / 10) % 10);	// 10의 자리 ASCII
			*(pszBuf + dwSfNameLen + 8) = 0x30 + (dwHAngle % 10);			// 1의 자리 ASCII

			pUnit->m_pdwBitRevT = m_pdwBitRevT;
#if !defined(XMOS_FHT_FUNC)
			pUnit->m_pSinTable = m_pSinTable;
#endif
			pUnit->m_dwCR_DataNum = dwCR_DataNum;
			pUnit->m_dwCR_SampleNum = dwCR_DataNum >> 1;
			pUnit->m_dwCR_InSegIdx = 0;

			RetCode = CDSCRUnitOpen(pUnit, pszBuf, m_pTaskUnitOutBuf, true);	// 방위 각에 대한 음장 파일들을 설정한다.
			if(RetCode != NO_ERR){
				if((dwIdx == 0) || ((dwIdx % 8) != 0)) {
					DSTRACE(("[Error - CDSConvolutionReverb%u::_LoadSoundField(%s), %d] Failed to open the Unit%u. Exiting...\n\r", m_dwTileNum, szName, RetCode, dwIdx + 1));
					break;
				} else {
					// 수직각 0도의 음장만 있는 경우 수직각 90,180도의 음장도 0도의 음장으로 설정
					dwIdx--;
					dwVAngle = 0;
					continue;
				}
			}

			dwHAngle += dwHAIncrement;
			if(dwHAngle >= 360) {
				dwHAngle = 0;
				dwVAngle += dwVAIncrement;
			}
		}

#if defined(USE_LFE_UNIT)
		// LFE 컨볼루션 리버브단을 연다.
		if( m_pLFEUnit ){
			DS_CopyString(pszBuf + dwSfNameLen, (dwBufSize - dwSfNameLen), "/lfe.wav");

			m_pLFEUnit->m_pdwBitRevT = m_pdwBitRevT;
#if !defined(XMOS_FHT_FUNC)
			m_pLFEUnit->m_pSinTable = m_pSinTable;
#endif
			m_pLFEUnit->m_dwCR_DataNum = dwCR_DataNum;
			m_pLFEUnit->m_dwCR_SampleNum = dwCR_DataNum >> 1;
			m_pLFEUnit->m_dwCR_InSegIdx = 0;

			RetCode = CDSCRUnitOpen(m_pLFEUnit, pszBuf, m_pTaskUnitOutBuf, true);
			if(RetCode != NO_ERR){
				DSTRACE(("[Error - CDSConvolutionReverb%u::_LoadSoundField(%s), %d] Failed to open the E-Unit%u. Exiting...\n\r", m_dwTileNum, szName, RetCode, dwIdx + 1));
			}
		}
#endif

		if (RetCode == NO_ERR) {
			m_Scale = FIXED_CONV( (DS_POW(10, m_ScaleGain_dB / 20.0) / dwCR_DataNum) / 2 );

			// double 버퍼를 윈도우 크기로 만든다.
			if(m_pTaskUnitInBuf == NULL) {
				NEW_DSBUFFER("CDSConvolutionReverb::_LoadSoundField()", m_pTaskUnitInBuf, POINT_T, (m_dwIN_CR_SampleNum * (m_uiUnitCount + 1)), ERR_MEM_NULL);
			}

			DS_BOOL bNeedNewDscrBuffer;
			if(m_pTaskResultBuf == NULL) {
				NEW_DSBUFFER("CDSConvolutionReverb::_LoadSoundField()", m_pTaskResultBuf, DSCRDATA, EX3D_UNIT_TASK_NUM, ERR_MEM_NULL);
				bNeedNewDscrBuffer = TRUE;
			} else {
				bNeedNewDscrBuffer = FALSE;
			}
			pDscrBuf = m_pTaskResultBuf;
			for(dwIdx = 0; dwIdx < EX3D_UNIT_TASK_NUM; dwIdx++){
				if(bNeedNewDscrBuffer == TRUE) {
					NEW_PDSCRD_BUFFER("CDSConvolutionReverb::_LoadSoundField()", pDscrBuf, m_dwIN_CR_SampleNum, RetCode);
				} else {
					CDSCRDataInit(pDscrBuf);
				}
				pDscrBuf++;
			}

			if(m_pTaskOverlapBuf == NULL) {
				NEW_DSBUFFER("CDSConvolutionReverb::_LoadSoundField()", m_pTaskOverlapBuf, DSCRDATA, EX3D_UNIT_TASK_NUM, ERR_MEM_NULL);
				bNeedNewDscrBuffer = TRUE;
			} else {
				bNeedNewDscrBuffer = FALSE;
			}
			pDscrBuf = m_pTaskOverlapBuf;
			for(dwIdx = 0; dwIdx < EX3D_UNIT_TASK_NUM; dwIdx++){
				if(bNeedNewDscrBuffer == TRUE) {
					NEW_PDSCRD_BUFFER("CDSConvolutionReverb::_LoadSoundField()", pDscrBuf, m_dwIN_CR_SampleNum, RetCode);
				} else {
					CDSCRDataInit(pDscrBuf);
				}
				pDscrBuf++;
			}

			if(m_ResultBuf.m_bAllocated == FALSE) {
				NEW_DSCRD_BUFFER("CDSConvolutionReverb::_LoadSoundField()", m_ResultBuf, m_dwIN_CR_SampleNum, RetCode);
			}
		}

		ConvolutionPauseSet(0, FALSE);
		DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField, ConvolutionPauseSet(FALSE)]\n\r", m_dwTileNum));
	}

	DSTRACE(("[CDSConvolutionReverb%u::_LoadSoundField(%s), RetCode:%d] Leaving...\n\r", m_dwTileNum, szName, RetCode));

	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: 불려온 EX-3D 음장을 없앤다.
	매개변수: 없음
	되돌림값: 없음
    비    고: 
    --------------------------------------------------------------------------------- */
static void UnloadSoundField()
{
	DSTRACE(("[CDSConvolutionReverb%u::UnloadSoundField()] Entering...\n\r", m_dwTileNum));

#if defined(USE_LFE_UNIT)
	if(m_pLFEUnit) {
		CDSCRUnitDestroyer(m_pLFEUnit);
	}
#endif

	if(m_pBUnits) {
		for (DWORD dwIdx = 0; dwIdx < m_uiUnitCount; dwIdx++) {
			CDSCRUnitDestroyer(m_pBUnits + dwIdx);
		}
	}

	*(PDWORD)m_pszSFName = 0;

#if 0
	DeleteDSArrayMem( m_pTaskUnitInBuf );

	PDSCRDATA pDscrBuf = m_pTaskUnitOutBuf;
	for(int i = 0; i < EX3D_UNIT_TASK_NUM; i++){
		CDSCRDataDestroyer(pDscrBuf);
		pDscrBuf++;
	}
	DeleteDSArrayMem( m_pTaskUnitOutBuf );

	pDscrBuf = m_pTaskResultBuf;
	for(int i = 0; i < EX3D_UNIT_TASK_NUM; i++){
		CDSCRDataDestroyer(pDscrBuf);
		pDscrBuf++;
	}
	DeleteDSArrayMem( m_pTaskResultBuf );

	pDscrBuf = m_pTaskOverlapBuf;
	for(int i = 0; i < EX3D_UNIT_TASK_NUM; i++){
		CDSCRDataDestroyer(pDscrBuf);
		pDscrBuf++;
	}
	DeleteDSArrayMem( m_pTaskOverlapBuf );

	CDSCRDataDestroyer(&m_ResultBuf);
#endif

	DSTRACE(("[CDSConvolutionReverb%u::UnloadSoundField()] Leaving...\n\r", m_dwTileNum));
}

#if defined(USE_LFE_UNIT)
/* --------------------------------------------------------------------------------
    기    능: EX-3D LFE 사용 유무 및 Gain 설정.
    매개변수: bEnable	-> LFE 사용 유무
			  idBGain	-> 감쇠 dB
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int SetLFE(DS_BOOL bEnable, INT32 idBGain)
{
	m_bEnableLFE = bEnable;
	if(bEnable) {
		m_LFEGain = FIXED_CONV( DS_POW(10, (idBGain / 20.0)) );
		m_uiLFEdB = idBGain;

		DSTRACE(("[CDSConvolutionReverb%u::SetLFE(%d)] m_LFEGain: %f\n\r", m_dwTileNum, idBGain, m_LFEGain));
	} else {
		m_LFEGain = 0;
		m_uiLFEdB = 100;

		DSTRACE(("[CDSConvolutionReverb%u::SetLFE(%d)] MUTE!! m_LFEGain: %f\n\r", m_dwTileNum, idBGain, m_LFEGain));
	}

	return NO_ERR;
}

DWORD GetLFEdB()
{
	return m_uiLFEdB;
}
#endif

/* ------------------------------------------------------------------------
	기    능: 비트 역방향표를 만든다.
	매개변수: pdwTable	-> 비트 역방향표 DWORD 포인터
			  dwLength	-> 비트 역방향표 길이
	되돌림값: 없음
	비    고:
  ------------------------------------------------------------------------ */
static void _MakeBitReverseTable(PDWORD pdwTable, DWORD dwLength)
{
	PDWORD pdwOffset;
	DWORD dwBits, dwV;

	// log(dwV, 2)의 unsigned int 값을 구한다.
	// 즉, 최상위 비트를 찾는다.
	dwBits = 0;
	dwV = dwLength;
	while (dwV > 1)
	{
		dwV >>= 1;
		dwBits++;
	}

	dwV = 0;
	pdwOffset = pdwTable;
	for (dwV = 0; dwV < dwLength; dwV++) {
		DWORD dwX = dwV;
		DWORD dwY = 0;
		DWORD dwV2 = dwBits;

		while (dwV2--)
		{
			dwY = (dwY + dwY) + (dwX & 1);
			dwX >>= 1;
		}

		*pdwOffset++ = dwY;
	}
}

#if !defined(XMOS_FHT_FUNC)
/* ------------------------------------------------------------------------
	기    능: 반 사인표를 만든다.
	매개변수: pTable	-> 반 사인표 double 포인터
			  dwLength	-> 반 사인표 길이
	되돌림값: 없음
	비    고:
  ------------------------------------------------------------------------ */
static void _MakeHalfSineTable(PPOINT_T pTable, DWORD dwLength)
{
	float fV = (float)DS_2PI / dwLength;
	PPOINT_T pS = pTable;
	DWORD dwIdx = 0;

	for (dwIdx = 0; dwIdx < dwLength; dwIdx++) *pS++ = FIXED_CONV( DS_SIN(fV * dwIdx) );
}
#endif

static DWORD NextPowerOf2(DWORD Val)
{
	DWORD nextPowerOf2 = 1;
	while (nextPowerOf2 < Val) {
		nextPowerOf2 *= 2;
	}

	return nextPowerOf2;
}
