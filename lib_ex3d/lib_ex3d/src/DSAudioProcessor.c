#define _DSAUDIOPROCESSOR_C_

#define LOCAL_Q_DEBUG	0
#define LOCAL_DEBUG		0

#include "DSAudioProcessor.h"

#include "DSmath.h"

#include "DSAudioDef.h"
#include "DSWaveFile.h"
#include "peakLimiter.h"

#if defined(_DEBUG) || (LOCAL_DEBUG == 1)
#undef DSTRACE
#define DSTRACE(_x_) debug_printf _x_
#else
#define DSTRACE(_x_)
#endif

#if (LOCAL_Q_DEBUG == 1)
#define DSTRACEQ(_x_) debug_printf _x_
#else
#define DSTRACEQ(_x_)
#endif

POINT_T fGainM3dB;
POINT_T fGainM6dB;

#if defined(USE_OS)
  #define CDSEVENT(pEvent, bManualReset, bInitialState, lpName)	CDSEvent(pEvent, bManualReset, bInitialState, lpName)
  #define CDSEVENT_DESTROYER(pEvent)							CDSEventDestroyer(pEvent)
  #define EVENT_SET(pEvent)										EventSet(pEvent)
  #define EVENT_RESET(pEvent)									EventReset(pEvent)
  #define EVENT_WAITFORSINGLE(pEvent, dwMilliseconds)			EventWaitForSingle(pEvent, dwMilliseconds)

  #define CDSLOCK(pLock)				CDSLock(pLock)
  #define CDSLOCK_DESTROYER(pLock)		CDSLockDestroyer(pLock)
  #define LOCK(pLock, dwTimeout_ms)		Lock(pLock, dwTimeout_ms)
  #define UNLOCK(pLock)					Unlock(pLock)

  int OnProcessing();
#else
  #define CDSEVENT(pEvent, bManualReset, bInitialState, lpName)
  #define CDSEVENT_DESTROYER(pEvent)
  #define EVENT_SET(pEvent)
  #define EVENT_RESET(pEvent)
  #define EVENT_WAITFORSINGLE(pEvent, dwMilliseconds)	OnProcessing()

  #define CDSLOCK(pLock)
  #define CDSLOCK_DESTROYER(pLock)
  #define LOCK(pLock, dwTimeout_ms)
  #define UNLOCK(pLock)
	
#endif
#if 0
static void LevelMeter(PPOINT_T pBuf, DWORD dwChannels, DWORD dwSampleNum);
#endif
static void InitProcessing();
static int UpdateParameterAndFading();

static void UpdateSizeVariable();
static int UpdateProcessBufSize(DS_BOOL bCreateQ);
static void Normalize(void * pInBuf, PPOINT_T pNormalizedBuf, DWORD dwInChannels, DWORD dwSampleNum);
static DWORD Denormalize_FadeProcess(PPOINT_T pInBuf, void * pOutBuf, DWORD dwSampleNum, FADE_STATE FadeState);

#if defined(USE_UPMIX)
static void Upmix(PPOINT_T pProcessBuf, DWORD dwSampleNum);
#endif
static DWORD SoundPanning(PPOINT_T pInBuf, PPOINT_T pPanOutBuf, DWORD dwInChannels, DWORD dwPanChannels, DWORD dwSampleNum);

static int DeQ(PBYTE pBuf, DWORD dwSize);
static int DeQEx(PBYTE pBuf, DWORD dwSize, DWORD dwMinSize);
#if defined(_DEBUG) || (LOCAL_DEBUG == 1)
static DWORD OutQueueFilledLength();
#endif
static DWORD OutQueueAvailableSize();
static int DequeueOutQ(PBYTE pBuf, DWORD dwSize);

static void CaculatePositionPanningGain(UINT uiInChannels);
static void _CaculatePanningGain(UINT uiInx, PEX3DCHANNEL pChannel, DS_BOOL bUseVAngle);

/* --------------------------------------------------------------------------------
    기    능: 3채널 이상 오디오 데이터를 스테레오 오디오 데이터로 만든다.
	          다운믹싱 방식: ITU-R BS.775(2012.08) 다운믹싱 방식
			  < 주의 >
			  매개변수의 유효성을 검사하지 않으며, 16비트 오디오 데이터만을 처리한다.
	매개변수: pInBuf	-> 입력 오디오 데이터 POINT_T 포인터
			  pOutBuf	-> 출력 오디오 데이터 POINT_T 포인터
			  dwSampleNum	-> 오디오 샘플 개수
			  uiChannels	-> 채널 개수
			  fGain			-> 감쇠 Gain
	되돌림값: 스테레오 오디오 데이터 길이
    비    고: 
    --------------------------------------------------------------------------------- */
int DSAP_DownMixToStereo(PPOINT_T pInBuf, PPOINT_T pOutBuf, DWORD dwSampleNum, UINT uiChannels, POINT_T fGain)
{
	PPOINT_T pFL = pInBuf;
	PPOINT_T pFR = pFL + dwSampleNum;
	PPOINT_T pC = pFR + dwSampleNum;
	PPOINT_T pLfe = pC + dwSampleNum;
	PPOINT_T pBL = pLfe + dwSampleNum;
	PPOINT_T pBR = pBL + dwSampleNum;
	PPOINT_T pSL = pBR + dwSampleNum;
	PPOINT_T pSR = pSL + dwSampleNum;
	PPOINT_T pBC = pSL;
	PPOINT_T pOutFL = pOutBuf;
	PPOINT_T pOutFR = pOutFL + dwSampleNum;
	POINT_T fC, fLfe, fBL, fBR, fSL, fSR, fBC;

	// DSTRACE(("[CDSAudioProcessor::DSAP_DownMixToStereo()], dwSampleNum: %u, uiChannels: %u, fGain:%f\n\r", dwSampleNum, uiChannels, FLOAT_CONV(fGain) ));

	switch(uiChannels) {
	case 1:
        for (int i = 0; i < dwSampleNum; i++) {
            *pOutFL++ = POINT_MUL_2( *pFL, fGain );

            pFL++;
		}
		break;
	case 2:
        for (int i = 0; i < dwSampleNum; i++) {
            *pOutFL++ = POINT_MUL_2( *pFL, fGain );
            *pOutFR++ = POINT_MUL_2( *pFR, fGain );

            pFL++, pFR++;
		}
		break;
	case 3:
        for (int i = 0; i < dwSampleNum; i++) {
			fC = POINT_MUL_2(*pC, fGainM3dB);

            *pOutFL++ = POINT_MUL_2( (*pFL + fC), fGain );
            *pOutFR++ = POINT_MUL_2( (*pFR + fC), fGain );

            pFL++, pFR++, pC++;
        }
		break;
	case 4:
        for (int i = 0; i < dwSampleNum; i++) {
			fBL = *pBL;
			fBR = *pBR;

            *pOutFL++ = POINT_MUL_2( (*pFL + fBL), fGain );
            *pOutFR++ = POINT_MUL_2( (*pFR + fBR), fGain );

            pFL++, pFR++, pBL++, pBR++;
        }
		break;
	case 5:
        for (int i = 0; i < dwSampleNum; i++) {
			fC = POINT_MUL_2(*pC, fGainM3dB);
			fBL = *pBL;
			fBR = *pBR;

            *pOutFL++ = POINT_MUL_2( (*pFL + fC + fBL), fGain );
            *pOutFR++ = POINT_MUL_2( (*pFR + fC + fBR), fGain );

            pFL++, pFR++, pC++, pBL++, pBR++;
        }
		break;
	case 6:		// 5.1(=6) 채널
        for (int i = 0; i < dwSampleNum; i++) {
			fC = POINT_MUL_2(*pC, fGainM3dB);
			fLfe = *pLfe;
			fBL = *pBL;
			fBR = *pBR;

            *pOutFL++ = POINT_MUL_2( (*pFL + fC + fLfe + fBL), fGain );
            *pOutFR++ = POINT_MUL_2( (*pFR + fC + fLfe + fBR), fGain );

            pFL++, pFR++, pC++, pLfe++, pBL++, pBR++;
        }
		break;
	case 7:
        for (int i = 0; i < dwSampleNum; i++) {
			fC = POINT_MUL_2(*pC, fGainM3dB);
			fLfe = *pLfe;
			fBL = *pBL;
			fBR = *pBR;
			fBC = *pBC;

            *pOutFL++ = POINT_MUL_2( (*pFL + fC + fLfe + fBL + fBC), fGain );
            *pOutFR++ = POINT_MUL_2( (*pFR + fC + fLfe + fBR + fBC), fGain );

            pFL++, pFR++, pC++, pLfe++, pBL++, pBR++, pBC++;
        }
		break;
	default:
		if(uiChannels > 7){	// 7.1(=8) 채널 이상
			for (int i = 0; i < dwSampleNum; i++) {
				fC = POINT_MUL_2(*pC, fGainM3dB);
				fLfe = *pLfe;
				fBL = *pBL;
				fBR = *pBR;
				fSL = *pSL;
				fSR = *pSR;

				*pOutFL++ = POINT_MUL_2( (*pFL + fC + fLfe + fBL + fSL), fGain );
				*pOutFR++ = POINT_MUL_2( (*pFR + fC + fLfe + fBR + fSR), fGain );

				pFL++, pFR++, pC++, pLfe++, pBL++, pBR++, pSL++, pSR++;
			}
		}
		break;
	}

	return (int)(dwSampleNum * 2 * sizeof(POINT_T));
}

/* --------------------------------------------------------------------------------
    기    능: DS 오디오 프로세서, CDSAudioProcessor 클래스 생성자
	매개변수: 없음
	되돌림값: 없음
    비    고: 
    --------------------------------------------------------------------------------- */
void CDSAudioProcessor()
{
	fGainM3dB = FIXED_CONV( DS_POW(10, -3/20.0) );
	fGainM6dB = FIXED_CONV( DS_POW(10, -6/20.0) );

#if defined(USE_OS)
	m_hThread = false;

	m_bThreadRunning = false;
	m_State = Stop_DSState;
#else
	m_hThread = false;
#endif

	m_dwID = 0;
	memset(&m_InQ, 0, sizeof(CDSBUFFER));
	m_dwInQPutLength = 0;
	memset(&m_OutQ, 0, sizeof(CDSBUFFER));

	m_pProcessBuf1 = NULL;
	m_pProcessBuf2 = NULL;
	m_dwProcessBuf1Size = 0;
	m_dwProcessBuf2Size = 0;

	m_dwSampleSize = 4096;
	m_bClear = false;
	m_bEnabledEX3D = true;
#if defined(USE_OS)
	m_bPause = false;
#endif
	m_bEndOfStream = false;
	m_bFullChannelOutput = false;
	m_AudioDataType = AUDIO_DATA_TYPE_INT16;

	m_bWaitProcess = false;

	m_uiEX3DAudioPanningType = ConstantPower_DSPanningType;
	//m_uiEX3DAudioPanningType = ConstantGain_DSPanningType;
#if defined(USE_UPMIX)
	m_bEnUpmixFunction = false;
#endif
#ifdef _DEBUG
	m_bFirstAudioData = false;
#endif

	m_fGainValue = 1.0f;
	//m_fOffGainValue = FIXED_CONV( DS_POW(10, (-4/20.0)) );		// -4dB: -0.2 = -4/20
	m_fOffGainValue = FIXED_CONV(1.0f);	// 0 dB

	m_dwSRHz = 48000;
	m_dwFlags = 0;

	CDSCharBuffer( &m_szCurrentSFName, EX3D_SOUNDFIELD_NAME_MAX_LENGTH );
	CDSCharBufferSet(&m_szCurrentSFName, (PCHAR)"exir2k_xmos_game_wm");

	ZeroMemory(&m_EX3DChannel, sizeof( EX3DCHANNEL ) * EX3D_CHANNEL_COUNT);
	for(int i=0; i<EX3D_CHANNEL_COUNT; i++ ) {
		m_EX3DChannel[i].ChGain = FIXED_CONV(1.0);
		m_EX3DChannel[i].VTargetBGain = FIXED_CONV(1.0);
	}

	ZeroMemory(&m_EX3DAngle, sizeof( EX3DANGLE ) * EX3D_CHANNEL_COUNT);
	CDSEVENT(&m_Event, false, false, NULL);
	CDSEVENT(&m_InQEvent, false, false, NULL);
	CDSEVENT(&m_OutQEvent, false, false, NULL);
	CDSConvolutionReverb(EX3D_TILE_MAIN);
}

/* --------------------------------------------------------------------------------
    기    능: DS 오디오 프로세서, CDSAudioProcessor 클래스 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고: 
    --------------------------------------------------------------------------------- */
void CDSAudioProcessorDestroyer()
{
	DSTRACE(("[CDSAudioProcessorDestroyer()] Entering...\n\r"));
	if( m_hThread ) CDSAudioProcessorClose();
	CDSConvolutionReverbDestroyer();
	destroyLimiter();
	CDSEVENT_DESTROYER(&m_Event);
	CDSEVENT_DESTROYER(&m_InQEvent);
	CDSEVENT_DESTROYER(&m_OutQEvent);

	CDSCharBufferDestroyer( &m_szCurrentSFName );
	DSTRACE(("[CDSAudioProcessorDestroyer()] Leaving...\n\r"));
}

