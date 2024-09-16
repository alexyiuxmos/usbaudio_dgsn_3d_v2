#include "EX3DAudio.h"
#include "DSAudioProcessor.h"

//extern short sLv[8] = {0, };

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EXTERN_C_BEGIN
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
int EX3DAudio_Open(	DWORD dwChannels,
					DWORD dwSampleSize, 
					DWORD dwSRHz, 
					DWORD dwAudioDataSize, 
					DWORD dwFlags,
					PCHAR pszSFName)
{
	int RetCode = NO_ERR;
	DSTRACE(("[EX3DAudio_Open()] Entering...\n\r"));

 	if( CDSAudioProcessorOpened() ){
		DSTRACE(("[Error - EX3DAudio_Open()] Opened!! Exiting...\n\r"));
		return ERR_OPENED;
	}

 	// SysTimer = hwtimer_alloc();

	CDSAudioProcessor();
	if(pszSFName) {
		PrepareEX3DSoundFields((PCHAR)"sfields");
		SetEX3DAudioSoundField(pszSFName);
	}

	RetCode = CDSAudioProcessorOpen(dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize, dwFlags);
	if(RetCode != NO_ERR) EX3DAudio_Close();
	DSTRACE(("[EX3DAudio_Open(), %d] Leaving...\n\r", RetCode));
	return RetCode;
}

int EX3DConv_Open(	DWORD dwTileNum,
					DWORD dwChannels,
					PCHAR pszSFName)
{
	int RetCode = NO_ERR;

    CDSConvolutionReverb(dwTileNum);
    CDSConvolutionReverbPrepareEX3DSoundFields();
    RetCode = CDSConvolutionReverbOpen(dwChannels, 0, pszSFName, false);

	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: EX-3D 오디오를 닫는다.
	매개변수: 없음
	되돌림값: NO_ERR	-> EX-3D 오디오 닫기 성공
			  NO_ERR 외	-> EX-3D 오디오 닫기 실패, 오류값
    비    고: 
  ------------------------------------------------------------------------ */
int EX3DAudio_Close()
{
	int RetCode = NO_ERR;
	DSTRACE(("[EX3DAudio_Close()] Entering...\n\r"));

 	if( CDSAudioProcessorOpened() ){
		//CDSAudioProcessorClose();
		CDSAudioProcessorDestroyer();
	}

	hwtimer_free(SysTimer);

	DSTRACE(("[EX3DAudio_Close(), %d] Leaving...\n\r", RetCode));
	return RetCode;
}

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
int EX3DAudio_ChangeParameter(DWORD dwChannels, DWORD dwSampleSize, DWORD dwSRHz, DWORD dwAudioDataSize)
{
	int RetCode = NO_ERR;
	DSTRACE(("[EX3DAudio_ChangeParameter()] Entering...\n\r"));

	RetCode = CDSAudioProcessorChangeParameter(dwChannels, dwSampleSize, dwSRHz, dwAudioDataSize);

	DSTRACE(("[EX3DAudio_ChangeParameter(), %d] Leaving...\n\r", RetCode));
	return RetCode;
}

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
int EX3DAudio_ProcessAudioData(PBYTE pInBuf, PBYTE pOutBuf, DWORD dwLength, DWORD dwFlags)
{
	return CDSAudioProcessorProcess(pInBuf, pOutBuf, dwLength, dwFlags);
}

/* ------------------------------------------------------------------------
    기    능: 오디오 입력 버퍼를 초기화시킨다.
	매개변수: 없음
	되돌림값: NO_ERR	-> 오디오 입력 버퍼 초기화 성공
			  NO_ERR 외	-> 오디오 입력 버퍼 초기화 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int EX3DAudio_ClearPlayerBuffer()
{
	return CDSAudioProcessorClear();
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 오디오 음장을 설정한다.
	매개변수: pszSFName	-> 음장 이름 문자열 CHAR 포인터
	되돌림값: NO_ERR	-> EX-3D 오디오 음장 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 음장 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetSoundField(PCHAR pszSFName)
{
	return SetEX3DAudioSoundField(pszSFName);
}

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
int EX3DAudio_SetAngle(DWORD dwChannelNumber, DWORD dwVAngle, DWORD dwHAngle, DS_BOOL bApply)
{
	return SetEX3DAudioAngle(dwChannelNumber, dwVAngle, dwHAngle, bApply);
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 오디오 채널별 Mute를 설정한다.
	매개변수: dwChannelNumber   -> 입력 오디오 채널 번호
			  bMute	    		-> Mute여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetMute(DWORD dwChannelNumber, DS_BOOL bMute)
{
	return SetEX3DAudioMute(dwChannelNumber, bMute);
}

/* --------------------------------------------------------------------------------
    기    능: EX3D 동작 여부를 설정한다.
	매개변수: bEnable	-> EX3D 동작 여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetEX3D(DS_BOOL bEnable)
{
	return enableEx3D(bEnable);
}

/* --------------------------------------------------------------------------------
    기    능: Upmix 동작 여부를 설정한다.
	매개변수: bEnable	-> Upmix 동작 여부
	되돌림값: NO_ERR	-> EX-3D 오디오 설정값 설정 성공
			  NO_ERR 외	-> EX-3D 오디오 설정값 설정 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetUpmix(DS_BOOL bEnable)
{
	return enableUpMix(bEnable);
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D LFE 사용 유무 및 Gain 설정.
    매개변수: bEnable	-> LFE 사용 유무
			  idBGain	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetLFE(DS_BOOL bEnable, INT32 idBGain)
{
#if defined(USE_LFE_UNIT)
	return SetEX3DAudioLFE( bEnable, idBGain );
#else
	return ERR_NOT_USE_LFE_UNIT;
#endif
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D Off 감쇠 Gain 설정.
    매개변수: idBGain	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetOffGain(INT32 idBGain)
{
	return SetEX3DAudioOffGain(idBGain);
}

/* --------------------------------------------------------------------------------
    기    능: EX-3D 출력(전체) Gain 설정.
    매개변수: idBGain	-> 감쇠 dB(0 이하값)
    되돌림값: NO_ERR    -> 성공
              NO_ERR 외	-> 실패, 오류값
    비    고:
    --------------------------------------------------------------------------------- */
int EX3DAudio_SetGain(INT32 idBGain)
{
	return SetEX3DAudioGain( idBGain );
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EXTERN_C_END
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
