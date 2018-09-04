////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "LibMWCapture/WinTypes.h"

#ifndef MWFOURCC
#define MWFOURCC(ch0, ch1, ch2, ch3)											\
        ((DWORD)(BYTE)(ch0) | ((DWORD)(BYTE)(ch1) << 8) |							\
        ((DWORD)(BYTE)(ch2) << 16) | ((DWORD)(BYTE)(ch3) << 24))
#endif

#define MWFOURCC_UNK		MWFOURCC('U', 'N', 'K', 'N')

// 8bits grey
#define MWFOURCC_GREY		MWFOURCC('G', 'R', 'E', 'Y')					// Y0, Y1, Y2, ...
#define MWFOURCC_Y800		MWFOURCC('Y', '8', '0', '0')					// = GREY
#define MWFOURCC_Y8			MWFOURCC('Y', '8', ' ', ' ')					// = GREY

// 16bits grey
#define MWFOURCC_Y16		MWFOURCC('Y', '1', '6', ' ')					// Y0, Y1, Y2

// RGB 15-32bits
#define MWFOURCC_RGB15		MWFOURCC('R', 'G', 'B', '5')					// R0, G0, B0, A0, ...
#define MWFOURCC_RGB16		MWFOURCC('R', 'G', 'B', '6')					// R0, G0, B0, R1, ...
#define MWFOURCC_RGB24		MWFOURCC('R', 'G', 'B', ' ')					// R0, G0, B0, R1, ...
#define MWFOURCC_RGBA		MWFOURCC('R', 'G', 'B', 'A')					// R0, G0, B0, A0, R1, ...
#define MWFOURCC_ARGB		MWFOURCC('A', 'R', 'G', 'B')					// A0, R0, G0, B0, A1, ...

#define MWFOURCC_BGR15		MWFOURCC('B', 'G', 'R', '5')					// B0, G0, R0, A0, ...
#define MWFOURCC_BGR16		MWFOURCC('B', 'G', 'R', '6')					// B0, G0, R0, B1, ...
#define MWFOURCC_BGR24		MWFOURCC('B', 'G', 'R', ' ')					// B0, G0, R0, B1, ...
#define MWFOURCC_BGRA		MWFOURCC('B', 'G', 'R', 'A')					// B0, G0, R0, A0, B1, ...
#define MWFOURCC_ABGR		MWFOURCC('A', 'B', 'G', 'R')					// A0, B0, G0, R0, A1, ...

// Planar YUV 8bits 4:2:2 (16bits)
#define MWFOURCC_NV16		MWFOURCC('N', 'V', '1', '6')					// Y Plane, UV Plane
#define MWFOURCC_NV61		MWFOURCC('N', 'V', '6', '1')					// Y Plane, VU Plane
#define MWFOURCC_I422		MWFOURCC('I', '4', '2', '2')					// Y Plane, U Plane, V Plane
#define MWFOURCC_YV16		MWFOURCC('Y', 'V', '1', '6')					// Y Plane, V Plane, U Plane

// Packed YUV 8bits 4:2:2 (16bits)
#define MWFOURCC_YUY2       MWFOURCC('Y', 'U', 'Y', '2')					// Y0, U01, Y1, V01, ...
#define MWFOURCC_YUYV       MWFOURCC('Y', 'U', 'Y', 'V')					// = YUY2
#define MWFOURCC_UYVY       MWFOURCC('U', 'Y', 'V', 'Y')					// U01, Y0, V01, Y1, ...

#define MWFOURCC_YVYU       MWFOURCC('Y', 'V', 'Y', 'U')					// Y0, V01, Y1, U01, ...
#define MWFOURCC_VYUY       MWFOURCC('V', 'Y', 'U', 'Y')					// V01, Y0, U01, Y1, ...

