#define _DSCHARBUFFER_C_

#include "DSCharBuffer.h"

static int New(PCDSCHARBUFFER pBuf, DWORD dwLength);

int CDSCharBuffer(PCDSCHARBUFFER pBuf, DWORD dwLength)
{
	DSCHECK_PTR("CDSCharBuffer()", pBuf, ERR_PARAMS_NULL);

	pBuf->m_pszMem = NULL;
	pBuf->m_dwMemSize = 0;
	if( dwLength ) New( pBuf, dwLength );

	return NO_ERR;
}

int CDSCharBufferDestroyer(PCDSCHARBUFFER pBuf)
{
	DSCHECK_PTR("CDSCharBufferDestroyer()", pBuf, ERR_PARAMS_NULL);

	DS_free(pBuf->m_pszMem);
	pBuf->m_pszMem = NULL;
	pBuf->m_dwMemSize = 0;

	return NO_ERR;
}

static int New(PCDSCHARBUFFER pBuf, DWORD dwLength)
{
	DWORD dwMemSize = (sizeof(char) * dwLength) + sizeof(DWORD);

	// 메모리 크기가 같으면 메모리를 다시 할당하지 않고 초기화만 한다.
	if(pBuf->m_pszMem && pBuf->m_dwMemSize == dwMemSize){
		*(PDWORD)pBuf->m_pszMem = 0;
	}else{
		DS_free(pBuf->m_pszMem);
		pBuf->m_pszMem = (PCHAR)DS_malloc(dwMemSize);
		DSCHECK_MEM("CDSCharBuffer::New()", pBuf->m_pszMem, ERR_MEM_NULL);

		pBuf->m_dwMemSize = dwMemSize;
		*(PDWORD)pBuf->m_pszMem = 0;
	}

	return NO_ERR;
}

int CDSCharBufferSet(PCDSCHARBUFFER pBuf, PCHAR pszData)
{
	DSCHECK_PTR("CDSCharBufferSet()", pBuf, ERR_PARAMS_NULL);

	return DS_CopyString(pBuf->m_pszMem, pBuf->m_dwMemSize, pszData);
}

int CDSCharBufferEmpty(PCDSCHARBUFFER pBuf)
{
	DSCHECK_PTR("CDSCharBufferEmpty()", pBuf, ERR_PARAMS_NULL);

	if( pBuf->m_pszMem ) *(PDWORD)pBuf->m_pszMem = 0;

	return NO_ERR;
}

