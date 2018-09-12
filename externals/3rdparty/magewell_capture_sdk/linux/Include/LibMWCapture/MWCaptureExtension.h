////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include "WinTypes.h"
#include "MWCommon.h"
#include "MWSMPTE.h"
#include "MWIEC60958.h"
#include "MWHDMIPackets.h"

#ifndef _MAX_PATH
#define _MAX_PATH   (512)
#endif

#pragma pack(push)
#pragma pack(1)

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef HPCICHANNEL
#define HPCICHANNEL int
#endif

#ifndef HCHANNEL
#define HCHANNEL void *
#endif

#ifndef MWCAP_PTR64
#define MWCAP_PTR64  MWCAP_PTR
#endif

#ifndef MWHANDLE
#define MWHANDLE MWCAP_PTR
#endif

#ifndef LPBYTE
#define LPBYTE unsigned char*
#endif

#ifndef HTIMER
#define HTIMER MWCAP_PTR
#endif

#ifndef HNOTIFY
#define HNOTIFY MWCAP_PTR
#endif

#ifndef HOSD
#define HOSD MWCAP_PTR
#endif

#ifndef LPVOID
#define LPVOID void *
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef HANDLE64
#define HANDLE64 MWCAP_PTR
#endif

#ifdef __cplusplus
}
#endif

////////////////////////////////////////////////////////////////////////////////
// Magewell Capture Extensions

////////////////////////////////////////////////////////////////////////////////
// Data structs
typedef CHAR									MWCAP_BOOL;

typedef union _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

typedef enum _MW_RESULT_ {
    MW_SUCCEEDED = 0x00,
    MW_FAILED,
    MW_ENODATA,
    MW_INVALID_PARAMS
} MW_RESULT;

#define INPUT_SOURCE(type, index)				(((type) << 8) | ((index) & 0xFF))
#define INPUT_TYPE(source)						((source) >> 8)
#define INPUT_INDEX(source)						((source) & 0xFF)

typedef enum _MWCAP_PRODUCT_ID {
	MWCAP_PRODUCT_ID_PRO_CAPTURE_AIO			= 0x00000102,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_DVI			= 0x00000103,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_HDMI			= 0x00000104,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_SDI			= 0x00000105,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_DUAL_SDI		= 0x00000106,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_DUAL_DVI		= 0x00000107,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_DUAL_HDMI		= 0x00000108,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_QUAD_SDI		= 0x00000109,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_QUAD_HDMI		= 0x00000110,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_MINI_HDMI		= 0x00000111,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_HDMI_4K		= 0x00000112,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_MINI_SDI		= 0x00000113,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_AIO_4K_PLUS	= 0x00000114,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_HDMI_4K_PLUS	= 0x00000115,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_DVI_4K			= 0x00000116,
    MWCAP_PRODUCT_ID_PRO_CAPTURE_AIO_4K			= 0x00000117,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_SDI_4K_PLUS	= 0x00000118,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_DUAL_HDMI_4K_PLUS	= 0x00000119,
	MWCAP_PRODUCT_ID_PRO_CAPTURE_DUAL_SDI_4K_PLUS	= 0x00000120,

    MWCAP_PRODUCT_ID_ECO_CAPTURE_OCTA_SDI		= 0x00000150,
	MWCAP_PRODUCT_ID_ECO_CAPTURE_DUAL_HDMI_M2	= 0x00000151,
	MWCAP_PRODUCT_ID_ECO_CAPTURE_HDMI_4K_M2		= 0x00000152,
	MWCAP_PRODUCT_ID_ECO_CAPTURE_DUAL_SDI_M2	= 0x00000153,
	MWCAP_PRODUCT_ID_ECO_CAPTURE_QUAD_SDI_M2	= 0x00000154,

	MWCAP_PRODUCT_ID_USB_CAPTURE_HDMI_PLUS 		= 0x00000204,
	MWCAP_PRODUCT_ID_USB_CAPTURE_SDI_PLUS 		= 0x00000205,
	MWCAP_PRODUCT_ID_USB_CAPTURE_HDMI 			= 0x00000206,
	MWCAP_PRODUCT_ID_USB_CAPTURE_SDI			= 0x00000207,
	MWCAP_PRODUCT_ID_USB_CAPTURE_DVI 			= 0x00000208,
	MWCAP_PRODUCT_ID_USB_CAPTURE_HDMI_4K 		= 0x00000209,
	MWCAP_PRODUCT_ID_USB_CAPTURE_SDI_4K			= 0x00000210,
	MWCAP_PRODUCT_ID_USB_CAPTURE_AIO 			= 0x00000211,
	MWCAP_PRODUCT_ID_USB_CAPTURE_AIO_4K 		= 0x00000212
} MWCAP_PRODUCT_ID;

// A/V input type & source
typedef enum _MWCAP_VIDEO_INPUT_TYPE {
	MWCAP_VIDEO_INPUT_TYPE_NONE					= 0x00,
	MWCAP_VIDEO_INPUT_TYPE_HDMI					= 0x01,
	MWCAP_VIDEO_INPUT_TYPE_VGA					= 0x02,
	MWCAP_VIDEO_INPUT_TYPE_SDI					= 0x04,
	MWCAP_VIDEO_INPUT_TYPE_COMPONENT			= 0x08,
	MWCAP_VIDEO_INPUT_TYPE_CVBS					= 0x10,
	MWCAP_VIDEO_INPUT_TYPE_YC					= 0x20
} MWCAP_VIDEO_INPUT_TYPE;

