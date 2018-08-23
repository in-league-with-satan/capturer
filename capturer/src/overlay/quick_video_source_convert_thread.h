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

#ifndef QUICK_VIDEO_SOURCE_CONVERT_THREAD_H
#define QUICK_VIDEO_SOURCE_CONVERT_THREAD_H

#include <QThread>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>

#include <atomic>

#include "frame_buffer.h"

class AVFrame;

class FFFormatConverter;
class DecklinkFrameConverter;

class QuickVideoSourceConvertThread : public QThread
{
    Q_OBJECT

public:
    explicit QuickVideoSourceConvertThread(QObject *parent=0);
    ~QuickVideoSourceConvertThread();

    FrameBuffer <Frame::ptr>::ptr frameBufferIn();
    FrameBuffer <Frame::ptr>::ptr frameBufferOut();

    bool fastYuv() const;

public slots:
    void setFastYuv(bool value);

protected:
    virtual void run();

private:
    FrameBuffer <Frame::ptr>::ptr frame_buffer_in;
    FrameBuffer <Frame::ptr>::ptr frame_buffer_out;

    std::atomic <bool> running;
    std::atomic <bool> fast_yuv;

    AVFrame *conv_src;
    AVFrame *conv_dst;

    FFFormatConverter *format_converter_ff;
    DecklinkFrameConverter *format_converter_dl;

signals:
    void gotFrame();
};

#endif // QUICK_VIDEO_SOURCE_CONVERT_THREAD_H
