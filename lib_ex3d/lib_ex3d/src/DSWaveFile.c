#define _DSWAVEFILE_C_

#include "DSWaveFile.h"
#include "DSmath.h"

#include <DSWaveEmbeddedExir2K_Xmos_Game_WM.h>
#include <DSWaveEmbeddedExir2K_Xmos_Movie_WM.h>
// #include <DSWaveEmbeddedExir2K_Xmos_Music_WM.h>

static BYTE m_WaveDataReadBuf[32];		// 최대 4채널, 64비트까지 지원.
static const unsigned char * m_pPosData;
static uint8_t m_cKey;

static int Open(PCSTR pszPath, DS_BOOL bCloseAfterGetHeader);

/* ------------------------------------------------------------------------
    기    능: WAV 파일, CDSWaveFile 클래스 생성자
	매개변수: pszPath				-> WAV 파일 경로 문자열, CHAR 포인터
									   NULL: 객체만 생성한다. 기본값
			  bCloseAfterGetHeader	-> true: 헤더 정보 구한 후 닫는다.
									  false: 파일을 닫지 않는다.
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
int CDSWaveFile(PCSTR pszPath, DS_BOOL bCloseAfterGetHeader)
{
	int RetCode = NO_ERR;

	m_WaveDataReadBuf[0] = 0;

	m_pPosData = NULL;
	m_cKey = 0x00;

	ZeroMemory(&g_WaveHeader, sizeof(DSWAV_HEADER));
	if( pszPath ) {
		RetCode = Open(pszPath, bCloseAfterGetHeader);
	}

	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: WAV 파일, CDSWaveFile 클래스 파괴자
	매개변수: 없음
	되돌림값: 없음
    비    고:
  ------------------------------------------------------------------------ */
void CDSWaveFileDestroyer()
{
}

/* ------------------------------------------------------------------------
    기    능: WAV 파일을 연다.
	매개변수: pszPath				-> WAV 파일 경로 문자열, CHAR 포인터
			  bCloseAfterGetHeader	-> true: 헤더 정보 구한 후 닫는다.
									  false: 파일을 닫지 않는다.
	되돌림값: NO_ERR	-> WAV 파일 열기 성공
			  NO_ERR 외	-> WAV 파일 열기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
#define PATH_DELIMITER '/'
static int Open(PCSTR pszPath, DS_BOOL bCloseAfterGetHeader)
{
	int RetCode = NO_ERR;
	DSTRACE(("[CDSWaveFile::Open(%s)] Entering...\n\r", pszPath ? pszPath : "null"));
	DSCHECK_PTR("CDSWaveFile::Open()", pszPath, ERR_PTRS_NULL);

	char *pPosName = (char *)strrchr(pszPath, PATH_DELIMITER);
	char *pDot = (char *)strrchr(pszPath, '.');		// 마지막 .wav 위치
	char *pSfName = (char *)pszPath;
	PDS_SF pSf = NULL;
	DWORD PosNum = 0;

	if((pPosName != NULL) && (pDot != NULL)) {
		*pDot = 0;		// position 이름만 남기기위해 .wav 제거
		*pPosName = 0;	// pSfName 위해 PATH_DELIMITER 삭제
		pPosName++;
	}
	
	if((pPosName != NULL) && (pSfName != NULL))
	{
		DSTRACE(("[CDSWaveFile::Open()] SfName:%s, PosName:%s\n\r", pSfName, pPosName));

		if(_stricmp(DSWaveEmbeddedExir2K_Xmos_Game_WM.SfName, pSfName) == 0) {
			pSf = &DSWaveEmbeddedExir2K_Xmos_Game_WM;
			PosNum = DSWaveEmbeddedExir2K_Xmos_Game_WM.PosNum;
		// } else if(_stricmp(DSWaveEmbeddedExir2K_Xmos_Movie_WM.SfName, pSfName) == 0) {
		// 	pSf = &DSWaveEmbeddedExir2K_Xmos_Movie_WM;
		// 	PosNum = DSWaveEmbeddedExir2K_Xmos_Movie_WM.PosNum;
		// } else if(_stricmp(DSWaveEmbeddedExir2K_Xmos_Music_WM.SfName, pSfName) == 0) {
		// 	pSf = &DSWaveEmbeddedExir2K_Xmos_Music_WM;
		// 	PosNum = DSWaveEmbeddedExir2K_Xmos_Music_WM.PosNum;
		}
	}

	RetCode = ERR_FAILED_TO_OPEN_FILE;
	if(pSf != NULL){
		unsigned int i;
		for(i = 0; i < PosNum; i++){
			if(_stricmp((pSf->SfPos + i)->Name, pPosName) == 0) {
				DSTRACE(("[CDSWaveFile::Open()] found PosName:%s, i:%d\n\r", (pSf->SfPos + i)->Name, i));
				break;
			}
		}

		if(i < PosNum) {
			unsigned int uiNameLength = strlen(pPosName);

			m_pPosData = (pSf->SfPos + i)->Data;
			m_cKey = 0x00;
			for(i=0; i<uiNameLength; i++) {
				m_cKey ^= *pPosName++;
			}
			memcpy(&g_WaveHeader, m_pPosData, sizeof(DSWAV_HEADER));	// LFE Header
			char *ptr = (char *)&g_WaveHeader;
			DSTRACE(("before ptr[0]:0x%02x\n\r", ptr[0]));
			for (i = 0; i < sizeof(DSWAV_HEADER); i++) {
				ptr[i] ^= m_cKey; // XOR
			}
			DSTRACE(("before ptr[0]:0x%02x\n\r", ptr[0]));
			DSTRACE(("[CDSWaveFile::Open()] dwDataSize:%d, wBlockAlign:%d, dwTransSize:%d, m_cKey:0x%02x\n\r", g_WaveHeader.dwDataSize, g_WaveHeader.wBlockAlign, g_WaveHeader.dwDataSize / g_WaveHeader.wBlockAlign, m_cKey));

			RetCode = NO_ERR;
		}
	}

	DSTRACE(("[CDSWaveFile::Open(%s), %d] Leaving...\n\r", pszPath, RetCode));
	return RetCode;
}

/* ------------------------------------------------------------------------
    기    능: 프레임 샘플들을 읽는다.
	매개변수: pContext	-> 읽은 프레임 샘플 처리문 콘텍스트 포인터
	되돌림값: > -1	-> 읽은 프레임 개수
			  < 0	-> 프레임 샘플 읽기 실패, 오류값
    비    고:
  ------------------------------------------------------------------------ */