// Planar YUV 8bits 4:2:0 (12bits)
#define MWFOURCC_I420       MWFOURCC('I', '4', '2', '0')					// Y Plane, U Plane, V Plane
#define MWFOURCC_IYUV       MWFOURCC('I', 'Y', 'U', 'V')					// = I420
#define MWFOURCC_NV12       MWFOURCC('N', 'V', '1', '2')					// Y Plane, UV Plane

#define MWFOURCC_YV12       MWFOURCC('Y', 'V', '1', '2')					// Y Plane, V Plane, U Plane
#define MWFOURCC_NV21       MWFOURCC('N', 'V', '2', '1')					// Y Plane, VU Plane

// Planar YUV 10bits 4:2:0 (24bits)
#define MWFOURCC_P010		MWFOURCC('P', '0', '1', '0')					// Y Plane, UV Plane

// Planar YUV 10bits 4:2:2 (32bits)
#define MWFOURCC_P210		MWFOURCC('P', '2', '1', '0')					// Y Plane, UV Plane

// Packed YUV 8bits 4:4:4 (24bits)
#define MWFOURCC_IYU2		MWFOURCC('I', 'Y', 'U', '2')					// U0, Y0, V0, U1, Y1, V1, ...
#define MWFOURCC_V308		MWFOURCC('v', '3', '0', '8')					// V0, Y0, U0, V1, Y1, U1, ...

// Packed YUV 8bits 4:4:4 (32bits)
#define MWFOURCC_AYUV		MWFOURCC('A', 'Y', 'U', 'V')					// A0, Y0, U0, V0, ...
#define MWFOURCC_UYVA		MWFOURCC('U', 'Y', 'V', 'A')					// U0, Y0, V0, A0, U1, Y1, ...
#define MWFOURCC_V408		MWFOURCC('v', '4', '0', '8')					// = MWFOURCC_UYVA
#define MWFOURCC_VYUA		MWFOURCC('V', 'Y', 'U', 'A')					// V0, Y0, U0, A0, V1, Y1, ...

// Packed YUV 10bits 4:4:4 (32bits)
#define MWFOURCC_Y410		MWFOURCC('Y', '4', '1', '0')					// U0, Y0, V0, A0, ...
#define MWFOURCC_V410		MWFOURCC('v', '4', '1', '0')					// A0, U0, Y0, V0, ...

// Packed RGB 10bits 4:4:4 (32bits)
#define MWFOURCC_RGB10		MWFOURCC('R', 'G', '1', '0')					// R0, G0, B0, A0, ...
#define MWFOURCC_BGR10		MWFOURCC('B', 'G', '1', '0')					// B0, G0, R0, A0, ...

static inline BOOLEAN FOURCC_IsRGB(
	DWORD dwFOURCC
	)
{
	switch (dwFOURCC) {
	case MWFOURCC_RGB15:
	case MWFOURCC_BGR15:
	case MWFOURCC_RGB16:
	case MWFOURCC_BGR16:
	case MWFOURCC_RGB24:
	case MWFOURCC_BGR24:
	case MWFOURCC_RGBA:
	case MWFOURCC_BGRA:
	case MWFOURCC_ARGB:
	case MWFOURCC_ABGR:
	case MWFOURCC_RGB10:
	case MWFOURCC_BGR10:
		return TRUE;
	default:
		return FALSE;
	}
}

static inline BOOLEAN FOURCC_IsPacked(
        DWORD dwFOURCC
        )
{
    switch (dwFOURCC) {
        case MWFOURCC_NV12:
        case MWFOURCC_NV21:
        case MWFOURCC_YV12:
        case MWFOURCC_IYUV:
        case MWFOURCC_I420:
        case MWFOURCC_I422:
        case MWFOURCC_YV16:
        case MWFOURCC_NV16:
        case MWFOURCC_NV61:
        case MWFOURCC_P010:
        case MWFOURCC_P210:
            return FALSE;
        default:
            return TRUE;
    }
}

