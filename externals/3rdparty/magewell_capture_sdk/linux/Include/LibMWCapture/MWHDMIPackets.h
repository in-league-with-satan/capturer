////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#pragma pack(push)
#pragma pack(1)

// AVI infoframe
#define HDMI_EC_XVYCC601			0x00
#define HDMI_EC_XVYCC709			0x01
#define HDMI_EC_SYCC601				0x02
#define HDMI_EC_ADOBEYCC601			0x03
#define HDMI_EC_ADOBERGB			0x04

#define HDMI_YQ_LIMITED_RANGE		0x00
#define HDMI_YQ_FULL_RANGE			0x01

#define HDMI_CN_NONE_OR_GRAPHICS	0x00
#define HDMI_CN_PHOTO				0x01
#define HDMI_CN_CINEMA				0x02
#define HDMI_CN_GAME				0x03

typedef struct _HDMI_VIC_FORMAT {
	WORD							cx;
	WORD							cy;
	BOOLEAN							bInterlaced;
	WORD							wAspectX;
	WORD							wAspectY;
	DWORD							dwFrameDuration;
} HDMI_VIC_FORMAT;

static const HDMI_VIC_FORMAT		g_aHDMIVICFormats[] = {
	/*  0  */ {	   0,    0, FALSE,  0, 0,      0, },
	/*  1 */  {	 640,  480, FALSE,  4, 3, 166667, },
	/*  2 */  {	 720,  480, FALSE,  4, 3, 166667, },
	/*  3 */  {	 720,  480, FALSE, 16, 9, 166667, },
	/*  4 */  {	1280,  720, FALSE, 16, 9, 166667, },
	/*  5 */  {	1920, 1080,  TRUE, 16, 9, 333333, },
	/*  6 */  {	 720,  480,  TRUE,  4, 3, 333333, },
	/*  7 */  {	 720,  480,  TRUE, 16, 9, 333333, },
	/*  8 */  {	 720,  240, FALSE,  4, 3, 166667, },
	/*  9 */  {	 720,  240, FALSE, 16, 9, 166667, },
	/* 10 */  {	2880,  480,  TRUE,  4, 3, 333333, },
	/* 11 */  {	2880,  480,  TRUE, 16, 9, 333333, },
	/* 12 */  {	2880,  240, FALSE,  4, 3, 166667, },
	/* 13 */  {	2880,  240, FALSE, 16, 9, 166667, },
	/* 14 */  {	1440,  480, FALSE,  4, 3, 166667, },
	/* 15 */  {	1440,  480, FALSE, 16, 9, 166667, },
	/* 16 */  {	1920, 1080, FALSE, 16, 9, 166667, },
	/* 17 */  {	 720,  576, FALSE,  4, 3, 200000, },
	/* 18 */  {	 720,  576, FALSE, 16, 9, 200000, },
	/* 19 */  {	1280,  720, FALSE, 16, 9, 200000, },
	/* 20 */  {	1920, 1080,  TRUE, 16, 9, 400000, },
	/* 21 */  {	 720,  576,  TRUE,  4, 3, 400000, },
	/* 22 */  {	 720,  576,  TRUE, 16, 9, 400000, },
	/* 23 */  {	 720,  288, FALSE,  4, 3, 200000, },
	/* 24 */  {	 720,  288, FALSE, 16, 9, 200000, },
	/* 25 */  {	2880,  576,  TRUE,  4, 3, 400000, },
	/* 26 */  {	2880,  576,  TRUE,  4, 3, 400000, },
	/* 27 */  {	2880,  288, FALSE,  4, 3, 200000, },
	/* 28 */  {	2880,  288, FALSE, 16, 9, 200000, },
	/* 29 */  {	1440,  576, FALSE,  4, 3, 200000, },
	/* 30 */  {	1440,  576, FALSE, 16, 9, 200000, },
	/* 31 */  {	1920, 1080, FALSE, 16, 9, 200000, },
	/* 32 */  {	1920, 1080, FALSE, 16, 9, 416667, },
	/* 33 */  {	1920, 1080, FALSE, 16, 9, 400000, },
	/* 34 */  {	1920, 1080, FALSE, 16, 9, 333333, },
	/* 35 */  {	2880,  480, FALSE,  4, 3, 166667, },
	/* 36 */  {	2880,  480, FALSE, 16, 9, 166667, },
	/* 37 */  {	2880,  576, FALSE,  4, 3, 200000, },
	/* 38 */  {	2880,  576, FALSE, 16, 9, 200000, },
	/* 39 */  {	1920, 1080,  TRUE, 16, 9, 400000, },
	/* 40 */  {	1920, 1080,  TRUE, 16, 9, 200000, },
	/* 41 */  {	1280,  720, FALSE, 16, 9, 100000, },
	/* 42 */  {	 720,  576, FALSE,  4, 3, 100000, },
	/* 43 */  {	 720,  576, FALSE, 16, 9, 100000, },
	/* 44 */  {	 720,  576,  TRUE,  4, 3, 200000, },
	/* 45 */  {	 720,  576,  TRUE, 16, 9, 200000, },
	/* 46 */  {	1920, 1080,  TRUE, 16, 9, 166667, },
	/* 47 */  {	1280,  720, FALSE, 16, 9,  83333, },
	/* 48 */  {	 720,  480, FALSE,  4, 3,  83333, },
	/* 49 */  {	 720,  480, FALSE, 16, 9,  83333, },
	/* 50 */  {	 720,  480,  TRUE,  4, 3, 166667, },
	/* 51 */  {	 720,  480,  TRUE, 16, 9, 166667, },
	/* 52 */  {	 720,  576, FALSE,  4, 3,  50000, },
	/* 53 */  {	 720,  576, FALSE, 16, 9,  50000, },
	/* 54 */  {	 720,  576,  TRUE,  4, 3, 100000, },
	/* 55 */  {	 720,  576,  TRUE, 16, 9, 100000, },
	/* 56 */  {	 720,  480, FALSE,  4, 3,  41667, },
	/* 57 */  {	 720,  480, FALSE, 16, 9,  41667, },
	/* 58 */  {	 720,  480,  TRUE,  4, 3,  83333, },
	/* 59 */  {	 720,  480,  TRUE, 16, 9,  83333, },
	/* 60 */  {	1280,  720, FALSE, 16, 9, 416667, },
	/* 61 */  {	1280,  720, FALSE, 16, 9, 400000, },
	/* 62 */  {	1280,  720, FALSE, 16, 9, 333333, },
	/* 63 */  {	1920, 1080, FALSE, 16, 9,  83333, },
	/* 64 */  {	1920, 1080, FALSE, 16, 9, 100000  },
};

