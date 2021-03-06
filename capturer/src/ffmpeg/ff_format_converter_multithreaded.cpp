/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>

#include "ff_format_converter_multithreaded.h"

FFFormatConverterMt::FFFormatConverterMt(QObject *parent)
    : QObject(parent)
{
}

void FFFormatConverterMt::useMultithreading(int thread_count)
{
    use_multithreading=thread_count>0;

    if(thread_count<1)
        thread_count=1;

    thread.resize(thread_count);

    for(int i=0; i<thread_count; ++i) {
        thread[i]=std::shared_ptr<FFFormatConverterThread>(new FFFormatConverterThread(i, this));

        connect(&thread[i]->frameBufferIn()->signaler, SIGNAL(frameSkipped()), SIGNAL(frameSkipped()), Qt::QueuedConnection);
        connect(&thread[i]->frameBufferOut()->signaler, SIGNAL(frameSkipped()), SIGNAL(frameSkipped()), Qt::QueuedConnection);

        if(use_multithreading)
            thread[i]->startThread();
    }
}

void FFFormatConverterMt::stop()
{
    for(int i=0; i<thread.size(); ++i) {
        thread[i]->stopThread();
        thread[i]->deleteLater();
    }

    thread.clear();

    format_src=AV_PIX_FMT_NONE;
    format_dst=AV_PIX_FMT_NONE;
    resolution_src=QSize();
    resolution_dst=QSize();
}

void FFFormatConverterMt::resetQueues()
{
    for(int i=0; i<thread.size(); ++i) {
        thread[i]->frameBufferIn()->clear();
        thread[i]->pauseWaiting();
        thread[i]->frameBufferOut()->clear();
    }

    queue_converted.clear();
}

bool FFFormatConverterMt::setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                                int color_space_src, int color_space_dst, int color_range_src, int color_range_dst,
                                FFFormatConverter::Filter::T filter, DecodeFrom210::Format::T format_210)
{
    index_thread_src=0;
    index_thread_dst=0;

    if(compareParams(format_src, resolution_src, format_dst, resolution_dst, color_space_src, color_space_dst, color_range_src, color_range_dst, filter, format_210))
        return true;

    qDebug() << "formats:" << av_get_pix_fmt_name(format_src) << av_get_pix_fmt_name(format_dst);

    this->format_src=format_src;
    this->format_dst=format_dst;

    this->resolution_src=resolution_src;
    this->resolution_dst=resolution_dst;

    this->color_space_src=color_space_src;
    this->color_space_dst=color_space_dst;

    this->color_range_src=color_range_src;
    this->color_range_dst=color_range_dst;

    this->filter=filter;

    this->format_210=format_210;

    bool result=true;

    for(int i=0; i<thread.size(); ++i) {
        if(!thread[i]->setup(format_src, resolution_src, format_dst, resolution_dst, color_space_src, color_space_dst, color_range_src, color_range_dst, filter, format_210))
            result=false;
    }

    return result;
}

bool FFFormatConverterMt::compareParams(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                                        int color_space_src, int color_space_dst, int color_range_src, int color_range_dst,
                                        FFFormatConverter::Filter::T filter, DecodeFrom210::Format::T format_210)
{
    if(this->format_src==format_src && this->format_dst==format_dst
            && this->resolution_src==resolution_src && this->resolution_dst==resolution_dst
            && this->color_space_src==color_space_src && this->color_space_dst==color_space_dst && this->color_range_src==color_range_src && this->color_range_dst==color_range_dst
            && this->filter==filter
            && this->format_210==format_210)
        return true;

    return false;
}

bool FFFormatConverterMt::isInAndOutEqual() const
{
    return format_src==format_dst && resolution_src==resolution_dst;
}

void FFFormatConverterMt::convert(Frame::ptr frame)
{
    AVFrameSP::ptr frame_out;

    if(isInAndOutEqual()) {
        frame_out=AVFrameSP::make(frame->video.pixel_format.toAVPixelFormat(),
                                  frame->video.size.width(),
                                  frame->video.size.height(), false);

        av_image_fill_arrays(frame_out->d->data, frame_out->d->linesize, frame->video.data_ptr, (AVPixelFormat)frame_out->d->format,
                             frame_out->d->width, frame_out->d->height, alignment);

        frame_out->d->pts=
                frame->video.pts;

        frame_out->time_base=
                frame->video.time_base;

        queue_converted.enqueue(frame_out);

        return;
    }

    if(!use_multithreading) {
        thread[0]->frameBufferIn()->append(frame);
        thread[0]->convert();

        frame_out=
                thread[0]->frameBufferOut()->take();

        if(frame_out)
            queue_converted.enqueue(frame_out);

        return;
    }

    QPair <int, int> buf_sizes;

    while(true) {
        buf_sizes=thread[index_thread_src]->frameBufferIn()->size();

        if(buf_sizes.first==0)
            break;

        QThread::usleep(24);
    }

    thread[index_thread_src]->frameBufferIn()->append(frame);

    //

    index_thread_src++;

    if(index_thread_src>=thread.size())
        index_thread_src=0;

    //

    while(frame_out=checkReady()) {
        queue_converted.enqueue(frame_out);
    }
}

AVFrameSP::ptr FFFormatConverterMt::result()
{
    if(queue_converted.isEmpty()) {
        return AVFrameSP::ptr();
    }

    return queue_converted.dequeue();
}

AVFrameSP::ptr FFFormatConverterMt::checkReady()
{
    AVFrameSP::ptr frame=
            thread[index_thread_dst]->frameBufferOut()->take();

    if(frame) {
        index_thread_dst++;

        if(index_thread_dst>=thread.size())
            index_thread_dst=0;
    }

    return frame;
}
