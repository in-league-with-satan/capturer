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
#include <QApplication>
//#include <QtQml/qqml.h>

// #include "libavutil/bswap.h"
#include "ff_tools.h"
#include "ff_format_converter.h"
#include "decklink_frame_converter.h"

#include "quick_video_source.h"

QuickVideoSource::QuickVideoSource(QObject *parent)
    : QThread(parent)
    , surface(nullptr)
    , half_fps(false)
    , fast_yuv(false)
    , yuv_src(nullptr)
    , yuv_dst(nullptr)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    startTimer(1);
}

QuickVideoSource::QuickVideoSource(bool thread, QObject *parent)
    : QThread(parent)
    , surface(nullptr)
    , half_fps(false)
    , fast_yuv(false)
    , yuv_src(nullptr)
    , yuv_dst(nullptr)
    , format_converter_ff(new FFFormatConverter())
    , format_converter_dl(new DecklinkFrameConverter())
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    if(thread) {
        start(QThread::LowPriority);
        // start(QThread::NormalPriority);

    } else
        startTimer(1);
}

QuickVideoSource::~QuickVideoSource()
{
    closeSurface();

    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }

    if(yuv_src)
        av_frame_free(&yuv_src);

    if(yuv_dst)
        av_frame_free(&yuv_dst);

    delete format_converter_ff;
    delete format_converter_dl;
}

FrameBuffer::ptr QuickVideoSource::frameBuffer()
{
    return frame_buffer;
}

QAbstractVideoSurface *QuickVideoSource::videoSurface() const
{
    return surface;
}

void QuickVideoSource::setVideoSurface(QAbstractVideoSurface *s)
{
    closeSurface();

    surface=s;
}

void QuickVideoSource::setHalfFps(bool value)
{
    half_fps=value;
}

bool QuickVideoSource::fastYuv() const
{
    return fast_yuv;
}

void QuickVideoSource::setFastYuv(bool value)
{
    fast_yuv=value;
}

void QuickVideoSource::run()
{
    Frame::ptr frame;

    bool skip_frame=false;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();


        if(!frame)
            continue;

        if(!surface)
            continue;

        skip_frame=!skip_frame;

        if(half_fps) {
            if(skip_frame) {
                frame.reset();
                continue;
            }
        }

        if(frame->video.rgb) {
            if(frame->video.size!=format.frameSize() || QVideoFrame::Format_ARGB32!=format.pixelFormat()) {
                closeSurface();

                format=QVideoSurfaceFormat(frame->video.size, QVideoFrame::Format_ARGB32);

                if(!surface->start(format)) {
                    qCritical() << "surface->start error" << surface->error();
                    format=QVideoSurfaceFormat();
                    continue;
                }
            }

            last_frame=QVideoFrame(frame->video.data_size, frame->video.size,
                                   DeckLinkVideoFrame::rowSize(frame->video.size.width(), frame->video_frame->GetPixelFormat()),
                                   QVideoFrame::Format_ARGB32);

            if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {
                memcpy(last_frame.bits(), frame->video.ptr_data, frame->video.data_size);
                last_frame.unmap();

            } else {
                qCritical() << "err frame.map write";
            }

        } else {
            QVideoFrame::PixelFormat fmt=QVideoFrame::Format_UYVY;

            if(frame->video.size!=format.frameSize() || fmt!=format.pixelFormat()) {
                closeSurface();

                format=QVideoSurfaceFormat(frame->video.size, fmt);

                if(!surface->start(format)) {
                    qCritical() << "surface->start error" << surface->error();
                    format=QVideoSurfaceFormat();
                    continue;
                }
            }

            last_frame=QVideoFrame(frame->video.data_size, frame->video.size,
                                   DeckLinkVideoFrame::rowSize(frame->video.size.width(), frame->video_frame->GetPixelFormat()),
                                   fmt);

            if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {
                if(fast_yuv)
                    memcpy(last_frame.bits(), frame->video.ptr_data, frame->video.data_size);

                last_frame.unmap();

            } else {
                qCritical() << "err frame.map write";
            }
        }


        last_frame.map(QAbstractVideoBuffer::ReadOnly);

        surface->present(last_frame);

        last_frame.unmap();

        frame.reset();
    }
}

