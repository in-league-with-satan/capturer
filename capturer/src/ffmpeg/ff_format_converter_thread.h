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

#ifndef FF_FORMAT_CONVERTER_THREAD_H
#define FF_FORMAT_CONVERTER_THREAD_H

#include <QThread>
#include <atomic>

#include "ff_format_converter.h"
#include "frame_buffer.h"
#include "decode_from_210.h"

class DecklinkFrameConverter;

class FFFormatConverterThread : public QThread
{
    Q_OBJECT

public:
    FFFormatConverterThread(int thread_index=0, QObject *parent=0);
    ~FFFormatConverterThread();

    FrameBuffer::ptr frameBufferIn();
    FrameBuffer::ptr frameBufferOut();

    bool setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
               FFFormatConverter::Filter::T filter=FFFormatConverter::Filter::cNull,
               DecodeFrom210::Format::T format_210=DecodeFrom210::Format::Disabled);

protected:
    void run();

private:
    FFFormatConverter *cnv_ff;
    DecklinkFrameConverter *cnv_decklink;

    DecodeFrom210 *from_210;

    DecodeFrom210::Format::T format_210=DecodeFrom210::Format::Disabled;

    FrameBuffer::ptr frame_buffer_in;
    FrameBuffer::ptr frame_buffer_out;

    std::atomic <bool> running;
    std::atomic <bool> in_progress;

    int thread_index;
};

#endif // FF_FORMAT_CONVERTER_THREAD_H
