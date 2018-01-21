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

#include "ff_format_converter_thread.h"

FFFormatConverterThread::FFFormatConverterThread(QObject *parent)
    : QThread(parent)
{
    ff=new FFFormatConverter();

    from_210=new DecodeFrom210();

    frame_buffer_in=FrameBuffer::make();
    frame_buffer_out=FrameBuffer::make();

    frame_buffer_in->setMaxSize(4);
    frame_buffer_out->setMaxSize(4);

    start(QThread::LowestPriority);
}

FFFormatConverterThread::~FFFormatConverterThread()
{
    running=false;

    frame_buffer_in->append(nullptr);

    while(isRunning()) {
        usleep(100);
    }

    delete ff;
}

FrameBuffer::ptr FFFormatConverterThread::frameBufferIn()
{
    return frame_buffer_in;
}

FrameBuffer::ptr FFFormatConverterThread::frameBufferOut()
{
    return frame_buffer_out;
}

bool FFFormatConverterThread::setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                                    FFFormatConverter::Filter::T filter, DecodeFrom210::Format::T format_210)
{
    frame_buffer_in->clear();

    while(in_progress)
        usleep(100);

    frame_buffer_out->clear();

    this->format_210=format_210;

    return ff->setup(format_src, resolution_src, format_dst, resolution_dst, true, filter);
}

void FFFormatConverterThread::run()
{
    running=true;
    in_progress=false;

    Frame::ptr frame_src;
    Frame::ptr frame_dst;

    QByteArray ba_src;
    QByteArray ba_dst;

    while(running) {
        frame_buffer_in->wait();

        frame_src=frame_buffer_in->take();

        if(frame_src) {
            in_progress=true;

            if(format_210!=DecodeFrom210::Format::Disabled) {
                frame_src=from_210->convert(format_210, frame_src);

                if(!frame_src) {
                    qCritical() << "from_210 err";
                    continue;
                }
            }

            ba_src=QByteArray((char*)frame_src->video.data_ptr, frame_src->video.data_size);

            ff->convert(&ba_src, &ba_dst);


            frame_dst=Frame::make();

            frame_dst->setData(ba_dst, frame_src->video.size, QByteArray(), 0, 0);


            frame_buffer_out->append(frame_dst);


            in_progress=false;


            frame_src.reset();
            frame_dst.reset();
        }
    }
}
