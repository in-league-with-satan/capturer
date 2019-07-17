/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QDateTime>

#include "framerate.h"

#include "dummy_device.h"

DummyDevice::DummyDevice(QObject *parent)
    : QThread(parent)
{
    type_flags=TypeFlag::video;

    framesize=QSize(1920, 1080);
    framerate=Framerate::toRational(60.);
    pixel_format=PixelFormat::rgb24;
    frame_counter=false;
    running=false;

    start(QThread::NormalPriority);

    while(!running_thread) {
        msleep(2);
    }
}

DummyDevice::~DummyDevice()
{
    running_thread=false;

    while(isRunning()) {
        msleep(30);
    }
}

SourceInterface::Type::T DummyDevice::type() const
{
    return Type::dummy;
}

bool DummyDevice::isActive()
{
    return running;
}

bool DummyDevice::gotSignal()
{
    return running;
}

DummyDevice::Framesizes DummyDevice::availableFramesizes()
{
    static Framesizes list;

    if(list.isEmpty()) {
        list << QSize(640, 480)
             << QSize(800, 600)
             << QSize(1280, 720)
             << QSize(1920, 1080)
             << QSize(3840, 2160);
    }

    return list;
}

void DummyDevice::run()
{
    std::srand(std::time(nullptr));


    mutex.lock();

    glazing_ribbon.init(framesize, 24, 1, true, QPainter::CompositionMode_SourceOver);
    glazing_ribbon.showFrameCounter(frame_counter);

    mutex.unlock();


    time_last_frame=QDateTime::currentMSecsSinceEpoch();

    qint64 t;

    QImage img;

    running_thread=true;

    while(running_thread) {
        if(running) {
            mutex.lock();

            img=glazing_ribbon.next();

            mutex.unlock();


            if(!img.isNull() && !subscription_list.isEmpty()) {
                Frame::ptr frame=Frame::make();

                frame->video.size=img.size();
                frame->video.data_size=frameBufSize(frame->video.size, PixelFormat::rgb24);
                frame->video.dummy.resize(frame->video.data_size);
                frame->video.data_ptr=(uint8_t*)frame->video.dummy.constData();
                frame->video.pixel_format=PixelFormat::rgb24;

                memcpy(frame->video.data_ptr, img.bits(), frame->video.data_size);

                t=16 - (QDateTime::currentMSecsSinceEpoch() - time_last_frame);

                if(t>0)
                    msleep(t);

                foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
                    buf->append(frame);

                time_last_frame=QDateTime::currentMSecsSinceEpoch();

            } else {
                msleep(30);
            }

        } else {
            msleep(300);
        }
    }
}

void DummyDevice::deviceStart()
{
    running=true;

    emit formatChanged(QString("%1p@60 8BitRGB").arg(framesize.load().height()));
    emit signalLost(false);
}

void DummyDevice::deviceStop()
{
    running=false;
}

void DummyDevice::deviceHold()
{
    deviceStop();
}

void DummyDevice::deviceResume()
{
    deviceStart();
}

void DummyDevice::setDevice(void *ptr)
{
    DummyDevice::Device *dev=reinterpret_cast<DummyDevice::Device*>(ptr);

    framesize=dev->frame_size;
    frame_counter=dev->show_frame_counter;

    mutex.lock();

    glazing_ribbon.init(framesize, 24, 1, true, QPainter::CompositionMode_SourceOver);
    glazing_ribbon.showFrameCounter(frame_counter);

    mutex.unlock();

    emit formatChanged(QString("%1p@60 8BitRGB").arg(framesize.load().height()));

    delete dev;
}
