#ifndef __DSDATATYPE_H__
#define __DSDATATYPE_H__

#include <inttypes.h>
#if !defined(__GCC_PI32V2__)
typedef int64_t __int64;
#endif

#ifndef LONGLONG
	#if __LP64__
		typedef long LONGLONG;
	#else
		typedef long long LONGLONG;
	#endif
#endif

#ifndef ULONGLONG
	#if __LP64__
		typedef unsigned long ULONGLONG;
	#else
		typedef unsigned long long ULONGLONG;
	#endif
#endif

typedef LONGLONG *PLONGLONG;
typedef ULONGLONG *PULONGLONG;

typedef union _LARGE_INTEGER{
	struct {
		uint32_t LowPart;
		int32_t HighPart;
	};
	struct {
		uint32_t LowPart;
		int32_t HighPart;
	} u;
	LONGLONG QuadPart;
}LARGE_INTEGER;
typedef LARGE_INTEGER *PLARGE_INTEGER;

typedef union _ULARGE_INTEGER{
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	};
	struct {
		uint32_t LowPart;
		uint32_t HighPart;
	} u;
	ULONGLONG QuadPart;
}ULARGE_INTEGER;
typedef ULARGE_INTEGER *PULARGE_INTEGER;

typedef char CHAR;
typedef char* PCHAR;

typedef uint8_t BYTE;
typedef uint8_t* PBYTE;

typedef int16_t INT16;
typedef int16_t* PINT16;

typedef uint16_t WORD;
typedef uint16_t* PWORD;

typedef int32_t INT32;
typedef int32_t* PINT32;

typedef uint32_t DWORD;
typedef uint32_t* PDWORD;

typedef int64_t INT64;
typedef int64_t* PINT64;

typedef uint64_t QWORD;
typedef uint64_t* PQWORD;

#ifndef	FLOAT
	typedef float	FLOAT;
#endif

#ifndef	PFLOAT
	typedef FLOAT*	PFLOAT;
#endif

#ifndef	LONG
#if __LP64__
	typedef int		LONG;
#else
	typedef long	LONG;
#endif
#endif

#ifndef	PLONG
	typedef LONG *PLONG;
#endif

#ifndef	ULONG
	#define	ULONG	DWORD
#endif

#ifndef	HANDLE
	typedef void *HANDLE;
#endif

#ifndef	PVOID
	typedef void *PVOID;
#endif

#ifndef	BOOLEAN
	typedef BYTE BOOLEAN;
#endif

#ifndef	DS_BOOL
	typedef int DS_BOOL;
#endif

#ifndef	WCHAR
	typedef unsigned short WCHAR;
#endif

#ifndef	PWCHAR
	typedef unsigned short *PWCHAR;
#endif

#ifndef	PUCHAR
	typedef unsigned char *PUCHAR;
#endif

#ifndef	UCHAR
	typedef unsigned char UCHAR;
#endif

#ifndef	UINT
	typedef unsigned int UINT;
#endif


#define __in_opt
#define _In_
#define VOID 					void

typedef char *NPSTR, *LPSTR, *PSTR;
typedef const char *LPCSTR, *PCSTR;
typedef const unsigned short *LPCWSTR, *PCWSTR;
typedef unsigned short *NWPSTR, *LPWSTR, *PWSTR;

typedef DS_BOOL *PBOOL, *LPBOOL;
typedef void *LPVOID;

#define EXTRACT_WORDS(hi,lo,d)                                  \
do {                                                            \
union dshape __u;                                             \
__u.value = (d);                                              \
(hi) = __u.bits >> 32;                                        \
(lo) = (uint32_t)__u.bits;                                    \
} while (0)

/* Get the more significant 32 bit int from a double.  */
#define GET_HIGH_WORD(i,d)                                      \
do {                                                            \
union dshape __u;                                             \
__u.value = (d);                                              \
(i) = __u.bits >> 32;                                         \
} while (0)

#define SET_HIGH_WORD(d,hi)                                     \
do {                                                            \
union dshape __u;                                             \
__u.value = (d);                                              \
__u.bits &= 0xffffffff;                                       \
__u.bits |= (uint64_t)(hi) << 32;                             \
(d) = __u.value;                                              \
} while (0)

typedef struct _SECURITY_ATTRIBUTES {
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	DS_BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _SYSTEMTIME {
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef float CONVOL_T;
typedef float * PCONVOL_T;

typedef enum {
	AUDIO_DATA_TYPE_INT16 = 0,
	AUDIO_DATA_TYPE_INT32,
	AUDIO_DATA_TYPE_FLOAT
} AUDIO_DATA_TYPE;

#ifndef EXTERN_C_BEGIN
#ifdef __cplusplus
#define EXTERN_C_BEGIN extern "C" {
#define EXTERN_C_END }
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#endif
#endif

#ifndef GUID_DEFINED
#define GUID_DEFINED
typedef struct _GUID_{
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[ 8 ];
}GUID;
#endif

/////////////////////////////////////////////////////////////////////
#ifndef	TRUE
#define	TRUE	1
#endif

#ifndef	true
#define	true	1
#endif

#ifndef	FALSE
#define	FALSE	0
#endif

#ifndef	false
#define	false	0
#endif

#ifndef	KB
#define	KB	1024
#endif
#ifndef	MB
#define	MB	(KB*KB)
#endif
#ifndef	GB
#define	GB	((QWORD)MB*KB)
#endif
#ifndef	TB
#define	TB	(GB*KB)
#endif

#ifndef	KHZ
#define	KHZ	1000
#endif
#ifndef	MHZ
#define	MHZ	1000000
#endif
#ifndef	GHZ
#define	GHZ	1000000000
#endif
#ifndef	THZ
#define	THZ	1000000000000
#endif

#define NANOS_PER_SECOND			1000000000L
#define NANOS_PER_MILLISECOND		1000000L

/////////////////////////////////////////////////////////////////////
//   비트 정의문
#define BIT0	0x01
#define BIT1	0x02
#define BIT2	0x04
#define BIT3	0x08
#define BIT4	0x10
#define BIT5	0x20
#define BIT6	0x40
#define BIT7	0x80 
#define BIT8	0x0100
#define BIT9	0x0200
#define BIT10	0x0400
#define BIT11	0x0800
#define BIT12	0x1000
#define BIT13	0x2000
#define BIT14	0x4000
#define BIT15	0x8000 
#define BIT16	0x00010000 
#define BIT17	0x00020000 
#define BIT18	0x00040000 
#define BIT19	0x00080000 
#define BIT20	0x00100000 
#define BIT21	0x00200000 
#define BIT22	0x00400000 
#define BIT23	0x00800000 
#define BIT24	0x01000000 
#define BIT25	0x02000000 
#define BIT26	0x04000000 
#define BIT27	0x08000000 
#define BIT28	0x10000000 
#define BIT29	0x20000000 
#define BIT30	0x40000000 
#define BIT31	0x80000000 
#define BIT63   (QWORD)0x8000000000000000

/////////////////////////////////////////////////////////////////////
typedef	int (*pFuncDSProc)(PVOID pContext, PVOID pData, DWORD dwValue);
#define PFDSProc __attribute__(( fptrgroup("ex3d") )) pFuncDSProc
/////////////////////////////////////////////////////////////////////
#endif //__DSDATATYPE_H__
