/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>

#ifdef __linux__

#include <linux/videodev2.h>

#endif

#include "DeckLinkAPI.h"
#include "decklink_video_frame.h"

#include "pixel_format.h"

PixelFormat::PixelFormat()
{
    ;
}

PixelFormat::PixelFormat(const int &value)
{
    if(isValid(value))
        d=(PixelFormat::T)value;
}

PixelFormat::PixelFormat(const AVPixelFormat &value)
{
    fromAVPixelFormat(value);
}

QList <PixelFormat> PixelFormat::list()
{
    static QList <PixelFormat> res;

    if(res.isEmpty()) {
        for(int i=undefined + 1; i<size; ++i)
            res << i;
    }

    return res;
}

AVPixelFormat PixelFormat::toAVPixelFormat() const
{
    switch((uint32_t)d) {
    case rgb24:
        return AV_PIX_FMT_RGB24;

    case bgr24:
        return AV_PIX_FMT_BGR24;

    case rgb0:
        return AV_PIX_FMT_RGB0;

    case bgr0:
        return AV_PIX_FMT_BGR0;

    case bgra:
        return AV_PIX_FMT_BGRA;

    case gbrp10le:
        return AV_PIX_FMT_GBRP10LE;

    case rgb48le:
        return AV_PIX_FMT_RGB48LE;

    case yuv420p:
        return AV_PIX_FMT_YUV420P;

    case yuv420p10:
        return AV_PIX_FMT_YUV420P10;

    case yuv422p:
        return AV_PIX_FMT_YUV422P;

    case yuyv422:
        return AV_PIX_FMT_YUYV422;

    case uyvy422:
        return AV_PIX_FMT_UYVY422;

    case yuv444p:
        return AV_PIX_FMT_YUV444P;

    case yuv422p10le:
        return AV_PIX_FMT_YUV422P10LE;

    case yuv444p10:
        return AV_PIX_FMT_YUV444P10;

    case yuv444p16le:
        return AV_PIX_FMT_YUV444P16LE;

    case p010le:
        return AV_PIX_FMT_P010LE;

    case nv12:
        return AV_PIX_FMT_NV12;

    case mjpeg:
        return AV_PIX_FMT_YUV422P;
        return AV_PIX_FMT_NONE;
    }

    return AV_PIX_FMT_NONE;
}

bool PixelFormat::fromAVPixelFormat(AVPixelFormat value)
{
    switch((uint32_t)value) {
    case AV_PIX_FMT_RGB24:
        d=rgb24;
        return true;

    case AV_PIX_FMT_BGR24:
        d=bgr24;
        return true;

    case AV_PIX_FMT_RGB0:
        d=rgb0;
        return true;

    case AV_PIX_FMT_BGR0:
        d=bgr0;
        return true;

    case AV_PIX_FMT_BGRA:
        d=bgra;
        return true;

    case AV_PIX_FMT_GBRP10LE:
        d=gbrp10le;
        return true;

    case AV_PIX_FMT_RGB48LE:
        d=rgb48le;
        return true;

    case AV_PIX_FMT_YUV420P:
        d=yuv420p;
        return true;

    case AV_PIX_FMT_YUV420P10:
        d=yuv420p10;
        return true;

    case AV_PIX_FMT_YUV422P:
    case AV_PIX_FMT_YUVJ422P:
        d=yuv422p;
        return true;

    case AV_PIX_FMT_YUYV422:
        d=yuyv422;
        return true;

    case AV_PIX_FMT_UYVY422:
        d=uyvy422;
        return true;

    case AV_PIX_FMT_YUV444P:
        d=yuv444p;
        return true;

    case AV_PIX_FMT_YUV422P10LE:
        d=yuv422p10le;
        return true;

    case AV_PIX_FMT_YUV444P10:
        d=yuv444p10;
        return true;

    case AV_PIX_FMT_YUV444P16LE:
        d=yuv444p16le;
        return true;

    case AV_PIX_FMT_P010LE:
        d=p010le;
        return true;

    case AV_PIX_FMT_NV12:
        d=nv12;
        return true;
    }

    qCritical() << "unhandled format:" << value << QString(av_get_pix_fmt_name(value));

    return false;
}

