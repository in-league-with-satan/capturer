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

#include "MWCapture.h"

#include "magewell_device_worker.h"

#include "magewell_device.h"


MagewellDevice::MagewellDevice(QObject *parent)
    : QThread(parent)
    , d(nullptr)
{
    audio_channels=AudioChannels::ch_8;

    running=false;

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

void MagewellDevice::init()
{
    MWCaptureInitInstance();
    MWRefreshDevice();
}

MagewellDevice::Devices MagewellDevice::availableDevices()
{
    static Devices list;

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

    return list;
}

PixelFormats MagewellDevice::supportedPixelFormats()
{
    static PixelFormats list;

    if(list.isEmpty()) {
        list << PixelFormat::bgr24
             // << PixelFormat::rgb0
             << PixelFormat::bgra
             // << PixelFormat::gbrp10le
             << PixelFormat::yuv420p
             << PixelFormat::yuyv422
             << PixelFormat::uyvy422
             // << PixelFormat::yuv422p10le
             << PixelFormat::p010le
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

    device=(*dev);

    delete dev;

    pixel_format=device.pixel_format;

    emit setPixelFormat(device.pixel_format);
    emit setColorFormat(device.color_format);
    emit setQuantizationRange(device.quantization_range);
    emit setPtsEnabled(device.pts_enabled);
    emit setDevice(QSize(device.index_board, device.index_channel));
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
    d=new MagewellDeviceWorker();

    d->moveToThread(this);

    connect(this, SIGNAL(setDevice(QSize)), d, SLOT(setDevice(QSize)), Qt::QueuedConnection);
    connect(this, SIGNAL(setPixelFormat(PixelFormat)), d, SLOT(setPixelFormat(PixelFormat)), Qt::QueuedConnection);
    connect(this, SIGNAL(setColorFormat(int)), d, SLOT(setColorFormat(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(setQuantizationRange(int)), d, SLOT(setQuantizationRange(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(setPtsEnabled(bool)), d, SLOT(setPtsEnabled(bool)), Qt::QueuedConnection);

    connect(this, SIGNAL(deviceStart()), d, SLOT(deviceStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(deviceStop()), d, SLOT(deviceStop()), Qt::QueuedConnection);

    connect(d, SIGNAL(signalLost(bool)), SIGNAL(signalLost(bool)), Qt::QueuedConnection);
    connect(d, SIGNAL(errorString(QString)), SIGNAL(errorString(QString)), Qt::QueuedConnection);
    connect(d, SIGNAL(formatChanged(QString)), SIGNAL(formatChanged(QString)), Qt::QueuedConnection);

    connect(d, &MagewellDeviceWorker::signalLost, [this](bool value){ signal_lost=value; });
    connect(d, &MagewellDeviceWorker::audioSampleSizeChanged, [this](AudioSampleSize::T value){ audio_sample_size=value; });
    connect(d, &MagewellDeviceWorker::audioChannelsChanged, [this](AudioChannels::T value){ audio_channels=value; });

    connect(d, SIGNAL(framerateChanged(AVRational)), SLOT(setFramerate(AVRational)), Qt::QueuedConnection);
    connect(d, SIGNAL(framesizeChanged(QSize)), SLOT(setFramesize(QSize)), Qt::QueuedConnection);

    connect(d, SIGNAL(audioSampleSizeChanged(SourceInterface::AudioSampleSize::T)), SLOT(setAudioSampleSize(SourceInterface::AudioSampleSize::T)), Qt::QueuedConnection);


    bool is_active;

    running=true;

    while(running) {
        mutex.lock();

        is_active=d->step();

        mutex.unlock();

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers);

        if(!is_active)
            msleep(50);
    }

    delete d;
}
