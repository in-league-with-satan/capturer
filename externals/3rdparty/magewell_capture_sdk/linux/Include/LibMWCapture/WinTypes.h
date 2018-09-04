#pragma once

#if defined(__linux__)
#include <linux/types.h>
#endif

#pragma pack(push)
#pragma pack(1)

#ifndef VOID
#define VOID void
#endif

#ifndef BYTE
#define BYTE unsigned char
#endif

#ifndef CHAR
#define CHAR char
#endif

#ifndef WORD
#define WORD unsigned short
#endif

#ifndef SHORT
#define SHORT short
#endif

#ifndef DWORD
#define DWORD unsigned int
#endif

#ifndef LONGLONG
#define LONGLONG long long
#endif

#ifndef ULONGLONG
#define ULONGLONG unsigned long long
#endif

#ifndef LONG
#define LONG long
#endif

#ifndef BOOL
#define BOOL CHAR
#endif
#ifndef BOOLEAN
#define BOOLEAN CHAR
#endif
#ifndef TRUE
#define TRUE true
#endif
#ifndef FALSE
#define FALSE false
#endif

typedef unsigned long long                         MWCAP_PTR;
typedef void* HANDLE;

typedef struct _RECT {
    int left;
    int top;
    int right;
    int bottom;
} RECT;

#pragma pack(pop)
