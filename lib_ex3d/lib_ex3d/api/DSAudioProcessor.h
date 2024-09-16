#ifndef __DSAUDIOPROCESSOR_H__
#define __DSAUDIOPROCESSOR_H__

#include "digisonic.h"

#if defined(USE_OS)
	#include "DSEvent.h"
	#include "DSLock.h"
#endif
#include "DSBuffer.h"
#include "DSConvolutionReverb.h"

// EX-3D 음원 channel 개수
#define EX3D_CHANNEL_COUNT					32

// m_EX3DChannel 플래그 비트
#define EX3D_CH_MUTE_BIT					BIT28		// 0: 음장 사용, 1: 음장 사용금지
#define EX3D_CH_VANGLE_CHANGED_BIT			BIT26		// 0: V Angle 변경 없음, 1: V Angle 변경
#define EX3D_CH_HANGLE_CHANGED_BIT			BIT25		// 0: H Angle 변경 없음, 1: H Angle 변경
#define EX3D_CH_VANGLE_SET_BIT				BIT24		// 0: V Angle 설정 안됨, 1: V Angle 설정
#define EX3D_CH_HANGLE_SET_BIT				BIT23		// 0: H Angle 설정 안됨, 1: H Angle 설정

// m_dwFlags 플래그 비트
#define DSAP_EX3D_ON_BIT					BIT31		// 1: EX-3D 켬, 0: EX-3D 끔
#define DSAP_EX3D_ONOFF_CHANGED_BIT			BIT30		// 1: EX-3D On/Off 변경, 0: EX-3D On/Off 변경 없음
#define DSAP_EX3D_CH_NUM_CHANGED_BIT		BIT29		// 1: Channel 수 바뀜, 0: Channel 수 유지
#define DSAP_EX3D_SOUNDFIELD_CHANGED_BIT	BIT28		// 1: EX-3D 음장 바뀜, 0: EX-3D 음장 유지
#define DSAP_EX3D_UPMIX_BIT					BIT26		// 1: Upmix On, 0: Upmix Off
#define DSAP_INPUT_LENGTH_AS_CR_LENGTH_BIT	BIT25		// 1: use input data length as convolution length, 0: use hrtf length as convolution length

typedef struct _EX3DCHANNEL_
{
	DWORD dwFlags;				// 플래그값
	WORD wHAngle;				// 수평각
	WORD wVAngle;				// 수직각
    WORD wTargetHAngle;			// 수평각
    WORD wTargetVAngle;			// 수직각

	POINT_T HBGain;				// 수평 시작 패닝 이득
	POINT_T HEGain;				// 수평 끝 패닝 이득
	POINT_T HTargetBGain;		// 수평 Target 시작 패닝 이득
	POINT_T HTargetEGain;		// 수평 Target 끝 패닝 이득

	POINT_T VBGain;				// 수직 시작 패닝 이득
	POINT_T VEGain;				// 수직 끝 패닝 이득
	POINT_T VTargetBGain;		// 수직 Target 시작 패닝 이득
	POINT_T VTargetEGain;		// 수직 Target 끝 패닝 이득

	POINT_T ChGain;				// 개별 Channel 이득
}EX3DCHANNEL, *PEX3DCHANNEL;

typedef struct _EX3DANGLE_
{
	WORD wTargetHAngle;			// 수평각
	WORD wTargetVAngle;			// 수직각
}EX3DANGLE, *PEX3DANGLE;

typedef enum {
	FADE_STATE_NONE = 0,
	FADE_STATE_3D_ONOFF_FADE_OUT,
	FADE_STATE_3D_ON_MUTE_WAIT,
	FADE_STATE_3D_ON_MUTE,
	FADE_STATE_CHG_CH_NUM_FADE_OUT,
	FADE_STATE_CHG_CH_NUM_MUTE_WAIT,
	FADE_STATE_CHG_CH_NUM_MUTE,
	FADE_STATE_CHG_SF_FADE_OUT,
	FADE_STATE_CHG_SF_MUTE_WAIT,
	FADE_STATE_CHG_SF_MUTE,
	FADE_STATE_FADE_IN
} FADE_STATE;

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSAUDIOPROCESSOR_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

EXTERN void CDSAudioProcessor();
EXTERN void CDSAudioProcessorDestroyer();


EXTERN int CDSAudioProcessorOpen(DWORD dwChannels,
			 DWORD dwSampleSize, 
			 DWORD dwSRHz, 
			 DWORD dwAudioDataSize,
		     DWORD dwFlags);
EXTERN int CDSAudioProcessorClose();
EXTERN int CDSAudioProcessorChangeParameter(DWORD dwChannels, DWORD dwSampleSize, DWORD dwSRHz, DWORD dwAudioDataSize);

