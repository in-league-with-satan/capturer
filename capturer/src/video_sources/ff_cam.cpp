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

#include "tools_cam.h"
#include "ff_cam_worker.h"

#include "ff_cam.h"


QList <Cam::Dev> dev_list;


FFCam::FFCam(QObject *parent)
    : QThread(parent)
{
    start();
}

FFCam::~FFCam()
{
    stop();

    quit();

    running=false;

    while(isRunning()) {
        msleep(30);
    }
}

QStringList FFCam::availableCameras()
{
    QStringList list;

    if(dev_list.isEmpty())
        updateDevList();

    foreach(Cam::Dev dev, dev_list) {
        list << dev.name;
    }

    return list;
}

QStringList FFCam::availableAudioInput()
{
    // return QStringList();

    auto future=std::async(std::launch::async, []()->QStringList {
        QStringList list;

        foreach(const QAudioDeviceInfo &device_info, QAudioDeviceInfo::availableDevices(QAudio::AudioInput))
            list << device_info.deviceName();

        return list;
    });

    if(future.wait_for(std::chrono::seconds(2))!=std::future_status::ready) {
        QMessageBox mb;
        mb.setWindowTitle("critical error");
        mb.setText("QAudioDeviceInfo::availableDevices function doesn't return control");
        mb.setIcon(QMessageBox::Critical);

        mb.exec();

        std::exit(1);
    }

    return future.get();
}

bool FFCam::setVideoDevice(int index)
{
    if(index<0 || index>=dev_list.size())
        return false;

    QMutexLocker ml(&mutex);

    index_device_video=index;

    d->setVideoDevice(dev_list[index_device_video]);

    qInfo() << "FFCam::setVideoDevice:" << index_device_video << dev_list[index_device_video].name;

    return true;
}

void FFCam::setAudioDevice(int index)
{
    QMutexLocker ml(&mutex);

    d->setAudioDevice(index);
}

QList <QSize> FFCam::supportedResolutions()
{
    if(dev_list.size() - 1<index_device_video)
        return QList<QSize>();


    QList <QSize> lst;

    QMap <int64_t, QList <QSize>> res_per_format;

    foreach(Cam::Format format, dev_list.at(index_device_video).format) {
        foreach(Cam::Resolution res, format.resolution) {
            if(res.framerate.isEmpty()) {
                qWarning() << "FFCam::supportedResolutions: framerate.isEmpty" << dev_list.at(index_device_video).name << res.size;
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

    qInfo() << "FFCam::supportedResolutions:" << dev_list.at(index_device_video).name << lst;

    return lst;
}

QList <int64_t> FFCam::supportedPixelFormats(QSize size)
{
    if(dev_list.size() - 1<index_device_video)
        return QList<int64_t>();

    QSet <int64_t> set;

    foreach(Cam::Format format, dev_list.at(index_device_video).format) {
        foreach(Cam::Resolution res, format.resolution) {
            if(res.size==size) {
                set << format.pixel_format;
            }
        }
    }

    return set.toList();
}

QList <AVRational> FFCam::supportedFramerates(QSize size, int64_t fmt)
{
    if(dev_list.size() - 1<index_device_video)
        return QList<AVRational>();

    foreach(Cam::Format format, dev_list.at(index_device_video).format) {
        if(format.pixel_format==fmt) {
            foreach(Cam::Resolution res, format.resolution) {
                if(res.size==size) {
                    // qInfo() << "FFCam::supportedFramerates" << size << PixelFormat(fmt).toString() << res.framerate.size();
                    return res.framerate;
                }
            }
        }
    }

    qWarning() << "FFCam::supportedFramerates: empty" << dev_list.at(index_device_video).name << size << PixelFormat(fmt).toString();

    return QList<AVRational>();
}

void FFCam::subscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    QMutexLocker ml(&mutex);

    d->subscribe(obj);
}

void FFCam::unsubscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    QMutexLocker ml(&mutex);

    d->unsubscribe(obj);
}

bool FFCam::isActive()
{
    QMutexLocker ml(&mutex);

    return d->isActive();
}

AVRational FFCam::currentFrameRate()
{
    QMutexLocker ml(&mutex);

    return d->currentFrameRate();
}

PixelFormat FFCam::pixelFormat()
{
    QMutexLocker ml(&mutex);

    return d->pixelFormat();
}

void FFCam::updateDevList()
{
    dev_list=ToolsCam::devList();

    ToolsCam::testDevList(dev_list);
}

void FFCam::run()
{
    running=true;


    d=new FFCamWorker();
    d->moveToThread(this);

    connect(this, SIGNAL(setConfig(QSize,AVRational,int64_t)), d, SLOT(setConfig(QSize,AVRational,int64_t)), Qt::QueuedConnection);
    connect(this, SIGNAL(startCam()), d, SLOT(startCam()), Qt::QueuedConnection);
    connect(this, SIGNAL(stop()), d, SLOT(stop()), Qt::QueuedConnection);


    bool is_active;


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