uint32_t PixelFormat::toV4L2PixelFormat() const
{
#ifdef __linux__

    switch((uint32_t)d) {
    case rgb24:
        return V4L2_PIX_FMT_RGB24;

    case bgr24:
        return V4L2_PIX_FMT_BGR24;

    case rgb0:
        return V4L2_PIX_FMT_RGB32;

    case bgr0:
        return V4L2_PIX_FMT_BGR32;

    case bgra:
        return V4L2_PIX_FMT_ABGR32;

    case gbrp10le:
        return 0;

    case rgb48le:
        return 0;

    case yuv420p:
        return V4L2_PIX_FMT_YUV420;

    case yuv420p10:
        return AV_PIX_FMT_YUV420P10;

    case yuv422p:
        return 0;

    case yuyv422:
        return V4L2_PIX_FMT_YUYV;

    case uyvy422:
        return V4L2_PIX_FMT_UYVY;

    case yuv444p:
        return V4L2_PIX_FMT_YUV444;

    case yuv422p10le:
        return 0;

    case yuv444p10:
        return 0;

    case yuv444p16le:
        return 0;

    case p010le:
        return 0;

    case nv12:
        return V4L2_PIX_FMT_NV12;

    case mjpeg:
        return V4L2_PIX_FMT_MJPEG;
    }

#endif

    return 0;
}

bool PixelFormat::fromV4L2PixelFormat(uint32_t value)
{
#ifdef __linux__

    switch(value) {
    case V4L2_PIX_FMT_RGB24:
        d=rgb24;
        return true;

    case V4L2_PIX_FMT_BGR24:
        d=bgr24;
        return true;

    case V4L2_PIX_FMT_RGB32:
        d=rgb0;
        return true;

    case V4L2_PIX_FMT_BGR32:
        d=bgr0;
        return true;

    case V4L2_PIX_FMT_ABGR32:
        d=bgra;
        return true;

    case V4L2_PIX_FMT_YUV420:
        d=yuv420p;
        return true;

    case AV_PIX_FMT_YUV420P10:
        d=yuv420p10;
        return true;

    case V4L2_PIX_FMT_YUYV:
        d=yuyv422;
        return true;

    case V4L2_PIX_FMT_UYVY:
        d=uyvy422;
        return true;

    case V4L2_PIX_FMT_YUV444:
        d=yuv444p;
        return true;

    case V4L2_PIX_FMT_NV12:
        d=nv12;
        return true;

    case V4L2_PIX_FMT_MJPEG:
        d=mjpeg;
        return true;
    }

#endif

    return false;
}

uint32_t PixelFormat::toBMDPixelFormat() const
{
    switch((uint32_t)d) {
    case bgra:
        return bmdFormat8BitBGRA; // bmdFormat8BitARGB

    case gbrp10le:
        return bmdFormat10BitRGB;

    case uyvy422:
        return bmdFormat8BitYUV;

    case yuv422p10le:
        return bmdFormat10BitYUV;
    }

    return 0;
}

bool PixelFormat::fromBMDPixelFormat(uint32_t value)
{
    switch(value) {
    case bmdFormat8BitBGRA:
        d=bgra;
        return true;

    case bmdFormat10BitRGB:
        d=gbrp10le;
        return true;

    case bmdFormat8BitYUV:
        d=uyvy422;
        return true;

    case bmdFormat10BitYUV:
        d=yuv422p10le;
        return true;
    }

    return false;
}