int CDSWaveFileReadFrameSamples(PDSCRUCONTEXT pDSCRUContext)
{
	WORD wBlockAlign = g_WaveHeader.wBlockAlign;
	WORD wBitsPerSample = g_WaveHeader.wBitsPerSample;
	int Cnt = 0;
	CONVOL_T fInvMaxVal;

	if(wBlockAlign > sizeof(m_WaveDataReadBuf)) {		// 최대 4채널, 64비트까지 지원.
		DSTRACE(("[Error - CDSWaveFile::ReadFrameSamples()] Invalid block align(%u > 64)!! Exiting...\n\r", wBlockAlign));
		return ERR_INVALID_PARAMS;
	}

	switch( wBitsPerSample ) {
	case 8:		fInvMaxVal = (CONVOL_T)1.0 / DS_MAX_INT8;		break;
	case 16:	fInvMaxVal = (CONVOL_T)1.0 / DS_MAX_INT16;		break;
	case 32:	fInvMaxVal = (CONVOL_T)1.0 / DS_MAX_INT32;		break;
	default:
		DSTRACE(("[Error - CDSWaveFile::ReadFrameSamples()] Unsupported Bits Per Sample(%u)!! Exiting....\n\r", wBitsPerSample));
		return ERR_UNSUPPORTED;
	}

	while( true ) {
		size_t dwRead = 0;
		DWORD dwOffset = Cnt * wBlockAlign;
		if(dwOffset < g_WaveHeader.dwDataSize) {
			const unsigned char *pPosData = m_pPosData + sizeof(DSWAV_HEADER) + dwOffset;
			PBYTE pData = m_WaveDataReadBuf;
			PBYTE pEndData = pData + wBlockAlign;

			while(pData < pEndData) {
				*pData = *pPosData ^ m_cKey;
				pData++;
				pPosData++;
			}
			dwRead = wBlockAlign;
		}

		if (dwRead != wBlockAlign) {
			DSTRACE(("[Error - CDSWaveFile::ReadFrameSamples()] Different dwRead(%u), block align(%u)!! Exiting....\n\r", dwRead, wBlockAlign));
			break;
		}

		// 값 범위: -1.0 ~ 1.0
		// 24비트는 지원하지 않는다.
		switch( wBitsPerSample ) {
		case 8: {
			PBYTE pOffset = m_WaveDataReadBuf;
			*pDSCRUContext->pL++ = FIXED_CONV(*pOffset * fInvMaxVal);
			pOffset++;
			*pDSCRUContext->pR++ = FIXED_CONV(*pOffset * fInvMaxVal);
			} break;
		case 16: {
			PINT16 pOffset = (PINT16)(m_WaveDataReadBuf);
			*pDSCRUContext->pL++ = FIXED_CONV(*pOffset * fInvMaxVal);
			pOffset++;
			*pDSCRUContext->pR++ = FIXED_CONV(*pOffset * fInvMaxVal);
			} break;
		case 32: {
			PINT32 pOffset = (PINT32)(m_WaveDataReadBuf);
			*pDSCRUContext->pL++ = FIXED_CONV(*pOffset * fInvMaxVal);
			pOffset++;
			*pDSCRUContext->pR++ = FIXED_CONV(*pOffset * fInvMaxVal);
			} break;
		}

		Cnt++;
		if(Cnt >= pDSCRUContext->dwSampleNum) {
			break;
		}
	}
	DSTRACE(("[CDSWaveFile::ReadFrameSamples()] ReadSample: %u, IR Buffer SampleNum: %u, Exiting...\n\r", Cnt, pDSCRUContext->dwSampleNum));

	return Cnt;
}
