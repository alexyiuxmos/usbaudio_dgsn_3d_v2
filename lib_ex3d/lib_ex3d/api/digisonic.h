#ifndef __DIGISONIC_H__
#define __DIGISONIC_H__

#include "DSBuild.h"

//#include <stdarg.h>
#include <unistd.h>
#include <wchar.h>
#include <string.h>

#include <stdlib.h>	
#include <stdint.h>	

#define DS_malloc(a)	malloc(a)
#define DS_free(a)		free(a)

#include "dserror.h"
#include "dsdatatype.h"
#include "DSPoint.h"

#include "DSWinAPI.h"
#include "c_dsp.h"
#define DEBUG_UNIT EX3D_DEBUG
#ifndef DEBUG_PRINT_ENABLE_EX3D_DEBUG
    #define DEBUG_PRINT_ENABLE_EX3D_DEBUG 1
#endif
#include "debug_print.h"

#undef TRACE
#define _DEBUG
#ifdef _DEBUG
	#define DBG 1
	#define DSTRACE(_x_) debug_printf _x_
	#ifndef TRACE
		#define TRACE	debug_printf
	#endif
	#define DSMEMLOG(_x_)  // debug_printf _x_
#else
	#define DBG 0
	#define DSTRACE(_x_)
	#define TRACE	
	#define DSMEMLOG(_x_)
#endif

#ifndef __T
#ifdef  _UNICODE
    #define __T(x)      L ## x
#else
    #define __T(x)      x
#endif
#endif

#ifndef _T
#define _T(x)       __T(x)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/* -----------------------------------------------------
 메모리 할당 포인터을 검사하는 매크로 함수.
