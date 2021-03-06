/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QMutexLocker>

#include "decklink_global.h"

#include "audio_tools.h"
#include "decklink_tools.h"

#include "decklink_convert_thread.h"


DlConvertThreadContainer *dl_convert_thread_container=nullptr;


void frameCompletedCallback(Frame::ptr frame)
{
    if(!dl_convert_thread_container)
        return;

    dl_convert_thread_container->frameCompleted(frame);
}

DlConvertThread::DlConvertThread(FrameCompletedCallback func_frame_completed, QObject *parent)
    : QThread(parent)
    , func_frame_completed(func_frame_completed)
{
    frame_video_src=nullptr;

#ifdef LIB_DECKLINK

    video_converter=CreateVideoConversionInstance();

#endif

    audio_channels=8;
    sample_size=16;

    start(QThread::LowPriority);

    // start(QThread::NormalPriority);
    // start(QThread::HighPriority);
    // start(QThread::TimeCriticalPriority);
}

DlConvertThread::~DlConvertThread()
{
    term();
}

void DlConvertThread::term()
{
    running=false;

    event.next();

    while(isRunning()) {
        msleep(30);
    }
}

void DlConvertThread::addFrame(IDeckLinkVideoFrame *frame,  IDeckLinkAudioInputPacket *audio_packet, uint8_t frame_counter, bool reset_counter)
{
    QMutexLocker ml(&mutex);

    frame_video_src=frame;
    frame_audio_src=audio_packet;

#ifdef LIB_DECKLINK

    frame_video_src->AddRef();
    frame_audio_src->AddRef();

#endif

    this->frame_counter=frame_counter;
    this->reset_counter=reset_counter;

    event.next();
}

void DlConvertThread::run()
{
    Frame::ptr frame;
//    void *d_audio;

    running=true;

    while(running) {
        event.wait();

        if(frame_video_src) {
            {
                QMutexLocker ml(&mutex);

                //

                frame=Frame::make();

//                frame->video.decklink_frame.init(QSize(frame_video_src->GetWidth(), frame_video_src->GetHeight()), bmdFormat8BitBGRA);

//                video_converter->ConvertFrame(frame_video_src, &frame->video.decklink_frame);

//                //

//                frame_audio_src->GetBytes(&d_audio);

//                frame->audio.raw.resize(frame_audio_src->GetSampleFrameCount()*audio_channels*(sample_size/8));

//                memcpy((void*)frame->audio.raw.constData(), d_audio, frame->audio.raw.size());

//                if(audio_channels==8) {
//                    if(sample_size==16)
//                        channelsRemap16(&frame->audio.raw);

//                    else
//                        channelsRemap32(&frame->audio.raw);
//                }

                frame->audio.channels=audio_channels;
                frame->audio.sample_size=sample_size;

                //

                frame->counter=frame_counter;
                frame->reset_counter=reset_counter;

                //

#ifdef LIB_DECKLINK

                frame_video_src->Release();
                frame_audio_src->Release();

#endif

                frame_video_src=nullptr;
                frame_audio_src=nullptr;
            }

            func_frame_completed(frame);

            frame.reset();
        }
    }
}

DlConvertThreadContainer::DlConvertThreadContainer(int thread_count, QObject *parent)
    : QObject(parent)
{
    this->thread_count=thread_count;

    thread_num=0;

    thread.resize(thread_count);

    for(int i=0; i<thread_count; ++i)
        thread[i]=new DlConvertThread(&frameCompletedCallback, this);

    dl_convert_thread_container=this;
}

DlConvertThreadContainer::~DlConvertThreadContainer()
{
}

void DlConvertThreadContainer::stopThreads()
{
    for(int i=0; i<thread_count; ++i)
        thread[i]->term();
}

void DlConvertThreadContainer::addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet, uint8_t counter, bool reset_counter)
{
    if(reset_counter) {
        qWarning() << "queue.clear";

        QMutexLocker ml(&mutex_subscription);

        queueClear();
    }

    thread[thread_num++]->addFrame(frame, audio_packet, counter, reset_counter);

    if(thread_num>=thread_count)
        thread_num=0;
}

void DlConvertThreadContainer::subscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void DlConvertThreadContainer::unsubscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    subscription_list.removeAll(obj);
}

void DlConvertThreadContainer::setAudioChannels(int value)
{
    for(int i=0; i<thread_count; ++i) {
        QMutexLocker ml(&thread[i]->mutex);

        thread[i]->audio_channels=value;
    }
}

void DlConvertThreadContainer::setSampleSize(int value)
{
    for(int i=0; i<thread_count; ++i) {
        QMutexLocker ml(&thread[i]->mutex);

        thread[i]->sample_size=value;
    }
}

void DlConvertThreadContainer::frameCompleted(Frame::ptr frame)
{
    QMutexLocker ml(&mutex_subscription);


    if(frame->reset_counter || queue.size()>(thread_count + 1)) {
        qWarning() << "reset queue" << frame->reset_counter << queue.size();

        last_frame_counter=frame->counter - 1;

        queueClear();
    }

    if(frame->counter!=uint8_t(last_frame_counter + 1)) {
        queue.append(frame);

    } else {
        last_frame_counter++;

        foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
            buf->append(frame);

        for(int i=0; i<queue.size(); ++i) {
            if(queue[i]->counter==uint8_t(last_frame_counter + 1)) {
                Frame::ptr f=queue[i];

                queue.removeAt(i--);

                last_frame_counter++;

                foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
                    buf->append(f);
            }
        }
    }
}

void DlConvertThreadContainer::queueClear()
{
    for(int i=0; i<queue.size(); ++i) {
        emit frameSkipped();
    }

    queue.clear();
}
