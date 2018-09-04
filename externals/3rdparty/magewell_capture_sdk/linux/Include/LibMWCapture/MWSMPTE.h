////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#pragma pack(push)
#pragma pack(1)

typedef enum _SDI_TYPE {
    SDI_TYPE_SD,
    SDI_TYPE_HD,
    SDI_TYPE_3GA,
    SDI_TYPE_3GB_DL,
    SDI_TYPE_3GB_DS,
    SDI_TYPE_DL_CH1,
    SDI_TYPE_DL_CH2,
    SDI_TYPE_6G_MODE1,
    SDI_TYPE_6G_MODE2
} SDI_TYPE;

typedef enum _SDI_SCANNING_FORMAT {
	SDI_SCANING_INTERLACED = 0,
	SDI_SCANING_SEGMENTED_FRAME = 1,
    SDI_SCANING_PROGRESSIVE = 3
} SDI_SCANNING_FORMAT;

typedef enum _ST352_STANDARD {
    ST352_STANDARD_483_576_270M_360M	= 0x1,
    ST352_STANDARD_720P_1_5G			= 0x4,
    ST352_STANDARD_1080_1_5G			= 0x5,
    ST352_STANDARD_1080_DL_1_5G			= 0x7,
    ST352_STANDARD_720P_3G				= 0x8,
    ST352_STANDARD_1080_3G				= 0x9,
    ST352_STANDARD_DL_3G				= 0xA,
    ST352_STANDARD_720P_DS_3G			= 0xB,
    ST352_STANDARD_1080_DS_3G			= 0xC,
    ST352_STANDARD_483_576_DS_3G		= 0xD,
    ST352_STANDARD_6G_MODE1				= 0x40,
    ST352_STANDARD_6G_MODE2				= 0x41
} ST352_STANDARD;

typedef enum _SDI_BIT_DEPTH {
	SDI_BIT_DEPTH_8BIT = 0,
	SDI_BIT_DEPTH_10BIT = 1,
	SDI_BIT_DEPTH_12BIT = 2
} SDI_BIT_DEPTH;

typedef enum _SDI_SAMPLING_STRUCT {
	SDI_SAMPLING_422_YCbCr = 0x00,
	SDI_SAMPLING_444_YCbCr = 0x01,
	SDI_SAMPLING_444_RGB = 0x02,
    SDI_SAMPLING_420_YCbCr = 0x03,
	SDI_SAMPLING_4224_YCbCrA = 0x04,
	SDI_SAMPLING_4444_YCbCrA = 0x05,
	SDI_SAMPLING_4444_RGBA = 0x06,
	SDI_SAMPLING_4224_YCbCrD = 0x08,
	SDI_SAMPLING_4444_YCbCrD = 0x09,
	SDI_SAMPLING_4444_RGBD = 0x0A,
	SDI_SAMPLING_444_XYZ = 0x0E
} SDI_SAMPLING_STRUCT;

typedef enum _SDI_DYNAMIC_RANGE {
    SDI_DYNAMIC_RANGE_100_PERCENT		= 0,
    SDI_DYNAMIC_RANGE_200_PERCENT		= 1,
    SDI_DYNAMIC_RANGE_400_PERCENT		= 2
} SDI_DYNAMIC_RANGE;

static const DWORD g_adwFrameDuration[] = {
    0,									// 0, Not defined
    0,									// 1, Reserved
    417083,								// 2, 24/1.001fps
    416667,								// 3, 24fps
    208542,								// 4, 48/1.001fps
    400000,								// 5, 25fps
    333667,								// 6, 30/1.001fps
    333333,								// 7, 30fps
    208333,								// 8, 48fps
    200000,								// 9, 50fps
    166833,								// A, 60/1.001fps
    166667,								// B, 60fps
    104167,								// C, 96fps
    100000,								// D, 100fps
    83417,								// E, 120/1.001 fps
    83333								// F, 120fps
};

typedef union _SMPTE_ST352_PAYLOAD_ID {
    DWORD	dwData;

    struct {
        BYTE	byStandard				: 7;	// ST352_STANDARD
        BYTE	byVersion				: 1;	// Must be 1

        BYTE	byPictureRate			: 4;	// g_adwFrameDuration
        BYTE	byReserved1				: 2;
        BYTE	byProgressivePicture	: 1;
        BYTE	byProgressiveTransport	: 1;	// Not valid for ST352_STANDARD_483_576_270M_360M, ST352_STANDARD_720P_1_5G, ST352_STANDARD_720P_DS_3G, ST352_STANDARD_483_576_DS_3G

        BYTE	bySamplingStruct		: 4;	// SDI_SAMPLING_STRUCT
        BYTE	byColorimetry			: 2;	// Valid for ST352_STANDARD_6G_MODE1, ST352_STANDARD_6G_MODE2
        BYTE	byHorzYSampling			: 1;	// Valid for ST352_STANDARD_483_576_270M_360M, ST352_STANDARD_483_576_DS_3G, ST352_STANDARD_1080_3G
        BYTE	byImageAspectRatio		: 1;	// Valid for ST352_STANDARD_483_576_270M_360M, ST352_STANDARD_483_576_DS_3G

        BYTE	byBitDepth				: 2;
        BYTE	byReserved3				: 1;
        BYTE	byDynamicRange			: 2;	// Valid for ST352_STANDARD_1080_DL_1_5G, ST352_STANDARD_720P_3G, ST352_STANDARD_1080_3G
        BYTE	byReserved4				: 1;
        BYTE	byChannelAssignment		: 1;	// Valid for ST352_STANDARD_1080_DL_1_5G
        BYTE	byReserved5				: 1;
    } V1;
} SMPTE_ST352_PAYLOAD_ID;

#pragma pack(pop)