void QuickVideoSource::timerEvent(QTimerEvent*)
{
    if(!surface)
        return;

    Frame::ptr frame=
            frame_buffer->take();

    if(!frame)
        return;

    if(frame->video.rgb) {
        if(frame->video.size!=format.frameSize() || QVideoFrame::Format_ARGB32!=format.pixelFormat()) {
            closeSurface();

            format=QVideoSurfaceFormat(frame->video.size, QVideoFrame::Format_ARGB32);

            if(!surface->start(format)) {
                qCritical() << "surface->start error" << surface->error();
                format=QVideoSurfaceFormat();
                return;
            }

            format_converter_dl->init(bmdFormat10BitRGB, frame->video.size, bmdFormat8BitBGRA, frame->video.size);
        }

        last_frame=QVideoFrame(DeckLinkVideoFrame::frameSize(frame->video.size, bmdFormat8BitBGRA),
                               frame->video.size,
                               DeckLinkVideoFrame::rowSize(frame->video.size.width(), bmdFormat8BitBGRA),
                               QVideoFrame::Format_ARGB32);

        if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {

            if(frame->video.rgb_10bit)
                format_converter_dl->convert(frame->video.ptr_data, last_frame.bits());

            else
                memcpy(last_frame.bits(), frame->video.ptr_data, frame->video.data_size);

            last_frame.unmap();

        } else {
            qCritical() << "err frame.map write";
        }

    } else {
        QVideoFrame::PixelFormat fmt=QVideoFrame::Format_UYVY;

        if(!fast_yuv)
            fmt=QVideoFrame::Format_YUV420P;

        if(frame->video.size!=format.frameSize() || fmt!=format.pixelFormat()) {
            closeSurface();

            format=QVideoSurfaceFormat(frame->video.size, fmt);

            if(!surface->start(format)) {
                qCritical() << "surface->start error" << surface->error();
                format=QVideoSurfaceFormat();
                return;
            }

            //

            if(yuv_src)
                av_frame_free(&yuv_src);

            if(yuv_dst)
                av_frame_free(&yuv_dst);


            yuv_src=alloc_frame(AV_PIX_FMT_UYVY422, frame->video.size.width(), frame->video.size.height(), false);
            yuv_dst=alloc_frame(AV_PIX_FMT_YUV420P, frame->video.size.width(), frame->video.size.height(), true);

            yuv_src->linesize[0]=DeckLinkVideoFrame::rowSize(frame->video.size.width(), frame->video_frame->GetPixelFormat());

            format_converter_ff->setup((AVPixelFormat)yuv_src->format, frame->video.size, (AVPixelFormat)yuv_dst->format, frame->video.size, false);
        }

        if(fast_yuv) {
            last_frame=QVideoFrame(frame->video.data_size, frame->video.size,
                                   DeckLinkVideoFrame::rowSize(frame->video.size.width(), frame->video_frame->GetPixelFormat()), fmt);

            if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {
                memcpy(last_frame.bits(), frame->video.ptr_data, frame->video.data_size);

                last_frame.unmap();

            } else {
                qCritical() << "err frame.map write";
            }

        } else {
            const int buf_size=av_image_get_buffer_size((AVPixelFormat)yuv_dst->format, yuv_dst->width, yuv_dst->height, 32);

            last_frame=QVideoFrame(buf_size, frame->video.size, yuv_dst->linesize[0], fmt);

            if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {
                yuv_src->data[0]=(uint8_t*)frame->video.ptr_data;

                format_converter_ff->convert(yuv_src, yuv_dst);

                av_image_copy_to_buffer(last_frame.bits(), buf_size, yuv_dst->data, yuv_dst->linesize, (AVPixelFormat)yuv_dst->format, yuv_dst->width, yuv_dst->height, 32);

                last_frame.unmap();

            } else {
                qCritical() << "err frame.map write";
            }
        }
    }

    last_frame.map(QAbstractVideoBuffer::ReadOnly);

    surface->present(last_frame);

    last_frame.unmap();

    frame.reset();
}

void QuickVideoSource::closeSurface()
{
    if(surface && surface->isActive() )
        surface->stop();
}