_FuncName_: 함수명
_pMem_: 검사할 포인터 매개변수
_Ret_: 오류 발생시 되돌릴 값
----------------------------------------------------- */
#define	CHECK_MEM( _FuncName_, _pMem_, _Ret_ )	\
if( !_pMem_ ){	\
DS_Print("[Error - %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, #_pMem_);	\
return (_Ret_);	\
}

#define	DSCHECK_MEM( _FuncName_, _pMem_, _Ret_ )	\
if( !_pMem_ ){	\
DSTRACE(("[Error - %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, #_pMem_));	\
return (_Ret_);	\
}

#define	CHECK_MEM_VOID( _FuncName_, _pMem_)	\
if( !_pMem_ ){	\
	DS_Print("[Error - %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, #_pMem_);	\
	return;	\
}

#define	CHECK_2MEMS( _FuncName_, _pMem1_, _pMem2_, _Ret_ )	\
if( !_pMem1_ || !_pMem2_ ){	\
DS_Print("[Error - %s] Memory allocation failure!! %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_pMem1_, _pMem1_, #_pMem2_, _pMem2_);	\
return (_Ret_);	\
}

#define	DSCHECK_MEM( _FuncName_, _pMem_, _Ret_ )	\
if( !_pMem_ ){	\
DSTRACE(("[Error - %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, #_pMem_));	\
return (_Ret_);	\
}

#define	CHECK_MEM_2( _FuncName_, _pName_, _pMem_, _Ret_ )	\
if( !_pMem_ ){	\
DS_Print("[Error - %s, %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, _pName_, #_pMem_);	\
return (_Ret_);	\
}

#define	CHECK_MEM_3( _FuncName_, _pMem_, _Ret_, _pRet_, _ValueOfpRet_)	\
if( !_pMem_ ){	\
    DS_Print("[Error - %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, #_pMem_);	\
    if( _pRet_ ) *_pRet_ = _ValueOfpRet_;	\
    return (_Ret_); }

#define	CHECK_MEM_4( _FuncName_, _pMem_, _Ret_, _rfRet_, _ValueOfrfRet_)	\
if( !_pMem_ ){	\
    DS_Print("[Error - %s] The '%s' memory allocation failure!! Exiting...\n", _FuncName_, #_pMem_);	\
    _rfRet_ = _ValueOfrfRet_;	\
    return (_Ret_); }

/* -----------------------------------------------------
 포인터를 검사하는 매크로 함수
_FuncName_: 함수명
_Ptr_: 검사할 포인터 매개변수
_Ret_: 오류 발생시 되돌릴 값
----------------------------------------------------- */
#define	CHECK_PTR( _FuncName_, _Ptr_, _Ret_ )	\
if( !_Ptr_ ){	\
DS_Print("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_);	\
return (_Ret_);	\
}
#define	CHECK_PTR_2( _FuncName_, _pCaller_, _Ptr_, _Ret_ )	\
if( !_Ptr_ ){	\
if( _pCaller_ ) DS_Print("[Error - %s, %s] '%s' is NULL!! Exiting...\n", _FuncName_, _pCaller_, #_Ptr_);	\
else DS_Print("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_);	\
return (_Ret_);	\
}
#define	CHECK_PTR_3( _FuncName_, _Ptr_, _Ret_, _pRet_, _ValueOfpRet_)	\
if( !_Ptr_ ){	\
DS_Print("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_);	\
if( _pRet_ ) *_pRet_ = _ValueOfpRet_;	\
return (_Ret_);}

#define	DSCHECK_PTR( _FuncName_, _Ptr_, _Ret_ )	\
if( !_Ptr_ ){	\
DSTRACE(("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_));	\
return (_Ret_);	\
}
#define	DSCHECK_PTR_2( _FuncName_, _pCaller_, _Ptr_, _Ret_ )	\
if( !_Ptr_ ){	\
if( _pCaller_ ) DSTRACE(("[Error - %s, %s] '%s' is NULL!! Exiting...\n", _FuncName_, _pCaller_, #_Ptr_));	\
else DSTRACE(("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_));	\
return (_Ret_);	\
}
#define	DSCHECK_PTR_3( _FuncName_, _Ptr_, _Ret_, _pRet_, _ValueOfpRet_)	\
if( !_Ptr_ ){	\
DSTRACE(("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_));	\
if( _pRet_ ) *_pRet_ = _ValueOfpRet_;	\
return (_Ret_);}

#define	DSCHECK_PTR_VOID( _FuncName_, _Ptr_)	\
if( !_Ptr_ ){	\
DSTRACE(("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_));	\
return ;	\
}
#define	DSCHECK_PTR_2_VOID( _FuncName_, _pCaller_, _Ptr_, _Ret_ )	\
if( !_Ptr_ ){	\
if( _pCaller_ ) DSTRACE(("[Error - %s, %s] '%s' is NULL!! Exiting...\n", _FuncName_, _pCaller_, #_Ptr_));	\
else DSTRACE(("[Error - %s] '%s' is NULL!! Exiting...\n", _FuncName_, #_Ptr_));	\
return ;	\
}

/* -----------------------------------------------------
2개 포인터 전달변수를 검사하는 매크로 함수
_FuncName_: 함수명
_Ptr1_: 포인터1
_Ptr2_: 포인터2
_Ret_: 오류 발생시 되돌릴 값
----------------------------------------------------- */
#define	CHECK_2PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ ){	\
DS_Print("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_);	\
return (_Ret_);	\
}

#define	DSCHECK_2PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ ){	\
DSTRACE(("[Error - %s] Invalid pointer!! %s: 0x%8p, %s: 0x%8p, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_));	\
return (_Ret_);	\
}

#define	DSCHECK_2PTRS_VOID( _FuncName_, _Ptr1_, _Ptr2_)	\
if( !_Ptr1_ || !_Ptr2_ ){	\
DSTRACE(("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_));	\
return ;	\
}

#define	CHECK_2PTRS_3( _FuncName_, _Ptr1_, _Ptr2_, _Ret_, _pRet_, _ValueOfpRet_)	\
if( !_Ptr1_ || !_Ptr2_ ){	\
DS_Print("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_);	\
if( _pRet_ ) *_pRet_ = _ValueOfpRet_;	\
return (_Ret_);}

#define	DSCHECK_2PTRS_3( _FuncName_, _Ptr1_, _Ptr2_, _Ret_, _pRet_, _ValueOfpRet_)	\
if( !_Ptr1_ || !_Ptr2_ ){	\
DSTRACE(("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_));	\
if( _pRet_ ) *_pRet_ = _ValueOfpRet_;	\
return (_Ret_);}

/* -----------------------------------------------------
3개 포인터 전달변수를 검사하는 매크로 함수
_FuncName_: 함수명
_Ptr1_: 포인터1
_Ptr2_: 포인터2
_Ptr3_: 포인터3
_Ret_: 오류 발생시 되돌릴 값
----------------------------------------------------- */
#define	CHECK_3PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_){	\
DS_Print("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_ );	\
return (_Ret_);	\
}

#define	DSCHECK_3PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_){	\
DSTRACE(("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_ ));	\
return (_Ret_);	\
}

#define	DSCHECK_3PTRS_2( _FuncName_, _pName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_){	\
DSTRACE(("[Error - %s, %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, _pName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_ ));	\
return (_Ret_);	\
}

/* -----------------------------------------------------
4개 포인터 전달변수를 검사하는 매크로 함수
_FuncName_: 함수명
_Ptr1_: 포인터1
_Ptr2_: 포인터2
_Ptr3_: 포인터3
_Ptr4_: 포인터4
_Ret_: 오류 발생시 되돌릴 값
----------------------------------------------------- */
#define	CHECK_4PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ptr4_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_|| !_Ptr4_){	\
DS_Print("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_, #_Ptr4_, _Ptr4_);	\
return (_Ret_);	\
}

#define	DSCHECK_4PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ptr4_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_|| !_Ptr4_){	\
DSTRACE(("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_, #_Ptr4_, _Ptr4_));	\
return (_Ret_);	\
}

/* -----------------------------------------------------
5개 포인터 전달변수를 검사하는 매크로 함수
_FuncName_: 함수명
_Ptr1_: 포인터1
_Ptr2_: 포인터2
_Ptr3_: 포인터3
_Ptr4_: 포인터4
_Ptr5_: 포인터5
_Ret_: 오류 발생시 되돌릴 값
----------------------------------------------------- */
#define	CHECK_5PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ptr4_, _Ptr5_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_|| !_Ptr4_|| !_Ptr5_){	\
DS_Print("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_, #_Ptr4_, _Ptr4_, #_Ptr5_, _Ptr5_);	\
return (_Ret_);	\
}

#define	DSCHECK_5PTRS( _FuncName_, _Ptr1_, _Ptr2_, _Ptr3_, _Ptr4_, _Ptr5_, _Ret_ )	\
if( !_Ptr1_ || !_Ptr2_ || !_Ptr3_|| !_Ptr4_|| !_Ptr5_){	\
DSTRACE(("[Error - %s] Invalid pointer!! %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, %s: 0x%08X, Exiting...\n", _FuncName_, #_Ptr1_, _Ptr1_, #_Ptr2_, _Ptr2_, #_Ptr3_, _Ptr3_, #_Ptr4_, _Ptr4_, #_Ptr5_, _Ptr5_));	\
return (_Ret_);	\
}

#define NEW_DSBUFFER(_pFunc_, _pNewBuf_, _type_, _Length_, _RetCode_)	_pNewBuf_ = (_type_ *)malloc(sizeof(_type_) * _Length_);	\
	DSCHECK_MEM(_pFunc_, _pNewBuf_, _RetCode_)

#define DeleteDSArrayMem( _pMem_ ) if( _pMem_ ){	\
free(_pMem_);	\
_pMem_ = NULL; }

#define CloseDSHandle( _hHandle_ ) if( _hHandle_ ){	\
CloseHandle(_hHandle_);	\
_hHandle_ = NULL; }

#define DS_InRange(Min,Number,Max) ((Min <= Number) && (Number <= Max))
#define DS_InRange1(Min,Number,Max) ((Min < Number) && (Number <= Max))
#define DS_InRange2(Min,Number,Max) ((Min <= Number) && (Number < Max))
#define DS_InRange3(Min,Number,Max) ((Min < Number) && (Number < Max))

#define DS_InRangeAbs(Min,Number,Max) ((( Min <= Number ) && ( Number <= Max )) || (( Max <= Number ) && ( Number <= Min )))
#define DS_SetErrorCode( _pErrorCode_, _ErrorCode_ ) if( _pErrorCode_ ) *_pErrorCode_ = _ErrorCode_

#define DS_IsNumber(_ch_)	DS_InRange3('/', _ch_, ':')
#define DS_ABS(_V1_, _V2_) ( _V1_ >= _V2_ ? _V1_ - _V2_ : _V2_ - _V1_ )

#define DS_IsLeapYear(_Year_) (!((_Year_)%4) && (((_Year_)%100) || !((_Year_)%400)))

#define DS_MIN(_a_, _b_, _c_) (_a_) > (_b_) ? ((_b_) > (_c_) ? (_c_) : (_b_)) : ((_a_) > (_c_) ? (_c_) : (_a_))
#define DS_MAX(_a_, _b_, _c_) (_a_) > (_b_) ? ((_a_) > (_c_) ? (_a_) : (_c_)) : ((_b_) > (_c_) ? (_b_) : (_c_))

#define DS_GETMAX(_a_, _b_) ((_a_) < (_b_) ? (_b_) : (_a_))

#define DS_ElapsedTimeDW(_dwBTime_, _dwTime_)	_dwTime_ = GetTickCount();	\
if( _dwTime_ >= _dwBTime_ ) _dwTime_ -= _dwBTime_;	\
else _dwTime_ = ((DWORD)-1 - _dwBTime_) + _dwTime_;

#define DS_ElapsedTimeQW(_qwBTime_, _qwTime_)	_qwTime_ = DS_GetTickTime();	\
if(_qwTime_ >= _qwBTime_) _qwTime_ -= _qwBTime_;	\
else _qwTime_ = ((QWORD)-1 - _qwBTime_) + _qwTime_;

#define DS_2PI		6.283185307179586476925286766559
#define DS_INVSQRT2	0.70710678118654752440084436210485

EXTERN_C_BEGIN
extern void* DSWinAPI_memcpy(void* pDest, const void* pSrc, size_t Dsize);
EXTERN_C_END

/////////////////////////////////////////////////////////////////
EXTERN_C_BEGIN
////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct _DSCONTEXT_
{
	PVOID pContext;
	PVOID pData;
	PCHAR pString;

	DWORD dwCmd;
	DWORD dwSize;
	DWORD dwValue;
	WORD  wValue;
	BYTE  Value;
	int   RetCode;
	QWORD qwValue;
	double fValue;
}DSCONTEXT, *PDSCONTEXT;

typedef struct _DSBUFFER_
{
	DWORD dwSize;			// DSBUFFER 구조체 크기
	DWORD dwLength;			// 길이
	DWORD dwValue;
	DWORD dwValue2;
	QWORD qwValue;
	QWORD qwValue2;
	PVOID pData;
	PCHAR pString;
}DSBUFFER, *PDSBUFFER;

typedef struct _DSBAG_
{
	PVOID pThis;
	PVOID pMem;
}DSBAG, *PDSBAG;

typedef struct _DSITEM32_
{
	char Buffer[32];
	DWORD dwValue;
}DSITEM32, *PDSITEM32;

typedef struct _DSITEM64_
{
	char Buffer[64];
	DWORD dwValue;
}DSITEM64, *PDSITEM64;

typedef struct _DSITEM128_
{
	char Buffer[128];
	DWORD dwValue;
}DSITEM128, *PDSITEM128;

typedef struct _DSSZITEM128S_
{
	PDSITEM128 pItem;
	DWORD dwLength;
	DWORD dwSize;
}DSSZITEM128S, *PDSSZITEM128S;

typedef struct _DSSZITEM_
{
	const char * pBuffer;
	DWORD dwSize;
}DSSZITEM, *PDSSZITEM;

typedef struct _DSITEM_
{
	PVOID pBuffer;
	DWORD dwSize;
}DSITEM, *PDSITEM;

typedef struct _DSITEMEx_
{
	PVOID pBuffer;
	DWORD dwSize;
	DWORD dwLength;
}DSITEMEx, *PDSITEMEx;

typedef struct _DSITEMQW_
{
	PVOID pBuffer;
	QWORD qwSize;
}DSITEMQW, *PDSITEMQW;

typedef struct _DSITEMQWEx_
{
	PVOID pBuffer;
	QWORD qwSize;
	QWORD qwLength;
}DSITEMQWEx, *PDSITEMQWEx;

typedef struct _DSDUBLE_
{
	double *pdbBuffer;
	DWORD dwLength;
}DSDUBLE, *PDSDUBLE;

typedef struct _DSQBUF_
{
	PBYTE pBuffer;
	DWORD dwSize;		// 버퍼 총 크기
	DWORD dwUsed;		// 사용된 버퍼 크기
}DSQBUF, *PDSQBUF;

typedef struct _DS_SF_POS_
{
  const char * Name;
  const unsigned char * Data;
}DS_SF_POS, *PDS_SF_POS;

typedef struct _DS_SF_
{
  const char * SfName;
  unsigned int PosNum;
  DS_SF_POS * SfPos;
}DS_SF, *PDS_SF;


////////////////////////////////////////////////////////////////////////////////////////////////////

typedef	int (*PFDSStreamProc)(PVOID pContext, PVOID pData, DWORD dwLength, PDWORD pdwProcessedLength);
typedef	int (*PFDSCommandProc)(DWORD dwCmd, PVOID pContext, PVOID pData, DWORD dwValue);

////////////////////////////////////////////////////////////////////////////////////////////////////
#define DS_Print	debug_printf

QWORD DS_GetTickTime(void);
LONG DS_CopyString(PCHAR pDest, DWORD dwLength, const char* pSrc);

// #define MEASURE_INTER_ELAPSED_TIME
// #define MEASURE_INTER_ELAPSED_TIME_CONV
#if defined(MEASURE_INTER_ELAPSED_TIME)
typedef enum {
	ELAPSED_TIME_LOG_STR_AUDIOPROCESS_START = 0,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_ENQ_INQ_END,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_DEQ_INQ_END,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_NORMALIZE_END,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_UPMIX_END,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_SOUNDPANNING_END,

	ELAPSED_TIME_LOG_STR_CONV_BUF_SET,
	ELAPSED_TIME_LOG_STR_CONV_RESULT_CHK,
	ELAPSED_TIME_LOG_STR_CONV_SCALE,

	ELAPSED_TIME_LOG_STR_CONV_BUF_READ_START,
	ELAPSED_TIME_LOG_STR_CONV_BUF_READ_END,
	ELAPSED_TIME_LOG_STR_CONV_BUF_NEXT_SET_START,
	ELAPSED_TIME_LOG_STR_CONV_BUF_NEXT_SET_END,
	ELAPSED_TIME_LOG_STR_CONV_RESULT_SET_START,
	ELAPSED_TIME_LOG_STR_CONV_RESULT_SET_END,

	ELAPSED_TIME_LOG_STR_CONVOLUTION_UNIT_END,
	ELAPSED_TIME_LOG_STR_CONVOLUTION_LFE_UNIT_END,
	ELAPSED_TIME_LOG_STR_CONVOLUTION_LIMITER_END,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_DENORMALIZE_FADEPROCESS_END,
	ELAPSED_TIME_LOG_STR_ONPROCESSING_ENQ_OUTQ_END,
	ELAPSED_TIME_LOG_STR_AUDIOPROCESS_DEQ_OUTQ_END,
	ELAPSED_TIME_LOG_STR_NUM
}ELAPSED_TIME_LOG_STR;

typedef struct {
	DWORD strIdx;
	DWORD tick;
} ELAPSED_TIME_STAMP;

void DS_ElapsedTimeStamp(DWORD strIdx);
void DS_ElapsedTimeStampLog();

#define DS_ELAPSEDTIMESTAMP(strIdx)	DS_ElapsedTimeStamp(strIdx)
#define DS_ELAPSEDTIMESTAMPLOG(dwTileNum)	DS_ElapsedTimeStampLog(dwTileNum)
#else	// #if defined(MEASURE_INTER_ELAPSED_TIME)
#define DS_ELAPSEDTIMESTAMP(strIdx)
#define DS_ELAPSEDTIMESTAMPLOG(dwTileNum)
#endif	// #if defined(MEASURE_INTER_ELAPSED_TIME)

/////////////////////////////////////////////////////////////////
EXTERN_C_END
////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //__DIGISONIC_H__
