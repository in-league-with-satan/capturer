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

#include <mutex>

#include "ff_tools.h"

int ff_lock_callback(void **mutex, enum AVLockOp op);

void av_log_callback(void*, int level, const char *fmt, va_list vl)
{
    // if(level>AV_LOG_ERROR)
    //     return;

    // if(level>AV_LOG_WARNING)
    //     return;

    if(level>AV_LOG_INFO)
        return;

    if(!fmt)
        return;

    qInfo().noquote() << "#" << QString().vsprintf(fmt, vl).remove("\n");
}

void initLibAV()
{
    av_register_all();
    avdevice_register_all();

    // av_lockmgr_register(&ff_lock_callback);

    av_log_set_callback(av_log_callback);
    // av_log_set_level(AV_LOG_ERROR);
}

QString ffErrorString(int code)
{
    char buf[AV_ERROR_MAX_STRING_SIZE];

    av_strerror(code, buf, AV_ERROR_MAX_STRING_SIZE);

    return QString(buf);
}

AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height, bool alloc_buffer)
{
    AVFrame *av_frame;
    int ret;

    av_frame=av_frame_alloc();

    if(!av_frame)
        return nullptr;

    av_frame->format=pix_fmt;
    av_frame->width=width;
    av_frame->height=height;

    // allocate the buffers for the frame data
    if(alloc_buffer) {
        // ret=av_frame_get_buffer(av_frame, alignment);
        ret=av_frame_get_buffer(av_frame, 0);

        if(ret<0) {
            qCritical() << "Could not allocate frame data";
            return nullptr;
        }
    }

    return av_frame;
}

AVPixelFormat correctPixelFormat(AVPixelFormat fmt)
{
    switch(fmt) {
    case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
    case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
    default: return fmt;
    }

    return fmt;
}

QString versionlibavutil()
{
    static QString ver=QString("%1.%2.%3").arg(LIBAVUTIL_VERSION_MAJOR).arg(LIBAVUTIL_VERSION_MINOR).arg(LIBAVUTIL_VERSION_MICRO);
    return ver;
}

QString versionlibavcodec()
{
    static QString ver=QString("%1.%2.%3").arg(LIBAVCODEC_VERSION_MAJOR).arg(LIBAVCODEC_VERSION_MINOR).arg(LIBAVCODEC_VERSION_MICRO);
    return ver;
}

QString versionlibavformat()
{
    static QString ver=QString("%1.%2.%3").arg(LIBAVFORMAT_VERSION_MAJOR).arg(LIBAVFORMAT_VERSION_MINOR).arg(LIBAVFORMAT_VERSION_MICRO);
    return ver;
}

QString versionlibavfilter()
{
    static QString ver=QString("%1.%2.%3").arg(LIBAVFILTER_VERSION_MAJOR).arg(LIBAVFILTER_VERSION_MINOR).arg(LIBAVFILTER_VERSION_MICRO);
    return ver;
}

QString versionlibswscale()
{
    static QString ver=QString("%1.%2.%3").arg(LIBSWSCALE_VERSION_MAJOR).arg(LIBSWSCALE_VERSION_MINOR).arg(LIBSWSCALE_VERSION_MICRO);
    return ver;
}

QString versionlibswresample()
{
    static QString ver=QString("%1.%2.%3").arg(LIBSWRESAMPLE_VERSION_MAJOR).arg(LIBSWRESAMPLE_VERSION_MINOR).arg(LIBSWRESAMPLE_VERSION_MICRO);
    return ver;
}

bool operator==(const AVRational &l, const AVRational &r)
{
    return l.den==r.den && l.num==r.num;
}

bool checkEncoder(const QString &encoder_name, const uint64_t &pixel_format)
{
    return checkEncoder(encoder_name, (AVPixelFormat)pixel_format);
}

bool checkEncoder(const QString &encoder_name, const AVPixelFormat &pixel_format)
{
    bool result=false;

    AVCodec *codec=
            avcodec_find_encoder_by_name(encoder_name.toLatin1().constData());

    AVCodecContext *codec_context=nullptr;

    int ret=0;

    if(!codec)
        goto exit;

    codec_context=avcodec_alloc_context3(codec);

    if(!codec_context)
        goto exit;

    codec_context->width=640;
    codec_context->height=480;
    codec_context->time_base={ 1000, 15000 };
    codec_context->pix_fmt=pixel_format;

    ret=avcodec_open2(codec_context, codec, nullptr);

    if(ret==0) {
        result=true;

    } else {
        // qCritical() << ffErrorString(ret);
    }

exit:
    if(codec_context)
        avcodec_free_context(&codec_context);

    return result;
}

bool isHighBitDepthBuild()
{
    static bool checked=false;
    static bool result=false;

    if(!checked) {
        checked=true;

        result=checkEncoder("libx264", AV_PIX_FMT_YUV420P10);
    }

    return result;
}

int ff_lock_callback(void **mutex, enum AVLockOp op)
{
    qInfo() << "ff_lock_callback" << op;

    static std::mutex m;

    switch(op) {
    case AV_LOCK_CREATE:
        *mutex=&m;
        break;

    case AV_LOCK_OBTAIN:
        ((std::mutex*)(*mutex))->lock();
        break;

    case AV_LOCK_RELEASE:
        ((std::mutex*)(*mutex))->unlock();
        break;

    case AV_LOCK_DESTROY:
        *mutex=0;
        break;
    }

    return 0;
}

