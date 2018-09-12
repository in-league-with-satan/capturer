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
    std::atomic <MWCAP_PTR> event_capture;
    MWCAP_PTR event_buf=0;
#else
    std::atomic <HANDLE> event_capture;
    HANDLE event_buf=0;
#endif

    HNOTIFY notify_event_buf=0;

    ULONGLONG status_bits=0;

    QByteArray ba_buffer;

    std::atomic <int> channels;
    std::atomic <int> sample_size;
    int sample_size_bytes;

    std::atomic <int64_t> readed;
    std::atomic <int64_t> prev_pts;

    AVRational video_framerate={ 0, 0 };
};

MagewellAudioThread::MagewellAudioThread(QObject *parent)
    : QThread(parent)
    , d(new MagewellAudioContext())
{
    d->event_capture=0;

    current_channel=0;

    start(QThread::HighPriority);
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
    return d->channels;
}

int MagewellAudioThread::sampleSize() const
{
    return (d->sample_size==16 ? 16 : 32);
}

QByteArray MagewellAudioThread::getDataAll()
{
    QMutexLocker ml(&mutex);

    QByteArray ba_copy=QByteArray(ba_data);

    ba_data.clear();

    return ba_copy;
}

QByteArray MagewellAudioThread::getData()
{
    QByteArray ba_copy;

    int64_t pts=0;

    QElapsedTimer t;

    t.start();

    while(t.elapsed()<100) {
        if(!d->event_capture) {
            break;
        }

        mutex.lock();

        ba_copy.append(ba_data);

        ba_data.clear();

        mutex.unlock();

        pts=av_rescale_q(sizeToPos(d->readed + ba_copy.size()), { 1, 1000 }, d->video_framerate);

        if(pts>d->prev_pts) {
            d->prev_pts=pts;
            break;
        }

        usleep(1);
    }

    d->readed+=ba_copy.size();

    return ba_copy;
}

int64_t MagewellAudioThread::sizeToPos(int64_t size) const
{
    return (double)size*1000./(double)(48000*d->sample_size_bytes*d->channels);
}

int64_t MagewellAudioThread::lastPts() const
{
    return d->prev_pts;
}

void MagewellAudioThread::run()
{
    running=true;

    int ret;

    while(running) {
        if(d->event_capture==0) {
            goto sleep;
        }


        MWWaitEvent(d->event_buf, mw_timeout);

        ret=MWGetNotifyStatus((HCHANNEL)current_channel.load(), d->notify_event_buf, &d->status_bits);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetNotifyStatus err" << ret;
            deviceStart();
            goto sleep;
        }


        if(d->status_bits&MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE) {
            updateAudioSignalInfo();
        }


        if(d->status_bits&MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED) {
            if(d->channels<2) {
                updateAudioSignalInfo();
                goto sleep;
            }

            MWCAP_AUDIO_CAPTURE_FRAME audio_frame;

            ret=MWCaptureAudioFrame((HCHANNEL)current_channel.load(), &audio_frame);

            if(ret!=MW_SUCCEEDED) {
                goto sleep;
            }

            if(d->sample_size_bytes<2)
                goto sleep;

            d->ba_buffer.fill(0);

            if(d->sample_size_bytes==2)
                copyAudioSamplesMagewell<uint16_t>((void*)audio_frame.adwSamples, (void*)d->ba_buffer.constData(), MWCAP_AUDIO_SAMPLES_PER_FRAME, d->channels);

            else
                copyAudioSamplesMagewell<uint32_t>((void*)audio_frame.adwSamples, (void*)d->ba_buffer.constData(), MWCAP_AUDIO_SAMPLES_PER_FRAME, d->channels);


            mutex.lock();

            ba_data.append(d->ba_buffer);

            if(sizeToPos(ba_data.size())>100) {
                // qInfo() << "ba_data.clear" << ba_data.size() << sizeToPos(ba_data.size());
                ba_data.clear();
            }

            mutex.unlock();

        } else {
            qDebug() << "!MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED";
        }

        qApp->processEvents();

        continue;

sleep:
        qApp->processEvents();
        msleep(50);
    }

    deviceStop();
}

void MagewellAudioThread::setChannel(MGHCHANNEL channel)
{
    qDebug() << channel;

    current_channel=channel;

    deviceStart();
}

void MagewellAudioThread::setVideoFramerate(AVRational fr)
{
    d->video_framerate=fr;

    updateAudioSignalInfo();
}

void MagewellAudioThread::deviceStart()
{
    deviceStop();

    qDebug() << "current_channel" << current_channel;

    d->event_capture=MWCreateEvent();

    if(d->event_capture==0) {
        qCritical() << "MWCreateEvent err";
        goto stop;
    }

    d->event_buf=MWCreateEvent();

    if(d->event_buf==0) {
        qCritical() << "MWCreateEvent err";
        goto stop;
    }


    d->notify_event_buf=MWRegisterNotify((HCHANNEL)current_channel.load(), d->event_buf,
                                         MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED | MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE);

    if(d->notify_event_buf==0) {
        qCritical() << "MWRegisterNotify err";
        goto stop;
    }

    //

    if(MWStartAudioCapture((HCHANNEL)current_channel.load())!=MW_SUCCEEDED) {
        qCritical() << "MWStartAudioCapture err";
        goto stop;
    }

    //

    d->readed=0;
    d->prev_pts=0;

    ba_data.clear();


    updateAudioSignalInfo();

    qInfo() << "ok";

    return;

stop:
    qCritical() << "start err";

    deviceStop();
}

void MagewellAudioThread::deviceStop()
{
    qInfo() << "stop";

    if(current_channel) {
        MWStopAudioCapture((HCHANNEL)current_channel.load());
    }

    if(d->notify_event_buf) {
        MWUnregisterNotify((HCHANNEL)current_channel.load(), d->notify_event_buf);
        d->notify_event_buf=0;
    }

    if(d->event_buf) {
        MWCloseEvent(d->event_buf);
        d->event_buf=0;
    }

    if(d->event_capture) {
        MWCloseEvent(d->event_capture);
        d->event_capture=0;
    }
}

void MagewellAudioThread::updateAudioSignalInfo()
{
    if(!d->event_capture)
        return;

    MWCAP_AUDIO_SIGNAL_STATUS signal_status={};

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

    //

    d->readed=0;
    d->prev_pts=0;

    //

    emit audioSampleSizeChanged(d->sample_size==16 ? SourceInterface::AudioSampleSize::bitdepth_16 : SourceInterface::AudioSampleSize::bitdepth_32);
    emit audioChannelsChanged((SourceInterface::AudioChannels::T)d->channels.load());

    qInfo() << signal_status.dwSampleRate << d->sample_size << d->channels;
}