static inline int FOURCC_GetBpp(
        DWORD dwFOURCC
        )
{
    switch (dwFOURCC) {
        case MWFOURCC_GREY:
        case MWFOURCC_Y800:
        case MWFOURCC_Y8:
            return 8;

        case MWFOURCC_I420:
        case MWFOURCC_IYUV:
        case MWFOURCC_YV12:
        case MWFOURCC_NV12:
        case MWFOURCC_NV21:
            return 12;

        case MWFOURCC_Y16:
        case MWFOURCC_RGB15:
        case MWFOURCC_BGR15:
        case MWFOURCC_RGB16:
        case MWFOURCC_BGR16:
        case MWFOURCC_YUY2:
        case MWFOURCC_YUYV:
        case MWFOURCC_UYVY:
        case MWFOURCC_YVYU:
        case MWFOURCC_VYUY:
        case MWFOURCC_I422:
        case MWFOURCC_YV16:
        case MWFOURCC_NV16:
        case MWFOURCC_NV61:
            return 16;

        case MWFOURCC_IYU2:
        case MWFOURCC_V308:
        case MWFOURCC_RGB24:
        case MWFOURCC_BGR24:
        case MWFOURCC_P010:
            return 24;

        case MWFOURCC_AYUV:
        case MWFOURCC_UYVA:
        case MWFOURCC_V408:
        case MWFOURCC_VYUA:
        case MWFOURCC_RGBA:
        case MWFOURCC_BGRA:
        case MWFOURCC_ARGB:
        case MWFOURCC_ABGR:
        case MWFOURCC_Y410:
        case MWFOURCC_V410:
        case MWFOURCC_P210:
            return 32;

        default:
            return 0;
    }
}

static inline DWORD FOURCC_CalcMinStride(
	DWORD dwFOURCC,
	int cx,
	DWORD dwAlign
	)
{
    BOOLEAN bPacked = FOURCC_IsPacked(dwFOURCC);

    DWORD cbLine;

    if (bPacked) {
        int nBpp = FOURCC_GetBpp(dwFOURCC);
        cbLine = (cx * nBpp) / 8;
    } else {
        switch (dwFOURCC) {
            case MWFOURCC_P010:
            case MWFOURCC_P210:
                cbLine = cx * 2;
                break;
            default:
                cbLine = cx;
                break;
        }
    }

    return (cbLine + dwAlign - 1) & ~(dwAlign - 1);
}

static inline DWORD FOURCC_CalcImageSize(
	DWORD dwFOURCC,
	int cx,
	int cy,
	DWORD cbStride
	)
{
    BOOLEAN bPacked = FOURCC_IsPacked(dwFOURCC);

    if (bPacked) {
        int nBpp = FOURCC_GetBpp(dwFOURCC);
        DWORD cbLine = (cx * nBpp) / 8;
        if (cbStride < cbLine)
            return 0;

        return cbStride * cy;
    }
    else {
        if (cbStride < (DWORD)cx)
            return 0;

        switch (dwFOURCC) {
            case MWFOURCC_NV12:
            case MWFOURCC_NV21:
            case MWFOURCC_YV12:
            case MWFOURCC_IYUV:
            case MWFOURCC_I420:
                if ((cbStride & 1) || (cy & 1))
                    return 0;
                return cbStride * cy * 3 / 2;
            case MWFOURCC_I422:
            case MWFOURCC_YV16:
            case MWFOURCC_NV16:
            case MWFOURCC_NV61:
                if (cbStride & 1)
                    return 0;
                return cbStride * cy * 2;
            case MWFOURCC_P010:
                if ((cbStride & 3) || (cy & 1))
                    return 0;
                return cbStride * cy * 3 / 2;
            case MWFOURCC_P210:
                if (cbStride & 3)
                    return 0;
                return cbStride * cy * 2;

            default:
                return 0;
        }
    }
}