typedef enum _MWCAP_AUDIO_INPUT_TYPE {
    MWCAP_AUDIO_INPUT_TYPE_NONE                 = 0x00,
	MWCAP_AUDIO_INPUT_TYPE_HDMI					= 0x01,
	MWCAP_AUDIO_INPUT_TYPE_SDI					= 0x02,
	MWCAP_AUDIO_INPUT_TYPE_LINE_IN				= 0x04,
	MWCAP_AUDIO_INPUT_TYPE_MIC_IN				= 0x08
} MWCAP_AUDIO_INPUT_TYPE;

typedef enum _MWCAP_PCIE_LINK_TYPE {
	MWCAP_PCIE_LINK_GEN_1						= 0x01,
	MWCAP_PCIE_LINK_GEN_2						= 0x02,
	MWCAP_PCIE_LINK_GEN_3						= 0x04,
	MWCAP_PCIE_LINK_GEN_4						= 0x08
} MWCAP_PCIE_LINK_TYPE;

typedef enum _MWCAP_VIDEO_TIMING_TYPE {
	MWCAP_VIDEO_TIMING_NONE						= 0x00000000,
	MWCAP_VIDEO_TIMING_LEGACY					= 0x00000001,
	MWCAP_VIDEO_TIMING_DMT						= 0x00000002,
	MWCAP_VIDEO_TIMING_CEA						= 0x00000004,
	MWCAP_VIDEO_TIMING_GTF						= 0x00000008,
	MWCAP_VIDEO_TIMING_CVT						= 0x00000010,
    MWCAP_VIDEO_TIMING_CVT_RB					= 0x00000020,
    MWCAP_VIDEO_TIMING_FAILSAFE					= 0x00002000
} MWCAP_VIDEO_TIMING_TYPE;

typedef enum _MWCAP_VIDEO_COLOR_FORMAT {
	MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN			= 0x00,
	MWCAP_VIDEO_COLOR_FORMAT_RGB				= 0x01,
	MWCAP_VIDEO_COLOR_FORMAT_YUV601				= 0x02,
	MWCAP_VIDEO_COLOR_FORMAT_YUV709				= 0x03,
	MWCAP_VIDEO_COLOR_FORMAT_YUV2020			= 0x04,
	MWCAP_VIDEO_COLOR_FORMAT_YUV2020C			= 0x05				// Constant luminance, not supported yet.
} MWCAP_VIDEO_COLOR_FORMAT;

typedef enum _MWCAP_VIDEO_QUANTIZATION_RANGE {
	MWCAP_VIDEO_QUANTIZATION_UNKNOWN			= 0x00,
	MWCAP_VIDEO_QUANTIZATION_FULL				= 0x01, 			// Black level: 0, White level: 255/1023/4095/65535
	MWCAP_VIDEO_QUANTIZATION_LIMITED			= 0x02				// Black level: 16/64/256/4096, White level: 235(240)/940(960)/3760(3840)/60160(61440)
} MWCAP_VIDEO_QUANTIZATION_RANGE;

typedef enum _MWCAP_VIDEO_SATURATION_RANGE {
	MWCAP_VIDEO_SATURATION_UNKNOWN				= 0x00,
	MWCAP_VIDEO_SATURATION_FULL					= 0x01, 			// Min: 0, Max: 255/1023/4095/65535
	MWCAP_VIDEO_SATURATION_LIMITED				= 0x02, 			// Min: 16/64/256/4096, Max: 235(240)/940(960)/3760(3840)/60160(61440)
	MWCAP_VIDEO_SATURATION_EXTENDED_GAMUT		= 0x03  			// Min: 1/4/16/256, Max: 254/1019/4079/65279
} MWCAP_VIDEO_SATURATION_RANGE;

typedef enum _MWCAP_VIDEO_FRAME_TYPE {
    MWCAP_VIDEO_FRAME_2D							= 0x00,
    MWCAP_VIDEO_FRAME_3D_TOP_AND_BOTTOM_FULL		= 0x01,
    MWCAP_VIDEO_FRAME_3D_TOP_AND_BOTTOM_HALF		= 0x02,
    MWCAP_VIDEO_FRAME_3D_SIDE_BY_SIDE_FULL			= 0x03,
    MWCAP_VIDEO_FRAME_3D_SIDE_BY_SIDE_HALF			= 0x04
} MWCAP_VIDEO_FRAME_TYPE;

typedef enum _MWCAP_VIDEO_DEINTERLACE_MODE {
    MWCAP_VIDEO_DEINTERLACE_WEAVE				= 0x00,
    MWCAP_VIDEO_DEINTERLACE_BLEND				= 0x01,
    MWCAP_VIDEO_DEINTERLACE_TOP_FIELD			= 0x02,
    MWCAP_VIDEO_DEINTERLACE_BOTTOM_FIELD		= 0x03
} MWCAP_VIDEO_DEINTERLACE_MODE;

typedef enum _MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE {
	MWCAP_VIDEO_ASPECT_RATIO_IGNORE				= 0x00,
	MWCAP_VIDEO_ASPECT_RATIO_CROPPING			= 0x01,
	MWCAP_VIDEO_ASPECT_RATIO_PADDING			= 0x02
} MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE;

typedef enum _MWCAP_VIDEO_SYNC_TYPE {
	VIDEO_SYNC_ALL								= 0x07,
	VIDEO_SYNC_HS_VS							= 0x01,
	VIDEO_SYNC_CS								= 0x02,
	VIDEO_SYNC_EMBEDDED							= 0x04
} MWCAP_VIDEO_SYNC_TYPE;

