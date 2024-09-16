#ifndef __DS_BUILD_H__
#define __DS_BUILD_H__

//#define _DEBUG

#define XMOS_SDK

// #define USE_OS						// Thread, Event, Lock 등의 외부 OS API 사용
#define USE_FIXED_POINT				// 내부 연산 고정소수점 사용
#define USE_PLANAR_ARRAY_INPUT		// Audio Input Data Planar 배열 사용
#define USE_ONLY_HORIZONTALITY		// 수평 방향(Vertical 90도)만 사용
// #define USE_LFE_UNIT				// LFE 채널 Convolution 사용
// #define USE_UPMIX					// Upmix 기능 사용

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#endif //__DS_BUILD_H__
