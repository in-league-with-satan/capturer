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

#include "decode_from_210.h"

struct Context {
    ~Context() {
        close();
    }

    int width=0;
    int height=0;

    AVCodec *codec=nullptr;
    AVCodecContext *codec_context=nullptr;

    AVFrame *av_frame=nullptr;

    AVPacket packet;

    DecodeFrom210::Format::T format=DecodeFrom210::Format::Disabled;

    void init();
    void close();
};

DecodeFrom210::DecodeFrom210()
{
    d=new Context();
}

DecodeFrom210::~DecodeFrom210()
{
    delete d;
}

AVPixelFormat DecodeFrom210::v210PixelFormat()
{
    return AV_PIX_FMT_YUV422P10LE;
}

AVPixelFormat DecodeFrom210::r210PixelFormat()
{
    return AV_PIX_FMT_GBRP10LE;
}

bool DecodeFrom210::convert(Format::T format, uint8_t *data, int size, int width, int height, AVFrame *frame)
{
    if(d->format!=format || d->width!=width || d->height!=height) {
        d->format=format;
        d->width=width;
        d->height=height;

        d->close();
        d->init();
    }

    if(format==Format::Disabled)
        return false;

    d->packet.data=(uint8_t*)data;
    d->packet.size=size;

    if(avcodec_send_packet(d->codec_context, &d->packet)==0) {
        const int ret=avcodec_receive_frame(d->codec_context, frame);

        if(ret<0 && ret!=AVERROR(EAGAIN)) {
            qCritical() << "avcodec_receive_frame" << ffErrorString(ret);
            return false;
        }

        return true;
    }

    qCritical() << "avcodec_send_packet err";

    return false;
}

Frame::ptr DecodeFrom210::convert(Format::T format, Frame::ptr frame)
{
    if(d->format!=format || d->width!=frame->video.size.width() || d->height!=frame->video.size.height()) {
        d->format=format;
        d->width=frame->video.size.width();
        d->height=frame->video.size.height();

        d->close();
        d->init();
    }

    if(format==Format::Disabled) {
        qCritical() << "Disabled";
        return Frame::ptr();
    }

    if(format==Format::R210) {
        Frame::ptr frame_result=frame->copyFrameSoundOnly();

        frame_result->video.pixel_format=frame->video.pixel_format;
        frame_result->video.data_size=av_image_get_buffer_size(r210PixelFormat(), d->width, d->height, alignment);
        frame_result->video.dummy.resize(frame_result->video.data_size);
        frame_result->video.data_ptr=(uint8_t*)frame_result->video.dummy.constData();
        frame_result->video.size=frame->video.size;

        uint32_t *src_ptr=(uint32_t*)frame->video.data_ptr;

        uint16_t *dst_av_frame_ptr_g=(uint16_t*)d->av_frame->data[0];
        uint16_t *dst_av_frame_ptr_b=(uint16_t*)d->av_frame->data[1];
        uint16_t *dst_av_frame_ptr_r=(uint16_t*)d->av_frame->data[2];

        uint32_t pixel;

        for(size_t i_row=0, rows=d->height, pos=0; i_row<rows; ++i_row) {
            for(size_t i_col=0, cols=d->width; i_col<cols; ++i_col, ++pos) {
                pixel=AV_BSWAP32C(src_ptr[pos]);

                dst_av_frame_ptr_b[pos]=pixel&0x3ff;
                dst_av_frame_ptr_g[pos]=(pixel&0xffc00) >> 10;
                dst_av_frame_ptr_r[pos]=(pixel&0x3ff00000) >> 20;
            }
        }

        av_image_copy_to_buffer((uint8_t*)frame_result->video.dummy.constData(), frame_result->video.dummy.size(), d->av_frame->data, d->av_frame->linesize,
                                (AVPixelFormat)d->av_frame->format, d->av_frame->width, d->av_frame->height, alignment);

        return frame_result;
    }

    d->packet.data=frame->video.data_ptr;
    d->packet.size=frame->video.data_size;

    if(avcodec_send_packet(d->codec_context, &d->packet)==0) {
        const int ret=avcodec_receive_frame(d->codec_context, d->av_frame);

        if(ret<0 && ret!=AVERROR(EAGAIN)) {
            qCritical() << "avcodec_receive_frame err" << ffErrorString(ret);
            return Frame::ptr();
        }

        Frame::ptr frame_result=frame->copyFrameSoundOnly();

        frame_result->video.pixel_format=frame->video.pixel_format;
        frame_result->video.data_size=av_image_get_buffer_size((AVPixelFormat)d->av_frame->format, d->width, d->height, alignment);
        frame_result->video.dummy.resize(frame_result->video.data_size);
        frame_result->video.data_ptr=(uint8_t*)frame_result->video.dummy.constData();
        frame_result->video.size=frame->video.size;

        av_image_copy_to_buffer((uint8_t*)frame_result->video.dummy.constData(), frame_result->video.dummy.size(), d->av_frame->data, d->av_frame->linesize,
                                (AVPixelFormat)d->av_frame->format, d->av_frame->width, d->av_frame->height, alignment);

        //

        return frame_result;
    }

    qCritical() << "avcodec_send_packet err";

    return Frame::ptr();
}

DecodeFrom210::Format::T DecodeFrom210::format() const
{
    return d->format;
}

void Context::init()
{
    if(format==DecodeFrom210::Format::Disabled)
        return;


    if(format==DecodeFrom210::Format::V210)
        codec=avcodec_find_decoder(AV_CODEC_ID_V210);

    else if(format==DecodeFrom210::Format::R210)
        codec=avcodec_find_decoder(AV_CODEC_ID_R210);

    else
        codec=nullptr;

    if(!codec) {
        qCritical() << "avcodec_find_decoder err";
        exit(1);
    }


    codec_context=avcodec_alloc_context3(codec);

    if(!codec_context) {
        qCritical() << "avcodec_alloc_context3 err";
        exit(1);
    }

    codec_context->width=width;
    codec_context->height=height;
    codec_context->thread_count=1;

    if(avcodec_open2(codec_context, codec, nullptr)<0) {
        qCritical() << "avcodec_open2 err";
        exit(1);
    }


    if(format==DecodeFrom210::Format::R210)
        av_frame=alloc_frame(DecodeFrom210::r210PixelFormat(), width, height);

    else
        av_frame=av_frame_alloc();
}

void Context::close()
{
    if(codec_context) {
        avcodec_close(codec_context);
        av_free(codec_context);

        codec_context=nullptr;
    }

    if(av_frame) {
        av_frame_free(&av_frame);
        av_frame=nullptr;
    }
}