typedef struct _MWCAP_VIDEO_SYNC_INFO	 {
	BYTE										bySyncType;
    BOOLEAN										bHSPolarity;
    BOOLEAN										bVSPolarity;
    BOOLEAN										bInterlaced;
	DWORD										dwFrameDuration;
	WORD										wVSyncLineCount;
	WORD										wFrameLineCount;
} MWCAP_VIDEO_SYNC_INFO;

typedef struct _MWCAP_VIDEO_TIMING {
	DWORD										dwType;
	DWORD										dwPixelClock;
    BOOLEAN										bInterlaced;
	BYTE										bySyncType;
    BOOLEAN										bHSPolarity;
    BOOLEAN										bVSPolarity;
	WORD										wHActive;
	WORD										wHFrontPorch;
	WORD										wHSyncWidth;
	WORD										wHBackPorch;
	WORD										wVActive;
	WORD										wVFrontPorch;
	WORD										wVSyncWidth;
	WORD										wVBackPorch;
} MWCAP_VIDEO_TIMING;

typedef struct _MWCAP_VIDEO_TIMING_SETTINGS {
    WORD										wAspectX;
    WORD										wAspectY;
    WORD										x;
    WORD										y;
    WORD										cx;
    WORD										cy;
    WORD										cxTotal;
    BYTE										byClampPos;
} MWCAP_VIDEO_TIMING_SETTINGS;

typedef struct _MWCAP_SIZE {
	WORD										cx;
	WORD										cy;
} MWCAP_SIZE;

typedef struct _MWCAP_RECT {
	WORD										x;
	WORD										y;
	WORD										cx;
	WORD										cy;
} MWCAP_RECT;

typedef struct _MWCAP_DWORD_PARAMETER_RANGE {
	DWORD										dwMin;
	DWORD										dwMax;
	DWORD										dwStep;
	DWORD										dwDefault;
} MWCAP_DWORD_PARAMETER_RANGE;

#define MWCAP_DWORD_PARAMETER_FLAG_AUTO			0x01

typedef struct _MWCAP_DWORD_PARAMETER_VALUE {
	DWORD										dwFlags;
	DWORD										dwValue;
} MWCAP_DWORD_PARAMETER_VALUE;

// Product informations
typedef struct _MWCAP_CHANNEL_INFO {
	WORD										wFamilyID;
	WORD										wProductID;
	CHAR										chHardwareVersion;
	BYTE										byFirmwareID;
	DWORD										dwFirmwareVersion;
	DWORD										dwDriverVersion;
	CHAR										szFamilyName[MW_FAMILY_NAME_LEN];
	CHAR										szProductName[MW_PRODUCT_NAME_LEN];
	CHAR										szFirmwareName[MW_FIRMWARE_NAME_LEN];
	CHAR										szBoardSerialNo[MW_SERIAL_NO_LEN];
	BYTE										byBoardIndex;
	BYTE										byChannelIndex;
} MWCAP_CHANNEL_INFO;

typedef struct _MWCAP_PRO_CAPTURE_INFO {
	BYTE										byPCIBusID;
	BYTE										byPCIDevID;
	BYTE										byLinkType;
	BYTE										byLinkWidth;
	BYTE										byBoardIndex;
	WORD										wMaxPayloadSize;
    WORD										wMaxReadRequestSize;
    DWORD										cbTotalMemorySize;
    DWORD										cbFreeMemorySize;
} MWCAP_PRO_CAPTURE_INFO;

typedef struct _MWCAP_VIDEO_CAPS {
    DWORD										dwCaps;
	WORD										wMaxInputWidth;
	WORD										wMaxInputHeight;
	WORD										wMaxOutputWidth;
	WORD										wMaxOutputHeight;
} MWCAP_VIDEO_CAPS;

typedef struct _MWCAP_AUDIO_CAPS {
	DWORD										dwCaps;
} MWCAP_AUDIO_CAPS;

// Firmware upgrade
typedef struct _MWCAP_FIRMWARE_STORAGE {
	DWORD										cbStorage;
	DWORD										cbEraseBlock;
	DWORD										cbProgramBlock;
	DWORD										cbHeaderOffset;
} MWCAP_FIRMWARE_STORAGE;

typedef struct _MWCAP_FIRMWARE_ERASE {
	DWORD										cbOffset;
	DWORD										cbErase;
} MWCAP_FIRMWARE_ERASE;

// Device misc controls
typedef enum _MWCAP_LED_MODE {
	MWCAP_LED_AUTO								= 0x00000000,
	MWCAP_LED_OFF								= 0x80000000,
	MWCAP_LED_ON								= 0x80000001,
	MWCAP_LED_BLINK								= 0x80000002,
	MWCAP_LED_DBL_BLINK							= 0x80000003,
	MWCAP_LED_BREATH							= 0x80000004
} MWCAP_LED_MODE;

// Signal status
typedef struct _MWCAP_SDI_SPECIFIC_STATUS {
	SDI_TYPE									sdiType;
	SDI_SCANNING_FORMAT							sdiScanningFormat;
	SDI_BIT_DEPTH								sdiBitDepth;
	SDI_SAMPLING_STRUCT							sdiSamplingStruct;
    BOOLEAN										bST352DataValid;
    DWORD										dwST352Data;
} MWCAP_SDI_SPECIFIC_STATUS;

