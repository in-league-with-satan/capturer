#include <QApplication>
#include <QDebug>
#include <QDateTime>

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

#include "ffmpeg.h"


FFMpeg::FFMpeg(QObject *parent) :
    QObject(parent)
{
    av_codec=nullptr;
    av_codec_context=nullptr;
    convert_context=nullptr;
    av_frame=nullptr;
    av_frame_converted=nullptr;


    avcodec_register_all();

    av_packet=new AVPacket();

    processing=false;
}

FFMpeg::~FFMpeg()
{
    delete av_packet;

    stopCoder();
}

AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height)
{
    AVFrame *av_frame=av_frame_alloc();

    if(!av_frame) {
        qCritical() << "Could not allocate video frame";
        return nullptr;
    }

    av_frame->format=pix_fmt;
    av_frame->width=width;
    av_frame->height=height;

    // the image can be allocated by any means and av_image_alloc() is
    // just the most convenient way if av_malloc() is to be used

    /*
    int size=avpicture_get_size(pix_fmt, width, height);

    uint8_t *buf=(uint8_t*)av_malloc(size);

    if(!buf) {
        qCritical() << "Could not allocate raw picture buffer";
        av_free(av_frame);
        return nullptr;
    }

    avpicture_fill((AVPicture*)av_frame, buf, pix_fmt, width, height);
    */

    int ret=av_image_alloc(av_frame->data, av_frame->linesize, width, height, pix_fmt, 32);

    if(ret<0) {
        qCritical() << "Could not allocate raw picture buffer";
        av_free(av_frame);
        return nullptr;
    }

    return av_frame;
}

bool FFMpeg::initVideoCoder(QSize size)
{
    int ret=0;

    // find the mpeg1 video encoder
    av_codec=avcodec_find_encoder(AV_CODEC_ID_H264);

    if(!av_codec) {
        qCritical() << "Codec not found";
        return false;
    }

    av_codec_context=avcodec_alloc_context3(av_codec);

    if(!av_codec_context) {
        qCritical() << "Could not allocate video codec context";
        return false;
    }

    // put sample parameters
    // av_codec_context->bit_rate=4000000;

    // resolution must be a multiple of two
    av_codec_context->width=size.width();
    av_codec_context->height=size.height();


    // frames per second
    av_codec_context->time_base=(AVRational){1001,60000};

    av_codec_context->gop_size=10; // emit one intra frame every ten frames
    av_codec_context->max_b_frames=1;
    av_codec_context->pix_fmt=AV_PIX_FMT_YUV444P;

    //    av_opt_set(av_codec_context->priv_data, "preset", "veryfast", 0);
    av_opt_set(av_codec_context->priv_data, "preset", "ultrafast", 0);
    av_opt_set(av_codec_context->priv_data, "crf", "6", 0);

    if (avcodec_open2(av_codec_context, av_codec, nullptr)<0) {
        qCritical() << "Could not open codec";
        return false;
    }

    av_frame=alloc_frame(AV_PIX_FMT_BGRA, av_codec_context->width, av_codec_context->height);

    if(!av_frame) {
        qCritical() << "Could not allocate video frame";
        return false;
    }

    av_frame_converted=alloc_frame(AV_PIX_FMT_YUV444P, av_codec_context->width, av_codec_context->height);

    if(!av_frame_converted) {
        qCritical() << "Could not allocate video frame";
        return false;
    }

    convert_context=sws_getContext(av_codec_context->width, av_codec_context->height,
                                   AV_PIX_FMT_BGRA,
                                   av_codec_context->width, av_codec_context->height,
                                   AV_PIX_FMT_YUV444P,
                                   SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);


    //

    f_out.setFileName(QString("%1/%2.mpg")
                      .arg(QApplication::applicationDirPath())
                      .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"))
                      );

    if(!f_out.open(QFile::ReadWrite)) {
        qCritical() << f_out.errorString();
        return false;
    }

    av_init_packet(av_packet);
    av_packet->data=nullptr;    // packet data will be allocated by the encoder
    av_packet->size=0;

    frame_num=0;

    processing=true;

    return true;
}

bool FFMpeg::appendFrame(QImage video_frame, QByteArray ba_audio)
{
    if(!processing)
        return false;

    video_frame.scanLine(0);

    avpicture_fill((AVPicture*)av_frame, video_frame.bits(), AV_PIX_FMT_BGRA, av_frame->width, av_frame->height);


    sws_scale(convert_context, av_frame->data, av_frame->linesize, 0, av_frame->height, av_frame_converted->data, av_frame_converted->linesize);

    av_frame_converted->pts=frame_num++;


    // encode the image

    int got_output;

    int ret=avcodec_encode_video2(av_codec_context, av_packet, av_frame_converted, &got_output);

    if(ret<0) {
        qCritical() << "Error encoding frame";
        processing=false;
        return false;
    }

    if(got_output) {
        f_out.write((char*)av_packet->data, av_packet->size);
        av_free_packet(av_packet);
    }

    return true;
}

bool FFMpeg::stopCoder()
{
    if(f_out.isOpen()) {
        uint8_t endcode[]={0, 0, 1, 0xb7};
        f_out.write((char*)endcode, 4);
        f_out.close();
    }

    if(av_codec_context) {
        avcodec_close(av_codec_context);
        av_free(av_codec_context);
        av_codec_context=nullptr;
    }

    if(av_frame) {
        av_frame_unref(av_frame);
        av_freep(&av_frame->data[0]);
        av_frame_free(&av_frame);
        av_frame=nullptr;
    }

    if(av_frame_converted) {
        av_frame_unref(av_frame_converted);
        av_freep(&av_frame_converted->data[0]);
        av_frame_free(&av_frame_converted);
        av_frame_converted=nullptr;
    }

    if(convert_context) {
        sws_freeContext(convert_context);
        convert_context=nullptr;
    }

    if(av_packet->data) {
        av_packet_unref(av_packet);
        av_packet->data=nullptr;
    }

    processing=false;
}