EXTERN int CDSAudioProcessorPut(PBYTE pData, DWORD dwLength, DWORD dwFlags);
EXTERN int CDSAudioProcessorProcess(PBYTE pInBuf, PBYTE pOutBuf, DWORD dwLength, DWORD dwFlags);
EXTERN int CDSAudioProcessorClear();

#if defined(USE_OS)
EXTERN int CDSAudioProcessorPause();
EXTERN int CDSAudioProcessorResume();
#else
EXTERN int OnProcessing();
#endif

EXTERN int PrepareEX3DSoundFields(PCHAR pszDirectory);
EXTERN int ReadEX3DSoundFields(PFDSProc pfProc, PVOID pContext, DS_BOOL bPrepare);

EXTERN int SetEX3DAudioSoundField(PCHAR pszSFName);
EXTERN int SetEX3DAudioAngle(DWORD dwChannelNumber, DWORD dwVAngle, DWORD dwHAngle, DS_BOOL bApply);
EXTERN int SetEX3DAudioMute(DWORD dwChannelNumber, DS_BOOL bMute);
EXTERN int enableEx3D(DS_BOOL value);
EXTERN DS_BOOL isEnableEx3D();
EXTERN int enableUpMix(DS_BOOL value);
EXTERN DS_BOOL isEnableUpMix();

EXTERN int SetEX3DAudioPanningType(UINT uiType);
EXTERN int SetEX3DAudioGain(INT32 iValue);
EXTERN int SetEX3DAudioOffGain(INT32 iValue);
#if defined(USE_LFE_UNIT)
EXTERN int SetEX3DAudioLFE(DS_BOOL bEnable, INT32 idBGain);
#endif

EXTERN DS_BOOL CDSAudioProcessorOpened();

EXTERN DWORD InQueueFilledLength();
EXTERN DWORD InQueueAvailableSize();

EXTERN DWORD m_dwID;										// 사용자 정의 ID, 기본값: 0

#if defined(USE_OS)
EXTERN DSEVENT m_Event;
EXTERN DSEVENT m_InQEvent;
EXTERN DSEVENT m_OutQEvent;

EXTERN DSLOCK m_InQLock;
EXTERN DSLOCK m_OutQLock;
#endif

EXTERN CDSBUFFER m_InQ;
EXTERN CDSBUFFER m_OutQ;
EXTERN DWORD m_dwInQPutLength;

EXTERN PBYTE m_pProcessBuf1;
EXTERN PBYTE m_pProcessBuf2;

EXTERN DWORD m_dwProcessBuf1Size;
EXTERN DWORD m_dwProcessBuf2Size;

EXTERN CDSCHARBUFFER m_szCurrentSFName;						// 현재 설정된 음장 이름

#if defined(USE_OS)

volatile EXTERN DS_BOOL m_hThread;
volatile EXTERN DS_BOOL m_bThreadRunning;
EXTERN int m_State;
EXTERN DS_BOOL m_bPause;

#else

volatile EXTERN DS_BOOL m_hThread;

#endif

#if defined(USE_UPMIX)
EXTERN DS_BOOL g_bUpmix;
#endif
EXTERN FADE_STATE g_FadeState;
EXTERN DWORD g_dwFadeProcessCnt;

EXTERN UINT g_uiOutChannels;

EXTERN DWORD g_dwOutDataSize;

EXTERN DWORD g_dwIN_CR_SampleNum;
EXTERN UINT g_uiUnitCount;
EXTERN DWORD g_dwInputDataSize;
EXTERN DWORD g_dwOutQMinSize;

EXTERN UINT m_uiEX3DAudioPanningType;

EXTERN DWORD m_dwChannels;
EXTERN DWORD m_dwSampleSize;
EXTERN DWORD m_dwSRHz;
EXTERN DWORD m_dwAudioDataSize;

EXTERN DWORD m_dwFlags;										// 플래그	
EXTERN EX3DCHANNEL m_EX3DChannel[ EX3D_CHANNEL_COUNT ];		// EX-3D 음원 위치
EXTERN EX3DANGLE m_EX3DAngle[ EX3D_CHANNEL_COUNT ];

EXTERN DS_BOOL m_bEnabledEX3D;
EXTERN DS_BOOL m_bUpdatedEX3D;
EXTERN DS_BOOL m_bClear;
EXTERN DS_BOOL m_bEndOfStream;
EXTERN DS_BOOL m_bFullChannelOutput;
EXTERN AUDIO_DATA_TYPE m_AudioDataType;
#if defined(USE_UPMIX)
EXTERN DS_BOOL m_bEnUpmixFunction;
#endif
EXTERN DS_BOOL m_bWaitProcess;

EXTERN CONVOL_T m_fGainValue;
EXTERN POINT_T m_fOffGainValue;

#ifdef _DEBUG
EXTERN DS_BOOL m_bFirstAudioData;
#endif

#endif	// __DSAUDIOPROCESSOR_H__