#pragma pack(push, 1)
typedef struct tagKS_BITMAPINFOHEADER {
  DWORD biSize;
  int   biWidth;
  int   biHeight;
  WORD  biPlanes;
  WORD  biBitCount;
  DWORD biCompression;
  DWORD biSizeImage;
  int   biXPelsPerMeter;
  int   biYPelsPerMeter;
  DWORD biClrUsed;
  DWORD biClrImportant;
} KS_BITMAPINFOHEADER, *PKS_BITMAPINFOHEADER;
#pragma pack(pop)
#define KS_BI_RGB           0L
#define KS_BI_RLE8          1L
#define KS_BI_RLE4          2L
#define KS_BI_BITFIELDS     3L
#define MWCAP_BITMAPINFOHEADER	KS_BITMAPINFOHEADER
#define MWCAP_BI_RGB			KS_BI_RGB
#define MWCAP_BI_BITFIELDS		KS_BI_BITFIELDS

static inline BOOLEAN FOURCC_IsMask(const DWORD * pdwMasks, DWORD dwRedMask, DWORD dwGreenMask, DWORD dwBlueMask)
{
	return ((pdwMasks[0] == dwRedMask) && (pdwMasks[1] == dwGreenMask) && (pdwMasks[2] == dwBlueMask));
}

static inline DWORD FOURCC_GetFromBitmapHeader(
	DWORD biCompression,
	WORD biBitCount,
	DWORD * pdwMasks
	)
{
	switch (biCompression) {
	case MWCAP_BI_RGB:
		switch (biBitCount) {
		case 16:
			return MWFOURCC_BGR15;
		case 24:
			return MWFOURCC_BGR24;
		case 32:
			return MWFOURCC_BGRA;
		default:
			return MWFOURCC_UNK;
		}
		break;

	case MWCAP_BI_BITFIELDS:
	{
		switch (biBitCount) {
		case 16:
			if (FOURCC_IsMask(pdwMasks, 0x0000F800, 0x000007E0, 0x0000001F))
				return MWFOURCC_BGR16;
			else if (FOURCC_IsMask(pdwMasks, 0x0000001F, 0x000007E0, 0x0000F800))
				return MWFOURCC_RGB16;
			else if (FOURCC_IsMask(pdwMasks, 0x00007C00, 0x000003E0, 0x0000001F))
				return MWFOURCC_BGR15;
			else if (FOURCC_IsMask(pdwMasks, 0x0000001F, 0x000003E0, 0x00007C00))
				return MWFOURCC_RGB15;
			else
				return MWFOURCC_UNK;

		case 24:
			if (FOURCC_IsMask(pdwMasks, 0x00FF0000, 0x0000FF00, 0x000000FF))
				return MWFOURCC_BGR24;
			else if (FOURCC_IsMask(pdwMasks, 0x000000FF, 0x0000FF00, 0x00FF0000))
				return MWFOURCC_RGB24;
			else
				return MWFOURCC_UNK;

		case 32:
			if (FOURCC_IsMask(pdwMasks, 0x00FF0000, 0x0000FF00, 0x000000FF))
				return MWFOURCC_BGRA;
			else if (FOURCC_IsMask(pdwMasks, 0x000000FF, 0x0000FF00, 0x00FF0000))
				return MWFOURCC_RGBA;
			else if (FOURCC_IsMask(pdwMasks, 0xFF000000, 0x00FF0000, 0x0000FF00))
				return MWFOURCC_ABGR;
			else if (FOURCC_IsMask(pdwMasks, 0x0000FF00, 0x00FF0000, 0xFF000000))
				return MWFOURCC_ARGB;
			else
				return MWFOURCC_UNK;

		default:
			return MWFOURCC_UNK;
		}
	}
	break;

	default:
		return biCompression;
	}
}

static inline DWORD FOURCC_GetFromBitmapHeader2(
	const MWCAP_BITMAPINFOHEADER * pbmih
	)
{
	DWORD *pdwMasks = (DWORD *)(pbmih + 1);
	return FOURCC_GetFromBitmapHeader(pbmih->biCompression, pbmih->biBitCount, pdwMasks);
}