static const HDMI_VIC_FORMAT		g_aHDMIExtVICFormats[] = {
	/*  0 */ { 	   0,    0, FALSE,  0, 0,      0,  },
	/*  1 */ {   3840, 2160, FALSE, 16, 9, 333333, },
	/*  2 */ {   3840, 2160, FALSE, 16, 9, 400000, },
	/*  3 */ {   3840, 2160, FALSE, 16, 9, 416667, },
	/*  4 */ {   4096, 2160, FALSE, 16, 9, 416667  },
};

static const BYTE g_abyDefaultYUV709VICs[] = {
    0, 1, 4, 5, 16, 19, 20, 31, 32, 33, 34, 39, 40, 46, 47, 60, 61, 62, 63, 64
};

// byRGB_YCbCr
typedef enum _HDMI_PIXEL_ENCODING {
    HDMI_ENCODING_RGB_444				= 0,
    HDMI_ENCODING_YUV_422				= 1,
    HDMI_ENCODING_YUV_444				= 2,
    HDMI_ENCODING_YUV_420				= 3
} HDMI_PXIEL_ENCODING;

typedef struct _HDMI_AVI_INFOFRAME_PAYLOAD {
	BYTE								byScanInfo : 2;
	BYTE								byBarDataPresent : 2;
	BYTE								byActiveFormatInfoPresent : 1;
	BYTE								byRGB_YCbCr : 2;
	BYTE								byFutureUseByte1 : 1;

	BYTE								byActivePortionAspectRatio : 4;
	BYTE								byCodedFrameAspectRatio : 2;
	BYTE								byColorimetry : 2;

	BYTE								byNonUniformPictureScaling : 2;
	BYTE								byRGBQuantizationRange : 2;
	BYTE								byExtendedColorimetry : 3;
	BYTE								byITContent : 1;

	BYTE								byVIC : 7;
	BYTE								byFutureUseByte4 : 1;

	BYTE								byPixelRepetitionFactor : 4;
	BYTE								byITContentType : 2;
	BYTE								byYCCQuantizationRange : 2;

	WORD								wEndOfTopBar;
	WORD								wStartOfBottomBar;
	WORD								wEndOfLeftBar;
	WORD								wStartOfRightBar;
} HDMI_AVI_INFOFRAME_PAYLOAD;

