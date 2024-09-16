#ifndef __EX3DAUDIODEF_H__
#define __EX3DAUDIODEF_H__

#include "dsdatatype.h"
////////////////////////////////////////////////////////////////////////////////
// Open 플래그 비트
#define EX3D_FULL_CHANNEL_OUTPUT_BIT		BIT0		// 1: 입력 받은 오디오 채널 개수대로 출력한다. 0: 2채널로 출력한다.
#define EX3D_UPMIX_BIT              		BIT1		// 1: Upmix 기능 Enable. 0: Upmix 기능 Disable.
#define EX3D_FLOATING_POINT_BIT				BIT7		// 1: 부동소숫점 오디오 데이터 0: PCM 오디오 데이터

// 스트림 플래그 비트
#define EX3D_END_OF_STREAM_BIT				BIT0		// 0: 스트림 끝이 아님, 1: 스트림 끝

#define UPMIX_CHANNEL_NUMBER				8

#define EX3D_MAX_CHANNEL_COUNT				32			// 최대 채널 개수: 32개

#define EX3D_SOUNDFIELD_NAME_MAX_LENGTH		64

typedef enum _EnumDSPanningType_
{
	None_DSPanningType = 0,								// 패닝 안함
	ConstantGain_DSPanningType,							// 일정 게인 패닝
	ConstantPower_DSPanningType,						// 일정 파워 패닝, 범위: 0 ~ 1
	ConstantPower2_DSPanningType,						// 일정 파워 패닝, 범위: 0 ~ 0.707
}EnumDSPanningType;

typedef enum _EnumDSState_
{
	Stop_DSState = 0,
	Run_DSState,
	Pause_DSState,
	Resume_DSState,
	RequestStop_DSState,
	RequestRun_DSState,
	RequestPause_DSState,
	RequestResume_DSState,
}EnumDSState;

////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //__EX3DAUDIODEF_H__