typedef struct _MWCAP_HDMI_VIDEO_TIMING {
    BOOLEAN										bInterlaced;
    DWORD										dwFrameDuration;
    WORD										wHSyncWidth;
    WORD										wHFrontPorch;
    WORD										wHBackPorch;
    WORD										wHActive;
    WORD										wHTotalWidth;
    WORD										wField0VSyncWidth;
    WORD										wField0VFrontPorch;
    WORD										wField0VBackPorch;
    WORD										wField0VActive;
    WORD										wField0VTotalHeight;
    WORD										wField1VSyncWidth;
    WORD										wField1VFrontPorch;
    WORD										wField1VBackPorch;
    WORD										wField1VActive;
    WORD										wField1VTotalHeight;
} MWCAP_HDMI_VIDEO_TIMING;

typedef struct _MWCAP_HDMI_SPECIFIC_STATUS {
	BOOLEAN										bHDMIMode;
	BOOLEAN										bHDCP;
	BYTE										byBitDepth;
    HDMI_PXIEL_ENCODING							pixelEncoding;
    BYTE										byVIC;
    BOOLEAN										bITContent;
    BOOLEAN										b3DFormat;
    BYTE										by3DStructure;
    BYTE										bySideBySideHalfSubSampling;
    MWCAP_HDMI_VIDEO_TIMING						videoTiming;
} MWCAP_HDMI_SPECIFIC_STATUS;

typedef struct _MWCAP_COMPONENT_SPECIFIC_STATUS {
    MWCAP_VIDEO_SYNC_INFO						syncInfo;
    BOOLEAN										bTriLevelSync;
    MWCAP_VIDEO_TIMING							videoTiming;			// Not valid for custom video timing
    MWCAP_VIDEO_TIMING_SETTINGS					videoTimingSettings;
} MWCAP_COMPONENT_SPECIFIC_STATUS;

typedef enum _MWCAP_SD_VIDEO_STANDARD {
	MWCAP_SD_VIDEO_NONE,
	MWCAP_SD_VIDEO_NTSC_M,
	MWCAP_SD_VIDEO_NTSC_433,
	MWCAP_SD_VIDEO_PAL_M,
	MWCAP_SD_VIDEO_PAL_60,
	MWCAP_SD_VIDEO_PAL_COMBN,
	MWCAP_SD_VIDEO_PAL_BGHID,
	MWCAP_SD_VIDEO_SECAM,
	MWCAP_SD_VIDEO_SECAM_60
} MWCAP_SD_VIDEO_STANDARD;

typedef struct _MWCAP_CVBS_YC_SPECIFIC_STATUS {
	MWCAP_SD_VIDEO_STANDARD						standard;
	BOOLEAN										b50Hz;
} MWCAP_CVBS_YC_SPECIFIC_STATUS;

typedef struct _MWCAP_INPUT_SPECIFIC_STATUS {
	BOOLEAN										bValid;
	DWORD										dwVideoInputType;
	union {
		MWCAP_SDI_SPECIFIC_STATUS				sdiStatus;
		MWCAP_HDMI_SPECIFIC_STATUS				hdmiStatus;
		MWCAP_COMPONENT_SPECIFIC_STATUS			vgaComponentStatus;
		MWCAP_CVBS_YC_SPECIFIC_STATUS			cvbsYcStatus;
	};
} MWCAP_INPUT_SPECIFIC_STATUS;

typedef enum _MWCAP_VIDEO_SIGNAL_STATE {
	MWCAP_VIDEO_SIGNAL_NONE,					// No signal detectd
	MWCAP_VIDEO_SIGNAL_UNSUPPORTED,				// Video signal status not valid
	MWCAP_VIDEO_SIGNAL_LOCKING,					// Video signal status valid but not locked yet
	MWCAP_VIDEO_SIGNAL_LOCKED					// Every thing OK
} MWCAP_VIDEO_SIGNAL_STATE;

typedef struct _MWCAP_VIDEO_SIGNAL_STATUS {
	MWCAP_VIDEO_SIGNAL_STATE					state;
	int											x;
	int											y;
	int											cx;
	int											cy;
	int											cxTotal;
	int											cyTotal;
	BOOLEAN										bInterlaced;
	DWORD										dwFrameDuration;
	int											nAspectX;
    int											nAspectY;
	BOOLEAN										bSegmentedFrame;
    MWCAP_VIDEO_FRAME_TYPE						frameType;
	MWCAP_VIDEO_COLOR_FORMAT					colorFormat;
	MWCAP_VIDEO_QUANTIZATION_RANGE				quantRange;
	MWCAP_VIDEO_SATURATION_RANGE				satRange;
} MWCAP_VIDEO_SIGNAL_STATUS;

typedef struct _MWCAP_AUDIO_SIGNAL_STATUS {
	WORD										wChannelValid;
	BOOLEAN										bLPCM;
	BYTE										cBitsPerSample;
	DWORD										dwSampleRate;
	BOOLEAN										bChannelStatusValid;
	IEC60958_CHANNEL_STATUS						channelStatus;
} MWCAP_AUDIO_SIGNAL_STATUS;

// Hardware timer
typedef struct _MWCAP_TIMER_EXPIRE_TIME {
    MWCAP_PTR                                   pvTimer;
    LONGLONG                                    llExpireTime;
} MWCAP_TIMER_EXPIRE_TIME;

