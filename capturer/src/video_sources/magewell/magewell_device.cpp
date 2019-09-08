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
#include <qcoreapplication.h>

#ifdef LIB_MWCAPTURE

#include "MWCapture.h"

#endif

#include "magewell_lib.h"

#include "magewell_device_worker.h"

#include "magewell_device.h"


MagewellDevice::MagewellDevice(int device_index, QObject *parent)
    : QThread(parent)
    , SourceInterface(device_index)
    , d(nullptr)
{
    type_flags=TypeFlag::audio | TypeFlag::video;

    audio_channels=AudioChannels::ch_8;

    running=false;
    on_hold=false;

    start(QThread::HighPriority);

    while(!running) {
        msleep(2);
    }
}

MagewellDevice::~MagewellDevice()
{
    running=false;

    while(isRunning()) {
        msleep(30);
    }
}

SourceInterface::Type::T MagewellDevice::type() const
{
    return Type::magewell;
}

bool MagewellDevice::isImplemented() const
{
#ifndef LIB_MWCAPTURE
    return false;
#endif

    return MagewellLib::isLoaded();
}

void MagewellDevice::init()
{
#ifdef LIB_MWCAPTURE

    MWCaptureInitInstance();
    MWRefreshDevice();

#endif
}

void MagewellDevice::release()
{
#ifdef LIB_MWCAPTURE

    MWCaptureExitInstance();

#endif
}

MagewellDevice::Devices MagewellDevice::availableDevices()
{
    static Devices list;

#ifdef LIB_MWCAPTURE

    if(!list.isEmpty())
        return list;

    const int channel_count=MWGetChannelCount();

    unsigned int temperature=0;

#ifdef __linux__
    char tmp[128]={};
#else
    wchar_t tmp[128]={};
#endif

    MWCAP_CHANNEL_INFO channel_info;

    MagewellDevice::Device dev;

    HCHANNEL channel;

    int ret;

    for(int i=0; i<channel_count; ++i) {
        ret=MWGetDevicePath(i, tmp);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetDevicePath err" << i;
            goto ad_close;
        }

#ifdef __linux__
        dev.path=QString(tmp);
#else
        dev.path=QString::fromWCharArray(tmp);
#endif
        //

        channel=MWOpenChannelByPath(tmp);

        if(channel==0) {
            qCritical() << "MWOpenChannelByPath err";
            continue;
        }

        //

        ret=MWGetChannelInfo(channel, &channel_info);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetChannelInfo err" << channel;
            goto ad_close;
        }

        dev.name=QString(channel_info.szProductName);
        dev.index_board=channel_info.byBoardIndex;
        dev.index_channel=channel_info.byChannelIndex;


        list.append(dev);

        //

        MWGetTemperature(channel, &temperature);

        qInfo() << channel << dev.name << dev.path;
        qInfo() << "temperature:" << temperature/10.;

ad_close:

        MWCloseChannel(channel);
    }

#endif

    return list;
}

PixelFormats MagewellDevice::supportedPixelFormats()
{
    static PixelFormats list;

    if(list.isEmpty()) {
        list << PixelFormat::bgr24
             // << PixelFormat::rgb0
             << PixelFormat::bgra
             // << PixelFormat::gbrp10
             << PixelFormat::yuv420p
             << PixelFormat::yuyv422
             << PixelFormat::uyvy422
             // << PixelFormat::yuv422p10
             << PixelFormat::yuv444p10
             << PixelFormat::p010
             << PixelFormat::nv12;
    }

    return list;
}

void MagewellDevice::subscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    QMutexLocker ml(&mutex);

    if(!d)
        return;

    d->subscribe(obj);
}

void MagewellDevice::unsubscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    QMutexLocker ml(&mutex);

    if(!d)
        return;

    d->unsubscribe(obj);
}

void MagewellDevice::setDevice(void *ptr)
{
    MagewellDevice::Device *dev=reinterpret_cast<MagewellDevice::Device*>(ptr);

    if(dev->name.isEmpty() || dev->path.isEmpty())
        return;

    device=(*dev);

    delete dev;

    pixel_format=device.pixel_format;

    emit updateDevice(device);
}

void MagewellDevice::deviceHold()
{
    on_hold=true;
}

void MagewellDevice::deviceResume()
{
    on_hold=false;
}

void MagewellDevice::setFramerate(AVRational fr)
{
    if(sender()!=d)
        return;

    framerate=fr;
}

void MagewellDevice::setFramesize(QSize r)
{
    if(sender()!=d)
        return;

    framesize=r;
}

void MagewellDevice::setAudioSampleSize(SourceInterface::AudioSampleSize::T value)
{
    if(sender()!=d)
        return;

    audio_sample_size=value;
}

void MagewellDevice::setAudioChannels(SourceInterface::AudioChannels::T value)
{
    if(sender()!=d)
        return;

    audio_channels=value;
}

bool MagewellDevice::isActive()
{
    return false;
}

void MagewellDevice::run()
{
    d=new MagewellDeviceWorker(&device_index);

    d->moveToThread(this);

    connect(this, SIGNAL(updateDevice(MagewellDevice::Device)), d, SLOT(setDevice(MagewellDevice::Device)), Qt::QueuedConnection);

    connect(this, SIGNAL(deviceStart()), d, SLOT(deviceStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(deviceStop()), d, SLOT(deviceStop()), Qt::QueuedConnection);

    connect(d, SIGNAL(signalLost(bool)), SIGNAL(signalLost(bool)), Qt::QueuedConnection);
    connect(d, SIGNAL(errorString(QString)), SIGNAL(errorString(QString)), Qt::QueuedConnection);
    connect(d, SIGNAL(formatChanged(QString)), SIGNAL(formatChanged(QString)), Qt::QueuedConnection);

    connect(d, &MagewellDeviceWorker::setMasteringDisplayMetadata, [this](AVMasteringDisplayMetadata value) { mastering_display_metadata=value; });

    connect(d, &MagewellDeviceWorker::signalLost, [this](bool value) { signal_lost=value; });
    connect(d, &MagewellDeviceWorker::audioSampleSizeChanged, [this](AudioSampleSize::T value) { audio_sample_size=value; });
    connect(d, &MagewellDeviceWorker::audioChannelsChanged, [this](AudioChannels::T value) { audio_channels=value; });

    connect(d, SIGNAL(framerateChanged(AVRational)), SLOT(setFramerate(AVRational)), Qt::QueuedConnection);
    connect(d, SIGNAL(framesizeChanged(QSize)), SLOT(setFramesize(QSize)), Qt::QueuedConnection);

    connect(d, SIGNAL(audioSampleSizeChanged(SourceInterface::AudioSampleSize::T)), SLOT(setAudioSampleSize(SourceInterface::AudioSampleSize::T)), Qt::QueuedConnection);


    bool is_active;

    running=true;

    while(running) {
        if(on_hold) {
            msleep(200);
            continue;
        }

        mutex.lock();

        is_active=d->step();

        mutex.unlock();

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

        if(!is_active)
            msleep(50);
    }

    delete d;
}
