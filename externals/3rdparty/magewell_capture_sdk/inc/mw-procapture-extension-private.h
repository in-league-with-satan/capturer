////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "mw-procapture-extension.h"
#include "mw-linux.h"

#pragma pack(push)
#pragma pack(1)

////////////////////////////////////////////////////////////////////////////////
// Magewell Capture Private Extensions
enum {
    // HDCP
    GET_HDCP_SUPPORT_NUM = 1,
    SET_HDCP_SUPPORT_NUM,

    GET_TOP_FIELD_FIRST_NUM,
    SET_TOP_FIELD_FIRST_NUM,
    GET_ENABLE_HD_SYNC_METER_NUM,
    SET_ENABLE_HD_SYNC_METER_NUM,
    GET_FRAME_DURATION_CHANGE_THRESHOLD_NUM,
    SET_FRAME_DURATION_CHANGE_THRESHOLD_NUM,
    GET_VIDEO_SYNC_SLICE_LEVEL_NUM,
    SET_VIDEO_SYNC_SLICE_LEVEL_NUM,
    GET_INPUT_SOUCE_SCAN_PERIOD_NUM,
    SET_INPUT_SOUCE_SCAN_PERIOD_NUM,
    GET_INPUT_SOUCE_SCAN_KEEP_DURATION_NUM,
    SET_INPUT_SOUCE_SCAN_KEEP_DURATION_NUM,
    GET_SCAN_INPUT_SOURCES_COUNT_NUM,
    GET_SCAN_INPUT_SOURCES_ARRAY_NUM,
    SET_SCAN_INPUT_SOURCES_ARRAY_NUM,
    GET_VAD_THRESHOLD_NUM,
    SET_VAD_THRESHOLD_NUM,
    GET_LOW_LATENCY_STRIPE_HEIGHT_NUM,
    SET_LOW_LATENCY_STRIPE_HEIGHT_NUM,

    // Diagnoise
    GET_I2C_REGISTER_NUM,
    SET_I2C_REGISTER_NUM,
    GET_GSPI_REGISTER_NUM,
    SET_GSPI_REGISTER_NUM
};

typedef struct _MWCAP_DEVICE_I2C_S {
    int                                         iChannel;
    BYTE                                        byDevAddr;
    BYTE                                        byRegAddr;
    int                                         iSize;
    BYTE                                        *pData;
} MWCAP_DEVICE_I2C_S;

typedef struct _MWCAP_DEVICE_GSPI_S {
    DWORD                                       dwChipSelects;
    WORD                                        wRegAddr;
    WORD                                        wData;
} MWCAP_DEVICE_GSPI_S;

// HDCP
#define MWCAP_IOCTL_GET_HDCP_SUPPORT                        _IOR ('P', GET_HDCP_SUPPORT_NUM, bool)
#define MWCAP_IOCTL_SET_HDCP_SUPPORT                        _IOW ('P', SET_HDCP_SUPPORT_NUM, bool)

#define MWCAP_IOCTL_GET_TOP_FIELD_FIRST                     _IOR ('P', GET_TOP_FIELD_FIRST_NUM, bool)
#define MWCAP_IOCTL_SET_TOP_FIELD_FIRST                     _IOW ('P', SET_TOP_FIELD_FIRST_NUM, bool)
#define MWCAP_IOCTL_GET_ENABLE_HD_SYNC_METER                _IOR ('P', GET_ENABLE_HD_SYNC_METER_NUM, bool)
#define MWCAP_IOCTL_SET_ENABLE_HD_SYNC_METER                _IOR ('P', SET_ENABLE_HD_SYNC_METER_NUM, bool)
#define MWCAP_IOCTL_GET_FRAME_DURATION_CHANGE_THRESHOLD     _IOR ('P', GET_FRAME_DURATION_CHANGE_THRESHOLD_NUM, DWORD)
#define MWCAP_IOCTL_SET_FRAME_DURATION_CHANGE_THRESHOLD     _IOW ('P', SET_FRAME_DURATION_CHANGE_THRESHOLD_NUM, DWORD)
#define MWCAP_IOCTL_GET_VIDEO_SYNC_SLICE_LEVEL              _IOR ('P', GET_VIDEO_SYNC_SLICE_LEVEL_NUM, DWORD)
#define MWCAP_IOCTL_SET_VIDEO_SYNC_SLICE_LEVEL              _IOW ('P', SET_VIDEO_SYNC_SLICE_LEVEL_NUM, DWORD)
#define MWCAP_IOCTL_GET_INPUT_SOUCE_SCAN_PERIOD             _IOR ('P', GET_INPUT_SOUCE_SCAN_PERIOD_NUM, DWORD)
#define MWCAP_IOCTL_SET_INPUT_SOUCE_SCAN_PERIOD             _IOW ('P', SET_INPUT_SOUCE_SCAN_PERIOD_NUM, DWORD)
#define MWCAP_IOCTL_GET_INPUT_SOUCE_SCAN_KEEP_DURATION      _IOR ('P', GET_INPUT_SOUCE_SCAN_KEEP_DURATION_NUM, DWORD)
#define MWCAP_IOCTL_SET_INPUT_SOUCE_SCAN_KEEP_DURATION      _IOW ('P', SET_INPUT_SOUCE_SCAN_KEEP_DURATION_NUM, DWORD)
#define MWCAP_IOCTL_GET_SCAN_INPUT_SOURCES_COUNT            _IOR ('P', GET_SCAN_INPUT_SOURCES_COUNT_NUM, DWORD)
#define MWCAP_IOCTL_GET_SCAN_INPUT_SOURCES_ARRAY            _IOR ('P', GET_SCAN_INPUT_SOURCES_ARRAY_NUM, MWCAP_INPUT_SOURCE_ARRAY)
#define MWCAP_IOCTL_SET_SCAN_INPUT_SOURCES_ARRAY            _IOW ('P', SET_SCAN_INPUT_SOURCES_ARRAY_NUM, MWCAP_INPUT_SOURCE_ARRAY)
#define MWCAP_IOCTL_GET_VAD_THRESHOLD                       _IOR ('P', GET_VAD_THRESHOLD_NUM, DWORD)
#define MWCAP_IOCTL_SET_VAD_THRESHOLD                       _IOW ('P', SET_VAD_THRESHOLD_NUM, DWORD)
#define MWCAP_IOCTL_GET_LOW_LATENCY_STRIPE_HEIGHT           _IOR ('P', GET_LOW_LATENCY_STRIPE_HEIGHT_NUM, int)
#define MWCAP_IOCTL_SET_LOW_LATENCY_STRIPE_HEIGHT           _IOW ('P', SET_LOW_LATENCY_STRIPE_HEIGHT_NUM, int)

// Diagnoise
#define MWCAP_IOCTL_GET_I2C_REGISTER_ID     _IOWR('P', GET_I2C_REGISTER_NUM, MWCAP_DEVICE_I2C_S)
#define MWCAP_IOCTL_SET_I2C_REGISTER_ID     _IOW ('P', SET_I2C_REGISTER_NUM, MWCAP_DEVICE_I2C_S)
#define MWCAP_IOCTL_GET_GSPI_REGISTER_ID    _IOWR('P', GET_GSPI_REGISTER_NUM, MWCAP_DEVICE_GSPI_S)
#define MWCAP_IOCTL_SET_GSPI_REGISTER_ID    _IOW ('P', SET_GSPI_REGISTER_NUM, MWCAP_DEVICE_GSPI_S)

#pragma pack(pop)
