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

#include "ff_tools.h"
#include "ff_format_converter.h"
#include "decklink_frame_converter.h"

#include "quick_video_source_convert_thread.h"

QuickVideoSourceConvertThread::QuickVideoSourceConvertThread(QObject *parent)
    : QThread(parent)
    , fast_yuv(false)
    , conv_src(nullptr)
    , conv_dst(nullptr)
    , format_converter_ff(new FFFormatConverter())
    , format_converter_dl(new DecklinkFrameConverter())
{
    frame_buffer_in=FrameBuffer<Frame::ptr>::make();
    frame_buffer_in->setMaxSize(1);

    frame_buffer_out=FrameBuffer<Frame::ptr>::make();
    frame_buffer_out->setMaxSize(1);

    start(QThread::LowPriority);
}

QuickVideoSourceConvertThread::~QuickVideoSourceConvertThread()
{
    running=false;

    while(isRunning()) {
        frame_buffer_in->append(nullptr);
        msleep(30);
    }

    if(conv_src)
        av_frame_free(&conv_src);

    if(conv_dst)
        av_frame_free(&conv_dst);

    delete format_converter_ff;
    delete format_converter_dl;
}

FrameBuffer<Frame::ptr>::ptr QuickVideoSourceConvertThread::frameBufferIn()
{
    return frame_buffer_in;
}

FrameBuffer<Frame::ptr>::ptr QuickVideoSourceConvertThread::frameBufferOut()
{
    return frame_buffer_out;
}

bool QuickVideoSourceConvertThread::fastYuv() const
{
    return fast_yuv;
}

void QuickVideoSourceConvertThread::setFastYuv(bool value)
{
    fast_yuv=value;
}

