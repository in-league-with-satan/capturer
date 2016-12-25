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

const int alignment=32;

static bool byteArrayToAvFrame(QByteArray *ba_src, AVFrame *frame_dst)
{
    // const size_t data_size=frame_dst->linesize[0]*frame_dst->height;
    const size_t data_size=av_image_get_buffer_size((AVPixelFormat)frame_dst->format, frame_dst->width, frame_dst->height, alignment);

    if(ba_src->size()!=data_size) {
        qCritical() << "wrong bytearray size";
        return false;
    }

    memcpy(frame_dst->data[0], ba_src->data(), data_size);

    // for(int y=0; y<frame_dst->height; y++) {
    //     memcpy(frame_dst->data[0] + y*frame_dst->linesize[0],
    //            ba_src->data() + y*frame_dst->linesize[0], frame_dst->linesize[0]*32/8);
    // }

    return true;
}

static bool avFrameToByteArray(AVFrame *frame_src, QByteArray *ba_dst)
{
    // const size_t data_size=frame_src->linesize[0]*frame_src->height;

    const size_t data_size=av_image_get_buffer_size((AVPixelFormat)frame_src->format, frame_src->width, frame_src->height, alignment);

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

static AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height)
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
