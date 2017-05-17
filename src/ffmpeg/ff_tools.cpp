#include <QDebug>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include "libswscale/swscale.h"
}

#include "ff_tools.h"


QString ffErrorString(int code)
{
    char buf[AV_ERROR_MAX_STRING_SIZE];

    av_strerror(code, buf, AV_ERROR_MAX_STRING_SIZE);

    return QString(buf);
}

AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height)
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
    ret=av_frame_get_buffer(av_frame, alignment);

    if(ret<0) {
        qCritical() << "Could not allocate frame data";
        return nullptr;
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
    }

    return fmt;
}

