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

#include "MWCapture.h"

#include "audio_tools.h"

#include "magewell_audio_thread.h"

const int mw_timeout=300;


struct MagewellAudioContext {
#ifdef __linux__
    MWCAP_PTR event_capture=0;
    MWCAP_PTR event_buf=0;
#else
    HANDLE event_capture=0;
    HANDLE event_buf=0;
#endif

    HNOTIFY notify_event_buf=0;

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
    current_channel=0;

    start(QThread::TimeCriticalPriority);
}

MagewellAudioThread::~MagewellAudioThread()
{
    deviceStop();

    running=false;

    while(isRunning())
        msleep(30);

    delete d;
}

int MagewellAudioThread::channels() const
{
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

void MagewellAudioThread::getData(QByteArray *data, long long *timestamp)
{
    QMutexLocker ml(&mutex);

    data->clear();

    data->append(ba_data);

    (*timestamp)=this->timestamp;

    ba_data.clear();
}

void MagewellAudioThread::run()
{
    running=true;

    int ret;

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


        MWWaitEvent(d->event_buf, mw_timeout);
        // MWWaitEvent(d->event_notify_buffering, INFINITE);


        ret=MWGetNotifyStatus((HCHANNEL)current_channel.load(), d->notify_event_buf, &d->status_bits);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetNotifyStatus err" << ret;
            continue;
        }


        if(d->status_bits&MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE) {
            updateAudioSignalInfo();
        }


        if(d->status_bits&MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED) {
            MWCAP_AUDIO_CAPTURE_FRAME audio_frame;

            ret=MWCaptureAudioFrame((HCHANNEL)current_channel.load(), &audio_frame);

            if(ret!=MW_SUCCEEDED)
                continue;


            d->ba_buffer.fill(0);

            if(d->sample_size_bytes==2)
                copyAudioSamplesMagewell<uint16_t>((void*)audio_frame.adwSamples, (void*)d->ba_buffer.constData(), MWCAP_AUDIO_SAMPLES_PER_FRAME, d->channels);

            else
                copyAudioSamplesMagewell<uint32_t>((void*)audio_frame.adwSamples, (void*)d->ba_buffer.constData(), MWCAP_AUDIO_SAMPLES_PER_FRAME, d->channels);


            mutex.lock();

            if(ba_data.isEmpty()) {
                // ret=MWGetDeviceTime((HCHANNEL)current_channel.load(), &timestamp);

                // if(ret!=MW_SUCCEEDED) {
                //     qCritical() << "MWGetDeviceTime err";
                //     continue;
                // }

                timestamp=audio_frame.llTimestamp;
            }

            ba_data.append(d->ba_buffer);

            mutex.unlock();

        } else {
            qDebug() << "!MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED";
        }

        qApp->processEvents();
    }

    deviceStop();
}

void MagewellAudioThread::setChannel(MGHCHANNEL channel)
{
    qDebug() << channel;

    current_channel=channel;

    deviceStop();
    deviceStart();
}

void MagewellAudioThread::deviceStart()
{
    deviceStop();

    qDebug() << "current_channel" << current_channel;

    d->event_capture=MWCreateEvent();

    if(d->event_capture==0) {
        qCritical() << "MWCreateEvent err";
        return;
    }

    d->event_buf=MWCreateEvent();

    if(d->event_buf==0) {
        qCritical() << "MWCreateEvent err";
        return;
    }


    d->notify_event_buf=MWRegisterNotify((HCHANNEL)current_channel.load(), d->event_buf,
                                         MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED | MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE);

    if(d->notify_event_buf==0) {
        qCritical() << "MWRegisterNotify err";
        return;
    }

    //

    int ret=MWStartAudioCapture((HCHANNEL)current_channel.load());

    if(ret!=MW_SUCCEEDED) {
        qCritical() << "MWStartAudioCapture err";
        return;
    }

    //

    updateAudioSignalInfo();

    qInfo() << "ok";
}

void MagewellAudioThread::deviceStop()
{
    if(current_channel)
        MWStopAudioCapture((HCHANNEL)current_channel.load());

    if(d->event_buf) {
        MWUnregisterNotify((HCHANNEL)current_channel.load(), d->notify_event_buf);
        d->notify_event_buf=0;
    }

    if(d->event_capture) {
        MWCloseEvent(d->event_capture);
        d->event_capture=0;
    }
}

void MagewellAudioThread::updateAudioSignalInfo()
{
    MWCAP_AUDIO_SIGNAL_STATUS signal_status;

    if(MWGetAudioSignalStatus((HCHANNEL)current_channel.load(), &signal_status)!=MW_SUCCEEDED)
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

    d->ba_buffer.resize(MWCAP_AUDIO_SAMPLES_PER_FRAME*d->channels*d->sample_size_bytes);

    emit audioSampleSizeChanged(d->sample_size==16 ? SourceInterface::AudioSampleSize::bitdepth_16 : SourceInterface::AudioSampleSize::bitdepth_32);
    emit audioChannelsChanged((SourceInterface::AudioChannels::T)d->channels.load());

    qDebug() << signal_status.dwSampleRate << d->sample_size << d->channels;
}
