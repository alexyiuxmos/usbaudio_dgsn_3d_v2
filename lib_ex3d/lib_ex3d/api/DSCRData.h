#ifndef __CDSCRDATA_H__
#define __CDSCRDATA_H__

#include "digisonic.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSCRDATA_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef struct _DSCRDATA_
{
	PPOINT_T m_pL;		// 왼쪽 데이터
	PPOINT_T m_pR;		// 오른쪽 데이터
	DWORD m_dwLength;

	DS_BOOL m_bAllocated;
}DSCRDATA, *PDSCRDATA;

EXTERN int CDSCRData(PDSCRDATA pData, DWORD dwLength);
EXTERN int CDSCRDataDestroyer(PDSCRDATA pData);

EXTERN void CDSCRDataInit(PDSCRDATA pData);

#define NEW_DSCRD_BUFFER(_pFunc_, _CRDBuf_, _Length_, _RetCode_) RetCode = CDSCRData(&_CRDBuf_, _Length_);	\
	if(RetCode != NO_ERR){	\
		DSTRACE(("[Error - %s, %d] '%s CDSCRData()' failed!! Exiting...\n", _pFunc_, RetCode, #_CRDBuf_));	\
		return _RetCode_;	}

#define NEW_PDSCRD_BUFFER(_pFunc_, _CRDBuf_, _Length_, _RetCode_) RetCode = CDSCRData(_CRDBuf_, _Length_);	\
	if(RetCode != NO_ERR){	\
		DSTRACE(("[Error - %s, %d] '%s CDSCRData()' failed!! Exiting...\n", _pFunc_, RetCode, #_CRDBuf_));	\
		return _RetCode_;	}

#endif	// __CDSCRDATA_H__
