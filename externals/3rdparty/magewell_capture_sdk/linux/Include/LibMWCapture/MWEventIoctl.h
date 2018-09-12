////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing)
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////
#ifndef __MW_EVENT_IOCTL__
#define __MW_EVENT_IOCTL__

#include "WinTypes.h"

#pragma pack(push)
#pragma pack(1)

typedef struct _MW_EVENT_WAIT {
    MWCAP_PTR               pvEvent;
    int                     timeout; //ms
} MW_EVENT_WAIT;

typedef struct _MW_EVENT_WAIT_MULTI {
    MWCAP_PTR               pvEvents; /* (MWCAP_PTR * pvEvents) */
    int                     count;
    int                     timeout; //ms
} MW_EVENT_WAIT_MULTI;

/* kernel event */
#define MW_IOCTL_KEVENT_ALLOC                _IOR ('E', 1, MWCAP_PTR)
#define MW_IOCTL_KEVENT_FREE                 _IOW ('E', 2, MWCAP_PTR)
#define MW_IOCTL_KEVENT_SET                  _IOW ('E', 3, MWCAP_PTR)
#define MW_IOCTL_KEVENT_CLEAR                _IOW ('E', 4, MWCAP_PTR)
#define MW_IOCTL_KEVENT_IS_SET               _IOW ('E', 5, MWCAP_PTR)
#define MW_IOCTL_KEVENT_TRY_WAIT             _IOW ('E', 6, MWCAP_PTR)
#define MW_IOCTL_KEVENT_WAIT                 _IOW ('E', 7, MW_EVENT_WAIT)
#define MW_IOCTL_KEVENT_WAIT_MULTI           _IOW ('E', 8, MW_EVENT_WAIT_MULTI)

#pragma pack(pop)

#endif /* __MW_EVENT_IOCTL__ */
