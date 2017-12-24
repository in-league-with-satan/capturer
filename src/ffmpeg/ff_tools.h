#ifndef FF_TOOLS_H
#define FF_TOOLS_H

#include <QString>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
}

const int alignment=32;

QString ffErrorString(int code);
AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height, bool alloc_buffer=true);
AVPixelFormat correctPixelFormat(AVPixelFormat fmt);

static QString versionlibavutil()
{
    return QString("%1.%2.%3").arg(LIBAVUTIL_VERSION_MAJOR).arg(LIBAVUTIL_VERSION_MINOR).arg(LIBAVUTIL_VERSION_MICRO);
}

static QString versionlibavcodec()
{
    return QString("%1.%2.%3").arg(LIBAVCODEC_VERSION_MAJOR).arg(LIBAVCODEC_VERSION_MINOR).arg(LIBAVCODEC_VERSION_MICRO);
}

static QString versionlibavformat()
{
    return QString("%1.%2.%3").arg(LIBAVFORMAT_VERSION_MAJOR).arg(LIBAVFORMAT_VERSION_MINOR).arg(LIBAVFORMAT_VERSION_MICRO);
}

static QString versionlibavfilter()
{
    return QString("%1.%2.%3").arg(LIBAVFILTER_VERSION_MAJOR).arg(LIBAVFILTER_VERSION_MINOR).arg(LIBAVFILTER_VERSION_MICRO);
}

static QString versionlibswscale()
{
    return QString("%1.%2.%3").arg(LIBSWSCALE_VERSION_MAJOR).arg(LIBSWSCALE_VERSION_MINOR).arg(LIBSWSCALE_VERSION_MICRO);
}

static QString versionlibswresample()
{
    return QString("%1.%2.%3").arg(LIBSWRESAMPLE_VERSION_MAJOR).arg(LIBSWRESAMPLE_VERSION_MINOR).arg(LIBSWRESAMPLE_VERSION_MICRO);
}

bool operator==(const AVRational &l, const AVRational &r);

#endif // FF_TOOLS_H
