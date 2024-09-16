#ifndef __DSWINAPI_H__
#define __DSWINAPI_H__

#include "digisonic.h"

#if defined(EXTERN)
#undef EXTERN
#endif
#if defined(_DSWINAPI_C_)
#define EXTERN
#else
#define EXTERN  extern
#endif

#include <xcore/hwtimer.h>
#include <timer.h>

EXTERN hwtimer_t SysTimer;

EXTERN void Sleep(DWORD dwMilliseconds);

inline int _wtoi(const wchar_t *str)
{
	return (int)wcstol(str, 0, 10);
}

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

#define MoveMemory RtlMoveMemory
#define CopyMemory DSWinAPI_memcpy
#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory

#define _stricmp(a, b) strncmp(a, b, strlen(b))
#define _strnicmp strncmp

#define _snprintf snprintf
#define _snwprintf swprintf

#define _open(_hFile_, _Openflag_, _PermissionMode_)    open(_hFile_, _Openflag_, _PermissionMode_)
#define _close(_hFile_)         close(_hFile_)
#define _fileno                 fileno
#define _filelengthi64(_hFile_) lseek64(_hFile_, 0, SEEK_END)
#define _lseeki64               lseek64
#define _telli64(_hFile_)       lseek64(_hFile_, 0, SEEK_CUR)
#define _commit(_hFile_)        fsync(_hFile_)
#define _read(_hFile_, _pBuffer_, _Size_)        read(_hFile_, _pBuffer_, _Size_)
#define _write(_hFile_, _pBuffer_, _Size_)       write(_hFile_, _pBuffer_, _Size_)
#define _chsize_s                                ftruncate64

///////////
#define _A_NORMAL       0x00    /* Normal file - No read/write restrictions */
#define _A_RDONLY       0x01    /* Read only file */
#define _A_HIDDEN       0x02    /* Hidden file */
#define _A_SYSTEM       0x04    /* System file */
#define _A_SUBDIR       0x10    /* Subdirectory */
#define _A_ARCH         0x20    /* Archive file */

#define _O_RDONLY       O_RDONLY
#define _O_WRONLY       O_WRONLY
#define _O_RDWR         O_RDWR
#define _O_APPEND       O_APPEND

#define _O_CREAT        O_CREAT
#define _O_TRUNC        O_TRUNC
#define _O_EXCL         O_EXCL

#define _O_BINARY		0
#define _O_RANDOM       0
#define _O_TEXT         0
#define _O_SEQUENTIAL   0
#define _O_SHORT_LIVED  0
#define _O_TEMPORARY    0
#define _O_WTEXT        0
#define _O_U8TEXT       0
#define _O_U16TEXT      0


#define _S_IREAD        S_IRUSR
#define _S_IWRITE       S_IWUSR

//////////////////////////////////////////////
#define STATUS_WAIT_0                    ((DWORD   )0x00000000L)
#define STATUS_ABANDONED_WAIT_0          ((DWORD   )0x00000080L)

#define WAIT_FAILED         ((DWORD)0xFFFFFFFF)
#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )

#define WAIT_ABANDONED      ((STATUS_ABANDONED_WAIT_0 ) + 0 )
#define WAIT_TIMEOUT        258L    // dderror

//////////////////////////////////////////////////////
typedef int SOCKET;
typedef struct in_addr IN_ADDR, *PIN_ADDR;
typedef struct sockaddr_in SOCKADDR_IN, *PSOCKADDR_IN;
typedef struct sockaddr SOCKADDR, *PSOCKADDR;
typedef struct timeval TIMEVAL;

#ifndef IPPROTO_PGM
    #define IPPROTO_PGM     113
#endif

#define ERROR_INVALID_HANDLE             6L
#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1
#define WSAGetLastError()   errno
#define closesocket         close

#define WSAENOTSOCK                      ENOTSOCK
#define WSA_INVALID_HANDLE              (ERROR_INVALID_HANDLE)
#define WSAETIMEDOUT                     ETIMEDOUT
#define WSAENOTCONN                      ENOTCONN
#define WSAECONNABORTED                  ECONNABORTED
#define WSAEWOULDBLOCK                   EWOULDBLOCK
#define WSAECONNRESET                    ECONNRESET
#define WSAESHUTDOWN                     ESHUTDOWN
#define WSAEISCONN                       EISCONN
#define WSAEADDRINUSE                    EADDRINUSE

#define SD_RECEIVE      SHUT_RD
#define SD_SEND         SHUT_WR
#define SD_BOTH         SHUT_RDWR

///////////////////////////////////////
EXTERN_C_BEGIN
/////////////////////////////////////////////////////////////////

EXTERN DWORD GetTickCount();

#define _atoi64 atoll
#define _strtoi64	strtoll
#define _strtoui64	strtoull

/////////////////////////////////////////////////////////////////////
EXTERN_C_END
#endif //__DSWINAPI_H__
