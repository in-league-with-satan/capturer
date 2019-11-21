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
#include <QElapsedTimer>
#include <qcoreapplication.h>

#ifdef LIB_MWCAPTURE

#include "MWCapture.h"

#endif

#include "audio_tools.h"
#include "magewell_device.h"

#include "magewell_audio_thread.h"

const int mw_timeout=300;


struct MagewellAudioContext
{
#ifdef LIB_MWCAPTURE

#ifdef __linux__
    std::atomic <MWCAP_PTR> event_capture;
    MWCAP_PTR event_buf=0;
#else
    std::atomic <HANDLE> event_capture;
    HANDLE event_buf=0;
#endif

    HNOTIFY notify_event_buf=0;

    ULONGLONG status_bits=0;

#endif // LIB_MWCAPTURE

    QByteArray ba_buffer;

    std::atomic <int> audio_remap_mode;

    std::atomic <int> channels;
    std::atomic <int> sample_size;
    int sample_size_bytes;

    std::atomic <int64_t> lltimestamp;

    int64_t readed;
    int64_t prev_pts;

    QElapsedTimer timer_frame_buffered_warning;

    AVRational video_framerate={ 0, 0 };
};

MagewellAudioThread::MagewellAudioThread(QObject *parent)
    : QThread(parent)
    , d(new MagewellAudioContext())
{
#ifdef LIB_MWCAPTURE

    d->event_capture=0;

    d->timer_frame_buffered_warning.start();

#endif

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
    if(d->channels==8 && d->audio_remap_mode!=MagewellDevice::Device::AudioRemapMode::disabled)
        return 6;

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

void MagewellAudioThread::getData(QByteArray *data, int64_t *pts)
{
    data->clear();

    mutex.lock();

    (*pts)=d->lltimestamp;

    data->append(ba_data);

    ba_data.clear();

    mutex.unlock();
}

void MagewellAudioThread::getData2(QByteArray *data, int64_t *pts)
{
#ifndef LIB_MWCAPTURE

    Q_UNUSED(data)
    Q_UNUSED(pts)

#else

    QByteArray ba_copy;

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

        *pts=av_rescale_q(sizeToPos(d->readed + ba_copy.size()), { 1, 1000 }, d->video_framerate);

        if(*pts>d->prev_pts) {
            d->prev_pts=*pts;
            break;
        }

        usleep(1);
    }

    d->readed+=ba_copy.size();

    *data=ba_copy;

#endif
}

int64_t MagewellAudioThread::sizeToPos(int64_t size) const
{
    int audio_channels=d->channels;

    if(d->audio_remap_mode!=MagewellDevice::Device::AudioRemapMode::disabled && d->channels==8)
        audio_channels=6;

    return (double)size*1000./(double)(48000*d->sample_size_bytes*audio_channels);
}

void MagewellAudioThread::run()
{
#ifdef LIB_MWCAPTURE

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

            if(d->event_capture!=0)
                deviceStart();

            goto sleep;
        }


        if(d->status_bits&MWCAP_NOTIFY_AUDIO_SIGNAL_CHANGE) {
            updateAudioSignalInfo();
        }


        if(d->status_bits&MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED) {
            if(d->channels<1) {
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


            QByteArray ba_ready=QByteArray(d->ba_buffer);

            if(d->audio_remap_mode!=MagewellDevice::Device::AudioRemapMode::disabled) {
                if(d->channels==8) {
                    if(d->sample_size_bytes==2)
                        map8channelsTo6<uint16_t>(&d->ba_buffer, &ba_ready, d->audio_remap_mode==MagewellDevice::Device::AudioRemapMode::sides_drop);

                    else
                        map8channelsTo6<uint32_t>(&d->ba_buffer, &ba_ready, d->audio_remap_mode==MagewellDevice::Device::AudioRemapMode::sides_drop);
                }
            }

            d->lltimestamp=audio_frame.llTimestamp;

            mutex.lock();

            ba_data.append(ba_ready);

            if(sizeToPos(ba_data.size())>100) {
                ba_data.clear();
            }

            mutex.unlock();

        } else {
            if(d->timer_frame_buffered_warning.elapsed()>1000) {
                d->timer_frame_buffered_warning.restart();
                qWarning() << "!MWCAP_NOTIFY_AUDIO_FRAME_BUFFERED";
            }

            // msleep(50);
        }

        qApp->processEvents();

        continue;

sleep:
        qApp->processEvents();
        msleep(50);
    }

    deviceStop();

#endif
}

void MagewellAudioThread::setChannel(MGHCHANNEL channel)
{
    qDebug() << channel;

    current_channel=channel;

    deviceStart();
}

void MagewellAudioThread::setAudioRemapMode(int value)
{
    d->audio_remap_mode=value;
}

void MagewellAudioThread::setVideoFramerate(AVRational fr)
{
    d->video_framerate=fr;

    updateAudioSignalInfo();
}

void MagewellAudioThread::deviceStart()
{
#ifdef LIB_MWCAPTURE

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

    ba_data.clear();


    updateAudioSignalInfo();

    qInfo() << "ok";

    return;

stop:
    qCritical() << "start err";

    deviceStop();

#endif
}

void MagewellAudioThread::deviceStop()
{
#ifdef LIB_MWCAPTURE

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

#endif
}

void MagewellAudioThread::updateAudioSignalInfo()
{
#ifdef LIB_MWCAPTURE

    if(!d->event_capture)
        return;

    MWCAP_AUDIO_SIGNAL_STATUS signal_status={};

    if(MWGetAudioSignalStatus((HCHANNEL)current_channel.load(), &signal_status)!=MW_SUCCEEDED)
        return;

    int channels=0;

    for(int i=0; i<4; ++i) {
        if(signal_status.wChannelValid&(0x01 << i))
            channels+=2;
    }

    if(channels<2)
        return;

    int sample_size=signal_status.cBitsPerSample;

    //

    if(channels!=d->channels || sample_size!=d->sample_size) {
        d->channels=channels;

        d->sample_size=sample_size;
        d->sample_size_bytes=(d->sample_size==16 ? 2 : 4);

        d->ba_buffer.resize(MWCAP_AUDIO_SAMPLES_PER_FRAME*d->channels*d->sample_size_bytes);

        //

        d->readed=0;
        d->prev_pts=0;

        //

        emit audioSampleSizeChanged(d->sample_size==16 ? SourceInterface::AudioSampleSize::bitdepth_16 : SourceInterface::AudioSampleSize::bitdepth_32);
        emit audioChannelsChanged((SourceInterface::AudioChannels::T)this->channels());

        qInfo().noquote() << signal_status.dwSampleRate << d->sample_size << QString("%1 (%2)").arg(d->channels).arg(this->channels());

    } else {
        qInfo() << "same audio signal";
    }

#endif
}

