#ifndef __basedef_h__
#define __basedef_h__

//#define IN
//#define OUT
//#define INOUT

#define SA_BOOL	int
#define SA_TRUE	1
#define SA_FALSE	0


#if defined( WIN32 )

#define SA_BOOL   BOOL
#define SA_TRUE   TRUE
#define SA_FALSE  FALSE

typedef char                    int8_t;
typedef unsigned char           uint8_t;
typedef short                   int16_t;
typedef unsigned short          uint16_t;
typedef int                     int32_t;
typedef unsigned int            uint32_t;

typedef __int64                 int64_t;
typedef unsigned __int64        uint64_t;

#define __BEGIN_ALIGNMENT__(x) __pragma(pack(push, x))
#define __END_ALIGNMENT__(x) __pragma(pack(pop))

#define __BEGIN_PACKED__ __pragma(pack(push, 1))
#define __END_PACKED__ __pragma(pack(pop))
#define __PACKED__

#define __STDCALL  __stdcall
#define INLINE	__inline
#define __LIKELY(x)	x
#define __UNLIKELY(x)	x

#define __DEPRECATED__ __declspec(deprecated)

#elif defined(__LINUX__) || defined(__ANDROID__) || defined(__LITEOS__) || defined(__CYGWIN__) || defined(__MAC_OS__) || defined(__HI3861__) || defined(__ALI_OS__)

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

#if 0
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;
#endif

#include <stdint.h>

//typedef long LONG;

typedef const char * LPCSTR;
typedef char * LPSTR;

#define __BEGIN_ALIGNMENT__(x)
#define __END_ALIGNMENT__(x)

#define __BEGIN_PACKED__
#define __END_PACKED__
#define __PACKED__ __attribute__((__packed__))

#define __STDCALL
#define INLINE inline
#define __LIKELY(x) __builtin_expect(!!(x), 1)
#define __UNLIKELY(x) __builtin_expect(!!(x), 0)

#if defined(__GNUC__) && (__GNUC__ >= 4 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 1))
#define __DEPRECATED__ __declspec(deprecated)
#else
#define __DEPRECATED__
#endif

#elif defined(__UCOS2__) || defined(__GP_CV4247__) // || defined(__FREERTOS__)

typedef char                    int8_t;
typedef unsigned char           uint8_t;
typedef short                   int16_t;
typedef unsigned short          uint16_t;
typedef int                     int32_t;
typedef unsigned int            uint32_t;

typedef __int64                 int64_t;
typedef unsigned __int64        uint64_t;

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;


#define __BEGIN_ALIGNMENT(x)__
#define __END_ALIGNMENT(x)__

#define __BEGIN_PACKED__
#define __END_PACKED__
#define __PACKED__ __attribute__((__packed__))

#define __STDCALL
#define INLINE	__inline
#define __LIKELY(x)	x
#define __UNLIKELY(x)	x

#define SA_BOOL	int
#define SA_TRUE	1
#define SA_FALSE	0

#define __DEPRECATED__

#elif defined(__JL_AC57__) || defined(__FREERTOS__) || defined(__JL_AC79__) || defined(__TXW81X__) || \
        defined(__BK_7258__) || defined(__BK_7252__)

typedef unsigned char BYTE;
typedef unsigned short WORD;
typedef unsigned long DWORD;

typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef unsigned int UINT;
typedef unsigned long ULONG;

#include <stdint.h>

#define __BEGIN_ALIGNMENT(x)__
#define __END_ALIGNMENT(x)__

#define __BEGIN_PACKED__
#define __END_PACKED__
#define __PACKED__ __attribute__((__packed__))

#define __STDCALL
#define INLINE	__inline

#define __DEPRECATED__

#else

#error "A platform must be specified !"
#endif

#endif
