#ifndef __EX3DAUDIO_H__
#define __EX3DAUDIO_H__

#include "dserror.h"
#include "EX3DAudioDef.h"

#if defined(_WINDOWS) || defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64)
#if defined(EX3D_SDK_API_EXPORTS)
#define EX3D_SDK_API __declspec(dllexport)
#else
#define EX3D_SDK_API __declspec(dllimport)
#endif
#else
#define EX3D_SDK_API
#endif

////////////////////////////////////////////////////////////////////////////////
EXTERN_C_BEGIN
////////////////////////////////////////////////////////////////////////////////////////////////////
/* ------------------------------------------------------------------------
    기    능: EX-3D 오디오를 연다.
	매개변수: dwChannels		-> 채널 개수
			  dwSampleSize		-> 샘플 바이트 크기
								   최소 크기: 64 바이트
			  dwSRHz			-> 샘플링 레이트, 단위: Hz
			  dwAudioDataSize	-> Audio data 바이트 크기
			  dwFlags			-> 플래그
			  pszSFName			-> 음장 이름 문자열 CHAR 포인터
	되돌림값: NO_ERR	-> EX-3D 오디오 열기 성공
			  NO_ERR 외	-> EX-3D 오디오 열기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
EX3D_SDK_API int EX3DAudio_Open(DWORD dwChannels, DWORD dwSampleSize, DWORD dwSRHz, DWORD dwAudioDataSize, DWORD dwFlags, PCHAR pszSFName);

EX3D_SDK_API int EX3DConv_Open(	DWORD dwTileNum, DWORD dwChannels, PCHAR pszSFName);

/* ------------------------------------------------------------------------
    기    능: EX-3D 오디오를 닫는다.
	매개변수: 없음
	되돌림값: NO_ERR	-> EX-3D 오디오 닫기 성공
			  NO_ERR 외	-> EX-3D 오디오 닫기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
EX3D_SDK_API int EX3DAudio_Close();

/* ------------------------------------------------------------------------
    기    능: EX-3D 오디오 Parameter 수정.
	매개변수: dwChannels		-> 채널 개수
			  dwSampleSize		-> 샘플 바이트 크기
								   최소 크기: 64 바이트
			  dwSRHz			-> 샘플링 레이트, 단위: Hz
			  dwAudioDataSize	-> Audio data 바이트 크기
	되돌림값: NO_ERR	-> EX-3D 오디오 Parameter 수정 성공
			  NO_ERR 외	-> EX-3D 오디오 Parameter 수정 실패, 오류값
    비    고: 
  ------------------------------------------------------------------------ */
EX3D_SDK_API int EX3DAudio_ChangeParameter(DWORD dwChannels, DWORD dwSampleSize, DWORD dwSRHz, DWORD dwAudioDataSize);

/* ------------------------------------------------------------------------
    기    능: 오디오 데이터를 EX3D Process 하여 출력한다.
	매개변수: pInBuf	-> 오디오 입력 데이터 BYTE 포인터
			  pOutInBuf	-> EX3D Process 출력 데이터 BYTE 포인터
			  dwLength	-> 오디오 데이터 길이(=바이트 크기)
			  dwFlags	-> 플래그
	되돌림값: > -1	-> EX3D Process 성공한 오디오 데이터 길이
			  < 0	-> EX3D Process 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
EX3D_SDK_API int EX3DAudio_ProcessAudioData(PBYTE pInBuf, PBYTE pOutBuf, DWORD dwLength, DWORD dwFlags);

/* ------------------------------------------------------------------------
    기    능: 오디오 입력 버퍼를 초기화시킨다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 오디오 입력 버퍼 초기화 성공
			  NO_ERR 외	-> 오디오 입력 버퍼 초기화 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
EX3D_SDK_API int EX3DAudio_ClearPlayerBuffer();

/* ------------------------------------------------------------------------
    기    능: 오디오 입력 버퍼가 비어있는지를 검사한다.
	매개변수: 없음
	되돌림값: TRUE	-> 빈 상태
			 FALSE	-> 빈 상태가 아님.
    비    고:
  ------------------------------------------------------------------------ */