typedef struct _MWCAP_TIMER_REGISTRATION_S {
    MWCAP_PTR                                   pvTimer;      // get
    MWCAP_PTR                                   pvEvent;      // set
} MWCAP_TIMER_REGISTRATION_S;

// Notifications
#define MWCAP_NOTIFY_INPUT_SORUCE_START_SCAN        0x0001ULL
#define MWCAP_NOTIFY_INPUT_SORUCE_STOP_SCAN         0x0002ULL
#define MWCAP_NOTIFY_INPUT_SORUCE_SCAN_CHANGE       0x0003ULL

#define MWCAP_NOTIFY_VIDEO_INPUT_SOURCE_CHANGE      0x0004ULL
#define MWCAP_NOTIFY_AUDIO_INPUT_SOURCE_CHANGE      0x0008ULL

// MWCAP_KSPROPERTY_INPUT_SPECIFIC_STATUS
#define MWCAP_NOTIFY_INPUT_SPECIFIC_CHANGE          0x0010ULL

// MWCAP_KSPROPERTY_VIDEO_SIGNAL_STATUS
#define MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE            0x0020ULL

// MWCAP_KSPROPERTY_AUDIO_SIGNAL_STATUS
#define MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE            0x0040ULL

// MWCAP_KSPROPERTY_VIDEO_FRAME_INFO
#define MWCAP_NOTIFY_VIDEO_FIELD_BUFFERING			0x0080ULL
#define MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING			0x0100ULL
#define MWCAP_NOTIFY_VIDEO_FIELD_BUFFERED			0x0200ULL
#define MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED			0x0400ULL
#define MWCAP_NOTIFY_VIDEO_SMPTE_TIME_CODE			0x0800ULL

// MWCAP_KSPROPERTY_AUDIO_CAPTURE_FRAME
#define MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED			0x1000ULL
#define MWCAP_NOTIFY_AUDIO_INPUT_RESET				0x2000ULL

// MWCAP_KSPROPERTY_VIDEO_SAMPLING_PHASE
#define MWCAP_NOTIFY_VIDEO_SAMPLING_PHASE_CHANGE	0x4000ULL

#define MWCAP_NOTIFY_LOOP_THROUGH_CHANGED			0x8000ULL
#define MWCAP_NOTIFY_LOOP_THROUGH_EDID_CHANGED		0x10000ULL

#define MWCAP_NOTIFY_NEW_SDI_ANC_PACKET				0x20000ULL

// MWCAP_KSPROPERTY_HDMI_INFOFRAME_VALID, MWCAP_KSPROPERTY_HDMI_INFOFRAME_PACKET
#define MWCAP_NOTIFY_HDMI_INFOFRAME_AVI			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_AVI))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_AUDIO		(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_AUDIO))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_SPD			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_SPD))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_MS			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_MS))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_VS			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_VS))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_ACP			(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_ACP))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_ISRC1		(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_ISRC1))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_ISRC2		(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_ISRC2))
#define MWCAP_NOTIFY_HDMI_INFOFRAME_GAMUT		(1ULL << (32 + MWCAP_HDMI_INFOFRAME_ID_GAMUT))

typedef struct _MWCAP_NOTIFY_REGISTRATION_S {
    MWCAP_PTR                                   pvNotify;      // get
    ULONGLONG                                   ullEnableBits; // set
    MWCAP_PTR                                   pvEvent;       // set
} MWCAP_NOTIFY_REGISTRATION_S;

typedef struct _MWCAP_NOTIFY_STATUS {
    MWCAP_PTR                                   pvNotify;      // set
    ULONGLONG                                   ullStatusBits; // get
} MWCAP_NOTIFY_STATUS;

typedef struct _MWCAP_NOTIFY_ENABLE {
    MWCAP_PTR                                   pvNotify;      // set
    ULONGLONG                                   ullEnableBits; // set
} MWCAP_NOTIFY_ENABLE;

// Video frame information
#define MWCAP_MAX_VIDEO_FRAME_COUNT				8

typedef struct _MWCAP_SMPTE_TIMECODE {
	BYTE 										byFrames;
	BYTE										bySeconds;
	BYTE										byMinutes;
	BYTE										byHours;
} MWCAP_SMPTE_TIMECODE;

typedef enum _MWCAP_VIDEO_FRAME_STATE {
    MWCAP_VIDEO_FRAME_STATE_INITIAL,
    MWCAP_VIDEO_FRAME_STATE_F0_BUFFERING,
    MWCAP_VIDEO_FRAME_STATE_F1_BUFFERING,
    MWCAP_VIDEO_FRAME_STATE_BUFFERED
} MWCAP_VIDEO_FRAME_STATE;

typedef struct _MWCAP_VIDEO_BUFFER_INFO {
    DWORD											cMaxFrames;

    BYTE											iNewestBuffering;
    BYTE											iBufferingFieldIndex;

    BYTE											iNewestBuffered;
    BYTE											iBufferedFieldIndex;

    BYTE											iNewestBufferedFullFrame;
    DWORD											cBufferedFullFrames;
} MWCAP_VIDEO_BUFFER_INFO;

typedef struct _MWCAP_VIDEO_FRAME_INFO {
    MWCAP_VIDEO_FRAME_STATE							state;

    BOOLEAN											bInterlaced;
    BOOLEAN											bSegmentedFrame;
    BOOLEAN											bTopFieldFirst;
    BOOLEAN											bTopFieldInverted;

    int												cx;
    int												cy;
    int												nAspectX;
    int												nAspectY;

    LONGLONG										allFieldStartTimes[2];
    LONGLONG										allFieldBufferedTimes[2];
    MWCAP_SMPTE_TIMECODE							aSMPTETimeCodes[2];
} MWCAP_VIDEO_FRAME_INFO;

