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
#include <qcoreapplication.h>

#include "decklink_thread.h"

#include "ff_encoder_thread.h"

FFEncoderThread::FFEncoderThread(FFEncoder::Mode::T mode, FFEncoderBaseFilename *base_filename, QString encoding_tool_name, QObject *parent)
    : QThread(parent)
    , base_filename(base_filename)
    , encoding_tool_name(encoding_tool_name)
    , mode(mode)
{
    frame_buffer=FrameBuffer<Frame::ptr>::make();

    frame_buffer->setMaxSize(120);

    frame_buffer->setEnabled(false);

    is_working=false;

    // start(QThread::LowestPriority);
    // start(QThread::NormalPriority);
    start(QThread::HighPriority);
    // start(QThread::HighestPriority);
    // start(QThread::TimeCriticalPriority);
}

FFEncoderThread::~FFEncoderThread()
{
    running=false;

    frame_buffer->setEnabled(true);
    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer<Frame::ptr>::ptr FFEncoderThread::frameBuffer()
{
    return frame_buffer;
}

bool FFEncoderThread::isWorking()
{
    return is_working;
}

void FFEncoderThread::setConfig(FFEncoder::Config cfg)
{
    emit sigSetConfig(cfg);
}

void FFEncoderThread::stopCoder()
{
    // frame_buffer->setEnabled(false);
    // frame_buffer->clear();

    emit sigStopCoder();
}

void FFEncoderThread::onStateChanged(bool state)
{
    is_working=state;

    if(state) {
        frame_buffer->clear();
        frame_buffer->setEnabled(true);

    } else {
        frame_buffer->setEnabled(false);
        frame_buffer->clear();
    }
}

void FFEncoderThread::run()
{
    FFEncoder *ffmpeg=new FFEncoder(mode);

    ffmpeg->moveToThread(this);

    connect(this, SIGNAL(sigSetConfig(FFEncoder::Config)), ffmpeg, SLOT(setConfig(FFEncoder::Config)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigStopCoder()), ffmpeg, SLOT(stopCoder()), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stats(FFEncoder::Stats)), SIGNAL(stats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stateChanged(bool)), SIGNAL(stateChanged(bool)), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stateChanged(bool)), SLOT(onStateChanged(bool)), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(errorString(QString)), SIGNAL(errorString(QString)), Qt::QueuedConnection);

    ffmpeg->setBaseFilename(base_filename);
    ffmpeg->setEncodingToolName(encoding_tool_name);

    Frame::ptr frame;

    running=true;

    while(running) {
        frame_buffer->wait();

        while(frame=frame_buffer->take()) {
            ffmpeg->appendFrame(frame);

            frame.reset();

            while(frame_buffer->size().first>=50) {
                ffmpeg->appendFrame(frame_buffer->take()->copyFrameSoundOnly());
            }
        }

        QCoreApplication::processEvents();
    }
}
