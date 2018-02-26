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

FFFormatConverterThread::FFFormatConverterThread(int thread_index, QObject *parent)
    : QThread(parent)
    , thread_index(thread_index)
{
    cnv_ff=new FFFormatConverter();
    from_210=new DecodeFrom210();

    frame_buffer_in=FrameBuffer::make();
    frame_buffer_out=FrameBuffer::make();

    frame_buffer_in->setMaxSize(128);
    frame_buffer_out->setMaxSize(128);

    start(QThread::LowestPriority);
}

FFFormatConverterThread::~FFFormatConverterThread()
{
    running=false;

    frame_buffer_in->append(nullptr);

    while(isRunning()) {
        usleep(100);
    }

    delete cnv_ff;
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

    return cnv_ff->setup(format_src, resolution_src, format_dst, resolution_dst, true, filter);
}

void FFFormatConverterThread::run()
{
    running=true;
    in_progress=false;

    Frame::ptr frame_src;
    Frame::ptr frame_dst;

    QByteArray ba_dst;

    while(running) {
        frame_buffer_in->wait();

        while(frame_src=frame_buffer_in->take()) {
            uint16_t frame_counter=frame_src->counter;

            in_progress=true;

            frame_dst=Frame::make();

            if(frame_src->video.data_ptr) {
                if(format_210!=DecodeFrom210::Format::Disabled)
                    frame_src=from_210->convert(format_210, frame_src);

                if(frame_src) {
                    frame_dst->setDataVideo(cnv_ff->convert(QByteArray((char*)frame_src->video.data_ptr, frame_src->video.data_size)), frame_src->video.size);

                } else {
                    //qCritical() << "FFFormatConverterThread::run frame_src nullptr";
                }
            }

            frame_dst->counter=frame_counter;

            frame_buffer_out->append(frame_dst);


            frame_src.reset();
            frame_dst.reset();

            in_progress=false;
        }
    }
}