void QuickVideoSourceConvertThread::run()
{
    Frame::ptr frame_src;
    Frame::ptr frame_dst;

    QByteArray ba_dst;

    uint8_t *data_conv=nullptr;
    size_t size_conv=0;
    QByteArray ba_conv;

    running=true;

    while(running) {
        frame_buffer_in->wait();

        frame_src=frame_buffer_in->take();

        if(!frame_src)
            continue;

        if(!frame_src->video.data_ptr)
            continue;

        if(frame_src->video.pixel_format.isDirect()
                && frame_src->video.pixel_format!=PixelFormat::rgb24
                && frame_src->video.pixel_format!=PixelFormat::bgr24) {
            frame_buffer_out->append(frame_src);
            emit gotFrame();
            continue;
        }

        frame_dst=Frame::make();

        if(frame_src->video.pixel_format.isRgb()) {
            ba_dst.resize(frameBufSize(frame_src->video.size, frame_src->video.pixel_format));
            frame_dst->video.pixel_format=PixelFormat::bgra;

            if(frame_src->video.pixel_format.is10bit() && frame_src->video.pixel_format.is210()) {
                format_converter_dl->init(bmdFormat10BitRGB, frame_src->video.size, bmdFormat8BitBGRA, frame_src->video.size);
                format_converter_dl->convert(frame_src->video.data_ptr, (void*)ba_dst.constData());

            } else if(frame_src->video.pixel_format!=PixelFormat::bgra) {

                if(!format_converter_ff->compareParams(frame_src->video.pixel_format.toAVPixelFormat(), frame_src->video.size, AV_PIX_FMT_BGRA, frame_src->video.size)) {
                    if(conv_src)
                        av_frame_free(&conv_src);

                    if(conv_dst)
                        av_frame_free(&conv_dst);

                    conv_src=alloc_frame(frame_src->video.pixel_format.toAVPixelFormat(), frame_src->video.size.width(), frame_src->video.size.height(), false);
                    conv_dst=alloc_frame(AV_PIX_FMT_BGRA, frame_src->video.size.width(), frame_src->video.size.height(), true);

                    conv_src->linesize[0]=DeckLinkVideoFrame::rowSize(frame_src->video.size.width(), bmdFormat8BitBGRA);

                    format_converter_ff->setup(frame_src->video.pixel_format.toAVPixelFormat(), frame_src->video.size, AV_PIX_FMT_BGRA, frame_src->video.size);
                }

                data_conv=frame_src->video.data_ptr;
                size_conv=frame_src->video.data_size;


                ba_dst.resize(frameBufSize(frame_src->video.size, PixelFormat::bgra));

                av_image_fill_arrays(conv_src->data, conv_src->linesize, data_conv, frame_src->video.pixel_format.toAVPixelFormat(),
                                     frame_src->video.size.width(), frame_src->video.size.height(), alignment);

                format_converter_ff->convert(conv_src, conv_dst);

                av_image_copy_to_buffer((uint8_t*)ba_dst.constData(), ba_dst.size(), conv_dst->data, conv_dst->linesize, (AVPixelFormat)conv_dst->format, conv_dst->width, conv_dst->height, alignment);


            } else {
                memcpy((void*)ba_dst.constData(), frame_src->video.data_ptr, frame_src->video.data_size);
            }

        } else {
            frame_dst->video.pixel_format.fromAVPixelFormat(AV_PIX_FMT_YUV420P);

            if(frame_src->video.pixel_format.is10bit() && frame_src->video.pixel_format.is210()) {
                size_conv=frameBufSize(frame_src->video.size, frame_src->video.pixel_format);

                ba_conv.resize(size_conv);

                data_conv=(uint8_t*)ba_conv.constData();

                format_converter_dl->init(bmdFormat10BitYUV, frame_src->video.size, bmdFormat8BitYUV, frame_src->video.size);
                format_converter_dl->convert(frame_src->video.data_ptr, (void*)ba_conv.constData());

            } else {
                data_conv=frame_src->video.data_ptr;
                size_conv=frame_src->video.data_size;
            }

            if(!format_converter_ff->compareParams(AV_PIX_FMT_UYVY422, frame_src->video.size, AV_PIX_FMT_YUV420P, frame_src->video.size)) {
                if(conv_src)
                    av_frame_free(&conv_src);

                if(conv_dst)
                    av_frame_free(&conv_dst);

                conv_src=alloc_frame(AV_PIX_FMT_UYVY422, frame_src->video.size.width(), frame_src->video.size.height(), false);
                conv_dst=alloc_frame(AV_PIX_FMT_YUV420P, frame_src->video.size.width(), frame_src->video.size.height(), true);

                conv_src->linesize[0]=DeckLinkVideoFrame::rowSize(frame_src->video.size.width(), (BMDPixelFormat)frame_src->video.pixel_format.toBMDPixelFormat());

                format_converter_ff->setup(AV_PIX_FMT_UYVY422, frame_src->video.size, AV_PIX_FMT_YUV420P, frame_src->video.size);
            }

            if(fast_yuv) {
                ba_dst.resize(size_conv);

                memcpy((void*)ba_dst.constData(), data_conv, size_conv);

            } else {
                ba_dst.resize(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, frame_src->video.size.width(), frame_src->video.size.height(), alignment));

                av_image_fill_arrays(conv_src->data, conv_src->linesize, data_conv, AV_PIX_FMT_UYVY422, frame_src->video.size.width(), frame_src->video.size.height(), alignment);

                format_converter_ff->convert(conv_src, conv_dst);

                av_image_copy_to_buffer((uint8_t*)ba_dst.constData(), ba_dst.size(), conv_dst->data, conv_dst->linesize, (AVPixelFormat)conv_dst->format, conv_dst->width, conv_dst->height, alignment);
            }
        }


        frame_dst->setData(ba_dst, frame_src->video.size, frame_src->audio.dummy, frame_src->audio.channels, frame_src->audio.sample_size);


        frame_buffer_out->append(frame_dst);


        frame_src.reset();
        frame_dst.reset();


        emit gotFrame();
    }
}
