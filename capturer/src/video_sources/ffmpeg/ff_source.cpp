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
#include <QAudioInput>
#include <QMessageBox>
#include <qcoreapplication.h>

#include <future>
#include <thread>
#include <chrono>

#include "tools_ff_source.h"
#include "ff_source_worker.h"

#include "ff_source.h"


QList <FFDevice::Dev> dev_list;


FFSource::FFSource(QObject *parent)
    : QThread(parent)
    , d(nullptr)
{
    audio_channels=AudioChannels::ch_2;
    audio_sample_size=AudioSampleSize::bitdepth_16;

    start();

    while(!running) {
        usleep(20);
    }
}

FFSource::~FFSource()
{
    deviceStop();

    qApp->processEvents();

    running=false;

    while(isRunning()) {
        msleep(30);
    }
}

SourceInterface::Type::T FFSource::type() const
{
    return Type::ffmpeg;
}

QStringList FFSource::availableVideoInput()
{
    QStringList list;

    if(dev_list.isEmpty())
        updateDevList();

    foreach(FFDevice::Dev dev, dev_list) {
        list << dev.name;
    }

    return list;
}

QStringList FFSource::availableAudioInput()
{
    auto future=std::async(std::launch::async, []()->QStringList {
        QStringList list;

        foreach(const QAudioDeviceInfo &device_info, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
            list << device_info.deviceName();

        return list;
    });

    if(future.wait_for(std::chrono::seconds(5))!=std::future_status::ready) {
        QMessageBox mb;
        mb.setWindowTitle("critical error");
        mb.setText("QAudioDeviceInfo::availableDevices function doesn't return control");
        mb.setIcon(QMessageBox::Critical);

        mb.exec();

        std::exit(1);
    }

    return future.get();
}

bool FFSource::setVideoDevice(int index)
{
    if(index>=dev_list.size())
        return false;

    QMutexLocker ml(&mutex);

    if(!d)
        return false;

    index_device_video=index;

    if(index_device_video<0) {
        d->setVideoDevice(FFDevice::Dev());
        type_flags&=~TypeFlag::video;

    } else {
        d->setVideoDevice(dev_list[index_device_video]);
        type_flags|=TypeFlag::video;
        qDebug() << index_device_video << dev_list[index_device_video].name;
    }

    return true;
}

void FFSource::setAudioDevice(int index)
{
    QMutexLocker ml(&mutex);

    if(!d)
        return;

    d->setAudioDevice(index);
}

void FFSource::setDevice(void *ptr)
{
    FFSource::Device *dev=reinterpret_cast<FFSource::Device*>(ptr);

    QMutexLocker ml(&mutex);

    if(d)
        d->setConfig(dev->size, dev->framerate, dev->pixel_format);

    delete dev;
}

QList <QSize> FFSource::supportedResolutions()
{
    if(index_device_video<0)
        return QList<QSize>();

    if(dev_list.size() - 1<index_device_video)
        return QList<QSize>();


    QList <QSize> lst;

    QMap <int64_t, QList <QSize>> res_per_format;

    foreach(FFDevice::Format format, dev_list.at(index_device_video).format) {
        foreach(FFDevice::Resolution res, format.resolution) {
            if(res.framerate.isEmpty()) {
                qWarning() << "framerate.isEmpty" << dev_list.at(index_device_video).name << res.size;
                continue;
            }

            res_per_format[format.pixel_format].append(res.size);

            if(!lst.contains(res.size)) {
                lst << res.size;
            }
        }
    }

    std::sort(lst.begin(), lst.end(),
              [](const QSize &l, const QSize &r) { return (l.width()==r.width() ? l.height()<r.height() : l.width()<r.width()); }
    );

    qDebug() << dev_list.at(index_device_video).name << lst;

    return lst;
}

QList <int> FFSource::supportedPixelFormats(QSize size)
{
    if(index_device_video<0)
        return QList<int>();

    if(dev_list.size() - 1<index_device_video)
        return QList<int>();

    QSet <int> set;

    foreach(FFDevice::Format format, dev_list.at(index_device_video).format) {
        foreach(FFDevice::Resolution res, format.resolution) {
            if(res.size==size) {
                set << format.pixel_format;
            }
        }
    }

    QList <int> list=set.toList();

    std::sort(list.begin(), list.end());

    return list;
}

QList <AVRational> FFSource::supportedFramerates(QSize size, int fmt)
{
    if(index_device_video<0)
        return QList<AVRational>();

    if(dev_list.size() - 1<index_device_video)
        return QList<AVRational>();

    foreach(FFDevice::Format format, dev_list.at(index_device_video).format) {
        if(format.pixel_format==fmt) {
            foreach(FFDevice::Resolution res, format.resolution) {
                if(res.size==size) {
                    // qDebug() << size << PixelFormat(fmt).toString() << res.framerate.size();
                    return res.framerate;
                }
            }
        }
    }

    qWarning() << "empty" << dev_list.at(index_device_video).name << size << PixelFormat(fmt).toString();

    return QList<AVRational>();
}

void FFSource::subscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    QMutexLocker ml(&mutex);

    if(!d)
        return;

    d->subscribe(obj);
}

void FFSource::unsubscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    QMutexLocker ml(&mutex);

    if(!d)
        return;

    d->unsubscribe(obj);
}

bool FFSource::isActive()
{
    QMutexLocker ml(&mutex);

    if(!d)
        return false;

    return d->isActive();
}

bool FFSource::gotSignal()
{
    return isActive();
}

AVRational FFSource::currentFrameRate()
{
    QMutexLocker ml(&mutex);

    if(!d)
        return { 1, 1 };

    return d->currentFrameRate();
}

PixelFormat FFSource::pixelFormat()
{
    QMutexLocker ml(&mutex);

    if(!d)
        return PixelFormat::undefined;

    return d->pixelFormat();
}

void FFSource::updateDevList()
{
    dev_list=ToolsFFSource::devList();

    ToolsFFSource::testDevList(dev_list);
}

void FFSource::run()
{
    d=new FFSourceWorker(this);
    d->moveToThread(this);

    connect(this, SIGNAL(setConfig(QSize,AVRational,int64_t)), d, SLOT(setConfig(QSize,AVRational,int64_t)), Qt::QueuedConnection);
    connect(this, SIGNAL(deviceStart()), d, SLOT(deviceStart()), Qt::QueuedConnection);
    connect(this, SIGNAL(deviceStop()), d, SLOT(deviceStop()), Qt::QueuedConnection);

    connect(d, SIGNAL(formatChanged(QString)), SIGNAL(formatChanged(QString)), Qt::QueuedConnection);
    connect(d, SIGNAL(errorString(QString)), SIGNAL(errorString(QString)), Qt::QueuedConnection);
    connect(d, SIGNAL(signalLost(bool)), SIGNAL(signalLost(bool)), Qt::QueuedConnection);


    bool is_active;

    running=true;

    while(running) {
        mutex.lock();

        is_active=d->step();

        mutex.unlock();


        if(is_active)
            usleep(1);

        else
            msleep(100);


        qApp->processEvents();
    }

    delete d;
}
