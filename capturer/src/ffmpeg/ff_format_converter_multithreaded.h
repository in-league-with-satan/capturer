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

#ifndef FF_FORMAT_CONVERTER_MULTITHREADED_H
#define FF_FORMAT_CONVERTER_MULTITHREADED_H

#include <QObject>

#include "ff_format_converter_thread.h"

class FFFormatConverterMt : public QObject
{
    Q_OBJECT

public:
    FFFormatConverterMt(uint8_t thread_count, QObject *parent=0);

    void useMultithreading(bool value);

    void resetQueues();

    bool setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
               FFFormatConverter::Filter::T filter=FFFormatConverter::Filter::cNull,
               DecodeFrom210::Format::T format_210=DecodeFrom210::Format::Disabled);

    bool compareParams(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                       FFFormatConverter::Filter::T filter=FFFormatConverter::Filter::cNull,
                       DecodeFrom210::Format::T format_210=DecodeFrom210::Format::Disabled);

    void convert(Frame::ptr frame);

    AVFrameSP::ptr result();

private:
    AVFrameSP::ptr checkReady();

    AVPixelFormat format_src;
    AVPixelFormat format_dst;

    QSize resolution_src;
    QSize resolution_dst;

    FFFormatConverter::Filter::T filter=FFFormatConverter::Filter::cNull;

    DecodeFrom210::Format::T format_210=DecodeFrom210::Format::Disabled;

    QVector <std::shared_ptr<FFFormatConverterThread>> thread;

    int index_thread_src;
    int index_thread_dst;

    QQueue <AVFrameSP::ptr> queue_converted;

    bool use_multithreading=false;

signals:
    void frameSkipped();
};

#endif // FF_FORMAT_CONVERTER_MULTITHREADED_H
