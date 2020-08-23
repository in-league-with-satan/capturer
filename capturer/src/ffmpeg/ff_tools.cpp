/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

    qInfo().noquote() << "#" << QString().vasprintf(fmt, vl).remove("\n");
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
        ret=av_frame_get_buffer(av_frame, alignment);
        // ret=av_frame_get_buffer(av_frame, 0);

        if(ret<0) {
            qCritical() << "av_frame_get_buffer err:" << ffErrorString(ret);
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

bool checkEncoderVaapi(const QString &encoder_name, const AVPixelFormat &pixel_format)
{
    bool result=false;

    AVCodec *codec=
            avcodec_find_encoder_by_name(encoder_name.toLatin1().constData());


    AVCodecContext *codec_context=nullptr;
    AVBufferRef *hw_device_ctx=nullptr;
    AVBufferRef *hw_frames_ref=nullptr;
    AVHWFramesContext *frames_ctx=nullptr;

    int ret=0;

    if(!codec)
        goto exit;


    ret=av_hwdevice_ctx_create(&hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, NULL, NULL, 0);

    if(ret<0) {
        // qCritical() << "Failed to create a VAAPI device. Error code:" << ffErrorString(ret);
        goto exit;
    }

    codec_context=avcodec_alloc_context3(codec);

    if(!codec_context)
        goto exit;

    codec_context->width=640;
    codec_context->height=480;
    codec_context->time_base={ 1, 15 };
    codec_context->framerate={ 15, 1 };
    codec_context->sample_aspect_ratio={ 1, 1 };
    codec_context->pix_fmt=AV_PIX_FMT_VAAPI;

    if(!(hw_frames_ref=av_hwframe_ctx_alloc(hw_device_ctx))) {
        // qCritical() << "Failed to create VAAPI frame context";
        goto exit;
    }

    frames_ctx=(AVHWFramesContext*)(hw_frames_ref->data);
    frames_ctx->format=AV_PIX_FMT_VAAPI;
    frames_ctx->sw_format=pixel_format;
    frames_ctx->width=codec_context->width;
    frames_ctx->height=codec_context->width;
    frames_ctx->initial_pool_size=20;

    ret=av_hwframe_ctx_init(hw_frames_ref);

    if(ret<0) {
        // qCritical() << "Failed to initialize VAAPI frame context:" << ffErrorString(ret);
        goto exit;
    }

    codec_context->hw_frames_ctx=av_buffer_ref(hw_frames_ref);

    if(!codec_context->hw_frames_ctx) {
        // qCritical() << "Failed to initialize VAAPI frame context:" << ffErrorString(AVERROR(ENOMEM));
        goto exit;
    }

    ret=avcodec_open2(codec_context, codec, nullptr);

    if(ret==0) {
        result=true;

    } else {
        // qCritical() << "avcodec_open2 err:" << ffErrorString(ret);
    }

exit:
    if(hw_frames_ref)
        av_buffer_unref(&hw_frames_ref);

    if(hw_device_ctx)
        av_buffer_unref(&hw_device_ctx);

    if(codec_context)
        avcodec_free_context(&codec_context);

    return result;
}

bool checkEncoder(const QString &encoder_name, const AVPixelFormat &pixel_format)
{
    if(encoder_name.contains("vaapi", Qt::CaseInsensitive))
        return checkEncoderVaapi(encoder_name, pixel_format);

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
    codec_context->time_base={ 1, 15 };
    codec_context->framerate={ 15, 1 };
    codec_context->sample_aspect_ratio={ 1, 1 };
    codec_context->pix_fmt=pixel_format;

    ret=avcodec_open2(codec_context, codec, nullptr);

    if(ret==0) {
        result=true;

    } else {
        // qCritical() << "avcodec_open2 err:" << ffErrorString(ret);
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
    qDebug() << op;

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

QString swsColorSpace::toString(int value)
{
    switch(value) {
    case DEFAULT:
        return QStringLiteral("default");

    case ITU709:
        return QStringLiteral("ITU709");

    case BT2020:
        return QStringLiteral("BT2020");

    case FCC:
        return QStringLiteral("FCC");

    case ITU601:
        return QStringLiteral("ITU601");

    case ITU624:
        return QStringLiteral("ITU624");

    case SMPTE170M:
        return QStringLiteral("SMPTE170M");

    case SMPTE240M:
        return QStringLiteral("SMPTE240M");

    default:
        ;
    }

    return QStringLiteral("unknown");
}

int swsColorSpace::toff(int value)
{
    switch(value) {
    case DEFAULT:
        return SWS_CS_DEFAULT;

    case ITU709:
        return SWS_CS_ITU709;

    case BT2020:
        return SWS_CS_BT2020;

    case FCC:
        return SWS_CS_FCC;

    case ITU601:
        return SWS_CS_ITU601;

    case ITU624:
        return SWS_CS_ITU624;

    case SMPTE170M:
        return SWS_CS_SMPTE170M;

    case SMPTE240M:
        return SWS_CS_SMPTE240M;

    default:
        ;
    }

    return SWS_CS_DEFAULT;
}

QString swsColorRange::toString(int value)
{
    switch(value) {
    case unspecified:
        return QStringLiteral("unspecified");

    case full:
        return QStringLiteral("JPEG (full)");

    case limited:
        return QStringLiteral("MPEG (limited)");

    default:
        ;
    }

    return QStringLiteral("unknown");
}

int swsColorRange::toff(int value)
{
   switch(value) {
    case unspecified:
        return AVCOL_RANGE_UNSPECIFIED;

    case full:
        return AVCOL_RANGE_JPEG;

    case limited:
        return AVCOL_RANGE_MPEG;

    default:
        ;
    }

    return AVCOL_RANGE_UNSPECIFIED;
}
