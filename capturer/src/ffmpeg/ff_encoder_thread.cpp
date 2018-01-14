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

#include "capture.h"

#include "ff_encoder_thread.h"

FFEncoderThread::FFEncoderThread(FFEncoder::Mode::T mode, FFEncoderBaseFilename *base_filename, QObject *parent)
    : QThread(parent)
    , mode(mode)
    , base_filename(base_filename)
{
    frame_buffer=FrameBuffer::make();

    frame_buffer->setMaxSize(120);

    frame_buffer->setEnabled(false);

    is_working=false;

    start(QThread::LowestPriority);

    // start(QThread::NormalPriority);
    // start(QThread::HighPriority);
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

FrameBuffer::ptr FFEncoderThread::frameBuffer()
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

    frame_buffer->clear();
    frame_buffer->setEnabled(true);
}

void FFEncoderThread::stopCoder()
{
    emit sigStopCoder();
}

void FFEncoderThread::onStateChanged(bool state)
{
    is_working=state;

    if(!state) {
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

    Frame::ptr frame;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            ffmpeg->appendFrame(frame);

            frame.reset();
        }

        QCoreApplication::processEvents();
    }
}
