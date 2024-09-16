#ifndef __DSCRUNIT_H__
#define __DSCRUNIT_H__

#include "digisonic.h"

#include "DSCRData.h"
#include "DSCharBuffer.h"

#define XMOS_FHT_FUNC

#define DEFAULT_IN_CR_SAMPLE_NUM	512

// Convolution 참고 사이트
// https://blog.robertelder.org/overlap-add-overlap-save/
// https://blog.robertelder.org/fast-multiplication-using-fourier-transform/

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSCRUNIT_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

typedef struct _IRFILTER_{
	DWORD m_dwSampleNum;

	PPOINT_T m_pLaddFilter;
	PPOINT_T m_pLsubFilter;
	PPOINT_T m_pRaddFilter;
	PPOINT_T m_pRsubFilter;
} IRFILTER, *PIRFILTER;

typedef struct _DSCRUNIT_{
	DS_BOOL m_bOpened;

#if DBG
	CDSCHARBUFFER m_szName;
	CDSCHARBUFFER m_szPath;
#endif

	PDWORD m_pdwBitRevT;
#if !defined(XMOS_FHT_FUNC)
	PPOINT_T m_pSinTable;
#endif

	DWORD m_dwCR_SampleNum;
	DWORD m_dwCR_DataNum;

	DWORD m_dwCR_SegNum;
	DWORD m_dwCR_InSegIdx;

	PPOINT_T m_pCR_InBuf;

	IRFILTER m_IR_Filter;
} DSCRUNIT, *PDSCRUNIT;

EXTERN void CDSCRUnit(PDSCRUNIT pUnit);
EXTERN void CDSCRUnitDestroyer(PDSCRUNIT pUnit);

#if DBG
EXTERN int CDSCRUnitSetName(PDSCRUNIT pUnit, PCHAR pszName);
#endif

EXTERN int CDSCRUnitOpen(PDSCRUNIT pUnit, PCSTR pszPath, PDSCRDATA pDscrBuf, DS_BOOL bAllocateCRUD);
EXTERN int CDSCRUnitClose(PDSCRUNIT pUnit);

EXTERN void CDSCRUnitProcess(PDSCRUNIT pUnit, PPOINT_T pData, PDSCRDATA pOutBuf, DWORD dwSampleNum);

#endif	// __DSCRUNIT_H__
