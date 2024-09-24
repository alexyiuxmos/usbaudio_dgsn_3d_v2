#ifndef __DSCONVOLUTIONREVERB_H__
#define __DSCONVOLUTIONREVERB_H__

#include "digisonic.h"

#include "DSCRUnit.h"
#include "EX3DAudioDef.h"
#include "swlock.h"

#define EX3D_UNIT_TASK_NUM					1
typedef enum {
	EX3D_TILE_MAIN = 0,
	EX3D_TILE_SUB1,
	EX3D_TILE_NUM
} EX3D_TILE;

// #define USE_2HANGLE_3D
// #define USE_3HANGLE_3D
// #define USE_4HANGLE_3D

#if defined(USE_2HANGLE_3D)
#define EX3D_CRUD_H_POSITION_COUNT			2				// 컨볼루션 리버브단 수평 위치 개수
#define EX3D_CRUD_STEP_H_ANGLE				180				// 컨볼루션 리버브단 각도 간격
#define EX3D_CRUD_START_H_ANGLE				90				// 컨볼루션 리버브단 시작 각도
#elif defined(USE_3HANGLE_3D)
#define EX3D_CRUD_H_POSITION_COUNT			3				// 컨볼루션 리버브단 수평 위치 개수
#define EX3D_CRUD_STEP_H_ANGLE				120				// 컨볼루션 리버브단 각도 간격
#define EX3D_CRUD_START_H_ANGLE				60				// 컨볼루션 리버브단 시작 각도
#elif defined(USE_4HANGLE_3D)
#define EX3D_CRUD_H_POSITION_COUNT			4				// 컨볼루션 리버브단 수평 위치 개수
#define EX3D_CRUD_STEP_H_ANGLE				90				// 컨볼루션 리버브단 각도 간격
#define EX3D_CRUD_START_H_ANGLE				45				// 컨볼루션 리버브단 시작 각도
#else
#define EX3D_CRUD_H_POSITION_COUNT			8				// 컨볼루션 리버브단 수평 위치 개수
#define EX3D_CRUD_STEP_H_ANGLE				45				// 컨볼루션 리버브단 각도 간격
#define EX3D_CRUD_START_H_ANGLE				0				// 컨볼루션 리버브단 시작 각도
#endif

#if defined(USE_ONLY_HORIZONTALITY)
#define EX3D_CRUD_TOTAL_POSITION_COUNT		EX3D_CRUD_H_POSITION_COUNT		// 컨볼루션 리버브단 Total 위치 개수(LFE 제외)
#else
#define EX3D_CRUD_TOTAL_POSITION_COUNT		EX3D_CRUD_H_POSITION_COUNT * 3	// 컨볼루션 리버브단 Total 위치 개수(LFE 제외)위치 개수
#endif

#define EX3D_CRUD_LFE_INDEX					EX3D_CRUD_TOTAL_POSITION_COUNT	// CR 입력 데이터 버퍼에서 LFE 데이터 0 base index

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSCONVOLUTIONREVERB_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef struct _EX3DSOUNDFIELD_
{
	char szName[ EX3D_SOUNDFIELD_NAME_MAX_LENGTH ];		// 음장 이름
	DWORD dwFlags;										// 플래그값
}EX3DSOUNDFIELD, *PEX3DSOUNDFIELD;

EXTERN int CDSConvolutionSetSoundField(PCHAR pszName);

EXTERN void CDSConvolutionReverb(DWORD dwTileNum);
EXTERN void CDSConvolutionReverbDestroyer();

EXTERN int CDSConvolutionReverbOpen(DWORD dwChannels, DWORD dwIN_CR_SampleNum, PCHAR pszName, DS_BOOL bCloseIfOpen);

EXTERN int CDSConvolutionReverbClose();

EXTERN int CDSConvolutionReverbProcess(PPOINT_T pData, DWORD dwIN_CR_SampleNum, DWORD CR_BitFiled);

EXTERN int CDSConvolutionReverbPrepareEX3DSoundFields();
EXTERN int CDSConvolutionReverbSetEX3DSoundField(PCHAR pszName);
EXTERN int CDSConvolutionReverbReadEX3DSoundFields(PFDSProc pfProc, PVOID pContext);
EXTERN int CDSConvolutionReverbClearEX3DSoundFieldsBuffer();

#if defined(USE_LFE_UNIT)
EXTERN int SetLFE(DS_BOOL bEnable, INT32 idBGain);
EXTERN DWORD GetLFEdB();
#endif

EXTERN PEX3DSOUNDFIELD GetEX3DSoundField(PCHAR pszName);
EXTERN DWORD IN_CR_SampleNum();
EXTERN UINT UnitCount();

EXTERN DS_BOOL IsOpened();

EXTERN void ConvolutionTaskInit();
EXTERN void ConvolutionTask(int taskIdx, chanend c_main_tile_to_sub_tile1);
EXTERN void ConvolutionProcess(int taskIdx, PPOINT_T pData);

EXTERN DS_BOOL m_bOpened;

EXTERN PCHAR m_pszSFName;			// 현재 음장 이름

EXTERN PEX3DSOUNDFIELD m_pSFs;		// 음장 디렉토리내의 음장을 읽어오기 위한 용도, 이 값으로 상위단(안드로이드라면 App UI(java 단))에서 음장 목록 표시
EXTERN DWORD m_dwSFCount;

EXTERN PDWORD m_pdwBitRevT;
#if !defined(XMOS_FHT_FUNC)
EXTERN PPOINT_T m_pSinTable;
#endif
EXTERN DWORD m_dwBitRevTSize;
EXTERN DWORD m_dwSinTableSize;

EXTERN POINT_T m_Scale;
EXTERN POINT_T m_ScaleGain_dB;

EXTERN PDSCRUNIT m_pBUnits;

EXTERN UINT m_uiUnitCount;
EXTERN DWORD m_StartHAngle;

EXTERN POINT_T* m_pTaskUnitInBuf;
EXTERN PDSCRDATA m_pTaskUnitOutBuf;
EXTERN PDSCRDATA m_pTaskResultBuf;
EXTERN PDSCRDATA m_pTaskOverlapBuf;
EXTERN DSCRDATA m_ResultBuf;

EXTERN swlock_t m_lockConvInBuf[EX3D_UNIT_TASK_NUM];
EXTERN PPOINT_T m_ConvInBuf[EX3D_UNIT_TASK_NUM];
EXTERN swlock_t m_lockConvResult;
EXTERN DS_BOOL m_bConvResult;
EXTERN swlock_t m_lockConvPause[EX3D_UNIT_TASK_NUM];
EXTERN DS_BOOL m_bConvPause[EX3D_UNIT_TASK_NUM];
EXTERN swlock_t m_lockConvPauseComplete;
EXTERN DS_BOOL m_bConvPauseComplete;

EXTERN DS_BOOL m_bIsIN_CR_SampleNumZero;
EXTERN DWORD m_dwIN_CR_SampleNum;

#if defined(USE_LFE_UNIT)
EXTERN PDSCRUNIT m_pLFEUnit;
EXTERN DS_BOOL m_bEnableLFE;
EXTERN POINT_T m_LFEGain;
EXTERN UINT m_uiLFEdB;
#endif

#endif	// __DSCONVOLUTIONREVERB_H__