/* --------------------------------------------------------------------------------
    기    능: DS 오디오 프로세서를 연다.
	매개변수: dwChannels			-> 채널 개수
			  dwSampleSize			-> 샘플 바이트 크기
									   최소 크기: 64 바이트
			  dwSRHz				-> 샘플링 레이트, 단위: Hz
			  dwAudioDataSize		-> Audio data 바이트 크기
			  dwFlags				-> 플래그
	되돌림값: NO_ERR	-> DS 오디오 프로세서 열기 성공
			  NO_ERR 외	-> DS 오디오 프로세서 열기 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSAudioProcessorOpen(DWORD dwChannels,
							DWORD dwSampleSize, 
							DWORD dwSRHz, 
							DWORD dwAudioDataSize,
							DWORD dwFlags)
{
	int RetCode = NO_ERR;
	DWORD dwIN_CR_SampleNum = 0;
	DSTRACE(("[CDSAudioProcessor::Open(%u, %u Hz, %u)] Entering... dwFlags:0x%08x\n\r", dwSampleSize, dwSRHz, dwAudioDataSize, dwFlags));

	if( m_hThread ){
		// 오디오 프로세서가 동작 중이다...
		RetCode = ERR_OPENED;
	}else{
#ifdef _DEBUG
		m_bFirstAudioData = false;
#endif

#if defined(USE_UPMIX)
		m_bEnUpmixFunction = dwFlags & EX3D_UPMIX_BIT ? true : false;
#endif
		m_dwChannels = dwChannels < 1 ? 1 : dwChannels;
		m_dwSampleSize = dwSampleSize < 64 ? 64 : dwSampleSize;
		m_dwAudioDataSize = dwAudioDataSize < 2 ? 2 : dwAudioDataSize;
		if(dwFlags & EX3D_FLOATING_POINT_BIT) {
			m_AudioDataType = AUDIO_DATA_TYPE_FLOAT;
		} else if(m_dwAudioDataSize == 2) {
			m_AudioDataType = AUDIO_DATA_TYPE_INT16;
		} else {
			m_AudioDataType = AUDIO_DATA_TYPE_INT32;
		}

		m_bWaitProcess = false;

		if(dwFlags & EX3D_FULL_CHANNEL_OUTPUT_BIT){
			m_bFullChannelOutput = true;
		}else{
			m_bFullChannelOutput = false;
		}

		m_dwFlags &= ~DSAP_EX3D_SOUNDFIELD_CHANGED_BIT;
		if(dwFlags & DSAP_INPUT_LENGTH_AS_CR_LENGTH_BIT) {
			dwIN_CR_SampleNum = m_dwSampleSize / m_dwChannels / m_dwAudioDataSize;
		}
		RetCode = CDSConvolutionReverbOpen(EX3D_CRUD_TOTAL_POSITION_COUNT, dwIN_CR_SampleNum, m_szCurrentSFName.m_pszMem, false);
		if(RetCode != NO_ERR) return RetCode;

		PeakLimiter(10, 100, 0.9999, 2, MAX_PEAKLIMITER_SAMPLE_RATE);
		if(dwSRHz > MAX_PEAKLIMITER_SAMPLE_RATE) dwSRHz = MAX_PEAKLIMITER_SAMPLE_RATE;
		setLimiterSampleRate(dwSRHz);
		m_dwSRHz = dwSRHz;
		//setLimiterAttack(0);

		CDSLOCK(&m_InQLock);
		CDSLOCK(&m_OutQLock);

		RetCode = UpdateProcessBufSize(true);
		if(RetCode != NO_ERR) return RetCode;

		CDSAudioProcessorClear();

#if defined(USE_OS)
		m_hThread = true;
		m_State = Run_DSState;
#else
		m_hThread = true;

		InitProcessing();
#endif
	}

#if defined(USE_UPMIX)
	DSTRACE(("[CDSAudioProcessor::Open(%u, %u Hz, %u), %d] Upmix:%d, DataType:%d, FullChOut:%d Leaving...\n\r", dwSampleSize, dwSRHz, dwAudioDataSize, RetCode, m_bEnUpmixFunction, m_AudioDataType, m_bFullChannelOutput));
#else
	DSTRACE(("[CDSAudioProcessor::Open(%u, %u Hz, %u), %d] DataType:%d, FullChOut:%d Leaving...\n\r", dwSampleSize, dwSRHz, dwAudioDataSize, RetCode, m_AudioDataType, m_bFullChannelOutput));
#endif
	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: DS 오디오 프로세서를 닫는다.
	매개변수: 없음
	되돌림값: NO_ERR	-> DS 오디오 프로세서 닫기 성공
			  NO_ERR 외	-> DS 오디오 프로세서 닫기 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSAudioProcessorClose()
{
	int RetCode = NO_ERR;
	DSTRACE(("[CDSAudioProcessor::Close()] Entering...\n\r"));

#if defined(USE_OS)
	m_State = RequestStop_DSState;
	EVENT_SET(&m_Event);

	if( m_hThread ){
		DSTRACE(("[CDSAudioProcessor::Close()] wait until m_hThread is false\n\r"));
		m_hThread = false;
		DSTRACE(("[CDSAudioProcessor::Close()] end task_kill for DataProc\n\r"));
	}
#else
	m_hThread = false;
#endif

	CDSConvolutionReverbClose();

	LOCK(&m_InQLock, 5000);
	CDSBufferDestroyer(&m_InQ);
	UNLOCK(&m_InQLock);
	CDSLOCK_DESTROYER(&m_InQLock);

	LOCK(&m_OutQLock, 5000);
	CDSBufferDestroyer(&m_OutQ);
	UNLOCK(&m_OutQLock);
	CDSLOCK_DESTROYER(&m_OutQLock);

	if(m_pProcessBuf1){
		DS_free(m_pProcessBuf1);
		m_pProcessBuf1 = NULL;
	}
	if(m_pProcessBuf2){
		DS_free(m_pProcessBuf2);
		m_pProcessBuf2 = NULL;
	}
	m_dwProcessBuf1Size = 0;
	m_dwProcessBuf2Size = 0;

	DSTRACE(("[CDSAudioProcessor::Close(), %d] Leaving...\n\r", RetCode));
	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: DS 오디오 프로세서 Parameter를 수정한다.
	매개변수: dwChannels			-> 채널 개수
			  dwSampleSize			-> 샘플 바이트 크기
									   최소 크기: 64 바이트
			  dwSRHz				-> 샘플링 레이트, 단위: Hz
			  dwAudioDataSize		-> Audio data 바이트 크기
	되돌림값: NO_ERR	-> DS 오디오 프로세서 열기 성공
			  NO_ERR 외	-> DS 오디오 프로세서 열기 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSAudioProcessorChangeParameter( DWORD dwChannels, DWORD dwSampleSize, DWORD dwSRHz, DWORD dwAudioDataSize)
{
	int RetCode = NO_ERR;
	DS_BOOL bChangedSize = false, bChangedRate = false;

	DSTRACE(("[CDSAudioProcessor::ChangeParameter(%u, %u, %u Hz, %u)] Entering...\n\r", dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize));

	if( !m_hThread ){
		// 오디오 프로세서가 동작 중이다...
		RetCode = ERR_CLOSED;
	}
	
	if(RetCode == NO_ERR){
		dwChannels = dwChannels < 1 ? 1 : dwChannels;
		dwSampleSize = dwSampleSize < 64 ? 64 : dwSampleSize;
		dwAudioDataSize = dwAudioDataSize < 2 ? 2 : dwAudioDataSize;

		if(m_dwSRHz != dwSRHz) {
			bChangedRate = true;
#if defined(USE_OS)
			CDSAudioProcessorPause();
#endif
		}

		if( (m_dwChannels != dwChannels) ||
			(m_dwSampleSize != dwSampleSize) ||
			(m_dwAudioDataSize != dwAudioDataSize) ) {
			bChangedSize = true;
#if defined(USE_OS)
			CDSAudioProcessorPause();
#endif

			m_dwChannels = dwChannels;
			m_dwSampleSize = dwSampleSize;
			m_dwAudioDataSize = dwAudioDataSize;
		}
	}

	if(bChangedRate) {
		if(dwSRHz > MAX_PEAKLIMITER_SAMPLE_RATE) {
			dwSRHz = MAX_PEAKLIMITER_SAMPLE_RATE;
		}

		if(dwSRHz != getLimiterSampleRate()) {
			setLimiterSampleRate(dwSRHz);
		}

		m_dwSRHz = dwSRHz;
	}

	if(bChangedSize) {
		m_dwFlags |= DSAP_EX3D_CH_NUM_CHANGED_BIT;
		m_bUpdatedEX3D = true;

		RetCode = UpdateProcessBufSize(false);
		if(RetCode == NO_ERR) {
			m_bWaitProcess = false;

            DSTRACE(("[CDSAudioProcessor::ChangeParameter()] success UpdateProcessBufSize()\n\r"));
		} else {
			DSTRACE(("[Error - CDSAudioProcessor::ChangeParameter()], Failed UpdateProcessBufSize()!!\n\r"));
#if defined(USE_OS)
			m_State = Stop_DSState;        // 종료
#endif
		}
	}

	if( RetCode == NO_ERR 
	    // && (bChangedRate || bChangedSize)
	  ) {
		CDSAudioProcessorClear();

#if defined(USE_OS)
		CDSAudioProcessorResume();
#endif
	}

#if defined(USE_UPMIX)
	DSTRACE(("[CDSAudioProcessor::ChangeParameter(%u, %u, %u Hz, %u), m_bEnUpmixFunction:%s, %d] Leaving...\n\r", dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize, m_bEnUpmixFunction ? "true" : "false", RetCode));
#else
	DSTRACE(("[CDSAudioProcessor::ChangeParameter(%u, %u, %u Hz, %u), %d] Leaving...\n\r", dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize, RetCode));
#endif

	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: DS 오디오 프로세서가 열려 있는지를 검사한다.
	매개변수: 없음
	되돌림값: true	-> DS 오디오 프로세서 열려 있음
			  false	-> DS 오디오 프로세서 닫혀 있음
    비    고: 
    --------------------------------------------------------------------------------- */
DS_BOOL CDSAudioProcessorOpened()
{
	return m_hThread ? true : false;
}