QVideoFrame::PixelFormat PixelFormat::toQPixelFormat() const
{
    switch((uint32_t)d) {
    case rgb24:
        return QVideoFrame::Format_RGB24;

    case bgr24:
        return QVideoFrame::Format_BGR24;

    case rgb0:
        return QVideoFrame::Format_RGB32;

    case bgr0:
        return QVideoFrame::Format_BGR32;

    case bgra:
        return QVideoFrame::Format_ARGB32;
        // return QVideoFrame::Format_BGRA32;

    // case gbrp10le:

    // case rgb48le:

    case yuv420p:
        return QVideoFrame::Format_YUV420P;

    // case yuv420p10:

    case yuv422p:
        return QVideoFrame::Format_YUV420P;

    // case yuyv422:

    case yuyv422:
        return QVideoFrame::Format_YUYV;

    case uyvy422:
        return QVideoFrame::Format_UYVY;

    case yuv444p:
        return QVideoFrame::Format_YUV444;

    // case yuv422p10le:

    // case yuv444p10:

    // case yuv444p16le:

    // case p010le:

    case nv12:
        return QVideoFrame::Format_NV12;

    case mjpeg:
        return QVideoFrame::Format_Jpeg;
    }

    return QVideoFrame::Format_Invalid;
}

bool PixelFormat::fromQPixelFormat(QVideoFrame::PixelFormat value)
{
    switch((uint32_t)value) {
    case QVideoFrame::Format_RGB24:
        d=rgb24;
        return true;

    case QVideoFrame::Format_BGR24:
        d=bgr24;
        return true;

    case QVideoFrame::Format_RGB32:
        d=rgb0;
        return true;

    case QVideoFrame::Format_BGR32:
        d=bgr0;
        return true;

    case QVideoFrame::Format_ARGB32:
    case QVideoFrame::Format_BGRA32:
        d=bgra;
        return true;

    case QVideoFrame::Format_YUV420P:
        d=yuv420p;
        return true;

    case QVideoFrame::Format_YUYV:
        d=yuyv422;
        return true;

    case QVideoFrame::Format_UYVY:
        d=uyvy422;
        return true;

    case QVideoFrame::Format_YUV444:
        d=yuv444p;
        return true;

    case QVideoFrame::Format_NV12:
        d=nv12;
        return true;

    case QVideoFrame::Format_Jpeg:
        d=mjpeg;
        return true;
    }

    return false;
}

#ifdef __WIN32__

GUID PixelFormat::toDshowPixelFormat() const
{
    switch((uint32_t)d) {
    case rgb24:
        return MEDIASUBTYPE_RGB24;

    case rgb0:
        return MEDIASUBTYPE_RGB32;

    case yuv420p:
        return MEDIASUBTYPE_IYUV;

    case yuyv422:
        return MEDIASUBTYPE_YUY2;
        return MEDIASUBTYPE_YUYV;

    case uyvy422:
        return MEDIASUBTYPE_UYVY;

    case nv12:
        return MEDIASUBTYPE_NV12;

    case mjpeg:
        return MEDIASUBTYPE_MJPG;
    }

    return MEDIASUBTYPE_None;
}

bool PixelFormat::fromDshowPixelFormat(const GUID &value)
{
    if(IsEqualGUID(value, MEDIASUBTYPE_RGB24)) {
        d=rgb24;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_RGB32)) {
        d=rgb0;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_IYUV)) {
        d=yuv420p;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_YUY2)) {
        d=yuyv422;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_YUYV)) {
        d=yuyv422;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_UYVY)) {
        d=uyvy422;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_NV12)) {
        d=nv12;
        return true;

    } else if(IsEqualGUID(value, MEDIASUBTYPE_MJPG)) {
        d=mjpeg;
        return true;
    }

    return false;
}

#endif

QString PixelFormat::toString(int value)
{
    switch(value) {
    case rgb24:
        return QStringLiteral("rgb24");

    case bgr24:
        return QStringLiteral("bgr24");

    case rgb0:
        return QStringLiteral("rgb0");

    case bgr0:
        return QStringLiteral("bgr0");

    case bgra:
        return QStringLiteral("bgra");

    case gbrp10le:
        return QStringLiteral("gbrp10le");

    case rgb48le:
        return QStringLiteral("rgb48le");

    case yuv420p:
        return QStringLiteral("yuv420p");

    case yuv420p10:
        return QStringLiteral("yuv420p10");

    case yuv422p:
        return QStringLiteral("yuv422p");

    case yuyv422:
        return QStringLiteral("yuyv422");

    case uyvy422:
        return QStringLiteral("uyvy422");

    case yuv444p:
        return QStringLiteral("yuv444p");

    case yuv422p10le:
        return QStringLiteral("yuv422p10le");

    case yuv444p10:
        return QStringLiteral("yuv444p10");

    case yuv444p16le:
        return QStringLiteral("yuv444p16le");

    case p010le:
        return QStringLiteral("p010le");

    case nv12:
        return QStringLiteral("nv12");

    case mjpeg:
        return QStringLiteral("mjpeg");
    }

    qWarning() << "unknown" << value;

    return QStringLiteral("unknown");
}

