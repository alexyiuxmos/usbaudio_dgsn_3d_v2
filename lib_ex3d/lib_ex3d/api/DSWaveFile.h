#ifndef __DSWAVEFILE_H__
#define __DSWAVEFILE_H__

#include "digisonic.h"

#pragma pack(push, 4)
#ifndef DSCRUCONTEXT_DEFINED
#define DSCRUCONTEXT_DEFINED
typedef struct _DSCRUCONTEXT_{
	PPOINT_T pL;
	PPOINT_T pR;

	DWORD dwSampleNum;
}DSCRUCONTEXT, *PDSCRUCONTEXT;
#endif

typedef struct _DSWAV_HEADER_{
    BYTE Marks[4];								// RIFF string
    DWORD dwFileSize;							// 실제 파일 크기 - 8, overall size of file in bytes
    BYTE FileType[4];							// WAVE string
    BYTE FormatChunkMarker[4];					// fmt string with trailing null char
    DWORD dwLengthOfFormatData;					// length of the format data
    WORD wFormatType;							// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    WORD wNumberOfChannels;                     // no.of channels
    DWORD dwSampleRate;							// sampling rate (blocks per second)
    DWORD dwByteRate;							// SampleRate * NumChannels * BitsPerSample/8
    WORD wBlockAlign;							// NumChannels * BitsPerSample/8
    WORD wBitsPerSample;						// bits per sample, 8- 8bits, 16- 16 bits etc
    BYTE DataChunkHeader[4];					// DATA string or FLLR string
    DWORD dwDataSize;							// NumSamples * NumChannels * BitsPerSample/8 - size of the next chunk that will be read
}DSWAV_HEADER, *PDSWAV_HEADER;

typedef struct _DSWAV_CHUNK_{
    BYTE Tag[4];								// 태그
    DWORD dwLength;								// 태그 데이터 길이
}DSWAV_CHUNK, *PDSWAV_CHUNK;

// fmt 태그 데이터
typedef struct _DSWAV_FMT_{
    WORD wFormatType;							// format type. 1-PCM, 3- IEEE float, 6 - 8bit A law, 7 - 8bit mu law
    WORD wNumberOfChannels;                     // no.of channels
    DWORD dwSampleRate;							// sampling rate (blocks per second)
    DWORD dwByteRate;							// SampleRate * NumChannels * BitsPerSample/8
    WORD wBlockAlign;							// NumChannels * BitsPerSample/8
    WORD wBitsPerSample;						// bits per sample, 8- 8bits, 16- 16 bits etc
	WORD wExtraDSize;							// wExtraDSize 이후 추가 정보 크기
	BYTE ExtraData[22];							// 필요한 경우, WAVE_FORMAT_EXTENSIBLE 추가 데이터
}DSWAV_FMT, *PDSWAV_FMT;

// MS WAVE_FORMAT_EXTENSIBLE 추가 데이터 구조체
typedef struct _DSWAV_FMT_EXTRADATA_{
	WORD wValidBitsPerSample;
	DWORD dwChannelMask;
	BYTE SubFormat[16];
}DSWAV_FMT_EXTRADATA, *PDSWAV_FMT_EXTRADATA;

typedef struct _DSWAV_FMT_CHUNK_{
	DSWAV_CHUNK Chunk;
	DSWAV_FMT Data;
}DSWAV_FMT_CHUNK, *PDSWAV_FMT_CHUNK;

#pragma pack(pop)

typedef struct _DSWAV_FRAMESAMPLE_{
	PDSWAV_HEADER pH;
	PBYTE pData;
}DSWAV_FRAMESAMPLE, *PDSWAV_FRAMESAMPLE;

// 주요 청크 데이터들을 구하기 위해 사용하는 구조체
typedef struct _DSWAV_CHUNKS_{
	DSWAV_FMT_CHUNK FmtChunk;
	QWORD qwFmtTagFOffset;				// Fmt 태그 파일 위치
	QWORD qwDFOffset;					// 오디오 데이터 파일 위치, 오디오 데이터 태그 파일 위치 = qwDFOffset - 8
	DWORD dwDLength;					// 오디오 데이터 길이
}DSWAV_CHUNKS, *PDSWAV_CHUNKS;

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSWAVEFILE_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

EXTERN int CDSWaveFile(PCSTR pszPath, DS_BOOL bCloseAfterGetHeader);
EXTERN void CDSWaveFileDestroyer();
EXTERN int CDSWaveFileReadFrameSamples(PDSCRUCONTEXT pDSCRUContext);

EXTERN DSWAV_HEADER g_WaveHeader;

#endif	//__DSWAVEFILE_H__