#if defined(USE_OS)
/* --------------------------------------------------------------------------------
    기    능: 오디오 데이터 처리문
	매개변수: 없음
	되돌림값: NO_ERR	-> 오디오 데이터 처리 성공
			  NO_ERR 외	-> 오디오 데이터 처리 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int OnProcessing()
{
	int RetCode = 0;

	while(m_State != Run_DSState) Sleep(1);

	InitProcessing();

	DS_BOOL bClear = false;
	// bClear = m_bClear; 로 설정해서는 안된다. 오디오 시작 후 몇 초뒤에 틱 소리가 나오는 문제를 일으킨다.

	m_bPause = false;

	while(m_State == Run_DSState)
	{
		if( m_bPause ){
			EVENT_WAITFORSINGLE(&m_Event, 10 );
			continue;
		}

		RetCode = UpdateParameterAndFading();
		if(RetCode != 0) {
			m_State = Stop_DSState;
			return RetCode;
		}
			
		// 4ms 대기
		EVENT_WAITFORSINGLE(&m_Event, 1);
		if(m_State != Run_DSState) break;

		if( bClear ){
			CDSConvolutionReverbClearEX3DSoundFieldsBuffer();
			resetLimiter();

			LOCK(&m_OutQLock, 5000);
			CDSBufferClear(&m_OutQ);
			UNLOCK(&m_OutQLock);
			DSTRACE(("[Warning - CDSAudioProcessor::OnProcessing()] m_OutQ Clear()!!\n\r"));
		}

		// 입력 Audio Data를 pInBuf로 가져온다
		// Mono Data인 경우 먼저 pProcessBuf1로 가져온 후 Stereo 확장하여 pInBuf에 넣는다.
        //DSTRACE(("[CDSAudioProcessor::OnProcessing()] m_OutQ AvailableSize: %lu, m_OutQ Size: %lu, In_CR_SampleSize: %lu, g_dwOutQMinSize: %lu\n\r",
        // 			CDSBufferAvailableSize(&m_OutQ), m_OutQ.m_dwSize, g_dwIN_CR_SampleNum * g_uiUnitCount * sizeof(POINT_T), g_dwOutQMinSize));
        RetCode = 0;
        if(OutQueueAvailableSize() >= g_dwOutQMinSize) {
            // 오디오 데이터를 가져온다.
			PBYTE pDBuf = m_pProcessBuf2;    // 오디오 데이터 버퍼
			if (m_bEndOfStream) {
				RetCode = DeQEx(pDBuf, g_dwInputDataSize, 0);
				// CR 윈도우 크기보다 작게 데이터를 읽을 경우 0으로 채운다.
				if (RetCode < 1) RetCode = 0;
				else if ((DWORD)RetCode < g_dwInputDataSize) {
					ZeroMemory(pDBuf + RetCode, g_dwInputDataSize - RetCode);
					RetCode = g_dwInputDataSize;
				}
			} else {
				if(InQueueFilledLength() >= g_dwInputDataSize) {
#if !defined(MEASURE_INTER_ELAPSED_TIME_CONV)
					DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_AUDIOPROCESS_START);
#endif

					DSTRACEQ(("b DeQ-I I:%d, O:%d, Size:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, g_dwInputDataSize));
					RetCode = DeQ(pDBuf, g_dwInputDataSize);
					DSTRACEQ(("a DeQ-I I:%d, O:%d, Ret:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, RetCode));

					DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_DEQ_INQ_END);
					//DSTRACE(("[CDSAudioProcessor::OnProcessing()] DeQ() g_dwInputDataSize:%d, ReadSize:%d \n\r", g_dwInputDataSize, RetCode));
				}
			}

			//DSTRACE(("[CDSAudioProcessor::OnProcessing(), %s] Data Length: %d, g_dwInputDataSize: %lu, m_OutQ AvailableSize: %lu, g_dwOutQMinSize: %lu, m_InQ.m_dwLength: %lu\n\r",
			//		m_bEndOfStream ? "END OF STREAM" : "", RetCode, g_dwInputDataSize, CDSBufferAvailableSize(&m_OutQ), g_dwOutQMinSize, m_InQ.m_dwLength));

			if(RetCode > 0) {
				EVENT_SET(&m_InQEvent);
			}
		}

		if(RetCode > 0) {
			Normalize(m_pProcessBuf2, (PPOINT_T)m_pProcessBuf1, m_dwChannels, g_dwIN_CR_SampleNum);
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_NORMALIZE_END);

			if(m_bEnabledEX3D == true) {
				// 컨볼루션 리버브 처리를 한다.

				// Upmix - 사용 Buffer: ProcessBuf1
				DWORD dwChannels;
#if defined(USE_UPMIX)
				if((m_bEnUpmixFunction == true) && (m_dwChannels == 2) && (g_bUpmix == true)) {
					dwChannels = 8;

					Upmix((PPOINT_T)m_pProcessBuf1, g_dwIN_CR_SampleNum);
					DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_UPMIX_END);
				} else
#endif
				{
					dwChannels = m_dwChannels;
				}

				// HRTF data 각도별 Panning
				DWORD CR_BitField = SoundPanning((PPOINT_T)m_pProcessBuf1, (PPOINT_T)m_pProcessBuf2, dwChannels, g_uiUnitCount, g_dwIN_CR_SampleNum);
				DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_SOUNDPANNING_END);

				if( (g_FadeState == FADE_STATE_CHG_SF_MUTE) || (g_FadeState == FADE_STATE_CHG_CH_NUM_MUTE) || (g_FadeState == FADE_STATE_3D_ON_MUTE) ) {
					CR_BitField = 0;	// 속도때문에 음장 변경시 첫 번째 convolution은 skip하도록 한다.
				}
				RetCode = CDSConvolutionReverbProcess((PPOINT_T)m_pProcessBuf2, g_dwIN_CR_SampleNum, CR_BitField);	// return 값은 Convolution data 샘플 개수
				//DSTRACE(("[CDSAudioProcessor::OnProcessing()] CDSConvolutionReverbProcess( %u ), RetCode: %u\n\r", g_dwIN_CR_SampleNum, RetCode));
			} else {
				// 3채널 이상 오디오 데이터를 2채널 오디오 데이터로 만든다.
				// EX3D 효과 증대를 위해 EX3D Off 시 출력을 감쇄 한다
				RetCode = DSAP_DownMixToStereo((PPOINT_T)m_pProcessBuf1, (PPOINT_T)m_pProcessBuf2, g_dwIN_CR_SampleNum, m_dwChannels, m_fOffGainValue);
				//DSTRACE(("[CDSAudioProcessor::OnProcessing()] DSAP_DownMixToStereo(%u, %u), RetCode:%u\n\r", g_dwIN_CR_SampleNum, m_dwChannels, RetCode));
			}
		}

		if(RetCode > 0) {
			POINT_T *LtRt[2] = {(PPOINT_T)m_pProcessBuf2, (PPOINT_T)m_pProcessBuf2 + g_dwIN_CR_SampleNum};

			applyLimiter_I(LtRt, g_dwIN_CR_SampleNum);
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONVOLUTION_LIMITER_END);

			RetCode = Denormalize_FadeProcess((PPOINT_T)m_pProcessBuf2, m_pProcessBuf1, g_dwIN_CR_SampleNum, g_FadeState);
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_DENORMALIZE_FADEPROCESS_END);
		}

		if(RetCode > 0) {
			DWORD dwOutBufSize = RetCode;

			// 최종 데이터를 출력 큐에 넣는다.
			LOCK(&m_OutQLock, 5000);
			DSTRACEQ(("b EnQ-O I:%d, O:%d, Size:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, dwOutBufSize));
			RetCode = CDSBufferEnQ(&m_OutQ, m_pProcessBuf1, dwOutBufSize, dwOutBufSize);
			DSTRACEQ(("a EnQ-O I:%d, O:%d, Ret:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, RetCode));
			UNLOCK(&m_OutQLock);
			EVENT_SET(&m_OutQEvent);
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_ENQ_OUTQ_END);
			//DSTRACE(("[CDSAudioProcessor::OnProcessing(), m_OutQ EnQ] AvailableSize:%d, dwOutBufSize:%d, RetCode:%d\n\r", CDSBufferAvailableSize(&m_OutQ), dwOutBufSize, RetCode));

			EVENT_SET(&m_Event);
		}

		bClear = m_bClear;
		m_bClear = false;

#if !defined(MEASURE_INTER_ELAPSED_TIME_CONV)
		DS_ELAPSEDTIMESTAMPLOG(0);
#endif
	}
	DSTRACE(("[DBG - CDSAudioProcessor::OnProcessing()] Main loop End!!\n\r"));

	m_State = Stop_DSState;

	DSTRACE(("[CDSAudioProcessor::OnProcessing(), %d] Leaving...\n\r", RetCode));
	return NO_ERR;
}
#else
extern double fLevel[8];

int OnProcessing()
{
	int RetCode = 0;
#if 0
    static DWORD prevTime = 0;
#endif

	RetCode = UpdateParameterAndFading();
	if(RetCode != 0)
		return RetCode;

	if( m_bClear ){
		m_bClear = false;

		CDSConvolutionReverbClearEX3DSoundFieldsBuffer();
		resetLimiter();
		CDSBufferClear(&m_OutQ);
		DSTRACE(("[Warning - CDSAudioProcessor::OnProcessing()] m_OutQ Clear()!!\n\r"));
	}

	// 입력 Audio Data를 pInBuf로 가져온다
	// Mono Data인 경우 먼저 pProcessBuf1로 가져온 후 Stereo 확장하여 pInBuf에 넣는다.
	//DSTRACE(("[CDSAudioProcessor::OnProcessing()] m_OutQ AvailableSize: %lu, m_OutQ Size: %lu, In_CR_SampleSize: %lu, g_dwOutQMinSize: %lu\n\r",
	// 			CDSBufferAvailableSize(&m_OutQ), m_OutQ.m_dwSize, g_dwIN_CR_SampleNum * g_uiUnitCount * sizeof(POINT_T), g_dwOutQMinSize));
	RetCode = 0;
	if(OutQueueAvailableSize() >= g_dwOutQMinSize) {
		// 오디오 데이터를 가져온다.
		PBYTE pDBuf = m_pProcessBuf2;    // 오디오 데이터 버퍼
		if (m_bEndOfStream) {
			RetCode = DeQEx(pDBuf, g_dwInputDataSize, 0);
			// CR 윈도우 크기보다 작게 데이터를 읽을 경우 0으로 채운다.
			if (RetCode < 1) RetCode = 0;
			else if ((DWORD)RetCode < g_dwInputDataSize) {
				ZeroMemory(pDBuf + RetCode, g_dwInputDataSize - RetCode);
				RetCode = g_dwInputDataSize;
			}
		} else {
			if(InQueueFilledLength() >= g_dwInputDataSize) {
				DSTRACEQ(("b DeQ-I I:%d, O:%d, Size:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, g_dwInputDataSize));
				RetCode = DeQ(pDBuf, g_dwInputDataSize);
				DSTRACEQ(("a DeQ-I I:%d, O:%d, Ret:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, RetCode));
				DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_DEQ_INQ_END);

				//DSTRACE(("[CDSAudioProcessor::OnProcessing()] DeQ() g_dwInputDataSize:%d, ReadSize:%d \n\r", g_dwInputDataSize, RetCode));
			}
		}

		//DSTRACE(("[CDSAudioProcessor::OnProcessing(), %s] Data Length: %d, g_dwInputDataSize: %lu, m_OutQ AvailableSize: %lu, g_dwOutQMinSize: %lu, m_InQ.m_dwLength: %lu\n\r",
		// 		m_bEndOfStream ? "END OF STREAM" : "", RetCode, g_dwInputDataSize, CDSBufferAvailableSize(&m_OutQ), g_dwOutQMinSize, m_InQ.m_dwLength));
	}

	if(RetCode > 0) {
		Normalize(m_pProcessBuf2, (PPOINT_T)m_pProcessBuf1, m_dwChannels, g_dwIN_CR_SampleNum);
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_NORMALIZE_END);

		if(m_bEnabledEX3D == true) {
			// 컨볼루션 리버브 처리를 한다.

			// Upmix - 사용 Buffer: ProcessBuf1
			DWORD dwChannels;
#if defined(USE_UPMIX)
			if((m_bEnUpmixFunction == true) && (m_dwChannels == 2) && (g_bUpmix == true)) {
				dwChannels = UPMIX_CHANNEL_NUMBER;

				Upmix((PPOINT_T)m_pProcessBuf1, g_dwIN_CR_SampleNum);
				DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_UPMIX_END);
			} else
#endif
			{
				dwChannels = m_dwChannels;
			}

#if 0
			if (++prevTime > 4) {
				LevelMeter((PPOINT_T)m_pProcessBuf1, dwChannels, g_dwIN_CR_SampleNum);
				prevTime = 0;
			}
#endif

			// HRTF data 각도별 Panning
			DWORD CR_BitField = SoundPanning((PPOINT_T)m_pProcessBuf1, (PPOINT_T)m_pProcessBuf2, dwChannels, g_uiUnitCount, g_dwIN_CR_SampleNum);
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_SOUNDPANNING_END);

			if( (g_FadeState == FADE_STATE_CHG_SF_MUTE) || (g_FadeState == FADE_STATE_CHG_CH_NUM_MUTE) || (g_FadeState == FADE_STATE_3D_ON_MUTE) ) {
				CR_BitField = 0;	// 속도때문에 음장 변경시 첫 번째 convolution은 skip하도록 한다.
			}
			RetCode = CDSConvolutionReverbProcess((PPOINT_T)m_pProcessBuf2, g_dwIN_CR_SampleNum, CR_BitField);	// return 값은 Convolution data 샘플 개수
			//DSTRACE(("[CDSAudioProcessor::OnProcessing()] CDSConvolutionReverbProcess( %u ), RetCode: %u\n\r", g_dwIN_CR_SampleNum, RetCode));
		} else {
#if 0
			if (++prevTime > 4) {
				LevelMeter((PPOINT_T)m_pProcessBuf1, m_dwChannels, g_dwIN_CR_SampleNum);
				prevTime = 0;
			}
#endif

			// 3채널 이상 오디오 데이터를 2채널 오디오 데이터로 만든다.
			// EX3D 효과 증대를 위해 EX3D Off 시 출력을 감쇄 한다
			RetCode = DSAP_DownMixToStereo((PPOINT_T)m_pProcessBuf1, (PPOINT_T)m_pProcessBuf2, g_dwIN_CR_SampleNum, m_dwChannels, m_fOffGainValue);
			//DSTRACE(("[CDSAudioProcessor::OnProcessing()] DSAP_DownMixToStereo(%u, %u), RetCode:%u\n\r", g_dwIN_CR_SampleNum, m_dwChannels, RetCode));
		}
	}

	if(RetCode > 0) {
		POINT_T *LtRt[2] = {(PPOINT_T)m_pProcessBuf2, (PPOINT_T)m_pProcessBuf2 + g_dwIN_CR_SampleNum};

		applyLimiter_I(LtRt, g_dwIN_CR_SampleNum);
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_CONVOLUTION_LIMITER_END);

		RetCode = Denormalize_FadeProcess((PPOINT_T)m_pProcessBuf2, m_pProcessBuf1, g_dwIN_CR_SampleNum, g_FadeState);
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_DENORMALIZE_FADEPROCESS_END);
	}

	if(RetCode > 0) {
		DWORD dwOutBufSize = RetCode;

		// 최종 데이터를 출력 큐에 넣는다.
		DSTRACEQ(("b EnQ-O I:%d, O:%d, Size:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, dwOutBufSize));
		RetCode = CDSBufferEnQ(&m_OutQ, m_pProcessBuf1, dwOutBufSize, dwOutBufSize);
		DSTRACEQ(("a EnQ-O I:%d, O:%d, Ret:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, RetCode));
		DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_ENQ_OUTQ_END);
		//DSTRACE(("[CDSAudioProcessor::OnProcessing(), m_OutQ EnQ] AvailableSize:%d, dwOutBufSize:%d, RetCode:%d\n\r", CDSBufferAvailableSize(&m_OutQ), dwOutBufSize, RetCode));
	}

	// DSTRACE(("[CDSAudioProcessor::OnProcessing(), %d] Leaving...\n\r", RetCode));
	return NO_ERR;
}
#endif

#if 0
static void LevelMeter(PPOINT_T pBuf, DWORD dwChannels, DWORD dwSampleNum)
{
	int i = 0, j = 0;
	POINT_T absValue, maxVal;
	POINT_T sLv[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	PPOINT_T p = pBuf;

	while (j < dwChannels) {
		i = 0;
		maxVal = 0;
		while (i < dwSampleNum) {
#if defined(USE_FIXED_POINT)
			absValue = abs(*p++);
#else
			absValue = fabs(*p++);
#endif
			if (maxVal < absValue) maxVal = absValue;
			i++;
		}
		sLv[j] = maxVal;
		++j;
	}

	for (i = 0; i < 8; i++) {
		DS_BOOL mute = (m_EX3DChannel[i].dwFlags & EX3D_CH_MUTE_BIT) ? TRUE : FALSE;
		if ( (i >= dwChannels) || 
			( (mute == TRUE) && (m_bEnabledEX3D == true) ) ) {
			fLevel[i] = 0;
		} else {
			fLevel[i] = FLOAT_CONV(sLv[i]);
		}
	}
}
#endif

static void InitProcessing()
{
#if defined(USE_UPMIX)
	g_bUpmix = false;
#endif
	g_FadeState = FADE_STATE_NONE;
	g_dwFadeProcessCnt = 0;	// Fade mute 유지 loop 회수

	DSTRACE(("[CDSAudioProcessor::InitProcessing()] Entering...\n\r"));

	m_bClear = false;
	m_bEndOfStream = false;


	// 이전 EX3D (음장)위치 정보를 그대로 사용할 수 있도록 초기화한다.
	// if( m_bEnabledEX3D )
	{
		PEX3DCHANNEL pCh = &m_EX3DChannel[ 0 ];
		PEX3DCHANNEL pECh = pCh + EX3D_CHANNEL_COUNT;

		m_bUpdatedEX3D = true;		
		while(pCh < pECh)
		{
			pCh->dwFlags |= (EX3D_CH_VANGLE_CHANGED_BIT | EX3D_CH_HANGLE_CHANGED_BIT);
			pCh++;
		}
	}
}

int UpdateParameterAndFading()
{
	int RetCode = 0;

	switch(g_FadeState) {
	case FADE_STATE_NONE:
		// EX-3D 설정이 바뀌었는지 검사한다.
		if( m_bUpdatedEX3D ){
			m_bUpdatedEX3D = false;

#if defined(USE_UPMIX)
			g_bUpmix = m_dwFlags & DSAP_EX3D_UPMIX_BIT ? true : false;
#endif

			// 채널 수 변경
			if( m_dwFlags & DSAP_EX3D_CH_NUM_CHANGED_BIT ) {
				m_dwFlags &= ~DSAP_EX3D_CH_NUM_CHANGED_BIT;

				g_FadeState = FADE_STATE_CHG_CH_NUM_FADE_OUT;
			}

			// 3D On/Off 전환
			if( m_dwFlags & DSAP_EX3D_ONOFF_CHANGED_BIT ) {
				if(g_FadeState == FADE_STATE_NONE) {
					m_dwFlags &= ~DSAP_EX3D_ONOFF_CHANGED_BIT;

					g_FadeState = FADE_STATE_3D_ONOFF_FADE_OUT;
				} else {
					// 우선순위가 높은 처리(채널수 변경)가 설정되었으면 다시 시도하도록 함
					m_bUpdatedEX3D = true;
				}
			}

			// 음장이 바뀌었다면 음장변경 fade 처리 설정
			if( m_dwFlags & DSAP_EX3D_SOUNDFIELD_CHANGED_BIT ) {
				if( (g_FadeState == FADE_STATE_NONE) && (m_bEnabledEX3D == true) ) {
					m_dwFlags &= ~DSAP_EX3D_SOUNDFIELD_CHANGED_BIT;

					g_FadeState = FADE_STATE_CHG_SF_FADE_OUT;
				} else {
					// 우선순위가 높은 처리(채널수 변경, 3D on/off)가 설정되었거나 3D off 상태이면 다시 시도하도록 함
					m_bUpdatedEX3D = true;
				}
			}
#if defined(USE_UPMIX)
			DSTRACE(("[CDSAudioProcessor::UpdateParameterAndFading()] g_FadeState: %u, m_bUpdatedEX3D:%u, g_bUpmix: %s\n\r", g_FadeState, m_bUpdatedEX3D, (g_bUpmix ? "true" : "false")));
#else
			DSTRACE(("[CDSAudioProcessor::UpdateParameterAndFading()] g_FadeState: %u, m_bUpdatedEX3D:%u\n\r", g_FadeState, m_bUpdatedEX3D));
#endif
			g_dwFadeProcessCnt = 0;

			CaculatePositionPanningGain(m_dwChannels);
		}
		break;

	case FADE_STATE_3D_ONOFF_FADE_OUT:
		if(g_dwFadeProcessCnt != 0) {
			m_bEnabledEX3D = m_dwFlags & DSAP_EX3D_ON_BIT ? true : false;
			DSTRACE(("[CDSAudioProcessor::UpdateParameterAndFading()] EX-3D: %s!!\n\r", m_bEnabledEX3D ? "Enabled" : "Disabled"));

			// EX3D On/Off 확인하여 내부 처리용 Size 변수 및 Fading 처리 Enable
			UpdateSizeVariable();
			if(m_bEnabledEX3D == false || InQueueFilledLength() < g_dwInputDataSize) {
				m_bWaitProcess = false;
				DSTRACE(("[CDSAudioProcessor(%d)::UpdateSizeVariable()] Clear m_bWaitProcess because length of InQ is not enough, g_dwInputDataSize:%d, Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, g_dwInputDataSize, InQueueFilledLength(), OutQueueFilledLength()));
			}

			if(m_bEnabledEX3D == true) {
				g_FadeState = FADE_STATE_3D_ON_MUTE_WAIT;
				g_dwFadeProcessCnt = 0;
			} else {
				g_FadeState = FADE_STATE_FADE_IN;
				g_dwFadeProcessCnt = 0;
			}
		}
		break;

	case FADE_STATE_3D_ON_MUTE_WAIT:
		if(g_dwFadeProcessCnt != 0) {
			CDSConvolutionReverbClearEX3DSoundFieldsBuffer();
			resetLimiter();

			g_FadeState = FADE_STATE_3D_ON_MUTE;
			g_dwFadeProcessCnt = 0;
		}
		break;

	case FADE_STATE_3D_ON_MUTE:
		if(g_dwFadeProcessCnt != 0) {
			g_FadeState = FADE_STATE_FADE_IN;
			g_dwFadeProcessCnt = 0;
		}
		break;

	case FADE_STATE_CHG_CH_NUM_FADE_OUT:
		if(g_dwFadeProcessCnt != 0) {
			g_FadeState = FADE_STATE_CHG_CH_NUM_MUTE_WAIT;
			g_dwFadeProcessCnt = 0;
		}
		break;

	case FADE_STATE_CHG_CH_NUM_MUTE_WAIT:
		if(g_dwFadeProcessCnt != 0) {
			RetCode = UpdateProcessBufSize(false);
			if(RetCode == NO_ERR) {
				DSTRACE(("[CDSAudioProcessor::UpdateParameterAndFading()] Change channel number\n\r"));
				if(InQueueFilledLength() < g_dwInputDataSize) {
					m_bWaitProcess = false;
					DSTRACE(("[CDSAudioProcessor(%d)::UpdateProcessBufSize()] Clear m_bWaitProcess because length of InQ is not enough, g_dwInputDataSize:%d, Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, g_dwInputDataSize, InQueueFilledLength(), OutQueueFilledLength()));
				}

				CDSConvolutionReverbClearEX3DSoundFieldsBuffer();
				resetLimiter();

				g_FadeState = FADE_STATE_CHG_CH_NUM_MUTE;
				g_dwFadeProcessCnt = 0;
			}else {
				DSTRACE(("[Error - CDSAudioProcessor::UpdateParameterAndFading()], Failed UpdateProcessBufSize()!! Exiting...\n\r"));
			}
		}
		break;

	case FADE_STATE_CHG_CH_NUM_MUTE:
		if(g_dwFadeProcessCnt != 0) {
			g_FadeState = FADE_STATE_FADE_IN;
			g_dwFadeProcessCnt = 0;
		}
		break;

	case FADE_STATE_CHG_SF_FADE_OUT:
		if(g_dwFadeProcessCnt != 0) {
			g_FadeState = FADE_STATE_CHG_SF_MUTE_WAIT;
			g_dwFadeProcessCnt = 0;
		}
		break;

	case FADE_STATE_CHG_SF_MUTE_WAIT:
		if(g_dwFadeProcessCnt != 0) {
			// 음장 변경
			RetCode = CDSConvolutionReverbSetEX3DSoundField(m_szCurrentSFName.m_pszMem);
			if(RetCode == NO_ERR) {
				DSTRACE(("[CDSAudioProcessor::UpdateParameterAndFading(), SF is Changed!!] EX-3D : %s\n\r", m_szCurrentSFName.m_pszMem));

				RetCode = UpdateProcessBufSize(false);
			}else{
				DSTRACE(("[Error - CDSAudioProcessor::UpdateParameterAndFading()], Failed to change SF(%s)!! Exiting...\n\r", m_szCurrentSFName.m_pszMem));
			}

			if(RetCode == NO_ERR) {
				if(InQueueFilledLength() < g_dwInputDataSize) {
					m_bWaitProcess = false;
					DSTRACE(("[CDSAudioProcessor(%d)::UpdateProcessBufSize()] Clear m_bWaitProcess because length of InQ is not enough, g_dwInputDataSize:%d, Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, g_dwInputDataSize, InQueueFilledLength(), OutQueueFilledLength()));
				}
				CDSConvolutionReverbClearEX3DSoundFieldsBuffer();
				resetLimiter();

				g_FadeState = FADE_STATE_CHG_SF_MUTE;
				g_dwFadeProcessCnt = 0;
			}else {
				DSTRACE(("[Error - CDSAudioProcessor::UpdateParameterAndFading()], Failed UpdateProcessBufSize()!! Exiting...\n\r"));
			}
		}
		break;

	case FADE_STATE_CHG_SF_MUTE:
		if(g_dwFadeProcessCnt != 0) {
			g_FadeState = FADE_STATE_FADE_IN;
			g_dwFadeProcessCnt = 0;
		}
		break;

	case FADE_STATE_FADE_IN:
		if(g_dwFadeProcessCnt != 0) {
			g_FadeState = FADE_STATE_NONE;
			g_dwFadeProcessCnt = 0;
		}
		break;

	default:
		DSTRACE(("[Error - CDSAudioProcessor::UpdateParameterAndFading()], g_FadeState:%d\n\r", g_FadeState));
		g_FadeState = FADE_STATE_NONE;
		break;
	}

	return RetCode;
}

int CDSAudioProcessorProcess(PBYTE pInBuf, PBYTE pOutBuf, DWORD dwLength, DWORD dwFlags)
{
	int RetCode;
	DS_BOOL bClear = false;
	DWORD dwClearLength = 0;

#if !defined(USE_OS)
	DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_AUDIOPROCESS_START);
#endif
	while(1) {
		RetCode = CDSAudioProcessorPut(pInBuf, dwLength, dwFlags);
		if(RetCode > 0) {
#if !defined(USE_OS)
			DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_ONPROCESSING_ENQ_INQ_END);				
			if (m_bWaitProcess == true) {
				OnProcessing();
			}
#endif
			break;
		} else if(RetCode < 0){
			bClear = true;
			dwClearLength = dwLength;
			break;
		} else {
#if defined(USE_OS)
			if (m_State != Run_DSState) {
				RetCode = ERR_NOT_RUN_STATE;
				break;
			} else 
#endif
			if(m_bWaitProcess == false) {	// 음장 변경으로 Convolution Size가 커진 경우 다시 m_InQ 가 채워질 때까지 Zero data 출력 하도록 함
				bClear = true;
				dwClearLength = dwLength;
				break;
			} else {
				DSTRACE(("[CDSAudioProcessor(%d)::Process()] DeQ InQ Waiting... , Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, InQueueFilledLength(), OutQueueFilledLength()));
				EVENT_WAITFORSINGLE(&m_InQEvent, 1);
			}
		}
	}

	if(RetCode > 0) {
		int DequeueLength = RetCode;
		DS_BOOL bFirstProcess = false;
		if ((m_bFullChannelOutput == false) && (m_dwChannels != 2)) {    // 입력이 2ch이 아닌 상태에서 2ch 출력인 경우 DequeueLength 조정
			DequeueLength = (DequeueLength / m_dwChannels) * 2;
		}

		if(m_bWaitProcess == false) {
			bFirstProcess = true;
		}
		while(1) {
			DSTRACEQ(("b DeQ-O I:%d, O:%d, Size:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, DequeueLength));
			int DeQRetCode = DequeueOutQ(pOutBuf, DequeueLength);
			DSTRACEQ(("a DeQ-O I:%d, O:%d, Ret:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, DeQRetCode));
			if(DeQRetCode > 0) {
				break;
			} else if (DeQRetCode < 0) {
				bClear = true;
				dwClearLength = DequeueLength;
				RetCode = DeQRetCode;
				break;
			} else {
				//DSTRACE(("[CDSAudioProcessor(%d)::Process(), %d] Deq OutQ m_bWaitProcess:%d Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, DequeueLength, m_bWaitProcess, InQueueFilledLength(), OutQueueFilledLength()));
#if defined(USE_OS)
				if (m_State != Run_DSState) {
					RetCode = ERR_NOT_RUN_STATE;
					break;
				} else 
#endif
				if(m_bWaitProcess == false) {	// 음장 변경으로 Convolution Size가 커진 경우 다시 m_InQ 가 채워질 때까지 Zero data 출력 하도록 함
					bClear = true;
					dwClearLength = DequeueLength;
					if(bFirstProcess == true) {
						DSTRACE(("[CDSAudioProcessor(%d)::Process(), %d] Zero Output, m_bWaitProcess:%d Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, DequeueLength, m_bWaitProcess, InQueueFilledLength(), OutQueueFilledLength()));
					}
					break;
				} else {
					DSTRACE(("[CDSAudioProcessor(%d)::Process()] EnQ OutQ Waiting... , Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, InQueueFilledLength(), OutQueueFilledLength()));
					EVENT_WAITFORSINGLE(&m_OutQEvent, 1);
				}
			}
		}

		if (m_bWaitProcess == false) {
            DWORD dwConvDataSize = g_dwIN_CR_SampleNum * m_dwChannels * m_dwAudioDataSize;
			if ( m_dwInQPutLength >= dwConvDataSize) {
				m_bWaitProcess = true;
				DSTRACE(("[CDSAudioProcessor(%d)::Process(), %d] InQ was filled enough, set m_bWaitProcess, m_dwInQPutLength:%d, dwConvDataSize:%d, Length-m_InQ:%d, m_OutQ:%d\n\r", m_dwID, DequeueLength, m_dwInQPutLength, dwConvDataSize, InQueueFilledLength(), OutQueueFilledLength()));
			}
		}
	}

	if(bClear) {
		ZeroMemory(pOutBuf, dwClearLength);
	}

#if !defined(USE_OS)
	DS_ELAPSEDTIMESTAMP(ELAPSED_TIME_LOG_STR_AUDIOPROCESS_DEQ_OUTQ_END);
	DS_ELAPSEDTIMESTAMPLOG(0);
#endif

	return RetCode;
}

static void Normalize(void * pInBuf, PPOINT_T pNormalizedBuf, DWORD dwInChannels, DWORD dwSampleNum)
{
	PPOINT_T pNormalize, pNormalizeEnd;

	// InBuf -> pNormalizedBuf 으로 정규화, Buffer Data 형식, LFE data 생성
	// 정규화: POINT_T(-1.0 ~ 1.0) Type(고정소수점 or 부동소수점)
	// Buffer 형식: Interleave -> Planar
	switch( m_AudioDataType ) {
	case AUDIO_DATA_TYPE_FLOAT:	{
#if defined(USE_PLANAR_ARRAY_INPUT)
		PFLOAT pOffset = (PFLOAT)pInBuf;
		pNormalize = pNormalizedBuf;
		pNormalizeEnd = pNormalize + (dwInChannels * dwSampleNum);
		while (pNormalize < pNormalizeEnd) {
			*pNormalize++ = FIXED_CONV(*pOffset);
			pOffset++;
		}
#else
		for (DWORD dwChIdx = 0; dwChIdx < dwInChannels; dwChIdx++) {
			PFLOAT pOffset = (PFLOAT) pInBuf + dwChIdx;
			pNormalize = pNormalizedBuf + (dwSampleNum * dwChIdx);
			pNormalizeEnd = pNormalize + dwSampleNum;

			while (pNormalize < pNormalizeEnd) {
				*pNormalize = FIXED_CONV(*pOffset);
				pNormalize++;
				pOffset += dwInChannels;
			}
		}
#endif
		} break;
	case AUDIO_DATA_TYPE_INT16:	{
#if defined(USE_PLANAR_ARRAY_INPUT)
		PINT16 pOffset = (PINT16)pInBuf;
		pNormalize = pNormalizedBuf;
		pNormalizeEnd = pNormalize + (dwInChannels * dwSampleNum);
		if(QVAL <= 16) {
			while (pNormalize < pNormalizeEnd) {
				*pNormalize++ = *pOffset >> (16 - QVAL);
				pOffset++;
			}
		} else {
			POINT_T NormalizeVal;
			while (pNormalize < pNormalizeEnd) {
				NormalizeVal = *pOffset++;
				*pNormalize++ = NormalizeVal << (QVAL - 16);
			}
		}
#else
		for (DWORD dwChIdx = 0; dwChIdx < dwInChannels; dwChIdx++) {
			PINT16 pOffset = (PINT16) pInBuf + dwChIdx;
			pNormalize = pNormalizedBuf + (dwSampleNum * dwChIdx);
			pNormalizeEnd = pNormalize + dwSampleNum;

			if(QVAL <= 16) {
				while (pNormalize < pNormalizeEnd) {
					*pNormalize++ = *pOffset >> (16 - QVAL);
					pOffset += dwInChannels;
				}
			} else {
				POINT_T NormalizeVal;
				while (pNormalize < pNormalizeEnd) {
					NormalizeVal = *pOffset;
					*pNormalize++ = NormalizeVal << (QVAL - 16);
					pOffset += dwInChannels;
				}
			}
		}
#endif
		} break;
	case AUDIO_DATA_TYPE_INT32:	{
#if defined(USE_PLANAR_ARRAY_INPUT)
	#if defined(XMOS_FHT_FUNC)
	    vect_s32_shr( pNormalizedBuf, pInBuf, dwInChannels * dwSampleNum, 32-QVAL);
	#else
		CONVOL_T fInvMaxVal = (CONVOL_T)1.0 / DS_MAX_INT32;
		PINT32 pOffset = (PINT32)pInBuf;
		pNormalize = pNormalizedBuf;
		pNormalizeEnd = pNormalize + (dwInChannels * dwSampleNum);
		while (pNormalize < pNormalizeEnd) {
			*pNormalize++ = FIXED_CONV(*pOffset * fInvMaxVal);
			pOffset++;
		}
	#endif
#else
		CONVOL_T fInvMaxVal = (CONVOL_T)1.0 / DS_MAX_INT32;
		for (DWORD dwChIdx = 0; dwChIdx < dwInChannels; dwChIdx++) {
			PINT32 pOffset = (PINT32) pInBuf + dwChIdx;
			pNormalize = pNormalizedBuf + (dwSampleNum * dwChIdx);
			pNormalizeEnd = pNormalize + dwSampleNum;

			while (pNormalize < pNormalizeEnd) {
				*pNormalize = FIXED_CONV(*pOffset * fInvMaxVal);
				pNormalize++;
				pOffset += dwInChannels;
			}
		}
#endif
		} break;
	}
}

static DWORD Denormalize_FadeProcess(PPOINT_T pInBuf, void * pOutBuf, DWORD dwSampleNum, FADE_STATE FadeState)
{
	DWORD OutBufSize = 0;
	DWORD i, dwSkipNum = 0;
	POINT_T fFadeInRate, fFadeOutRate, fRate, fVal;
	PPOINT_T pfInOffset = pInBuf;

	if(FadeState != FADE_STATE_NONE) {
		DSTRACE(("[CDSAudioProcessor::Denormalize_FadeProcess(), FadeState:%d]\n\r", FadeState));
	}

	// 반정규화: Convolution Type(POINT_T) -> Input Audio data Type(INT16 or INT32 or float)
	if(m_bFullChannelOutput) {
		dwSkipNum = g_uiOutChannels - 2;	// L/R 2channel 제외
		OutBufSize = g_dwIN_CR_SampleNum * m_dwChannels * m_dwAudioDataSize;
		memset(pOutBuf, 0, OutBufSize);
	}

	switch( m_AudioDataType ) {
	case AUDIO_DATA_TYPE_FLOAT:
		{
			PFLOAT pfOutOffset = (PFLOAT)pOutBuf;
			PPOINT_T pfInOffset2 = pInBuf + dwSampleNum;
#if defined(USE_PLANAR_ARRAY_INPUT)
			PFLOAT pfOutOffset2 = pfOutOffset + dwSampleNum;
			PFLOAT pfOutOffsetEnd = pfOutOffset2 + dwSampleNum;
#endif

			switch(FadeState) {
			case FADE_STATE_NONE:
#if defined(USE_PLANAR_ARRAY_INPUT)
				while(pfOutOffset < pfOutOffsetEnd) {
					*pfOutOffset++ = (float)(FLOAT_CONV(*pfInOffset));
					pfInOffset++;
				}
#else
				for (i = 1; i <= dwSampleNum; i++) {
					*pfOutOffset++ = (float)(FLOAT_CONV(*pfInOffset));
					*pfOutOffset++ = (float)(FLOAT_CONV(*pfInOffset2));

					pfOutOffset += dwSkipNum;
					pfInOffset++, pfInOffset2++;
				}
#endif
				break;
			case FADE_STATE_3D_ON_MUTE_WAIT:
			case FADE_STATE_3D_ON_MUTE:
			case FADE_STATE_CHG_CH_NUM_MUTE_WAIT:
			case FADE_STATE_CHG_CH_NUM_MUTE:
			case FADE_STATE_CHG_SF_MUTE_WAIT:
			case FADE_STATE_CHG_SF_MUTE:
#if defined(USE_PLANAR_ARRAY_INPUT)
				while(pfOutOffset < pfOutOffsetEnd) {
					*pfOutOffset++ = 0;
				}
#else
				for (i = 1; i <= dwSampleNum; i++) {
					*pfOutOffset++ = 0;		       		// Left
					*pfOutOffset++ = 0;       			// Right

					pfOutOffset += dwSkipNum;
				}
#endif
				break;
			case FADE_STATE_3D_ONOFF_FADE_OUT:
			case FADE_STATE_CHG_CH_NUM_FADE_OUT:
			case FADE_STATE_CHG_SF_FADE_OUT:
				fRate = FIXED_CONV(1.0 / dwSampleNum);
				for (i = 1; i <= dwSampleNum; i++) {
					fFadeOutRate = (dwSampleNum - i) * fRate;

					fVal = POINT_MUL_2(*pfInOffset, fFadeOutRate);	// Left
					*pfOutOffset++ = (float)(FLOAT_CONV(fVal));

#if defined(USE_PLANAR_ARRAY_INPUT)
					fVal = POINT_MUL_2(*pfInOffset2, fFadeOutRate);	// Right
					*pfOutOffset2++ = (float)(FLOAT_CONV(fVal));
#else
					fVal = POINT_MUL_2(*pfInOffset2, fFadeOutRate);	// Right
					*pfOutOffset++ = (float)(FLOAT_CONV(fVal));

					pfOutOffset += dwSkipNum;
#endif
					pfInOffset++, pfInOffset2++;
				}
				break;
			case FADE_STATE_FADE_IN:
				fRate = FIXED_CONV(1.0 / dwSampleNum);
				for (i = 1; i <= dwSampleNum; i++) {
					fFadeInRate = i * fRate;

					fVal = POINT_MUL_2(*pfInOffset, fFadeInRate);	// Left
					*pfOutOffset++ = (float)(FLOAT_CONV(fVal));

#if defined(USE_PLANAR_ARRAY_INPUT)
					fVal = POINT_MUL_2(*pfInOffset2, fFadeInRate);	// Right
					*pfOutOffset2++ = (float)(FLOAT_CONV(fVal));
#else
					fVal = POINT_MUL_2(*pfInOffset2, fFadeInRate);	// Right
					*pfOutOffset++ = (float)(FLOAT_CONV(fVal));

					pfOutOffset += dwSkipNum;
#endif
					pfInOffset++, pfInOffset2++;
				}
				break;
			default:
				break;
			}

			if(OutBufSize == 0) {
				OutBufSize = dwSampleNum * sizeof(float) * 2;
			}
		}
		break;
	case AUDIO_DATA_TYPE_INT16:
		{
			PINT16 piOutOffset = (PINT16)pOutBuf;
			PPOINT_T pfInOffset2 = pInBuf + dwSampleNum;
#if defined(USE_PLANAR_ARRAY_INPUT)
			PINT16 piOutOffset2 = piOutOffset + dwSampleNum;
			PINT16 piOutOffsetEnd = piOutOffset2 + dwSampleNum;
#endif

			switch(FadeState) {
			case FADE_STATE_NONE:
#if defined(USE_PLANAR_ARRAY_INPUT)
				while(piOutOffset < piOutOffsetEnd) {
					*piOutOffset++ = (INT16)(FLOAT_CONV(*pfInOffset) * (float)INT16_MAX);
					pfInOffset++;
				}
#else
				for (i = 1; i <= dwSampleNum; i++) {
					*piOutOffset++ = (INT16)(FLOAT_CONV(*pfInOffset) * (float)INT16_MAX);
					*piOutOffset++ = (INT16)(FLOAT_CONV(*pfInOffset2) * (float)INT16_MAX);

					piOutOffset += dwSkipNum;
					pfInOffset++, pfInOffset2++;
				}
#endif
				break;
			case FADE_STATE_3D_ON_MUTE_WAIT:
			case FADE_STATE_3D_ON_MUTE:
			case FADE_STATE_CHG_CH_NUM_MUTE_WAIT:
			case FADE_STATE_CHG_CH_NUM_MUTE:
			case FADE_STATE_CHG_SF_MUTE_WAIT:
			case FADE_STATE_CHG_SF_MUTE:
#if defined(USE_PLANAR_ARRAY_INPUT)
				while(piOutOffset < piOutOffsetEnd) {
					*piOutOffset++ = 0;
				}
#else
				for (i = 1; i <= dwSampleNum; i++) {
					*piOutOffset++ = 0;       			// Left
					*piOutOffset++ = 0;       			// Right

					piOutOffset += dwSkipNum;
				}
#endif
				break;
			case FADE_STATE_3D_ONOFF_FADE_OUT:
			case FADE_STATE_CHG_CH_NUM_FADE_OUT:
			case FADE_STATE_CHG_SF_FADE_OUT:
				fRate = FIXED_CONV(1.0 / dwSampleNum);
				for (i = 1; i <= dwSampleNum; i++) {
					fFadeOutRate = (dwSampleNum - i) * fRate;

					fVal = POINT_MUL_2(*pfInOffset, fFadeOutRate);	// Left
					*piOutOffset++ = (INT16)(FLOAT_CONV(fVal) * (float)INT16_MAX);

#if defined(USE_PLANAR_ARRAY_INPUT)
					fVal = POINT_MUL_2(*pfInOffset2, fFadeOutRate);	// Right
					*piOutOffset2++ = (INT16)(FLOAT_CONV(fVal) * (float)INT16_MAX);
#else
					fVal = POINT_MUL_2(*pfInOffset2, fFadeOutRate);	// Right
					*piOutOffset++ = (INT16)(FLOAT_CONV(fVal) * (float)INT16_MAX);

					piOutOffset += dwSkipNum;
#endif
					pfInOffset++, pfInOffset2++;
				}
				break;
			case FADE_STATE_FADE_IN:
				fRate = FIXED_CONV(1.0 / dwSampleNum);
				for (i = 1; i <= dwSampleNum; i++) {
					fFadeInRate = i * fRate;

					fVal = POINT_MUL_2(*pfInOffset, fFadeInRate);	// Left
					*piOutOffset++ = (INT16)(FLOAT_CONV(fVal) * (float)INT16_MAX);

#if defined(USE_PLANAR_ARRAY_INPUT)
					fVal = POINT_MUL_2(*pfInOffset2, fFadeInRate);	// Right
					*piOutOffset2++ = (INT16)(FLOAT_CONV(fVal) * (float)INT16_MAX);
#else
					fVal = POINT_MUL_2(*pfInOffset2, fFadeInRate);	// Right
					*piOutOffset++ = (INT16)(FLOAT_CONV(fVal) * (float)INT16_MAX);

					piOutOffset += dwSkipNum;
#endif
					pfInOffset++, pfInOffset2++;
				}
				break;
			default:
				break;
			}

			if(OutBufSize == 0) {
				OutBufSize = dwSampleNum * sizeof(INT16) * 2;
			}
		}
		break;
	case AUDIO_DATA_TYPE_INT32:
		{
			PINT32 piOutOffset = (PINT32)pOutBuf;
			PPOINT_T pfInOffset2 = pInBuf + dwSampleNum;
#if defined(USE_PLANAR_ARRAY_INPUT)
			PINT32 piOutOffset2 = piOutOffset + dwSampleNum;
			PINT32 piOutOffsetEnd = piOutOffset2 + dwSampleNum;
#endif

			switch(FadeState) {
			case FADE_STATE_NONE:
#if defined(USE_PLANAR_ARRAY_INPUT)
				while(piOutOffset < piOutOffsetEnd) {
					*piOutOffset++ = (INT32)(FLOAT_CONV(*pfInOffset) * (float)INT32_MAX);
					pfInOffset++;
				}
#else
				for (i = 1; i <= dwSampleNum; i++) {
					*piOutOffset++ = (INT32)(FLOAT_CONV(*pfInOffset) * (float)INT32_MAX);
					*piOutOffset++ = (INT32)(FLOAT_CONV(*pfInOffset2) * (float)INT32_MAX);

					piOutOffset += dwSkipNum;
					pfInOffset++, pfInOffset2++;
				}
#endif
				break;
			case FADE_STATE_3D_ON_MUTE_WAIT:
			case FADE_STATE_3D_ON_MUTE:
			case FADE_STATE_CHG_CH_NUM_MUTE_WAIT:
			case FADE_STATE_CHG_CH_NUM_MUTE:
			case FADE_STATE_CHG_SF_MUTE_WAIT:
			case FADE_STATE_CHG_SF_MUTE:
#if defined(USE_PLANAR_ARRAY_INPUT)
				while(piOutOffset < piOutOffsetEnd) {
					*piOutOffset++ = 0;
				}
#else
				for (i = 1; i <= dwSampleNum; i++) {
					*piOutOffset++ = 0;       			// Left
					*piOutOffset++ = 0;       			// Right

					piOutOffset += dwSkipNum;
				}
#endif
				break;
			case FADE_STATE_3D_ONOFF_FADE_OUT:
			case FADE_STATE_CHG_CH_NUM_FADE_OUT:
			case FADE_STATE_CHG_SF_FADE_OUT:
				fRate = FIXED_CONV(1.0 / dwSampleNum);
				for (i = 1; i <= dwSampleNum; i++) {
					fFadeOutRate = (dwSampleNum - i) * fRate;

					fVal = POINT_MUL_2(*pfInOffset, fFadeOutRate);	// Left
					*piOutOffset++ = (INT32)(FLOAT_CONV(fVal) * (float)INT32_MAX);

#if defined(USE_PLANAR_ARRAY_INPUT)
					fVal = POINT_MUL_2(*pfInOffset2, fFadeOutRate);	// Right
					*piOutOffset2++ = (INT32)(FLOAT_CONV(fVal) * (float)INT32_MAX);
#else
					fVal = POINT_MUL_2(*pfInOffset2, fFadeOutRate);	// Right
					*piOutOffset++ = (INT32)(FLOAT_CONV(fVal) * (float)INT32_MAX);

					piOutOffset += dwSkipNum;
#endif
					pfInOffset++, pfInOffset2++;
				}
				break;
			case FADE_STATE_FADE_IN:
				fRate = FIXED_CONV(1.0 / dwSampleNum);
				for (i = 1; i <= dwSampleNum; i++) {
					fFadeInRate = i * fRate;

					fVal = POINT_MUL_2(*pfInOffset, fFadeInRate);	// Left
					*piOutOffset++ = (INT32)(FLOAT_CONV(fVal) * (float)INT32_MAX);

#if defined(USE_PLANAR_ARRAY_INPUT)
					fVal = POINT_MUL_2(*pfInOffset2, fFadeInRate);	// Right
					*piOutOffset2 = (INT32)(FLOAT_CONV(fVal) * (float)INT32_MAX);
#else
					fVal = POINT_MUL_2(*pfInOffset2, fFadeInRate);	// Right
					*piOutOffset++ = (INT32)(FLOAT_CONV(fVal) * (float)INT32_MAX);

					piOutOffset += dwSkipNum;
#endif
					pfInOffset++, pfInOffset2++;
				}
				break;
			default:
				break;
			}

			if(OutBufSize == 0) {
				OutBufSize = dwSampleNum * sizeof(INT32) * 2;
			}
		}
		break;
	}

	g_dwFadeProcessCnt++;

	return OutBufSize;
}

static void UpdateSizeVariable()
{
	/* --------------------------------------------------------------
	   < 주의 >
	   모노(1채널) 오디오인 경우,
	   CDSAudioSL 호출단의 오디오 큐 버퍼 크기가 최소 2채널로 설정되어 있기 때문에
	   출력 샘플 크기는 입력 샘플 크기의 2배가 된다.
	   전체 채널 출력이 아닌 경우는 2채널로 출력된다.
	   -------------------------------------------------------------- */
	if((m_dwChannels == 1) || (m_bFullChannelOutput == false)){
		g_uiOutChannels = 2;
	}else {
		g_uiOutChannels = m_dwChannels;
	}

	// Loop 진입전 Size 변수 초기화
	g_dwIN_CR_SampleNum = IN_CR_SampleNum();
	g_uiUnitCount = UnitCount();

	g_dwOutQMinSize = g_dwIN_CR_SampleNum * g_uiOutChannels * m_dwAudioDataSize;
	g_dwInputDataSize = g_dwIN_CR_SampleNum * m_dwChannels * m_dwAudioDataSize;

	DSTRACE(("[CDSAudioProcessor::OnProcessing(), %s CHANNEL OUTPUT] g_dwIN_CR_SampleNum: %u, g_dwOutQMinSize: %u, g_dwInputDataSize: %u\n\r",
			m_bFullChannelOutput ? "FULL" : "2", g_dwIN_CR_SampleNum, g_dwOutQMinSize, g_dwInputDataSize));
}