typedef struct _HDMI_HDR_INFOFRAME_PAYLOAD {
	BYTE								byEOTF;
	BYTE								byMetadataDescriptorID;

	BYTE								display_primaries_lsb_x0;
	BYTE								display_primaries_msb_x0;
	BYTE								display_primaries_lsb_y0;
	BYTE								display_primaries_msb_y0;

	BYTE								display_primaries_lsb_x1;
	BYTE								display_primaries_msb_x1;
	BYTE								display_primaries_lsb_y1;
	BYTE								display_primaries_msb_y1;

	BYTE								display_primaries_lsb_x2;
	BYTE								display_primaries_msb_x2;
	BYTE								display_primaries_lsb_y2;
	BYTE								display_primaries_msb_y2;

	BYTE								white_point_lsb_x;
	BYTE								white_point_msb_x;
	BYTE								white_point_lsb_y;
	BYTE								white_point_msb_y;

	BYTE								max_display_mastering_lsb_luminance;
	BYTE								max_display_mastering_msb_luminance;
	BYTE								min_display_mastering_lsb_luminance;
	BYTE								min_display_mastering_msb_luminance;

	BYTE								maximum_content_light_level_lsb;
	BYTE								maximum_content_light_level_msb;

	BYTE								maximum_frame_average_light_level_lsb;
	BYTE								maximum_frame_average_light_level_msb;
} HDMI_HDR_INFOFRAME_PAYLOAD;


// Audio infoframe
#define HDMI_AUDIO_CODING_TYPE_STREAM	0x00
#define HDMI_AUDIO_CODING_TYPE_PCM		0x01
#define HDMI_AUDIO_CODING_TYPE_AC3		0x02
#define HDMI_AUDIO_CODING_TYPE_MPEG1	0x03
#define HDMI_AUDIO_CODING_TYPE_MP3		0x04
#define HDMI_AUDIO_CODING_TYPE_MPEG2	0x05
#define HDMI_AUDIO_CODING_TYPE_AAC_LC	0x06
#define HDMI_AUDIO_CODING_TYPE_DTS		0x07
#define HDMI_AUDIO_CODING_TYPE_ATRAC	0x08
#define HDMI_AUDIO_CODING_TYPE_DSD		0x09
#define HDMI_AUDIO_CODING_TYPE_EAC3		0x0A
#define HDMI_AUDIO_CODING_TYPE_DTS_HD	0x0B

#define HDMI_AUDIO_SAMPLE_SIZE_STREAM	0x00
#define HDMI_AUDIO_SAMPLE_SIZE_16BIT	0x01
#define HDMI_AUDIO_SAMPLE_SIZE_20BIT	0x02
#define HDMI_AUDIO_SAMPLE_SIZE_24BIT	0x03