// Video capture
typedef struct _MWCAP_VIDEO_CAPTURE_OPEN {
    MWCAP_PTR                                       hEvent;
} MWCAP_VIDEO_CAPTURE_OPEN;

#define MWCAP_VIDEO_MAX_NUM_OSD_RECTS			4

#define MWCAP_VIDEO_FRAME_ID_NEWEST_BUFFERED		(-1)
#define MWCAP_VIDEO_FRAME_ID_NEWEST_BUFFERING		(-2)
#define MWCAP_VIDEO_FRAME_ID_NEXT_BUFFERED			(-3)
#define MWCAP_VIDEO_FRAME_ID_NEXT_BUFFERING			(-4)

#define MWCAP_VIDEO_FRAME_ID_EMPTY					(-100)		// Used in MWCAP_VIDEO_CAPTURE_STATUS

#define MWCAP_VIDEO_PROCESS_FLIP					0x00000001
#define MWCAP_VIDEO_PROCESS_MIRROR					0x00000002


typedef struct _MWCAP_VIDEO_CAPTURE_FRAME {
	// Processing parameters
	DWORD										dwFOURCC;
	WORD										cx;
	WORD										cy;
	int											nAspectX;
	int											nAspectY;
	MWCAP_VIDEO_COLOR_FORMAT					colorFormat;
	MWCAP_VIDEO_QUANTIZATION_RANGE				quantRange;
	MWCAP_VIDEO_SATURATION_RANGE				satRange;

    SHORT										sContrast;			// [50, 200]
    SHORT										sBrightness;		// [-100, 100]
    SHORT										sSaturation;		// [0, 200]
    SHORT										sHue;				// [-90, 90]

	RECT										rectSource;
	RECT										rectTarget;

	MWCAP_VIDEO_DEINTERLACE_MODE				deinterlaceMode;
	MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE		aspectRatioConvertMode;

	// Source frame
    int 										iSrcFrame;

    // OSD (within rectTarget and [0,0-cx,cy))
    MWCAP_PTR   								pOSDImage;
    RECT										aOSDRects[MWCAP_VIDEO_MAX_NUM_OSD_RECTS];
    int											cOSDRects;

	// Buffer parameters
	BOOLEAN										bPhysicalAddress;
	union {
        MWCAP_PTR   							pvFrame;
		LARGE_INTEGER							liPhysicalAddress;
	};

	DWORD										cbFrame;
	DWORD										cbStride;

	BOOLEAN										bBottomUp;

    // 0: Not use partial notify
    WORD										cyPartialNotify;

    DWORD										dwProcessSwitchs;		// MWCAP_VIDEO_PROCESS_xx

	// Context
    MWCAP_PTR   								pvContext;
} MWCAP_VIDEO_CAPTURE_FRAME;

typedef struct _MWCAP_VIDEO_CAPTURE_STATUS {
    MWCAP_PTR   								pvContext;

	BOOLEAN										bPhysicalAddress;
	union {
        MWCAP_PTR   							pvFrame;
		LARGE_INTEGER							liPhysicalAddress;
    };

    int 										iFrame;

	BOOLEAN										bFrameCompleted;
	WORD										cyCompleted;
    WORD										cyCompletedPrev;
} MWCAP_VIDEO_CAPTURE_STATUS;

// Audio capture
#define MWCAP_AUDIO_FRAME_SYNC_CODE				0xFECA0357
#define MWCAP_AUDIO_SAMPLES_PER_FRAME			192
#define MWCAP_AUDIO_MAX_NUM_CHANNELS			8

// Audio samples are 32bits wide, cBitsPerSample of high bits are valid
// Sample layout: 0L, 1L, 2L, 3L, 0R, 1R, 2R, 3R
typedef struct _MWCAP_AUDIO_CAPTURE_FRAME {
    DWORD										cFrameCount;
    DWORD										iFrame;
	DWORD										dwSyncCode;
	DWORD										dwReserved;
	LONGLONG									llTimestamp;
	DWORD										adwSamples[MWCAP_AUDIO_SAMPLES_PER_FRAME * MWCAP_AUDIO_MAX_NUM_CHANNELS];
} MWCAP_AUDIO_CAPTURE_FRAME;

// HDMI status
typedef enum _MWCAP_HDMI_INFOFRAME_ID {
	MWCAP_HDMI_INFOFRAME_ID_AVI,
	MWCAP_HDMI_INFOFRAME_ID_AUDIO,
	MWCAP_HDMI_INFOFRAME_ID_SPD,
	MWCAP_HDMI_INFOFRAME_ID_MS,
	MWCAP_HDMI_INFOFRAME_ID_VS,
	MWCAP_HDMI_INFOFRAME_ID_ACP,
	MWCAP_HDMI_INFOFRAME_ID_ISRC1,
	MWCAP_HDMI_INFOFRAME_ID_ISRC2,
	MWCAP_HDMI_INFOFRAME_ID_GAMUT,
	MWCAP_HDMI_INFOFRAME_ID_VBI,
	MWCAP_HDMI_INFOFRAME_ID_HDR,
	MWCAP_HDMI_INFOFRAME_COUNT
} MWCAP_HDMI_INFOFRAME_ID;