static int UpdateProcessBufSize(DS_BOOL bCreateQ)
{
	int RetCode = NO_ERR;

	UpdateSizeVariable();

	// 음장 변경으로 dwNewBufSize 가 m_dwProcessBuf1Size 메모리보다 클 경우 메모리를 다시 할당받는다.
#if defined(USE_UPMIX)
	DWORD dwNewBufSize = g_dwIN_CR_SampleNum * UPMIX_CHANNEL_NUMBER * sizeof(POINT_T);
#else
	DWORD dwNewBufSize = g_dwIN_CR_SampleNum * m_dwChannels * sizeof(POINT_T);
#endif
	if (dwNewBufSize > m_dwProcessBuf1Size) {
		if (m_pProcessBuf1) {
			DS_free(m_pProcessBuf1);
		}
		m_pProcessBuf1 = (PBYTE)DS_malloc(dwNewBufSize);
		if(m_pProcessBuf1){
			m_dwProcessBuf1Size = dwNewBufSize;
			DSTRACE(("[CDSAudioProcessor::UpdateProcessBufSize(), m_pProcessBuf1] Re-allocated the 'm_pProcessBuf1(Size: %u)' memory!!\n\r", m_dwProcessBuf1Size));
		}
		else {
			DSTRACE(("[Error - CDSAudioProcessor::UpdateProcessBufSize(), m_pProcessBuf1] Failed to allocate the 'Mem2(Size: %u)' memory!! Exiting...\n\r", dwNewBufSize));
			RetCode = ERR_MEM_NULL;
		}
	}

	// 음장 변경으로 dwNewBufSize 가 m_dwProcessBuf2Size 메모리보다 클 경우 메모리를 다시 할당받는다.
	if(RetCode == NO_ERR) {
		dwNewBufSize = g_dwIN_CR_SampleNum * (g_uiUnitCount + 1) * sizeof(POINT_T);    // Panning시 마지막 영역을 LFE 용으로 사용
		if (dwNewBufSize > m_dwProcessBuf2Size) {
			if (m_pProcessBuf2) {
				DS_free(m_pProcessBuf2);
			}
			m_pProcessBuf2 = (PBYTE)DS_malloc(dwNewBufSize);
			if (m_pProcessBuf2) {
				m_dwProcessBuf2Size = dwNewBufSize;
				DSTRACE(("[CDSAudioProcessor::UpdateProcessBufSize(), m_pProcessBuf2] Re-allocated the 'm_pProcessBuf2(Size: %u)' memory!!\n\r", m_dwProcessBuf2Size));
			} else {
				DSTRACE(("[Error - CDSAudioProcessor::UpdateProcessBufSize(), m_pProcessBuf2] Failed to allocate the 'Mem2(Size: %u)' memory!! Exiting...\n\r", dwNewBufSize));
				RetCode = ERR_MEM_NULL;
			}
		}
	}

	if(RetCode == NO_ERR) {
		dwNewBufSize = (g_dwIN_CR_SampleNum * g_uiOutChannels * m_dwAudioDataSize) +	// OutQMinSize
					   ((m_dwSampleSize * g_uiOutChannels) / m_dwChannels);			// OutSampleSize
		if(bCreateQ == true) {
			RetCode = CDSBuffer(&m_OutQ, dwNewBufSize);
			DSTRACE(("[CDSAudioProcessor::UpdateProcessBufSize(), m_OutQ] allocated the 'm_OutQ(Size: %u)' memory!!\n\r", dwNewBufSize));
		} else {
			LOCK(&m_OutQLock, 5000);
			if (m_OutQ.m_dwSize < dwNewBufSize) {
				if (CDSBufferExpansion(&m_OutQ, dwNewBufSize) == NO_ERR) {                // 출력 버퍼 크기
					DSTRACE(("[CDSAudioProcessor::UpdateProcessBufSize(), m_OutQ] Re-allocated the 'm_OutQ(Size: %u)' memory!!\n\r", dwNewBufSize));
				} else {
					RetCode = ERR_MEM_NULL;
				}
			}
			UNLOCK(&m_OutQLock);
		}
	}

	if(RetCode == NO_ERR) {
		dwNewBufSize = (g_dwIN_CR_SampleNum * m_dwChannels * m_dwAudioDataSize) + m_dwSampleSize;
		if(bCreateQ == true) {
			RetCode = CDSBuffer(&m_InQ, dwNewBufSize);
			DSTRACE(("[CDSAudioProcessor::UpdateProcessBufSize(), m_InQ] allocated the 'm_InQ(Size: %u)' memory!!\n\r", dwNewBufSize));
		} else {
			LOCK(&m_InQLock, 5000);
			if (m_InQ.m_dwSize < dwNewBufSize) {
				if (CDSBufferExpansion(&m_InQ, dwNewBufSize) == NO_ERR) {                // 출력 버퍼 크기
					DSTRACE(("[CDSAudioProcessor::UpdateProcessBufSize(), m_InQ] Re-allocated the 'm_InQ(Size: %u)' memory!!\n\r", dwNewBufSize));
				} else {
					RetCode = ERR_MEM_NULL;
				}
			}
			UNLOCK(&m_InQLock);
		}
	}

	return RetCode;
}

