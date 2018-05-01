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
    , yuv_src(nullptr)
    , yuv_dst(nullptr)
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

    if(yuv_src)
        av_frame_free(&yuv_src);

    if(yuv_dst)
        av_frame_free(&yuv_dst);

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

    uint8_t *data_yuv422=nullptr;
    size_t size_yuv422=0;
    QByteArray ba_yuv422;

    running=true;

    while(running) {
        frame_buffer_in->wait();

        frame_src=frame_buffer_in->take();

        if(!frame_src)
            continue;

        frame_dst=Frame::make();

        frame_dst->video.source_rgb=frame_src->video.source_rgb;

        if(frame_src->video.source_rgb) {
            ba_dst.resize(DeckLinkVideoFrame::frameSize(frame_src->video.size, bmdFormat8BitBGRA));

            if(frame_src->video.source_10bit) {
                format_converter_dl->init(bmdFormat10BitRGB, frame_src->video.size, bmdFormat8BitBGRA, frame_src->video.size);
                format_converter_dl->convert(frame_src->video.data_ptr, (void*)ba_dst.constData());

            } else {
                memcpy((void*)ba_dst.constData(), frame_src->video.data_ptr, frame_src->video.data_size);
            }

        } else {
            if(frame_src->video.source_10bit) {
                size_yuv422=DeckLinkVideoFrame::frameSize(frame_src->video.size, bmdFormat8BitYUV);

                ba_yuv422.resize(size_yuv422);

                data_yuv422=(uint8_t*)ba_yuv422.constData();

                format_converter_dl->init(bmdFormat10BitYUV, frame_src->video.size, bmdFormat8BitYUV, frame_src->video.size);
                format_converter_dl->convert(frame_src->video.data_ptr, (void*)ba_yuv422.constData());

            } else {
                data_yuv422=frame_src->video.data_ptr;
                size_yuv422=frame_src->video.data_size;
            }

            if(!format_converter_ff->compareParams(AV_PIX_FMT_UYVY422, frame_src->video.size, AV_PIX_FMT_YUV420P, frame_src->video.size)) {
                if(yuv_src)
                    av_frame_free(&yuv_src);

                if(yuv_dst)
                    av_frame_free(&yuv_dst);

                yuv_src=alloc_frame(AV_PIX_FMT_UYVY422, frame_src->video.size.width(), frame_src->video.size.height(), false);
                yuv_dst=alloc_frame(AV_PIX_FMT_YUV420P, frame_src->video.size.width(), frame_src->video.size.height(), true);

                yuv_src->linesize[0]=DeckLinkVideoFrame::rowSize(frame_src->video.size.width(), frame_src->video_frame->GetPixelFormat());

                format_converter_ff->setup(AV_PIX_FMT_UYVY422, frame_src->video.size, AV_PIX_FMT_YUV420P, frame_src->video.size);
            }

            if(fast_yuv) {
                ba_dst.resize(size_yuv422);

                memcpy((void*)ba_dst.constData(), data_yuv422, size_yuv422);

            } else {
                ba_dst.resize(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, frame_src->video.size.width(), frame_src->video.size.height(), alignment));

                av_image_fill_arrays(yuv_src->data, yuv_src->linesize, data_yuv422, AV_PIX_FMT_UYVY422, frame_src->video.size.width(), frame_src->video.size.height(), alignment);

                format_converter_ff->convert(yuv_src, yuv_dst);

                av_image_copy_to_buffer((uint8_t*)ba_dst.constData(), ba_dst.size(), yuv_dst->data, yuv_dst->linesize, (AVPixelFormat)yuv_dst->format, yuv_dst->width, yuv_dst->height, alignment);
            }
        }

        frame_dst->setData(ba_dst, frame_src->video.size, frame_src->audio.dummy, frame_src->audio.channels, frame_src->audio.sample_size);


        frame_buffer_out->append(frame_dst);


        frame_src.reset();
        frame_dst.reset();


        emit gotFrame();
    }
}