typedef enum _MWCAP_HDMI_INFOFRAME_MASK {
	MWCAP_HDMI_INFOFRAME_MASK_AVI				= (1 << MWCAP_HDMI_INFOFRAME_ID_AVI),
	MWCAP_HDMI_INFOFRAME_MASK_AUDIO				= (1 << MWCAP_HDMI_INFOFRAME_ID_AUDIO),
	MWCAP_HDMI_INFOFRAME_MASK_SPD				= (1 << MWCAP_HDMI_INFOFRAME_ID_SPD),
	MWCAP_HDMI_INFOFRAME_MASK_MS				= (1 << MWCAP_HDMI_INFOFRAME_ID_MS),
	MWCAP_HDMI_INFOFRAME_MASK_VS				= (1 << MWCAP_HDMI_INFOFRAME_ID_VS),
	MWCAP_HDMI_INFOFRAME_MASK_ACP				= (1 << MWCAP_HDMI_INFOFRAME_ID_ACP),
	MWCAP_HDMI_INFOFRAME_MASK_ISRC1				= (1 << MWCAP_HDMI_INFOFRAME_ID_ISRC1),
	MWCAP_HDMI_INFOFRAME_MASK_ISRC2				= (1 << MWCAP_HDMI_INFOFRAME_ID_ISRC2),
	MWCAP_HDMI_INFOFRAME_MASK_GAMUT				= (1 << MWCAP_HDMI_INFOFRAME_ID_GAMUT),
	MWCAP_HDMI_INFOFRAME_MASK_VBI				= (1 << MWCAP_HDMI_INFOFRAME_ID_VBI),
	MWCAP_HDMI_INFOFRAME_MASK_HDR				= (1 << MWCAP_HDMI_INFOFRAME_ID_HDR)
} MWCAP_HDMI_INFOFRAME_MASK;

typedef struct _MWCAP_VIDEO_ASPECT_RATIO {
	int											nAspectX;
	int											nAspectY;
} MWCAP_VIDEO_ASPECT_RATIO;

typedef struct _MWCAP_VIDEO_CONNECTION_FORMAT {
    // Valid flag
    BOOLEAN										bConnected;

    // Basic information
    LONG										cx;
    LONG										cy;
    DWORD										dwFrameDuration;
    DWORD										dwFOURCC;

    // Preferred parameters
    int											nAspectX;
    int											nAspectY;
    MWCAP_VIDEO_COLOR_FORMAT					colorFormat;
    MWCAP_VIDEO_QUANTIZATION_RANGE				quantRange;
    MWCAP_VIDEO_SATURATION_RANGE				satRange;
} MWCAP_VIDEO_CONNECTION_FORMAT;

typedef struct _MWCAP_VIDEO_PROCESS_SETTINGS {
    DWORD										dwProcessSwitchs;
    RECT										rectSource;
    int											nAspectX;
    int											nAspectY;
    BOOLEAN										bLowLatency;
    MWCAP_VIDEO_COLOR_FORMAT					colorFormat;
    MWCAP_VIDEO_QUANTIZATION_RANGE				quantRange;
    MWCAP_VIDEO_SATURATION_RANGE				satRange;
    MWCAP_VIDEO_DEINTERLACE_MODE				deinterlaceMode;
    MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE		aspectRatioConvertMode;
} MWCAP_VIDEO_PROCESS_SETTINGS;

#define MWCAP_VIDEO_MAX_NUM_PREFERRED_TIMINGS	8

typedef struct _MWCAP_VIDEO_CREATE_IMAGE {
    WORD                                        cx; // set
    WORD                                        cy; // set
    MWCAP_PTR                                   pvImage; // get
} MWCAP_VIDEO_CREATE_IMAGE;

typedef struct _MWCAP_VIDEO_IMAGE_REF {
    MWCAP_PTR                                   pvImage; // set
    int                                         nRefCount; // get
} MWCAP_VIDEO_IMAGE_REF;

typedef struct _MWCAP_VIDEO_UPLOAD_IMAGE {
    // Destination parameters
    MWCAP_PTR                                   pvDestImage;
    MWCAP_VIDEO_COLOR_FORMAT					cfDest;
    WORD										xDest;
    WORD										yDest;
    WORD										cxDest;
    WORD										cyDest;

    MWCAP_VIDEO_QUANTIZATION_RANGE				quantRangeDest;
    MWCAP_VIDEO_SATURATION_RANGE				satRangeDest;

    // Source parameters
    BOOLEAN										bSrcPhysicalAddress;
    union {
        MWCAP_PTR                               pvSrcFrame;
        LARGE_INTEGER							liSrcPhysicalAddress;
    };

    DWORD										cbSrcFrame;
    DWORD										cbSrcStride;

    WORD										cxSrc;
    WORD										cySrc;
    BOOLEAN										bSrcBottomUp;
    BOOLEAN										bSrcPixelAlpha;
    BOOLEAN										bSrcPixelXBGR;
} MWCAP_VIDEO_UPLOAD_IMAGE;

typedef struct _MWCAP_VIDEO_OSD_SETTINGS {
    BOOLEAN										bEnable;
    char                                        szPNGFilePath[_MAX_PATH];
} MWCAP_VIDEO_OSD_SETTINGS;

typedef struct _MWCAP_VIDEO_OSD_IMAGE {
    MWCAP_PTR                                   pvOSDImage;
    RECT										aOSDRects[MWCAP_VIDEO_MAX_NUM_OSD_RECTS];
    int											cOSDRects;
} MWCAP_VIDEO_OSD_IMAGE;