#if defined(USE_UPMIX)
static void Upmix(PPOINT_T pProcessBuf, DWORD dwSampleNum)
{
    PPOINT_T pInOffsetL = pProcessBuf;
    PPOINT_T pInOffsetR = pInOffsetL + dwSampleNum;
	PPOINT_T pUpmixOffsetC = pInOffsetR + dwSampleNum;
	PPOINT_T pUpmixOffsetLFE = pUpmixOffsetC + dwSampleNum;		// pProcessBuf + (dwSampleNum * 3)
	PPOINT_T pUpmixOffsetBL = pUpmixOffsetLFE + dwSampleNum;	// pProcessBuf + (dwSampleNum * 4)
	PPOINT_T pUpmixOffsetBR = pUpmixOffsetBL + dwSampleNum;		// pProcessBuf + (dwSampleNum * 5)
  #if (UPMIX_CHANNEL_NUMBER == 8)
	PPOINT_T pUpmixOffsetSL = pUpmixOffsetBR + dwSampleNum;		// pProcessBuf + (dwSampleNum * 6)
	PPOINT_T pUpmixOffsetSR = pUpmixOffsetSL + dwSampleNum;		// pProcessBuf + (dwSampleNum * 7)
  #endif
	PPOINT_T pInEOffsetL = pInOffsetL + dwSampleNum;

	while(pInOffsetL < pInEOffsetL) {
		//POINT_T LR = *pInOffsetL + *pInOffsetR;
		POINT_T HalfLeft = POINT_MUL_2(*pInOffsetL, fGainM6dB);				//-6dB
		POINT_T HalfRight = POINT_MUL_2(*pInOffsetR, fGainM6dB);			//-6dB
		POINT_T LR = HalfLeft + HalfRight;									//-6dB(L+R)
		POINT_T Center = POINT_MUL_2(LR, fGainM3dB);						// -9dB(L+R) => -6dB-3dB=-9dB
		POINT_T SrLeft = POINT_MUL_2((HalfLeft - HalfRight), fGainM3dB);	// -9dB(L-R) => -6dB-3dB=-9dB
		POINT_T SrRight = POINT_MUL_2((HalfRight - HalfLeft), fGainM3dB);	// -9dB(R-L) => -6dB-3dB=-9dB

		pInOffsetL++, pInOffsetR++;
		*pUpmixOffsetC++ = Center;		// Center
  #if defined(USE_LFE_UNIT)
		*pUpmixOffsetLFE++ = Center;	// Back Center(LFE), LFE로 -6dB(L+R)를 사용
  #else
		*pUpmixOffsetLFE++ = 0;			// Back Center(LFE) 
  #endif
		*pUpmixOffsetBL++ = SrLeft;		// Back Left
		*pUpmixOffsetBR++ = SrRight;	// Back Right
  #if (UPMIX_CHANNEL_NUMBER == 8)
		*pUpmixOffsetSL++ = SrLeft;		// Surround Left
		*pUpmixOffsetSR++ = SrRight;	// Surround Right
  #endif
	}
//    pInOffsetL = pProcessBuf;
//    pInOffsetR = pInOffsetL + dwSampleNum;
//    pUpmixOffsetC = pInOffsetR + dwSampleNum;
//    pUpmixOffsetLFE = pUpmixOffsetC + dwSampleNum;
//    pUpmixOffsetBL = pUpmixOffsetLFE + dwSampleNum;
//    pUpmixOffsetBR = pUpmixOffsetBL + dwSampleNum;
//    pUpmixOffsetSL = pUpmixOffsetBR + dwSampleNum;
//    pUpmixOffsetSR = pUpmixOffsetSL + dwSampleNum;
//    DSTRACE(("[CDSAudioProcessor::Upmix()] L: %f, R: %f, C: %f, LFE: %f, BL: %f, BR: %f, SL: %f, SR: %f\n\r",
//             FLOAT_CONV(*pInOffsetL), FLOAT_CONV(*pInOffsetR), FLOAT_CONV(*pUpmixOffsetC), FLOAT_CONV(*pUpmixOffsetLFE),
//             FLOAT_CONV(*pUpmixOffsetBL), FLOAT_CONV(*pUpmixOffsetBR), FLOAT_CONV(*pUpmixOffsetSL), FLOAT_CONV(*pUpmixOffsetSR) ));

}
#endif

