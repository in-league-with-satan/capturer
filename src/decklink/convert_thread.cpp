#include <QDebug>
#include <QMutexLocker>

#include "DeckLinkAPI.h"

#include "audio_tools.h"
#include "decklink_tools.h"

#include "convert_thread.h"


DlConvertThreadContainer *convert_thread_container=nullptr;


void frameCompletedCallback(Frame::ptr frame)
{
    if(!convert_thread_container)
        return;

    convert_thread_container->frameCompleted(frame);
}

DlConvertThread::DlConvertThread(FrameCompletedCallback func_frame_completed, QObject *parent)
    : QThread(parent)
    , func_frame_completed(func_frame_completed)
{
    frame_video_src=nullptr;

    video_converter=CreateVideoConversionInstance();

    audio_channels=8;

    setTerminationEnabled();

    start(QThread::NormalPriority);
    // start(QThread::HighPriority);
    // start(QThread::TimeCriticalPriority);
}

DlConvertThread::~DlConvertThread()
{
    terminate();
}

void DlConvertThread::addFrame(IDeckLinkVideoFrame *frame,  IDeckLinkAudioInputPacket *audio_packet, uint8_t frame_counter, bool reset_counter)
{
    QMutexLocker ml(&mutex);

    frame_video_src=frame;
    frame_audio_src=audio_packet;

    frame_video_src->AddRef();
    frame_audio_src->AddRef();

    this->frame_counter=frame_counter;
    this->reset_counter=reset_counter;

    event.next();
}

void DlConvertThread::run()
{
    Frame::ptr frame;
    void *d_audio;

    while(true) {
        event.wait();

        if(frame_video_src) {
            {
                QMutexLocker ml(&mutex);

                //

                frame=Frame::make();

                frame->video.decklink_frame.init(QSize(frame_video_src->GetWidth(), frame_video_src->GetHeight()), bmdFormat8BitBGRA);

                video_converter->ConvertFrame(frame_video_src, &frame->video.decklink_frame);

                //

                frame_audio_src->GetBytes(&d_audio);

                frame->audio.raw.resize(frame_audio_src->GetSampleFrameCount()*audio_channels*(16/8));

                memcpy(frame->audio.raw.data(), d_audio, frame->audio.raw.size());

                if(audio_channels==8)
                    channelsRemap(&frame->audio.raw);

                //

                frame->counter=frame_counter;
                frame->reset_counter=reset_counter;

                //

                frame_video_src->Release();
                frame_audio_src->Release();

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

    convert_thread_container=this;
}

DlConvertThreadContainer::~DlConvertThreadContainer()
{
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

void DlConvertThreadContainer::subscribe(FrameBuffer *obj)
{
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void DlConvertThreadContainer::unsubscribe(FrameBuffer *obj)
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

        foreach(FrameBuffer *buf, subscription_list)
            buf->append(frame);

        for(int i=0; i<queue.size(); ++i) {
            if(queue[i]->counter==uint8_t(last_frame_counter + 1)) {
                Frame::ptr f=queue[i];

                queue.removeAt(i--);

                last_frame_counter++;

                foreach(FrameBuffer *buf, subscription_list)
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