#define HDMI_AUDIO_SAMPLE_RATE_STREAM	0x00
#define HDMI_AUDIO_SAMPLE_RATE_32000	0x01
#define HDMI_AUDIO_SAMPLE_RATE_44100	0x02
#define HDMI_AUDIO_SAMPLE_RATE_48000	0x03
#define HDMI_AUDIO_SAMPLE_RATE_88200	0x04
#define HDMI_AUDIO_SAMPLE_RATE_96000	0x05
#define HDMI_AUDIO_SAMPLE_RATE_176400	0x06
#define HDMI_AUDIO_SAMPLE_RATE_192000	0x07

typedef struct _HDMI_AUDIO_INFOFRAME_PAYLOAD {
	BYTE								byChannelCount : 3;		// +1 for channel count
	BYTE								byReserved1 : 1;
	BYTE								byAudioCodingType : 4;

	BYTE								bySampleSize : 2;
	BYTE								bySampleFrequency : 3;
	BYTE								byReserved2 : 3;

	BYTE								byAudioCodingExtensionType : 5;
	BYTE								byReserved3 : 3;

	BYTE								byChannelAllocation;

	BYTE								byLFEPlaybackLevel : 2;
	BYTE								byReserved4 : 1;
	BYTE								byLevelShiftValue : 4;
	BYTE								byDownMixInhibitFlag : 1;
} HDMI_AUDIO_INFOFRAME_PAYLOAD;

// SPD Infoframe
#define HDMI_SPD_SORUCE_UNKOWN			0x00
#define HDMI_SPD_SORUCE_DIGITAL_STB		0x01
#define HDMI_SPD_SORUCE_DVD_PLAYER		0x02
#define HDMI_SPD_SORUCE_D_VHS			0x03
#define HDMI_SPD_SORUCE_HDD_RECORDER	0x04
#define HDMI_SPD_SORUCE_DVC				0x05
#define HDMI_SPD_SORUCE_DSC				0x06
#define HDMI_SPD_SORUCE_VIDEO_CD		0x07
#define HDMI_SPD_SORUCE_GAME			0x08
#define HDMI_SPD_SORUCE_PC_GENERAL		0x09
#define HDMI_SPD_SORUCE_BLUE_RAY_DISC	0x0A
#define HDMI_SPD_SORUCE_SUPER_AUDIO_CD	0x0B
#define HDMI_SPD_SORUCE_HD_DVD			0x0C
#define HDMI_SPD_SORUCE_PMP				0x0D

typedef struct _HDMI_SPD_INFOFRAME_PAYLOAD {
	CHAR								achVendorName[8];
	CHAR								achProductDescription[16];
	BYTE								bySourceInformation;
} HDMI_SPD_INFOFRAME_PAYLOAD;

// HDMI 1.4b VS Infoframe
#define HDMI14B_VS_REGISTRATION_ID		0x000C03

#define HDMI14B_VS_FORMAT_NONE			0x00
#define HDMI14B_VS_FORMAT_EXT_RES		0x01
#define HDMI14B_VS_FORMAT_3D_FORMAT		0x02

// by3DStructure
#define HDMI14B_3DS_FRAME_PACKING		0x00
#define HDMI14B_3DS_FIELD_ALTERNATIVE	0x01
#define HDMI14B_3DS_LINE_ALTERNATIVE	0x02
#define HDMI14B_3DS_SIDE_BY_SIDE_FULL	0x03
#define HDMI14B_3DS_TOP_AND_BOTTOM		0x06
#define HDMI14B_3DS_SIDE_BY_SIDE_HALF	0x08

// by3DExtData
#define HDMI_SUB_SAMPLING_HORIZONTAL_00			0x00
#define HDMI_SUB_SAMPLING_HORIZONTAL_01			0x01
#define HDMI_SUB_SAMPLING_HORIZONTAL_10			0x02
#define HDMI_SUB_SAMPLING_HORIZONTAL_11			0x03
#define HDMI_SUB_SAMPLING_QUINCUNX_ODD_ODD		0x04
#define HDMI_SUB_SAMPLING_QUINCUNX_ODD_EVEN		0x05
#define HDMI_SUB_SAMPLING_QUINCUNX_EVEN_ODD		0x06
#define HDMI_SUB_SAMPLING_QUINCUNX_EVEN_EVEN	0x07