typedef struct _MWCAP_VIDEO_CUSTOM_TIMING {
    MWCAP_VIDEO_SYNC_INFO						syncInfo;
    MWCAP_VIDEO_TIMING_SETTINGS					videoTimingSettings;
} MWCAP_VIDEO_CUSTOM_TIMING;

typedef struct _MWCAP_VIDEO_PIN_BUFFER {
    MWCAP_PTR                                       pvBuffer;
    DWORD                                           cbBuffer;
    int                                             mem_type;   /* see mw-dma-mem.h */
    unsigned long long                              reserved;
} MWCAP_VIDEO_PIN_BUFFER;

typedef enum _MW_VIDEO_CAPTURE_MODE {
    MW_VIDEO_CAPTURE_NORMAL = 0x00,
    MW_VIDEO_CAPTURE_LOW_LATENCY,
} MW_VIDEO_CAPTURE_MODE;

typedef enum _MWCAP_AUDIO_CAPTURE_NODE {
        MWCAP_AUDIO_CAPTURE_NODE_DEFAULT,
        MWCAP_AUDIO_CAPTURE_NODE_EMBEDDED_CAPTURE,
        MWCAP_AUDIO_CAPTURE_NODE_MICROPHONE,
        MWCAP_AUDIO_CAPTURE_NODE_USB_CAPTURE,
        MWCAP_AUDIO_CAPTURE_NODE_LINE_IN,
} MWCAP_AUDIO_CAPTURE_NODE;

typedef enum _MWCAP_AUDIO_NODE {
	MWCAP_AUDIO_MICROPHONE,
	MWCAP_AUDIO_HEADPHONE,
	MWCAP_AUDIO_LINE_IN,
	MWCAP_AUDIO_LINE_OUT,
	MWCAP_AUDIO_EMBEDDED_CAPTURE,
	MWCAP_AUDIO_EMBEDDED_PLAYBACK,
	MWCAP_AUDIO_USB_CAPTURE,
	MWCAP_AUDIO_USB_PLAYBACK
} MWCAP_AUDIO_NODE;

typedef void(*LPFN_VIDEO_CAPTURE_CALLBACK)(MWCAP_PTR pbFrame, DWORD cbFrame, DWORD cbStride, MWCAP_VIDEO_FRAME_INFO* pFrameInfo, void* pvContent);
typedef void(*LPFN_AUDIO_CAPTURE_CALLBACK)(MWCAP_AUDIO_CAPTURE_FRAME* pAudioCaptureFrame, void* pvContent);
typedef void(*LPFN_TIMER_CALLBACK)(HTIMER pTimer, void* pvContent);
typedef void(*LPFN_NOTIFY_CALLBACK)(MWCAP_PTR pNotify, DWORD dwEnableBits, void* pvContent);

typedef void (*VIDEO_CAPTURE_CALLBACK)(BYTE *pBuffer, long iBufferLen, void* pParam);
typedef void (*AUDIO_CAPTURE_CALLBACK)(const BYTE * pbFrame, int cbFrame, uint64_t u64TimeStamp, void* pParam);

typedef struct _MWCAP_SDI_ANC_TYPE {
	BYTE											byId;
	BOOLEAN											bHANC;
	BOOLEAN											bVANC;
	BYTE											byDID;
	BYTE											bySDID;
} MWCAP_SDI_ANC_TYPE;

typedef struct _MWCAP_SDI_ANC_PACKET {
	BYTE											byDID;
	BYTE											bySDID;
	BYTE											byDC;
	BYTE											abyUDW[255];
	BYTE											abyReserved[2];
} MWCAP_SDI_ANC_PACKET;



typedef struct _MWCAP_VIDEO_ECO_CAPTURE_OPEN {
	MWCAP_PTR64										hEvent;

	DWORD											dwFOURCC;
	WORD											cx;
	WORD											cy;
	LONGLONG										llFrameDuration;	// -1 for input frame rate
} MWCAP_VIDEO_ECO_CAPTURE_OPEN;

typedef struct _MWCAP_VIDEO_ECO_CAPTURE_SETTINGS {
	MWCAP_VIDEO_COLOR_FORMAT						colorFormat;
	MWCAP_VIDEO_QUANTIZATION_RANGE					quantRange;
	MWCAP_VIDEO_SATURATION_RANGE					satRange;
	SHORT											sContrast;			// [50, 200]
	SHORT											sBrightness;		// [-100, 100]
	SHORT											sSaturation;		// [0, 200]
	SHORT											sHue;				// [-90, 90]
} MWCAP_VIDEO_ECO_CAPTURE_SETTINGS;

typedef struct _MWCAP_VIDEO_ECO_CAPTURE_FRAME {
	MWCAP_PTR64										pvFrame;
	DWORD											cbFrame;
	DWORD											cbStride;

	BOOLEAN											bBottomUp;
	MWCAP_VIDEO_DEINTERLACE_MODE					deinterlaceMode;

	MWCAP_PTR64										pvContext;
} MWCAP_VIDEO_ECO_CAPTURE_FRAME;

typedef struct _MWCAP_VIDEO_ECO_CAPTURE_STATUS {
	MWCAP_PTR64										pvContext;
	MWCAP_PTR64										pvFrame;
	LONGLONG										llTimestamp;
} MWCAP_VIDEO_ECO_CAPTURE_STATUS;

#pragma pack(pop)

