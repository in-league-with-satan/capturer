/******************************************************************************

Copyright © 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <qcoreapplication.h>

#include <chrono>
#include <ctime>

#include "framerate.h"
#include "screen_capture_worker_dda.h"

#include "screen_capture.h"

ScreenCapture::ScreenCapture(int device_index, QObject *parent)
    : QThread(parent)
    , SourceInterface(device_index)
    , d(nullptr)
{
    framerate=Framerate::toRational(60.);

    running=false;
    on_hold=false;

    start(QThread::HighPriority);

    while(!running) {
        msleep(2);
    }
}

ScreenCapture::~ScreenCapture()
{
    running=false;

    while(isRunning()) {
        msleep(30);
    }
}

QStringList ScreenCapture::availableAudioInput() const
{
    if(!d)
        return QStringList();

    return d->availableAudioInput();
}

int ScreenCapture::indexAudioInput(const QString &name)
{
    QStringList dev_list=availableAudioInput();

    for(int i=0; i<dev_list.size(); ++i)
        if(QString::compare(name, dev_list[i])==0)
            return i;

    return -1;
}

SourceInterface::Type::T ScreenCapture::type() const
{
    return SourceInterface::Type::screen_capture;
}

bool ScreenCapture::isImplemented() const
{
    return true; //?
}

bool ScreenCapture::isActive()
{
    return !signal_lost;
}

bool ScreenCapture::gotSignal()
{
    return !signal_lost;
}

void ScreenCapture::deviceHold()
{
    ;
}

void ScreenCapture::deviceResume()
{
    ;
}

void ScreenCapture::setDevice(void *ptr)
{
    Device *dev=reinterpret_cast<Device*>(ptr);

    if(d) {
        d->setAudioDevice(dev->audio_device_name);
    }

    setUpperFramerateLimit((ScreenCapture::FramerateLimit::T)dev->framerate_limit);

    delete dev;
}

void ScreenCapture::setAudioDevice(QString device_name)
{
    if(d)
        d->setAudioDevice(device_name);
}

void ScreenCapture::setUpperFramerateLimit(ScreenCapture::FramerateLimit::T lim)
{
    framerate=Framerate::toRational(ScreenCapture::FramerateLimit::value(lim));
}

void ScreenCapture::run()
{
    d=new ScreenCaptureWorkerDda(this);

    d->moveToThread(this);

    connect(this, SIGNAL(deviceStart()), d, SLOT(deviceStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(deviceStop()), d, SLOT(deviceStop()), Qt::QueuedConnection);

    connect(d, SIGNAL(signalLost(bool)), SIGNAL(signalLost(bool)), Qt::QueuedConnection);
    connect(d, SIGNAL(errorString(QString)), SIGNAL(errorString(QString)), Qt::QueuedConnection);
    connect(d, SIGNAL(formatChanged(QString)), SIGNAL(formatChanged(QString)), Qt::QueuedConnection);

    bool is_active;

    running=true;

    QElapsedTimer timer;
    timer.restart();

    double avg_frame_time=0;
    std::chrono::high_resolution_clock::time_point frame_start_time_point;

    while(running) {
        if(on_hold) {
            msleep(200);
            continue;
        }

        mutex.lock();

        frame_start_time_point=std::chrono::high_resolution_clock::now();

        is_active=d->step();

        avg_frame_time=(avg_frame_time + std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - frame_start_time_point).count())*.5;

        mutex.unlock();

        if(timer.elapsed()>=1000) {
            qDebug() << "fps:" << 1000*1000*1000/avg_frame_time;
            timer.restart();
        }

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

        if(!is_active)
            msleep(50);
    }

    delete d;
}
