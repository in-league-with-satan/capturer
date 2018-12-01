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
#include <QLibrary>
#include <QTimer>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>

#include <iostream>

#ifdef __linux__

#include <stdlib.h>

#endif

#include "nv_lib.h"

#include "nv_tools.h"

struct NvToolsPrivate
{
    NvLib lib;

    int dev_size=0;

    QList <int> list_mon;

    QTimer *timer;

    QStringList device_list;
    bool device_list_ready=false;

    int *gpu_handle[NVAPI_MAX_PHYSICAL_GPUS]={ 0 };
    unsigned int gpu_usage[NVAPI_MAX_PHYSICAL_GPUS][NVAPI_MAX_USAGES_PER_GPU]={ 0 };
    NV_GPU_THERMAL_SETTINGS nv_gpu_thermal_settings[NVAPI_MAX_PHYSICAL_GPUS];

    QProcess proc;

    QMutex mutex;
};

NvTools::NvTools(QObject *parent)
    : QThread(parent)
    , d(new NvToolsPrivate())

{
    d->lib.cuInit(0);

#ifdef __linux__

    setenv("DISPLAY", ":0", 0);

#else

    d->proc.setProcessChannelMode(QProcess::MergedChannels);

    availableDevices();

    d->lib.nvapiEnumPhysicalGPUs(d->gpu_handle, &d->dev_size);

    for(int i=0; i<NVAPI_MAX_PHYSICAL_GPUS; ++i) {
        if(!d->gpu_handle[i])
            continue;

        d->gpu_usage[i][0]=(NVAPI_MAX_USAGES_PER_GPU*4) | 0x10000;

        d->nv_gpu_thermal_settings[i].version=sizeof(NV_GPU_THERMAL_SETTINGS) | (1 << 16);
        d->nv_gpu_thermal_settings[i].count=0;
        d->nv_gpu_thermal_settings[i].sensor[0].controller=-1;
        d->nv_gpu_thermal_settings[i].sensor[0].target=NVAPI_THERMAL_TARGET_GPU;
    }

#endif

    start();
}

NvTools::~NvTools()
{
    while(isRunning()) {
        quit();
        msleep(1);
    }

    delete d;
}

QStringList NvTools::availableDevices()
{
    if(d->device_list_ready)
        return d->device_list;

    d->device_list_ready=true;

    if(!d->lib.loadedCuda())
        return d->device_list;

    d->dev_size=0;

    int result=d->lib.cuDeviceGetCount(&d->dev_size);

    if(result!=0) {
        const char *str;

        d->lib.cuGetErrorName(result, &str);

        qCritical() << QString(str);
    }

    char str[255];

    for(int i=0; i<d->dev_size; ++i) {
        memset(str, 0, sizeof(char)*255);

        d->lib.cuDeviceGetName(str, 255, i);

        d->device_list << QString(str);
    }

    return d->device_list;
}

void NvTools::monitoringStart(int index)
{
    QMutexLocker ml(&d->mutex);

    if(!d->lib.loadedCuda())
        return;

    if(index<0 || index>=NVAPI_MAX_PHYSICAL_GPUS)
        return;

#ifndef __linux__

    if(!d->gpu_handle[index])
        return;

#endif

    if(!d->list_mon.contains(index))
        d->list_mon.append(index);
}

void NvTools::monitoringStop(int index)
{
    QMutexLocker ml(&d->mutex);

    d->list_mon.removeAll(index);
}

void NvTools::onTimer()
{
    QMutexLocker ml(&d->mutex);

    if(d->device_list.isEmpty()) {
        d->timer->stop();
        return;
    }

    NvState state;

#ifdef __linux__

    foreach(const int &index, d->list_mon) {
        d->proc.start(QString("nvidia-settings -q=[gpu:%1]/GPUUtilization -q=[gpu:%1]/GPUCoreTemp").arg(index));
        d->proc.waitForFinished();

        QStringList lst=QString(d->proc.readAll())
                .remove(QStringLiteral("\n"))
                .remove(QStringLiteral("'"))
                .remove(QStringLiteral(","))
                .split(QStringLiteral(" "),
                       QString::SkipEmptyParts);

        if(!lst.contains(QStringLiteral("GPUUtilization")) || lst.size()<6) {
            d->timer->stop();
            return;
        }

        foreach(QString str, lst) {
            bool ok=false;
            double t=str.toDouble(&ok);

            if(ok)
                state.temperature=t;

            if(str.startsWith(QStringLiteral("graphics=")))
                state.graphic_processing_unit=str.remove(QStringLiteral("graphics=")).toInt();

            if(str.startsWith(QStringLiteral("memory=")))
                state.memory_controller_unit=str.remove(QStringLiteral("memory=")).toInt();

            if(str.startsWith(QStringLiteral("video=")))
                state.video_processing_unit=str.remove(QStringLiteral("video=")).toInt();
        }

        state.dev_name=d->device_list.at(index);

        emit stateChanged(state);
    }

#else

    foreach(const int &index, d->list_mon) {
        if(d->device_list.size()<=index)
            return;

        d->lib.nvapiGPUGetUsages(d->gpu_handle[index], d->gpu_usage[index]);
        d->lib.nvapiGPUGetThermalSettings(d->gpu_handle[index], 0, &d->nv_gpu_thermal_settings[index]);

        state.dev_name=d->device_list.at(index);
        state.temperature=d->nv_gpu_thermal_settings[index].sensor[0].temp_current;
        state.graphic_processing_unit=d->gpu_usage[index][3];
        state.memory_controller_unit=d->gpu_usage[index][7];
        state.video_processing_unit=d->gpu_usage[index][11];

        emit stateChanged(state);
    }

#endif
}

void NvTools::run()
{
    d->timer=new QTimer();
    d->timer->moveToThread(this);
    d->timer->setInterval(1000);

    connect(d->timer, SIGNAL(timeout()), SLOT(onTimer()));

    d->timer->start();

    exec();
}