typedef struct _HDMI14B_VS_DATA_EXT_RES {
	BYTE								byHDMI_VIC;
} HDMI14B_VS_DATA_EXT_RES;

typedef struct _HDMI14B_VS_DATA_3D_FORMAT {
	BYTE								byReserved1 : 3;
	BYTE								by3DMetaPresent : 1;
	BYTE								by3DStructure : 4;

	BYTE								byReserved2 : 4;
	BYTE								by3DExtData : 4;

	BYTE								by3DMetadataLength : 5;
	BYTE								by3DMetadataType : 3;
} HDMI14B_VS_DATA_3D_FORMAT;

typedef struct _HDMI14B_VS_DATA {
	BYTE								byReserved1 : 5;
	BYTE								byHDMIVideoFormat : 3;

	union {
		HDMI14B_VS_DATA_EXT_RES			vsDataExtRes;
		HDMI14B_VS_DATA_3D_FORMAT		vsData3DFormat;
	};
} HDMI14B_VS_DATA;

// Generic VS Infoframe
typedef struct _HDMI_VS_INFOFRAME_PAYLOAD {
	BYTE								abyRegistrationId[3];

	union {
		BYTE							abyVSData[24];
		HDMI14B_VS_DATA					vsDataHDMI14B;
	};
} HDMI_VS_INFOFRAME_PAYLOAD;

static inline DWORD hdmi_payload_GetRegistrationId(HDMI_VS_INFOFRAME_PAYLOAD *payload)
{
    return payload->abyRegistrationId[0] | (payload->abyRegistrationId[1] << 8)
            | (payload->abyRegistrationId[2] << 16);
}

// Generic Infoframe
#define HDMI_INFOFRAME_TYPE_VS			0x81
#define HDMI_INFOFRAME_TYPE_AVI			0x82
#define HDMI_INFOFRAME_TYPE_SPD			0x83
#define HDMI_INFOFRAME_TYPE_AUDIO		0x84
#define HDMI_INFOFRAME_TYPE_MS			0x85
#define HDMI_INFOFRAME_TYPE_VBI			0x86
#define HDMI_INFOFRAME_TYPE_HDR			0x87


typedef struct _HDMI_INFOFRAME_HEADER {
	BYTE								byPacketType;
	BYTE								byVersion;
	BYTE								byLength : 5;
	BYTE								byReservedZero : 3;
} HDMI_INFOFRAME_HEADER;

typedef struct _HDMI_INFOFRAME_PACKET {
	HDMI_INFOFRAME_HEADER				header;
	BYTE								byChecksum;

	union {
		BYTE							abyPayload[27];
		HDMI_AVI_INFOFRAME_PAYLOAD		aviInfoFramePayload;
		HDMI_AUDIO_INFOFRAME_PAYLOAD	audioInfoFramePayload;
		HDMI_SPD_INFOFRAME_PAYLOAD		spdInfoFramePayload;
		HDMI_VS_INFOFRAME_PAYLOAD		vsInfoFramePayload;
        HDMI_HDR_INFOFRAME_PAYLOAD      hdrInfoFramePayload;
	};
} HDMI_INFOFRAME_PACKET;

static inline BOOLEAN hdmi_info_IsValid(HDMI_INFOFRAME_PACKET *info)
{
    unsigned char *pbyData = (unsigned char *)&info->header;
    BYTE cbData = info->header.byLength + sizeof(info->header) + 1;

    BYTE bySum = 0;
    while (cbData-- != 0)
        bySum += *pbyData++;

    return (bySum == 0);
}

#pragma pack(pop)

