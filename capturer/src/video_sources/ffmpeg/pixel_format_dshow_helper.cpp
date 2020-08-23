/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>

#include "pixel_format.h"

#include "pixel_format_dshow_helper.h"

#ifdef __WIN32__

#include <initguid.h>

DEFINE_GUID(MEDIASUBTYPE_H264_alt, 0x34363248, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);
DEFINE_GUID(MEDIASUBTYPE_I420_alt, 0x30323449, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

GUID toDshowPixelFormat(uint32_t pix_fmt)
{
    switch(pix_fmt) {
    case PixelFormat::rgb24:
        return MEDIASUBTYPE_RGB24;

    case PixelFormat::rgb0:
        return MEDIASUBTYPE_RGB32;

    case PixelFormat::yuv420p:
        return MEDIASUBTYPE_IYUV;

    case PixelFormat::yuyv422:
        return MEDIASUBTYPE_YUY2;
        return MEDIASUBTYPE_YUYV;

    case PixelFormat::uyvy422:
        return MEDIASUBTYPE_UYVY;

    case PixelFormat::nv12:
        return MEDIASUBTYPE_NV12;

    case PixelFormat::mjpeg:
        return MEDIASUBTYPE_MJPG;
    }

    return MEDIASUBTYPE_None;
}

uint32_t fromDshowPixelFormat(const GUID &value)
{
    if(IsEqualGUID(value, MEDIASUBTYPE_RGB24)) {
        return PixelFormat::rgb24;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_RGB32)) {
        return PixelFormat::rgb0;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_I420_alt)) {
        return PixelFormat::yuv420p;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_IYUV)) {
        return PixelFormat::yuv420p;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_YV12)) {
        return PixelFormat::yvu420p;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_YUY2)) {
        return PixelFormat::yuyv422;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_YUYV)) {
        return PixelFormat::yuyv422;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_UYVY)) {
        return PixelFormat::uyvy422;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_NV12)) {
        return PixelFormat::nv12;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_MJPG)) {
        return PixelFormat::mjpeg;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_H264_alt)) {
        return PixelFormat::h264;
    }

    return PixelFormat::undefined;
}

#endif

