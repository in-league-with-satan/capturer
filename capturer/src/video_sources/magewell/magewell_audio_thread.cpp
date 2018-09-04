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
#include <QElapsedTimer>
#include <qcoreapplication.h>

#ifdef __linux__
#include "win-types.h"
#include "mw-common.h"
#include "mw-fourcc.h"
#include "lib-mw-capture.h"
#endif

#include "audio_tools.h"

#include "magewell_audio_thread.h"

const int mw_timeout=300;


struct MagewellAudioContext {
#ifdef __linux__
    MWCAP_PTR event_capture=0;
    MWCAP_PTR event_notify_buffering=0;

    HNOTIFY notify_buffering=0;
#endif

    ULONGLONG status_bits=0;

    QByteArray ba_buffer;

    std::atomic <int> channels;
    std::atomic <int> sample_size;
    int sample_size_bytes;
};


MagewellAudioThread::MagewellAudioThread(QObject *parent)
    : QThread(parent)
    , d(new MagewellAudioContext())
{
    current_channel=-1;

    start();
}

MagewellAudioThread::~MagewellAudioThread()
{
    running=false;


    while(isRunning())
        msleep(30);


    delete d;
}

int MagewellAudioThread::channels() const
{
    return 8;
    return d->channels;
}

int MagewellAudioThread::sampleSize() const
{
    return (d->sample_size==16 ? 16 : 32);
}

QByteArray MagewellAudioThread::getData()
{
    QMutexLocker ml(&mutex);

    QByteArray ba_copy=QByteArray(ba_data);

    ba_data.clear();

    return ba_copy;
}

void MagewellAudioThread::run()
{
    running=true;

    int ret;

#ifdef __linux__

    while(running) {
        if(d->event_capture==0) {
            qApp->processEvents();
            msleep(300);
            continue;
        }

        if(d->channels<2) {
            updateAudioSignalInfo();
            qApp->processEvents();
            msleep(300);
            continue;
        }


        MWWaitEvent(d->event_notify_buffering, mw_timeout);

        ret=MWGetNotifyStatus(current_channel, d->notify_buffering, &d->status_bits);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetNotifyStatus err";
            continue;
        }


        if(d->status_bits==MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE) {
            updateAudioSignalInfo();
        }


        if(d->status_bits==MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED) {
            MWCAP_AUDIO_CAPTURE_FRAME audio_frame;

            while(true) {
                ret=MWCaptureAudioFrame(current_channel, &audio_frame);

                if(MW_ENODATA==ret)
                    break;
            }


            int size=MWCAP_AUDIO_SAMPLES_PER_FRAME*MWCAP_AUDIO_MAX_NUM_CHANNELS*d->sample_size_bytes;

            d->ba_buffer.resize(size);

            memcpy((void*)d->ba_buffer.constData(), (void*)audio_frame.adwSamples, size);


            if(d->sample_size==16)
                channelsRemapMagewell<int16_t>((void*)d->ba_buffer.constData(), d->ba_buffer.size());

            else
                channelsRemapMagewell<int32_t>((void*)d->ba_buffer.constData(), d->ba_buffer.size());


            mutex.lock();

            ba_data.append(d->ba_buffer);

            mutex.unlock();

        } else {
            qDebug() << "!MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED";
        }

        LONGLONG device_time;

        ret=MWGetDeviceTime(current_channel, &device_time);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetDeviceTime err";
            continue;
        }

        qApp->processEvents();
    }

    captureStop();

#endif
}

void MagewellAudioThread::setChannel(int channel)
{
    current_channel=channel;

    captureStop();
    captureStart();
}

void MagewellAudioThread::captureStart()
{
#ifdef __linux__

    captureStop();

    qDebug() << "current_channel" << current_channel;

    d->event_capture=MWCreateEvent();

    if(d->event_capture <= 0) {
        qCritical() << "MWCreateEvent err";
        return;
    }

    d->event_notify_buffering=MWCreateEvent();

    if(d->event_notify_buffering <= 0) {
        qCritical() << "MWCreateEvent err";
        return;
    }


    d->notify_buffering=MWRegisterNotify(current_channel, d->event_notify_buffering,
                                         MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED | MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE);

    if(d->notify_buffering<=0) {
        qCritical() << "MWRegisterNotify err";
        return;
    }

    //

    int ret=MWStartAudioCapture(current_channel);

    if(ret!=MW_SUCCEEDED) {
        qCritical() << "MWStartAudioCapture err";
        return;
    }

    //

    updateAudioSignalInfo();

#endif
}

void MagewellAudioThread::captureStop()
{
#ifdef __linux__

    MWStopAudioCapture(current_channel);

    if(d->event_notify_buffering) {
        MWUnregisterNotify(current_channel, d->notify_buffering);
        d->notify_buffering=0;
    }

    if(d->event_capture) {
        MWCloseEvent(d->event_capture);
        d->event_capture=0;
    }

#endif
}

void MagewellAudioThread::updateAudioSignalInfo()
{
#ifdef __linux__

    MWCAP_AUDIO_SIGNAL_STATUS signal_status;

    if(MWGetAudioSignalStatus(current_channel, &signal_status)!=MW_SUCCEEDED)
        return;

    d->channels=0;

    for(int i=0; i<4; ++i) {
        if(signal_status.wChannelValid&(0x01 << i))
            d->channels+=2;
    }

    if(d->channels<2)
        return;

    d->sample_size=signal_status.cBitsPerSample;
    d->sample_size_bytes=(d->sample_size==16 ? 2 : 4);

    d->ba_buffer.resize(MWCAP_AUDIO_SAMPLES_PER_FRAME*d->channels*(signal_status.cBitsPerSample/8));

    emit audioSampleSizeChnanged(d->sample_size==16 ? SourceInterface::AudioSampleSize::bitdepth_16 : SourceInterface::AudioSampleSize::bitdepth_32);

    qDebug() << signal_status.dwSampleRate << d->sample_size << d->channels;

#endif
}
