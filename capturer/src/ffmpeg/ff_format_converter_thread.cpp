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
    , in_progress(false)
{
    cnv_ff=new FFFormatConverter();
    from_210=new DecodeFrom210();

    frame_buffer_in=FrameBuffer::make();
    frame_buffer_out=FrameBuffer::make();

    frame_buffer_in->setMaxSize(128);
    frame_buffer_out->setMaxSize(128);
}

FFFormatConverterThread::~FFFormatConverterThread()
{
    stopThread();

    delete cnv_ff;
    delete from_210;
}

FrameBuffer::ptr FFFormatConverterThread::frameBufferIn()
{
    return frame_buffer_in;
}

FrameBuffer::ptr FFFormatConverterThread::frameBufferOut()
{
    return frame_buffer_out;
}

void FFFormatConverterThread::startThread()
{
    if(!isRunning()) {
        start(QThread::LowestPriority);
    }
}

void FFFormatConverterThread::stopThread()
{
    running=false;

    frame_buffer_in->append(nullptr);

    while(isRunning()) {
        usleep(100);
    }
}

bool FFFormatConverterThread::setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                                    FFFormatConverter::Filter::T filter, DecodeFrom210::Format::T format_210)
{
    frame_buffer_in->clear();

    pauseWaiting();

    frame_buffer_out->clear();

    this->format_210=format_210;

    return cnv_ff->setup(format_src, resolution_src, format_dst, resolution_dst, true, filter);
}

void FFFormatConverterThread::convert()
{
    Frame::ptr frame_src;
    Frame::ptr frame_dst;

    while(frame_src=frame_buffer_in->take()) {
        work(&frame_src, &frame_dst);

        frame_buffer_out->append(frame_dst);
    }
}

void FFFormatConverterThread::pauseWaiting()
{
    while(in_progress)
        usleep(100);
}

void FFFormatConverterThread::run()
{
    // qInfo() << "FFFormatConverterThread::run" << QThread::currentThreadId();

    running=true;
    in_progress=false;

    Frame::ptr frame_src;
    Frame::ptr frame_dst;

    while(running) {
        frame_buffer_in->wait();

        while(frame_src=frame_buffer_in->take()) {
            in_progress=true;

            frame_dst=Frame::make();

            work(&frame_src, &frame_dst);

            frame_buffer_out->append(frame_dst);

            in_progress=false;
        }
    }
}

void FFFormatConverterThread::work(Frame::ptr *frame_src, Frame::ptr *frame_dst)
{
    (*frame_dst)=Frame::make();

    if((*frame_src)->video.data_ptr) {
        if(format_210!=DecodeFrom210::Format::Disabled)
            (*frame_src)=from_210->convert(format_210, (*frame_src));

        if((*frame_src)) {
            (*frame_dst)->setDataVideo(cnv_ff->convert(QByteArray((char*)(*frame_src)->video.data_ptr, (*frame_src)->video.data_size)), (*frame_src)->video.size);

        } else {
            //qCritical() << "FFFormatConverterThread::run frame_src nullptr";
        }
    }
}