static DWORD SoundPanning(PPOINT_T pInBuf, PPOINT_T pPanOutBuf, DWORD dwInChannels, DWORD dwPanChannels, DWORD dwSampleNum)
{
	DWORD CR_BitField = 0;
	POINT_T fTmpVal;

	// pPanOutBuf(m_pProcessBuf2) 마지막 dwSampleNum 크기의 버퍼를 LFE convolution 용 buffer로 사용
	memset(pPanOutBuf, 0, dwSampleNum * (dwPanChannels + 1) * sizeof(POINT_T)); // clean-up (8channels + 1channel(LFE)) x (4 or 2)bytes(POINT_T)
	for (DWORD inChIdx = 0; inChIdx < dwInChannels; inChIdx++) {
		PEX3DCHANNEL pCh = &m_EX3DChannel[inChIdx];
		DS_BOOL bMute = (DS_BOOL)(pCh->dwFlags & EX3D_CH_MUTE_BIT);

#if defined(USE_LFE_UNIT)
		if( (inChIdx == 3) && ((dwInChannels == 6) || (dwInChannels == 8)) ) {    // LFE channel
            if(bMute == false)
			{
                PPOINT_T pLFEBufOffset = pPanOutBuf + (dwSampleNum * EX3D_CRUD_TOTAL_POSITION_COUNT);
                PPOINT_T pLFEBufEnd = pLFEBufOffset + dwSampleNum;
                PPOINT_T pLFEDataOffset = pInBuf + (inChIdx *  dwSampleNum);
                while (pLFEBufOffset < pLFEBufEnd) {
                    *pLFEBufOffset = *pLFEDataOffset;
                    pLFEBufOffset++;
                    pLFEDataOffset++;
                }
                //pLFEBufOffset = pPanOutBuf + (dwSampleNum * EX3D_CRUD_TOTAL_POSITION_COUNT);
                //pLFEDataOffset = pInBuf + (inChIdx *  dwSampleNum);
                //DSTRACE(("[CDSAudioProcessor::SoundPanning()] *pLFEDataOffset: %f, *pLFEBufOffset: %f\n\r", *pLFEDataOffset, *pLFEBufOffset));
            }
			continue;
		}
#endif

		int currHAngle = pCh->wTargetHAngle;
		int prevHAngle = pCh->wHAngle;
		int currVAngle = pCh->wTargetVAngle;
		int prevVAngle = pCh->wVAngle;
		//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, dwSampleNum:%d, dwPanChannels:%d, currHAngle:%d, prevHAngle:%d, currVAngle:%d, prevVAngle:%d\n\r",
		//	inChIdx, dwSampleNum, dwPanChannels, currHAngle, prevHAngle, currVAngle, prevVAngle));

		POINT_T HTargetBGain = bMute ? 0 : pCh->HTargetBGain;
		POINT_T HTargetEGain = bMute ? 0 : pCh->HTargetEGain;
		POINT_T VTargetBGain = bMute ? 0 : pCh->VTargetBGain;
		POINT_T VTargetEGain = bMute ? 0 : pCh->VTargetEGain;
		POINT_T ChGain = pCh->ChGain;

		// DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, bMute:%d, HBGain:%f, HEGain:%f, HTargetBGain:%f, HTargetEGain:%f, VBGain:%f, VEGain:%f, VTargetBGain:%f, VTargetEGain:%f\n\r",
		// 	inChIdx, bMute, FLOAT_CONV(pCh->HBGain), FLOAT_CONV(pCh->HEGain), FLOAT_CONV(HTargetBGain), FLOAT_CONV(HTargetEGain), FLOAT_CONV(pCh->VBGain), FLOAT_CONV(pCh->VEGain), FLOAT_CONV(VTargetBGain), FLOAT_CONV(VTargetEGain)));

		fTmpVal = POINT_MUL_2(pCh->VBGain, pCh->HBGain);
		POINT_T fPrevVBHBGain = POINT_MUL_2(fTmpVal, ChGain);
		fTmpVal = POINT_MUL_2(pCh->VBGain, pCh->HEGain);
		POINT_T fPrevVBHEGain = POINT_MUL_2(fTmpVal, ChGain);
		fTmpVal = POINT_MUL_2(VTargetBGain, HTargetBGain);
		POINT_T fCurrVBHBGain = POINT_MUL_2(fTmpVal, ChGain);
		fTmpVal = POINT_MUL_2(VTargetBGain, HTargetEGain);
		POINT_T fCurrVBHEGain = POINT_MUL_2(fTmpVal, ChGain);
#if !defined(USE_ONLY_HORIZONTALITY)
		fTmpVal = POINT_MUL_2(pCh->VEGain * pCh->HBGain);
		POINT_T fPrevVEHBGain = POINT_MUL_2(fTmpVal, ChGain);
		fTmpVal = POINT_MUL_2(pCh->VEGain * pCh->HEGain);
		POINT_T fPrevVEHEGain = POINT_MUL_2(fTmpVal, ChGain);
		fTmpVal = POINT_MUL_2(VTargetEGain * HTargetBGain);
		POINT_T fCurrVEHBGain = POINT_MUL_2(fTmpVal, ChGain);
		fTmpVal = POINT_MUL_2(VTargetEGain * HTargetEGain);
		POINT_T fCurrVEHEGain = POINT_MUL_2(fTmpVal, ChGain);
		//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, fCurrVBHBGain:%f, fCurrVBHEGain:%f, fPrevVBHBGain:%f, fPrevVBHEGain:%f, fCurrVEHBGain:%f, fCurrVEHEGain:%f, fPrevVEHBGain:%f, fPrevVEHEGain:%f\n\r",
		//	inChIdx, fCurrVBHBGain, fCurrVBHEGain, fPrevVBHBGain, fPrevVBHEGain, fCurrVEHBGain, fCurrVEHEGain, fPrevVEHBGain, fPrevVEHEGain));
#else
		// DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, fCurrVBHBGain:%f, fCurrVBHEGain:%f, fPrevVBHBGain:%f, fPrevVBHEGain:%f\n\r",
		// 	inChIdx, FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain), FLOAT_CONV(fPrevVBHBGain), FLOAT_CONV(fPrevVBHEGain)));
#endif

		DWORD currCR_HB_Idx = currHAngle / EX3D_CRUD_STEP_H_ANGLE;
		DWORD currCR_HE_Idx = (currCR_HB_Idx + 1) % EX3D_CRUD_H_POSITION_COUNT;
		DWORD prevCR_HB_Idx = prevHAngle / EX3D_CRUD_STEP_H_ANGLE;
		DWORD prevCR_HE_Idx = (prevCR_HB_Idx + 1) % EX3D_CRUD_H_POSITION_COUNT;

#if defined(USE_ONLY_HORIZONTALITY)
		DWORD currCR_VB_Idx = 0;
		DWORD prevCR_VB_Idx = 0;
#else
		DWORD currCR_VB_Idx = currVAngle / 90;
		DWORD prevCR_VB_Idx = prevVAngle / 90;
#endif

		DWORD currCR_VBHB_Idx = currCR_VB_Idx * EX3D_CRUD_H_POSITION_COUNT + currCR_HB_Idx;
		DWORD currCR_VBHE_Idx = currCR_VB_Idx * EX3D_CRUD_H_POSITION_COUNT + currCR_HE_Idx;
		DWORD prevCR_VBHB_Idx = prevCR_VB_Idx * EX3D_CRUD_H_POSITION_COUNT + prevCR_HB_Idx;
		DWORD prevCR_VBHE_Idx = prevCR_VB_Idx * EX3D_CRUD_H_POSITION_COUNT + prevCR_HE_Idx;
#if !defined(USE_ONLY_HORIZONTALITY)
		// 1개의 수직각에 대한 음장 data만 있거나 수직각이 180인 경우 수직 end index 값을 begin index와 동일하게 설정
		// 위 조건의 경우 수직 begin 각도의 data에 수직 end 각도의 data가 더해지나 수직 end 각도의 gain이 0이 되어 있으므로
		// 실제 begin 각도의 data에는 영향을 주지 않는다.
		// 다만 불필요한 연산을 수행하는 부분에 대해서는 삭제 방법 검토필요 !!
		DWORD currCR_VE_Idx = ((dwPanChannels <= EX3D_CRUD_H_POSITION_COUNT) || (currCR_VB_Idx >= 2)) ? currCR_VB_Idx : currCR_VB_Idx + 1;
		DWORD prevCR_VE_Idx = ((dwPanChannels <= EX3D_CRUD_H_POSITION_COUNT) || (prevCR_VB_Idx >= 2)) ? prevCR_VB_Idx : prevCR_VB_Idx + 1;

		DWORD currCR_VEHB_Idx = currCR_VE_Idx * EX3D_CRUD_H_POSITION_COUNT + currCR_HB_Idx;
		DWORD currCR_VEHE_Idx = currCR_VE_Idx * EX3D_CRUD_H_POSITION_COUNT + currCR_HE_Idx;
		DWORD prevCR_VEHB_Idx = prevCR_VE_Idx * EX3D_CRUD_H_POSITION_COUNT + prevCR_HB_Idx;
		DWORD prevCR_VEHE_Idx = prevCR_VE_Idx * EX3D_CRUD_H_POSITION_COUNT + prevCR_HE_Idx;
		//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, currCR_VBHB_Idx:%d, currCR_VBHE_Idx:%d, prevCR_VBHB_Idx:%d, prevCR_VBHE_Idx:%d, currCR_VEHB_Idx:%d, currCR_VEHE_Idx:%d, prevCR_VEHB_Idx:%d, prevCR_VEHE_Idx:%d\n\r",
		//		inChIdx, currCR_VBHB_Idx, currCR_VBHE_Idx, prevCR_VBHB_Idx, prevCR_VBHE_Idx, currCR_VEHB_Idx, currCR_VEHE_Idx, prevCR_VEHB_Idx, prevCR_VEHE_Idx));
#else
		//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, currCR_VBHB_Idx:%d, currCR_VBHE_Idx:%d, prevCR_VBHB_Idx:%d, prevCR_VBHE_Idx:%d\n\r",
		//		inChIdx, currCR_VBHB_Idx, currCR_VBHE_Idx, prevCR_VBHB_Idx, prevCR_VBHE_Idx));
#endif

		DWORD i = 1;
		//DWORD FadingCnt = (dwSampleNum >= 1024) ? 1024 : dwSampleNum;
		DWORD FadingCnt = dwSampleNum;

		PPOINT_T pCR_AngleBuf_currCR_VBHB = pPanOutBuf + (currCR_VBHB_Idx * dwSampleNum);
		PPOINT_T pCR_AngleBuf_currCR_VBHE = pPanOutBuf + (currCR_VBHE_Idx * dwSampleNum);
		PPOINT_T pCR_AngleBuf_prevCR_VBHB = pPanOutBuf + (prevCR_VBHB_Idx * dwSampleNum);
		PPOINT_T pCR_AngleBuf_prevCR_VBHE = pPanOutBuf + (prevCR_VBHE_Idx * dwSampleNum);
#if !defined(USE_ONLY_HORIZONTALITY)
		PPOINT_T pCR_AngleBuf_currCR_VEHB = pPanOutBuf + (currCR_VEHB_Idx * dwSampleNum);
		PPOINT_T pCR_AngleBuf_currCR_VEHE = pPanOutBuf + (currCR_VEHE_Idx * dwSampleNum);
		PPOINT_T pCR_AngleBuf_prevCR_VEHB = pPanOutBuf + (prevCR_VEHB_Idx * dwSampleNum);
		PPOINT_T pCR_AngleBuf_prevCR_VEHE = pPanOutBuf + (prevCR_VEHE_Idx * dwSampleNum);
		//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, pCR_AngleBuf_currCR_VBHB:0x%08X, pCR_AngleBuf_currCR_VBHE:0x%08X, pCR_AngleBuf_prevCR_VBHB:0x%08X, pCR_AngleBuf_prevCR_VBHE:0x%08X, pCR_AngleBuf_currCR_VEHB:0x%08X, pCR_AngleBuf_currCR_VEHE:0x%08X, pCR_AngleBuf_prevCR_VEHB:0x%08X, pCR_AngleBuf_prevCR_VEHE:0x%08X\n\n\r",
		//	inChIdx, pCR_AngleBuf_currCR_VBHB, pCR_AngleBuf_currCR_VBHE, pCR_AngleBuf_prevCR_VBHB, pCR_AngleBuf_prevCR_VBHE, pCR_AngleBuf_currCR_VEHB, pCR_AngleBuf_currCR_VEHE, pCR_AngleBuf_prevCR_VEHB, pCR_AngleBuf_prevCR_VEHE));
#else
		//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, pCR_AngleBuf_currCR_VBHB:0x%08X, pCR_AngleBuf_currCR_VBHE:0x%08X, pCR_AngleBuf_prevCR_VBHB:0x%08X, pCR_AngleBuf_prevCR_VBHE:0x%08X\n\n\r",
		//	inChIdx, pCR_AngleBuf_currCR_VBHB, pCR_AngleBuf_currCR_VBHE, pCR_AngleBuf_prevCR_VBHB, pCR_AngleBuf_prevCR_VBHE));
#endif

		PPOINT_T pChannelBuf = pInBuf + (inChIdx *  dwSampleNum);
		POINT_T fFadeInRate;
		POINT_T fFadeOutRate;
		POINT_T fRate = FIXED_CONV(1.0 / FadingCnt);

		if ((currHAngle != prevHAngle) || (currVAngle != prevVAngle)){ // cross-fade
			for (i = 1; i <= FadingCnt; i++) {
				fFadeOutRate = POINT_MUL_2((FadingCnt - i), fRate);
				fFadeInRate = POINT_MUL_2(i, fRate);

				fTmpVal = POINT_MUL_2(*pChannelBuf, fPrevVBHBGain);
				*pCR_AngleBuf_prevCR_VBHB += POINT_MUL_2(fTmpVal, fFadeOutRate);	// 이전 각도의 Begin CRUnit을 위한 Data
				fTmpVal = POINT_MUL_2(*pChannelBuf, fPrevVBHEGain);
				*pCR_AngleBuf_prevCR_VBHE += POINT_MUL_2(fTmpVal, fFadeOutRate);	// 이전 각도의 End CRUnit을 위한 Data
				fTmpVal = POINT_MUL_2(*pChannelBuf, fCurrVBHBGain);
				*pCR_AngleBuf_currCR_VBHB += POINT_MUL_2(fTmpVal, fFadeInRate);		// 현재 각도의 Begin CRUnit을 위한 Data
				fTmpVal = POINT_MUL_2(*pChannelBuf, fCurrVBHEGain);
				*pCR_AngleBuf_currCR_VBHE += POINT_MUL_2(fTmpVal, fFadeInRate);		// 현재 각도의 End CRUnit을 위한 Data
				pCR_AngleBuf_prevCR_VBHB++;
				pCR_AngleBuf_prevCR_VBHE++;
				pCR_AngleBuf_currCR_VBHB++;
				pCR_AngleBuf_currCR_VBHE++;

#if !defined(USE_ONLY_HORIZONTALITY)
				//if(dwPanChannels > 8)
				// if문으로 수행유무 테스트 결과 2% 정도 속도 향상이 있으나 효과 미미하여 주석처리함
				// (박효신 숨 2ch LFE off, BRIR sample수-8192 => if문으로 skip: 51ms, 조건없이 수행:52ms)
				{
					fTmpVal = POINT_MUL_2(*pChannelBuf * fPrevVEHBGain);
					*pCR_AngleBuf_prevCR_VEHB += POINT_MUL_2(fTmpVal, fFadeOutRate);
					fTmpVal = POINT_MUL_2(*pChannelBuf * fPrevVEHEGain);
					*pCR_AngleBuf_prevCR_VEHE += POINT_MUL_2(fTmpVal, fFadeOutRate);
					fTmpVal = POINT_MUL_2(*pChannelBuf, fCurrVEHBGain);
					*pCR_AngleBuf_currCR_VEHB += POINT_MUL_2(fTmpVal, fFadeInRate);
					fTmpVal = POINT_MUL_2(*pChannelBuf, fCurrVEHEGain);
					*pCR_AngleBuf_currCR_VEHE += POINT_MUL_2(fTmpVal, fFadeInRate);
					pCR_AngleBuf_prevCR_VEHB++;
					pCR_AngleBuf_prevCR_VEHE++;
					pCR_AngleBuf_currCR_VEHB++;
					pCR_AngleBuf_currCR_VEHE++;
				}
#endif

				pChannelBuf++;
			}

			CR_BitField |=  (fPrevVBHBGain != 0 ? (0x01 << prevCR_VBHB_Idx) : 0) |
							(fPrevVBHEGain != 0 ? (0x01 << prevCR_VBHE_Idx) : 0) |
							(fCurrVBHBGain != 0 ? (0x01 << currCR_VBHB_Idx) : 0) |
							(fCurrVBHEGain != 0 ? (0x01 << currCR_VBHE_Idx) : 0);
#if !defined(USE_ONLY_HORIZONTALITY)
			if(dwPanChannels > 8) {
				CR_BitField |=  (fPrevVEHBGain != 0. ? (0x01 << prevCR_VEHB_Idx) : 0) |
								(fPrevVEHEGain != 0. ? (0x01 << prevCR_VEHE_Idx) : 0) |
								(fCurrVEHBGain != 0. ? (0x01 << currCR_VEHB_Idx) : 0) |
								(fCurrVEHEGain != 0. ? (0x01 << currCR_VEHE_Idx) : 0);
			}

			DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, currCR_VBHB_Idx:%d, currCR_VBHE_Idx:%d, prevCR_VBHB_Idx:%d, prevCR_VBHE_Idx:%d, currCR_VEHB_Idx:%d, currCR_VEHE_Idx:%d, prevCR_VEHB_Idx:%d, prevCR_VEHE_Idx:%d\n\r",
					inChIdx, currCR_VBHB_Idx, currCR_VBHE_Idx, prevCR_VBHB_Idx, prevCR_VBHE_Idx, currCR_VEHB_Idx, currCR_VEHE_Idx, prevCR_VEHB_Idx, prevCR_VEHE_Idx));
			DSTRACE(("[CDSAudioProcessor::SoundPanning()-Fading Angle] inChIdx:%d, prevHAngle:%d, currHAngle:%d, prevVAngle:%d, currVAngle:%d, fPrevVBHBGain:%f, fPrevVBHEGain:%f, fCurrVBHBGain:%f, fCurrVBHEGain:%f, fPrevVEHBGain:%f, fPrevVEHEGain:%f, fCurrVEHBGain:%f, fCurrVEHEGain:%f\n\r",
					inChIdx, prevHAngle, currHAngle,prevVAngle, currVAngle,
					FLOAT_CONV(fPrevVBHBGain), FLOAT_CONV(fPrevVBHEGain), FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain),
					FLOAT_CONV(fPrevVEHBGain), FLOAT_CONV(fPrevVEHEGain), FLOAT_CONV(fCurrVEHBGain), FLOAT_CONV(fCurrVEHEGain) ));
#else
			DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, currCR_VBHB_Idx:%d, currCR_VBHE_Idx:%d, prevCR_VBHB_Idx:%d, prevCR_VBHE_Idx:%d\n\r",
					inChIdx, currCR_VBHB_Idx, currCR_VBHE_Idx, prevCR_VBHB_Idx, prevCR_VBHE_Idx));
			DSTRACE(("[CDSAudioProcessor::SoundPanning()-Fading Angle] inChIdx:%d, prevHAngle:%d, currHAngle:%d, prevVAngle:%d, currVAngle:%d, fPrevVBHBGain:%f, fPrevVBHEGain:%f, fCurrVBHBGain:%f, fCurrVBHEGain:%f\n\r",
					inChIdx, prevHAngle, currHAngle, prevVAngle, currVAngle,
					FLOAT_CONV(fPrevVBHBGain), FLOAT_CONV(fPrevVBHEGain), FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain) ));
