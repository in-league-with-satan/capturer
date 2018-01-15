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
#include "quick_video_source_convert_thread.h"

#include "quick_video_source.h"

QuickVideoSource::QuickVideoSource(QObject *parent)
    : QObject(parent)
    , surface(nullptr)
    , fast_yuv(false)
{
    convert_thread=new QuickVideoSourceConvertThread(this);

    connect(convert_thread, SIGNAL(gotFrame()), SLOT(checkFrame()), Qt::QueuedConnection);
}

QuickVideoSource::~QuickVideoSource()
{
    closeSurface();
}

FrameBuffer::ptr QuickVideoSource::frameBuffer()
{
    return convert_thread->frameBufferIn();
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

bool QuickVideoSource::fastYuv() const
{
    return fast_yuv;
}

void QuickVideoSource::setFastYuv(bool value)
{
    convert_thread->setFastYuv(value);
    fast_yuv=value;
}

void QuickVideoSource::checkFrame()
{
    if(!surface)
        return;

    Frame::ptr frame=
            convert_thread->frameBufferOut()->take();

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
        }

        last_frame=QVideoFrame(DeckLinkVideoFrame::frameSize(frame->video.size, bmdFormat8BitBGRA),
                               frame->video.size,
                               DeckLinkVideoFrame::rowSize(frame->video.size.width(), bmdFormat8BitBGRA),
                               QVideoFrame::Format_ARGB32);

        if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {
            memcpy(last_frame.bits(), frame->video.ptr_data, frame->video.data_size);

            last_frame.unmap();

        } else {
            qCritical() << "err frame.map write";
        }

    } else {
        QVideoFrame::PixelFormat fmt=QVideoFrame::Format_YUV420P;
        int buf_size;
        int line_size;

        if(fast_yuv) {
            fmt=QVideoFrame::Format_UYVY;

            buf_size=av_image_get_buffer_size(AV_PIX_FMT_UYVY422, frame->video.size.width(), frame->video.size.height(), 32);
            line_size=av_image_get_linesize(AV_PIX_FMT_UYVY422, frame->video.size.width(), 0);

        } else {
            buf_size=av_image_get_buffer_size(AV_PIX_FMT_YUV420P, frame->video.size.width(), frame->video.size.height(), 32);
            line_size=av_image_get_linesize(AV_PIX_FMT_YUV420P, frame->video.size.width(), 0);
        }

        if(frame->video.size!=format.frameSize() || fmt!=format.pixelFormat()) {
            closeSurface();

            format=QVideoSurfaceFormat(frame->video.size, fmt);

            if(!surface->start(format)) {
                qCritical() << "surface->start error" << surface->error();
                format=QVideoSurfaceFormat();
                return;
            }
        }

        last_frame=QVideoFrame(buf_size, frame->video.size, line_size, fmt);

        if(last_frame.map(QAbstractVideoBuffer::WriteOnly)) {
            memcpy(last_frame.bits(), frame->video.ptr_data, buf_size);

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

void QuickVideoSource::closeSurface()
{
    if(surface && surface->isActive() )
        surface->stop();
}