QString PixelFormat::toStringView(int value)
{
    switch(value) {
    case rgb24:
        return QStringLiteral("rgb24");

    case bgr24:
        return QStringLiteral("bgr24");

    case rgb0:
        return QStringLiteral("rgb0");

    case bgr0:
        return QStringLiteral("bgr0");

    case bgra:
        return QStringLiteral("bgra");

    case gbrp10le:
        return QStringLiteral("gbrp10le (r210)");

    case rgb48le:
        return QStringLiteral("rgb48le");

    case yuv420p:
        return QStringLiteral("yuv420p");

    case yuv420p10:
        return QStringLiteral("yuv420p10");

    case yuv422p:
        return QStringLiteral("yuv422p");

    case yuyv422:
        return QStringLiteral("yuyv422");

    case uyvy422:
        return QStringLiteral("uyvy422");

    case yuv444p:
        return QStringLiteral("yuv444p");

    case yuv422p10le:
        return QStringLiteral("yuv422p10le (v210)");

    case yuv444p10:
        return QStringLiteral("yuv444p10");

    case yuv444p16le:
        return QStringLiteral("yuv444p16le");

    case p010le:
        return QStringLiteral("p010le");

    case nv12:
        return QStringLiteral("nv12");

    case mjpeg:
        return QStringLiteral("mjpeg");
    }

    return QStringLiteral("unknown");
}

QString PixelFormat::toString() const
{
    return toString(d);
}

QString PixelFormat::toStringView() const
{
    return toStringView(d);
}

bool PixelFormat::fromString(const QString &value)
{
    for(int i=undefined + 1; i<size; ++i) {
        if(value==toString(i)) {
            d=(PixelFormat::T)i;
            return true;
        }
    }

    return false;
}

bool PixelFormat::isValid() const
{
    return d!=undefined;
}

bool PixelFormat::isValid(int value)
{
    for(int i=undefined + 1; i<size; ++i)
        if(value==i)
            return true;

    return false;
}

bool PixelFormat::isRgb() const
{
    return d==rgb24
            || d==bgr24
            || d==rgb0
            || d==bgr0
            || d==bgra
            || d==gbrp10le
            || d==rgb48le;
}

bool PixelFormat::is10bit() const
{
    return d==gbrp10le
            || d==yuv420p10
            || d==yuv422p10le
            || d==yuv444p10
            || d==yuv444p16le
            || d==p010le;
}

bool PixelFormat::is210() const
{
    return d==gbrp10le
            || d==yuv422p10le;
}

bool PixelFormat::isDirect() const
{
    return d==rgb24
            || d==bgr24
            || d==rgb0
            || d==bgr0
            || d==bgra
            || d==yuv420p
            || d==yuv422p
            || d==yuyv422
            || d==uyvy422
            || d==yuv444p
            || d==nv12;
}

PixelFormat &PixelFormat::operator=(int other)
{
    if(isValid(other))
        d=(PixelFormat::T)other;

    return (*this);
}

PixelFormat::operator int() const
{
    return (int)d;
}

int frameBufSize(QSize size, PixelFormat pixel_format)
{
    if(!pixel_format.isValid())
        return 0;

    if(pixel_format.toBMDPixelFormat()!=0)
        return DeckLinkVideoFrame::frameSize(size, (BMDPixelFormat)pixel_format.toBMDPixelFormat());

    const AVPixelFormat av_pix_fmt=pixel_format.toAVPixelFormat();

    if(av_pix_fmt!=AV_PIX_FMT_NONE)
        return av_image_get_buffer_size(av_pix_fmt, size.width(), size.height(), alignment);

    return 0;
}