#endif
		} else {
			if ((fPrevVBHBGain != fCurrVBHBGain) || (fPrevVBHEGain != fCurrVBHEGain)
#if !defined(USE_ONLY_HORIZONTALITY)
				|| (fPrevVEHBGain != fCurrVEHBGain) || (fPrevVEHEGain != fCurrVEHEGain)
#endif
				) {
				POINT_T fDiffVBHBGain = fCurrVBHBGain - fPrevVBHBGain;
				POINT_T fDiffVBHEGain = fCurrVBHEGain - fPrevVBHEGain;
#if !defined(USE_ONLY_HORIZONTALITY)
				POINT_T fDiffVEHBGain = fCurrVEHBGain - fPrevVEHBGain;
				POINT_T fDiffVEHEGain = fCurrVEHEGain - fPrevVEHEGain;
#endif

				for (i = 1; i <= FadingCnt; i++) {
					POINT_T fDiffGain;
					fFadeInRate = POINT_MUL_2(i, fRate);

					fDiffGain = POINT_MUL_2(fDiffVBHBGain, fFadeInRate);
					*pCR_AngleBuf_currCR_VBHB += POINT_MUL_2(*pChannelBuf, (fPrevVBHBGain + fDiffGain));	// 현재 각도의 Begin CRUnit을 위한 Data
					fDiffGain = POINT_MUL_2(fDiffVBHEGain, fFadeInRate);
					*pCR_AngleBuf_currCR_VBHE += POINT_MUL_2(*pChannelBuf, (fPrevVBHEGain + fDiffGain));	// 현재 각도의 End CRUnit을 위한 Data
					pCR_AngleBuf_currCR_VBHB++;
					pCR_AngleBuf_currCR_VBHE++;

#if !defined(USE_ONLY_HORIZONTALITY)
					//if(dwPanChannels > 8)
					{
						fDiffGain = POINT_MUL_2(fDiffVEHBGain * fFadeInRate);
						*pCR_AngleBuf_currCR_VEHB += POINT_MUL_2(*pChannelBuf, (fPrevVEHBGain + fDiffGain));
						fDiffGain = POINT_MUL_2(fDiffVEHEGain * fFadeInRate);
						*pCR_AngleBuf_currCR_VEHE += POINT_MUL_2(*pChannelBuf, (fPrevVEHEGain + fDiffGain));
						pCR_AngleBuf_currCR_VEHB++;
						pCR_AngleBuf_currCR_VEHE++;
					}
#endif

					pChannelBuf++;
				}

				CR_BitField |=  (0x01 << currCR_VBHB_Idx ) | (0x01 << currCR_VBHE_Idx);
#if !defined(USE_ONLY_HORIZONTALITY)
				if(dwPanChannels > 8) {
					CR_BitField |=  (0x01 << currCR_VEHB_Idx) | (0x01 << currCR_VEHE_Idx);
				}
#endif

#if defined(USE_ONLY_HORIZONTALITY)
  #if (LOCAL_DEBUG == 1) || defined(_DEBUG)
				POINT_T fDiffPCVBHBGain = POINT_MUL_2(fDiffVBHBGain, fFadeInRate);
				POINT_T fDiffPCVBHEGain = POINT_MUL_2(fDiffVBHEGain, fFadeInRate);
  #endif
				DSTRACE(("[CDSAudioProcessor::SoundPanning()-Fading Gain] inChIdx:%d, prevHAngle:%d, currHAngle:%d, fPrevVBHBGain:%f, fPrevVBHEGain:%f, fCurrVBHBGain:%f, fCurrVBHEGain:%f, fDiffVBHBGain:%f, fDiffVBHEGain:%f, fVBHBGain:%f, fVBHEGain:%f\n\r",
						inChIdx, prevHAngle, currHAngle,
						FLOAT_CONV(fPrevVBHBGain), FLOAT_CONV(fPrevVBHEGain), FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain), FLOAT_CONV(fDiffVBHBGain), FLOAT_CONV(fDiffVBHEGain), 
						FLOAT_CONV(fPrevVBHBGain + (fDiffPCVBHBGain)), FLOAT_CONV(fPrevVBHEGain + (fDiffPCVBHEGain)) ));
#else
				DSTRACE(("[CDSAudioProcessor::SoundPanning()-Fading Gain] inChIdx:%d, prevHAngle:%d, currHAngle:%d, fPrevVBHBGain:%f, fPrevVBHEGain:%f, fCurrVBHBGain:%f, fCurrVBHEGain:%f, fDiffVBHBGain:%f, fDiffVBHEGain:%f, fVBHBGain:%f, fVBHEGain:%f, fPrevVEHBGain:%f, fPrevVEHEGain:%f, fCurrVEHBGain:%f, fCurrVEHEGain:%f, fDiffVEHBGain:%f, fDiffVEHEGain:%f, fVEHBGain:%f, fVEHEGain:%f\n\r",
						inChIdx, prevHAngle, currHAngle,
						FLOAT_CONV(fPrevVBHBGain), FLOAT_CONV(fPrevVBHEGain), FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain), FLOAT_CONV(fDiffVBHBGain), FLOAT_CONV(fDiffVBHEGain), FLOAT_CONV(fPrevVBHBGain + (fDiffVBHBGain * fFadeInRate)), FLOAT_CONV(fPrevVBHEGain + (fDiffVBHEGain * fFadeInRate)),
						FLOAT_CONV(fPrevVEHBGain), FLOAT_CONV(fPrevVEHEGain), FLOAT_CONV(fCurrVEHBGain), FLOAT_CONV(fCurrVEHEGain), FLOAT_CONV(fDiffVEHBGain), FLOAT_CONV(fDiffVEHEGain), FLOAT_CONV(fPrevVEHBGain + (fDiffVEHBGain * fFadeInRate)), FLOAT_CONV(fPrevVEHEGain + (fDiffVEHEGain * fFadeInRate)) ));
#endif
			}
		}

		pCh->wHAngle = currHAngle;
		pCh->wVAngle = currVAngle;
		pCh->HBGain = HTargetBGain;
		pCh->HEGain = HTargetEGain;
		pCh->VBGain = VTargetBGain;
		pCh->VEGain = VTargetEGain;

		if(bMute == false) {
			// fading 처리의 포인터 변수를 동일하게 사용하므로 별도 정의할 필요 없어 주석처리
			//PPOINT_T pCR_AngleBuf_currCR_VBHB = pPanOutBuf + (currCR_VBHB_Idx * dwSampleNum) + i;
			//PPOINT_T pCR_AngleBuf_currCR_VBHE = pPanOutBuf + (currCR_VBHE_Idx * dwSampleNum) + i;
			//PPOINT_T pCR_AngleBuf_currCR_VEHB = pPanOutBuf + (currCR_VEHB_Idx * dwSampleNum) + i;
			//PPOINT_T pCR_AngleBuf_currCR_VEHE = pPanOutBuf + (currCR_VEHE_Idx * dwSampleNum) + i;
			//pChannelBuf = pInBuf + (inChIdx * dwSampleNum) + i;

			if(i < dwSampleNum) {
				CR_BitField |=  (fCurrVBHBGain != 0 ? (0x01 << currCR_VBHB_Idx) : 0) |
								(fCurrVBHEGain != 0 ? (0x01 << currCR_VBHE_Idx) : 0);
#if !defined(USE_ONLY_HORIZONTALITY)
				if(dwPanChannels > 8) {
					CR_BitField |=  (fCurrVEHBGain != 0. ? (0x01 << currCR_VEHB_Idx) : 0) |
									(fCurrVEHEGain != 0. ? (0x01 << currCR_VEHE_Idx) : 0);
				}
#endif
			}
			for (; i <= dwSampleNum; i++) {
				*pCR_AngleBuf_currCR_VBHB += POINT_MUL_2(*pChannelBuf, fCurrVBHBGain);	// 현재 각도의 Begin CRUnit을 위한 Data
				*pCR_AngleBuf_currCR_VBHE += POINT_MUL_2(*pChannelBuf, fCurrVBHEGain);	// 현재 각도의 End CRUnit을 위한 Data

				pCR_AngleBuf_currCR_VBHB++;
				pCR_AngleBuf_currCR_VBHE++;

#if !defined(USE_ONLY_HORIZONTALITY)
				//if(dwPanChannels > 8)
				// loop문 내부 조건으로 수행유무 테스트 결과 2% 정도 속도 향상이 있으나 효과 미미하여 주석처리함
				// (박효신 숨 2ch LFE off, BRIR sample수-8192 => 조건 정지: 51ms, 조건없이 수행:52ms)
				{
					*pCR_AngleBuf_currCR_VEHB += POINT_MUL_2(*pChannelBuf, fCurrVEHBGain);
					*pCR_AngleBuf_currCR_VEHE += POINT_MUL_2(*pChannelBuf, fCurrVEHEGain);
					pCR_AngleBuf_currCR_VEHB++;
					pCR_AngleBuf_currCR_VEHE++;
				}
#endif

				pChannelBuf++;
			}

#if defined(USE_ONLY_HORIZONTALITY)
			// DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, i:%d, fCurrVBHBGain:%f, fCurrVBHEGain:%f\n\r",
			// 		inChIdx, i, FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain) ));
#else
			//DSTRACE(("[CDSAudioProcessor::SoundPanning()] inChIdx:%d, i:%d, fCurrVBHBGain:%f, fCurrVBHEGain:%f, fCurrVEHBGain:%f, fCurrVEHEGain:%f\n\r",
			//		inChIdx, i, FLOAT_CONV(fCurrVBHBGain), FLOAT_CONV(fCurrVBHEGain), FLOAT_CONV(fCurrVEHBGain), FLOAT_CONV(fCurrVEHEGain)));
#endif
		}
	}

	return CR_BitField;
}

/* ------------------------------------------------------------------------
    기    능: 오디오 데이터를 버퍼에 넣는다.
	매개변수: pData		-> 데이터 바이트 포인터
			  dwLength	-> 데이터 바이트 길이
			  dwFlags	-> 플래그
			               EX3D_END_OF_STREAM_BIT: 스트림 끝 여부 비트
	되돌림값: > -1	-> 넣은 바이트 길이
			  < 0	-> 데이터 넣기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSAudioProcessorPut(PBYTE pData, DWORD dwLength, DWORD dwFlags)
{
	int RetCode = NO_ERR;

#if defined(USE_OS)
	if(m_State != Run_DSState){
		DSTRACE(("[Error - CDSAudioProcessor::Put()] Not Run state(%d)!! Exiting...\n\r", m_State));
		return ERR_NOT_RUN_STATE;
	}
#endif

	if( m_InQ.m_pBuf )
	{
#ifdef _DEBUG
		if(m_bFirstAudioData == false){
			m_bFirstAudioData = true;
			DSTRACE(("[CDSAudioProcessor::Put()] SystemTimeMilliSec: %lu\n\r", DS_GetTickTime()/1000));
		}
#endif

		if(dwLength > m_dwSampleSize) {
			m_dwSampleSize = dwLength;
			RetCode = UpdateProcessBufSize(false);
		}

		if(RetCode == NO_ERR) {
			LOCK(&m_InQLock, 5000);
			DSTRACEQ(("b EnQ-I I:%d, O:%d, Size:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, dwLength));
			RetCode = CDSBufferEnQ(&m_InQ, pData, dwLength, dwLength);
			m_dwInQPutLength = m_InQ.m_dwLength;
			DSTRACEQ(("a EnQ-I I:%d, O:%d, Ret:%d\n\r", m_InQ.m_dwLength, m_OutQ.m_dwLength, RetCode));
			UNLOCK(&m_InQLock);
			//DSTRACE(("[CDSAudioProcessor::Put()], m_InQ AvailableSize:%d, dwLength:%d, RetCode:%d\n\r", CDSBufferAvailableSize(&m_InQ), dwLength, RetCode));
			EVENT_SET(&m_Event);
		}
	}
	else RetCode = ERR_NO_BUFFER;	

	if(dwFlags & EX3D_END_OF_STREAM_BIT){
		DSTRACE(("[Warning - CDSAudioProcessor::Put()] EX3D_END_OF_STREAM_BIT\n\r"));
		m_bEndOfStream = true;
	}
	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 오디오 기록 버퍼에 기록된 길이를 구한다.
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
DWORD InQueueFilledLength()
{
	DWORD dwLength;

	LOCK(&m_InQLock, 5000);
	dwLength = m_InQ.m_dwLength;
	UNLOCK(&m_InQLock);

	return dwLength;
}

/* ------------------------------------------------------------------------
    기    능: 기록 가능한 오디오 버퍼 크기를 구한다.
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
DWORD InQueueAvailableSize()
{
	DWORD dwLength;

	LOCK(&m_InQLock, 5000);
	dwLength = CDSBufferAvailableSize(&m_InQ);
	UNLOCK(&m_InQLock);

	return dwLength;
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
static int DeQ(PBYTE pBuf, DWORD dwSize)
{
	int RetCode;

	LOCK(&m_InQLock, 5000);
	RetCode = CDSBufferDeQ(&m_InQ, pBuf, dwSize, dwSize);
	UNLOCK(&m_InQLock);

	return RetCode;
}

static int DeQEx(PBYTE pBuf, DWORD dwSize, DWORD dwMinSize)
{
	int RetCode;

	LOCK(&m_InQLock, 5000);
	RetCode = CDSBufferDeQ(&m_InQ, pBuf, dwSize, dwMinSize);
	UNLOCK(&m_InQLock);

	return RetCode;
}

#if defined(_DEBUG) || (LOCAL_DEBUG == 1)
static DWORD OutQueueFilledLength()
{
	DWORD dwLength;

	LOCK(&m_OutQLock, 5000);
	dwLength = m_OutQ.m_dwLength;
	UNLOCK(&m_OutQLock);

	return dwLength;
}
#endif

static DWORD OutQueueAvailableSize()
{
	DWORD dwLength;

	LOCK(&m_OutQLock, 5000);
	dwLength = CDSBufferAvailableSize(&m_OutQ);
	UNLOCK(&m_OutQLock);

	return dwLength;
}

static int DequeueOutQ(PBYTE pBuf, DWORD dwSize)
{
	int RetCode;

	LOCK(&m_OutQLock, 5000);
	RetCode = CDSBufferDeQ(&m_OutQ, pBuf, dwSize, dwSize);
	UNLOCK(&m_OutQLock);

	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 오디오 프로세서 버퍼를 초기화한다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 오디오 프로세서 버퍼 초기화 성공
			  NO_ERR 외	-> 오디오 프로세서 버퍼 초기화 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSAudioProcessorClear()
{
	int RetCode = NO_ERR;

	DSTRACE(("[Warning - CDSAudioProcessor::Clear()]\n\r"));

	m_bClear = true;
	LOCK(&m_InQLock, 5000);
	if(m_InQ.m_pBuf) {
		RetCode = CDSBufferClear(&m_InQ);
	}
	UNLOCK(&m_InQLock);

	m_bWaitProcess = false;
	m_dwInQPutLength = 0;

	EVENT_SET(&m_Event);
	EVENT_RESET(&m_InQEvent);
	EVENT_RESET(&m_OutQEvent);

	return RetCode;
}

int SetEX3DAudioSoundField(PCHAR pszSFName)
{
	int RetCode = NO_ERR;

	if( GetEX3DSoundField(pszSFName) ) {    // 음장이 있는지를 검사한다.
		if( _stricmp(m_szCurrentSFName.m_pszMem, pszSFName) ){
			CDSCharBufferSet( &m_szCurrentSFName, pszSFName );

			m_dwFlags |= DSAP_EX3D_SOUNDFIELD_CHANGED_BIT;

			m_bUpdatedEX3D = true;
		}

		DSTRACE(("[CDSAudioProcessor::SetEX3DAudioSoundField()] Sound Field: %s, m_dwFlags:0x%08X\n\r", pszSFName, m_dwFlags));
	} else {
		DSTRACE(("[Error - CDSAudioProcessor::SetEX3DAudioSoundField( %s )] Not found!! Exiting...\n\r", pszSFName));
		RetCode = ERR_NOT_FOUND;
	}

	return RetCode;
}

int SetEX3DAudioAngle(DWORD dwChannelNumber, DWORD dwVAngle, DWORD dwHAngle, DS_BOOL bApply)
{
	int RetCode = NO_ERR;

	dwChannelNumber--;
	if(dwChannelNumber < EX3D_CHANNEL_COUNT){
		PEX3DCHANNEL pCh = &m_EX3DChannel[ dwChannelNumber ];
		UINT uiHA = (dwHAngle + (360 - m_StartHAngle)) % 360;
#if !defined(USE_ONLY_HORIZONTALITY)
		UINT uiVA;

		if(dwVAngle > 180) uiVA = 180;
		else uiVA = dwVAngle;
#endif

		m_EX3DAngle[dwChannelNumber].wTargetHAngle = (WORD)uiHA;
		if((pCh->dwFlags & EX3D_CH_HANGLE_SET_BIT) && (pCh->wHAngle == uiHA)) {
			// Pause 상태에서 각도를 계속 변경하여 다시 동일한 각도로 설정되는 경우 불필요한 처리를 줄이기 위해 flag 해제
			pCh->dwFlags &= ~EX3D_CH_HANGLE_CHANGED_BIT;
		} else {
			pCh->dwFlags |= EX3D_CH_HANGLE_CHANGED_BIT | EX3D_CH_HANGLE_SET_BIT;
		}

#if !defined(USE_ONLY_HORIZONTALITY)
		m_EX3DAngle[dwChannelNumber].wTargetVAngle = (WORD)uiVA;
		if((pCh->dwFlags & EX3D_CH_VANGLE_SET_BIT) && (pCh->wVAngle == uiVA)) {
			// Pause 상태에서 각도를 계속 변경하여 다시 동일한 각도로 설정되는 경우 불필요한 처리를 줄이기 위해 flag 해제
			pCh->dwFlags &= ~EX3D_CH_VANGLE_CHANGED_BIT;
		} else {
			pCh->dwFlags |= EX3D_CH_VANGLE_CHANGED_BIT | EX3D_CH_VANGLE_SET_BIT;
		}
#endif

		if( bApply ) m_bUpdatedEX3D = true;

		DSTRACE(("[CDSAudioProcessor::SetEX3DAudioAngle()] Apply: %s, Position: %u, Org HAngle: %u, VAngle: %u, Target HAngle: %u, VAngle: %u, dwFlags: 0x%x\n\r",
				(bApply ? "true" : "false"), dwChannelNumber + 1,
				pCh->wHAngle, pCh->wVAngle,
				m_EX3DAngle[dwChannelNumber].wTargetHAngle, m_EX3DAngle[dwChannelNumber].wTargetVAngle,
				pCh->dwFlags
				));
	} else {
		DSTRACE(("[Error - CDSAudioProcessor::SetEX3DAudioAngle()] Channel number(%d) exceeded max position number(%d), Exiting...\n\r", dwChannelNumber, EX3D_CHANNEL_COUNT));
		RetCode = ERR_NOT_FOUND;
	}

	return RetCode;
}

int SetEX3DAudioMute(DWORD dwChannelNumber, DS_BOOL bMute)
{
	int RetCode = NO_ERR;

	dwChannelNumber--;
	if(dwChannelNumber < EX3D_CHANNEL_COUNT){
		PEX3DCHANNEL pCh = &m_EX3DChannel[ dwChannelNumber ];

		if( bMute ){
			pCh->dwFlags |= EX3D_CH_MUTE_BIT;
		}else{
			pCh->dwFlags &= ~EX3D_CH_MUTE_BIT;
		}

		m_bUpdatedEX3D = true;

		DSTRACE(("[CDSAudioProcessor::SetEX3DAudioMute()] Position: %u, Mute:%s\n\r", dwChannelNumber + 1, (bMute ? "true" : "false")));
	} else {
		DSTRACE(("[Error - CDSAudioProcessor::SetEX3DAudioMute()] Channel number(%d) exceeded max position number(%d), Exiting...\n\r", dwChannelNumber + 1, EX3D_CHANNEL_COUNT));
		RetCode = ERR_NOT_FOUND;
	}

	return RetCode;
}

int enableEx3D(DS_BOOL value) {
	if (isEnableEx3D() != value) {
		if (value) m_dwFlags |= DSAP_EX3D_ON_BIT;
		else m_dwFlags &= ~(DSAP_EX3D_ON_BIT);

		m_dwFlags |= DSAP_EX3D_ONOFF_CHANGED_BIT;
		m_bUpdatedEX3D = true;
	}

    DSTRACE(("[CDSAudioProcessor::enableEx3D(value:%d)] m_dwFlags: 0x%08X\n\r", value, m_dwFlags));

	return NO_ERR;
}

DS_BOOL isEnableEx3D() {
    return (m_dwFlags & DSAP_EX3D_ON_BIT) == DSAP_EX3D_ON_BIT;
}

int enableUpMix(DS_BOOL value) {
#if defined(USE_UPMIX)
    DS_BOOL oldValue = (m_dwFlags & DSAP_EX3D_UPMIX_BIT) == DSAP_EX3D_UPMIX_BIT;
    if ((m_bEnUpmixFunction == true) && (oldValue != value)) {
		if( value ) m_dwFlags |= DSAP_EX3D_UPMIX_BIT;
		else m_dwFlags &= ~(DSAP_EX3D_UPMIX_BIT);

		m_bUpdatedEX3D = true;
	}

    DSTRACE(("[CDSAudioProcessor::enableUpMix(value:%d)] oldValue:%d, m_dwFlags: 0x%08X\n\r", value, oldValue, m_dwFlags));
#endif

	return NO_ERR;
}
    
DS_BOOL isEnableUpMix() {
#if defined(USE_UPMIX)
    DS_BOOL bUpmix = ((m_dwFlags & DSAP_EX3D_UPMIX_BIT) == DSAP_EX3D_UPMIX_BIT );
    DS_BOOL bResult = m_bEnUpmixFunction ? bUpmix : false;
    return bResult;
#else
	return false;
#endif
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장들을 읽는다.
	매개변수: pfProc	-> EX-3D 음장 읽기 처리문
			  pContext	-> EX-3D 음장 읽기 처리문 콘텍스트
			  bPrepare	-> true: EX-3D 음장 정보들을 새로 고친 후 읽는다.
						  false: 현재 EX-3D 음장 정보를을 읽는다.
	되돌림값: NO_ERR	-> EX-3D 음장 읽기 처리 성공
			  NO_ERR 외	-> EX-3D 음장 읽기 처리 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int ReadEX3DSoundFields(PFDSProc pfProc, PVOID pContext, DS_BOOL bPrepare)
{
	if( bPrepare ){
		int RetCode = CDSConvolutionReverbPrepareEX3DSoundFields();
		if(RetCode != NO_ERR) return RetCode;
	}
	return CDSConvolutionReverbReadEX3DSoundFields(pfProc, pContext);
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장들을 준비한다.
	매개변수: pszDirectory	-> EX-3D 음장 최상위 디렉토리 경로 문자열 CHAR 포인터			  
	되돌림값: NO_ERR	-> EX-3D 음장들 준비 성공
			  NO_ERR 외	-> EX-3D 음장들 준비 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int PrepareEX3DSoundFields(PCHAR pszDirectory)
{
	return CDSConvolutionReverbPrepareEX3DSoundFields();
}

#if defined(USE_OS)
/* --------------------------------------------------------------------------------
    기    능: 일시 중지한다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 일시 중지 성공
			  NO_ERR 외	-> 일시 중지 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSAudioProcessorPause()
{
	int RetCode = NO_ERR;
	DSTRACE(("[CDSAudioProcessor::Pause()] Entering...\n\r"));

	m_bPause = true;
	EVENT_SET(&m_Event);

	DSTRACE(("[CDSAudioProcessor::Pause(), %d] Leaving...\n\r", RetCode));
	return RetCode;
}

/* --------------------------------------------------------------------------------
    기    능: 일시 중지된 동작을 다시 시작한다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 일시 중지된 동작 다시 시작 성공
			  NO_ERR 외	-> 일시 중지된 동작 다시 시작 실패, 오류값
    비    고: 
    --------------------------------------------------------------------------------- */
