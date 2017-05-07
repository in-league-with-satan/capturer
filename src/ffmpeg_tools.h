#ifndef FFMPEG_TOOLS_H
#define FFMPEG_TOOLS_H

#include <QDebug>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavfilter/avfilter.h>
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}

const int alignment=32;


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


/*
static inline bool byteArrayToAvFrame(QByteArray *ba_src, AVFrame *frame_dst)
{
    // const size_t data_size=frame_dst->linesize[0]*frame_dst->height;
    const int data_size=av_image_get_buffer_size((AVPixelFormat)frame_dst->format, frame_dst->width, frame_dst->height, alignment);

    if(ba_src->size()!=data_size) {
        qCritical() << "wrong bytearray size" << ba_src->size() << data_size;
        return false;
    }

    memcpy(frame_dst->data[0], ba_src->data(), data_size);

    // for(int y=0; y<frame_dst->height; y++) {
    //     memcpy(frame_dst->data[0] + y*frame_dst->linesize[0],
    //            ba_src->data() + y*frame_dst->linesize[0], frame_dst->linesize[0]*32/8);
    // }

    return true;
}

static inline bool avFrameToByteArray(AVFrame *frame_src, QByteArray *ba_dst)
{
    // const int data_size=frame_src->linesize[0]*frame_src->height;

    const int data_size=av_image_get_buffer_size((AVPixelFormat)frame_src->format, frame_src->width, frame_src->height, alignment);

    if(ba_dst->size()!=data_size)
        ba_dst->resize(data_size);

    // memcpy(ba_dst->data(), frame_src->data[0], data_size);

    for(int y=0; y<frame_src->height; y++) {
        memcpy(ba_dst->data() + y*frame_src->linesize[0],
                frame_src->data[0] + y*frame_src->linesize[0],
                frame_src->linesize[0]*32/8);
    }

    return true;
}
*/

static AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height)
{
    /*
    AVFrame *av_frame;
    int ret;

    av_frame=av_frame_alloc();

    if(!av_frame) {
        qCritical() << "Could not allocate video frame";
        return nullptr;
    }

    av_frame->format=pix_fmt;
    av_frame->width=width;
    av_frame->height=height;

    // the image can be allocated by any means and av_image_alloc() is
    // just the most convenient way if av_malloc() is to be used
    ret=av_image_alloc(av_frame->data, av_frame->linesize, width, height, pix_fmt, 32);

    if(ret<0) {
        qCritical() << "Could not allocate frame data";
        return nullptr;
    }

    return av_frame;
    */

    ////////////////


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

#endif // FFMPEG_TOOLS_H