EX3D_SDK_API DS_BOOL EX3DAudio_IsPlayerBufferEmpty();

/* --------------------------------------------------------------------------------
    기    능: EX-3D 음장들을 읽는다.
	매개변수: pfProc	-> EX-3D 음장 읽기 처리문
			  pContext	-> EX-3D 음장 읽기 처리문 콘텍스트
			  dwFlags	-> 플래그값
						   비트0
							  1	-> EX-3D 음장 정보들을 새로 고친 후 읽는다.
						      0	-> 현재 EX-3D 음장 정보를을 읽는다.
	되돌림값: NO_ERR	-> EX-3D 음장 읽기 처리 성공
			  NO_ERR 외	-> EX-3D 음장 읽기 처리 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_ReadSoundFields(PFDSProc pfProc, PVOID pContext, DWORD dwFlags);

/* --------------------------------------------------------------------------------
    기    능: EX-3D 오디오 설정값을 설정한다.
	매개변수: pszSFName	-> 음장 이름 문자열 CHAR 포인터
			  dwAngle	-> 방위각
			               비트0..14: 수평각, 0 ~ 359
						   비트16..30: 수직각, 0 ~ 359
			  dwFlags	-> 플래그값
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetSFSettings(PCHAR pszSFName, DWORD dwAngle, DWORD dwFlags);

/* --------------------------------------------------------------------------------
    기    능: EX-3D 오디오 음장을 설정한다.
	매개변수: pszSFName	-> 음장 이름 문자열 CHAR 포인터
	되돌림값: NO_ERR	-> EX-3D 오디오 음장 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 음장 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetSoundField(PCHAR pszSFName);

/* --------------------------------------------------------------------------------
    기    능: EX-3D 오디오 채널별 방위각을 설정한다.
	매개변수: dwChannelNumber   -> 입력 오디오 채널 번호
			  dwVAngle      	-> 수직각, 0~180
			  dwHAngle      	-> 수평각, 0~359
			  bApply	        -> 적용여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetAngle(DWORD dwChannelNumber, DWORD dwVAngle, DWORD dwHAngle, DS_BOOL bApply);

/* --------------------------------------------------------------------------------
    기    능: EX-3D 오디오 채널별 Mute를 설정한다.
	매개변수: dwChannelNumber   -> 입력 오디오 채널 번호
			  bMute	    		-> Mute여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetMute(DWORD dwChannelNumber, DS_BOOL bMute);

/* --------------------------------------------------------------------------------
    기    능: EX3D 동작 여부를 설정한다.
	매개변수: bEnable	-> EX3D 동작 여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetEX3D(DS_BOOL bEnable);

/* --------------------------------------------------------------------------------
    기    능: Upmix 동작 여부를 설정한다.
	매개변수: bEnable	-> Upmix 동작 여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetUpmix(DS_BOOL bEnable);

/* --------------------------------------------------------------------------------
    기    능: EX-3D LFE 사용 유무 및 Gain 설정.
    매개변수: bEnable	-> LFE 사용 유무
			  idBGain	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetLFE(DS_BOOL bEnable, INT32 idBGain);

/* --------------------------------------------------------------------------------
    기    능: EX-3D Off 감쇠 Gain 설정.
    매개변수: idBGain	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetOffGain(INT32 idBGain);

/* --------------------------------------------------------------------------------
    기    능: EX-3D 출력(전체) Gain 설정.
    매개변수: idBGain	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetGain(INT32 idBGain);

/* --------------------------------------------------------------------------------
    기    능: EX-3D 채널별 Gain 설정.
    매개변수: dwChannelNumber   -> 입력 오디오 채널 번호
			  idBGain       	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
EX3D_SDK_API int EX3DAudio_SetChannelGain(UINT uiChannelNumber, INT32 idBGain);

////////////////////////////////////////////////////////////////////////////////
EXTERN_C_END
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //__EX3DAUDIO_H__