int CDSAudioProcessorResume()
{
	int RetCode = NO_ERR;
	DSTRACE(("[CDSAudioProcessor::Resume()] Entering...\n\r"));
	
	m_bPause = false;
	EVENT_SET(&m_Event);

	DSTRACE(("[CDSAudioProcessor::Resume(), %d] Leaving...\n\r", RetCode));
	return RetCode;
}
#endif

/* --------------------------------------------------------------------------------
    기    능: EX-3D 게인(세기)를 설정한다.
	매개변수: uiValue	-> 0 이하: dB 게인 값, 0 초과: predefine dB 게인 값
	되돌림값: 없음
    비    고:
    --------------------------------------------------------------------------------- */
int SetEX3DAudioGain(INT32 iValue)
{
	int RetCode = NO_ERR;

	if(iValue > 0) {
		DSTRACE(("[Error - CDSAudioProcessor::SetEX3DAudioGain(%u)] Invalid parameter!! Exiting...\n\r", iValue));
		RetCode = ERR_INVALID_PARAMS;
	}

	if(RetCode == NO_ERR) {
		m_fGainValue = (CONVOL_T) DS_POW(10, (iValue / 20.0));
		DSTRACE(("[CDSAudioProcessor::SetEX3DAudioGain(%d)] m_fGainValue:%f\n\r", iValue, m_fGainValue));

		UINT uiChannels = m_dwChannels;
		if(uiChannels < 8)
			uiChannels = 8;
		for (UINT i = 0; i < uiChannels; i++) {
			_CaculatePanningGain(i, &m_EX3DChannel[i], false);
			_CaculatePanningGain(i, &m_EX3DChannel[i], true);
		}
	}

	return RetCode;
}

int SetEX3DAudioOffGain(INT32 iValue)
{
	int RetCode = NO_ERR;

	if(iValue > 0) {
		if(iValue < 4) {	// 기존 android demo app에 사용했던 Gain 설정과 호환을 위해 남겨둠, 추 후 삭제 예정
			iValue = -2 * iValue;
			//DSTRACE(("[CDSAudioProcessor::SetEX3DAudioOffGain()] %d dB\n\r", iValue));
		} else {
			DSTRACE(("[Error - CDSAudioProcessor::SetEX3DAudioOffGain(%u)] Unknown type!! Exiting...\n\r", iValue));
			RetCode = ERR_INVALID_TYPE;
		}
	}

	if(RetCode == NO_ERR) {
		m_fOffGainValue = FIXED_CONV( DS_POW(10, (iValue / 20.0)) );
		DSTRACE(("[CDSAudioProcessor::SetEX3DAudioOffGain(%d)] m_fOffGainValue:%f\n\r", iValue, FLOAT_CONV(m_fOffGainValue) ));
	}

	return RetCode;
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
int SetEX3DAudioLFE(DS_BOOL bEnable, INT32 idBGain)
{
	//SetEX3DAudioMute(4, !bEnable);	// set Mute/Unmute LFE channel

	return SetLFE( bEnable, idBGain );
}
#endif

static void CaculatePositionPanningGain(UINT uiInChannels)
{
#if defined(USE_UPMIX)
	if(m_bEnUpmixFunction == true)
		uiInChannels = 8;  // Upmix Channel 수를 나타내는 변수로 변경할 것
#endif

	for(UINT i = 0; i < uiInChannels; i++){
		PEX3DCHANNEL pCh = &m_EX3DChannel[ i ];

#if !defined(USE_ONLY_HORIZONTALITY)
		if(pCh->dwFlags & EX3D_CH_VANGLE_CHANGED_BIT) {
			pCh->dwFlags &= ~EX3D_CH_VANGLE_CHANGED_BIT;
			pCh->wTargetVAngle = m_EX3DAngle[i].wTargetVAngle;
			_CaculatePanningGain(i, pCh, true);
			DSTRACE(("[CDSAudioProcessor::OnProcessing()] EX-3D Position%d: V-Angle Updated!!\n\r", i + 1));
		}
#endif

		if(pCh->dwFlags & EX3D_CH_HANGLE_CHANGED_BIT) {
			pCh->dwFlags &= ~EX3D_CH_HANGLE_CHANGED_BIT;
			pCh->wTargetHAngle = m_EX3DAngle[i].wTargetHAngle;
			_CaculatePanningGain(i, pCh, false);
			DSTRACE(("[CDSAudioProcessor::OnProcessing()] EX-3D Position%d: H-Angle Updated!!\n\r", i + 1));
		}
	}
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 패닝 이득을 계산한다.
	매개변수: uiInx			-> 위치 색인번호
			  pChannel		-> EX-3D 위치 EX3DCHANNEL 구조체 포인터
			  bUseVAngle	-> true: 수직각을 사용한다.
							  false: 수평각을 사용한다.
	되돌림값: 없음
    비    고:
    --------------------------------------------------------------------------------- */
#define VERTICAL_ANGLE_RANGE	45	// 0~90 사이의 값을 설정
									// 45의미: Top, Bottom 의 HRTF Data 가 수직각 45(90-45)도, 135(90+45)도의 Data 임을 의미함
static void _CaculatePanningGain(UINT uiInx, PEX3DCHANNEL pChannel, DS_BOOL bUseVAngle)
{
	UINT uiSetAngle, uiBAngle, uiEAngle, uiAngleStep;
	POINT_T *pTargetBGain, *pTargetEGain;
	CONVOL_T fBGain, fEGain, fV;
	UINT uiTopAngle = 90 - VERTICAL_ANGLE_RANGE;
	UINT uiBottomAngle = 90 + VERTICAL_ANGLE_RANGE;

	//DSTRACE(("[CDSAudioProcessor::_CaculatePanningGain(%u, %s)] Entering...\n\r", uiInx, bUseVAngle ? "V" : "H"));

	// 설정각, 시작각, 끝각을 구한다.
	if( bUseVAngle ){
		// 유효 수직각 범위를 (90-VERTICAL_ANGLE_RANGE) ~ (90+VERTICAL_ANGLE_RANGE)범위로 한정
		// VERTICAL_ANGLE_RANGE 를 벗어난 경우 각각 분배없이(Gain값 1) Top, Bottom 의 HRTF Data 와 Convolution 함
		uiSetAngle = pChannel->wTargetVAngle;
		if(uiSetAngle < uiTopAngle) uiSetAngle = uiTopAngle;
		else if(uiSetAngle > uiBottomAngle) uiSetAngle = uiBottomAngle;

		pTargetBGain = &(pChannel->VTargetBGain);
		pTargetEGain = &(pChannel->VTargetEGain);

		uiAngleStep = VERTICAL_ANGLE_RANGE;
	}else{
		uiSetAngle = pChannel->wTargetHAngle;
		pTargetBGain = &(pChannel->HTargetBGain);
		pTargetEGain = &(pChannel->HTargetEGain);

		uiAngleStep = EX3D_CRUD_STEP_H_ANGLE;
	}

	uiBAngle = (uiSetAngle / uiAngleStep) * uiAngleStep;
	uiEAngle = uiBAngle + uiAngleStep;

	if((bUseVAngle == true) && (uiBAngle >= uiBottomAngle)) {
		// 수직각이 Bottom Range를 벗어난 경우 Begin Gain 0, End Gain 1로 하여 Bottom HRTF만 적용되도록 한다.
		fBGain = 0.0f;
		fEGain = 1.0f;
	}else {
		switch (m_uiEX3DAudioPanningType) {
			case ConstantGain_DSPanningType:
				// 일반 패닝 방식
				fBGain = (CONVOL_T) (uiEAngle - uiSetAngle);
				fBGain /= uiAngleStep;        // (끝각 - 설정각)/(끝각 - 시작각)

				fEGain = (CONVOL_T) (uiSetAngle - uiBAngle);
				fEGain /= uiAngleStep;        // (설정각 - 시작각)/(끝각 - 시작각)
				break;

			case ConstantPower_DSPanningType:
				// 일정 파워 패닝 방식
				fV = ((CONVOL_T) M_PI * (uiSetAngle - uiBAngle)) / (2 * uiAngleStep);
				fBGain = DS_COS(fV);
				fEGain = DS_SIN(fV);
				break;

			case ConstantPower2_DSPanningType:
				// 일정 파워 패닝 방식 2
				fV = ((CONVOL_T) M_PI * (uiSetAngle - uiBAngle)) / (2 * uiAngleStep);
				fBGain = DS_COS(fV);
				fEGain = DS_SIN(fV);

				// 45도일 때, 0.707이 아닌 0.5가 되도록 했으며 최대 값이 0.707을 넘지 않도록 했다.
				// 아래 코드를 제거할 경우 종단에 리미터를 반드시 연결해야 한다.
				fV = (CONVOL_T) DS_SIN(M_PI * 0.25) * 2;
				fBGain /= fV;
				fEGain /= fV;
				break;

			default:
				// 패닝 안함
				fBGain = 1.0f;
				fEGain = 0.0f;
				break;
		}
	}
	*pTargetBGain = FIXED_CONV(fBGain * m_fGainValue);
	*pTargetEGain = FIXED_CONV(fEGain * m_fGainValue);

	DSTRACE(("[CDSAudioProcessor::_CaculatePanningGain(%u, %s), %s] uiBAngle: %u, uiEAngle: %u, uiSetAngle: %u, m_fGainValue: %f, fBGain: %f, TargetBGain: %f, fEGain: %f, TargetEGain: %f\n\r",
			uiInx, (bUseVAngle ? "V" : "H"),
			(m_uiEX3DAudioPanningType == ConstantGain_DSPanningType) ? "Constant Gain" :
			(m_uiEX3DAudioPanningType == ConstantPower_DSPanningType) ? "Constant Power" :
			(m_uiEX3DAudioPanningType == ConstantPower2_DSPanningType) ? "Constant Power2" : "None",
			uiBAngle, uiEAngle, uiSetAngle, m_fGainValue,
			bUseVAngle ? FLOAT_CONV(pChannel->VBGain) : FLOAT_CONV(pChannel->HBGain),
			FLOAT_CONV(*pTargetBGain),
			bUseVAngle ? FLOAT_CONV(pChannel->VEGain) : FLOAT_CONV(pChannel->HEGain),
			FLOAT_CONV(*pTargetEGain)
			));
}
