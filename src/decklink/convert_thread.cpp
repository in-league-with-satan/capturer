#include <QMutexLocker>

#include "DeckLinkAPI.h"

#include "frame_buffer.h"
#include "audio_tools.h"

#include "convert_thread.h"

DlConvertThread::DlConvertThread(QObject *parent) :
    QThread(parent)
{
    frame_video_src=nullptr;

    video_frame_converted_720p=nullptr;
    video_frame_converted_1080p=nullptr;
    video_frame_converted_2160p=nullptr;

    video_converter=CreateVideoConversionInstance();

    audio_channels=8;

    // start(QThread::TimeCriticalPriority);
    start(QThread::HighPriority);
}

DlConvertThread::~DlConvertThread()
{
}

void DlConvertThread::addFrame(IDeckLinkVideoFrame *frame,  IDeckLinkAudioInputPacket *audio_packet)
{
    QMutexLocker ml(&mutex);

    frame_video_src=frame;
    frame_audio_src=audio_packet;

    frame_video_src->AddRef();
    frame_audio_src->AddRef();
}

void DlConvertThread::subscribeForAll(FrameBuffer *obj)
{
    QMutexLocker ml(&mutex_subscription);

    if(!l_full.contains(obj))
        l_full.append(obj);
}

void DlConvertThread::subscribeForAudio(FrameBuffer *obj)
{
    QMutexLocker ml(&mutex_subscription);

    if(!l_audio.contains(obj))
        l_audio.append(obj);
}

void DlConvertThread::unsubscribe(FrameBuffer *obj)
{
    QMutexLocker ml(&mutex_subscription);

    l_full.removeAll(obj);
    l_audio.removeAll(obj);
}

void DlConvertThread::run()
{
    FrameBuffer::Frame frame;

    void *d_video;
    void *d_audio;

    IDeckLinkMutableVideoFrame *frame_out=nullptr;

    while(true) {
        if(frame_video_src) {
            {
                QMutexLocker ml(&mutex);

                if(frame_video_src->GetWidth()==1280) {
                    video_converter->ConvertFrame(frame_video_src, video_frame_converted_720p);

                    frame_out=video_frame_converted_720p;

                } else if(frame_video_src->GetWidth()==1920) {
                    video_converter->ConvertFrame(frame_video_src, video_frame_converted_1080p);

                    frame_out=video_frame_converted_1080p;

                } else {
                    video_converter->ConvertFrame(frame_video_src, video_frame_converted_2160p);

                    frame_out=video_frame_converted_2160p;
                }

                //

                frame_out->GetBytes(&d_video);

                frame.ba_video.resize(frame_out->GetRowBytes() * frame_out->GetHeight());

                memcpy(frame.ba_video.data(), d_video, frame.ba_video.size());

                frame.size_video=QSize(frame_out->GetWidth(), frame_out->GetHeight());

                frame.bmd_pixel_format=frame_out->GetPixelFormat();

                //

                frame_audio_src->GetBytes(&d_audio);

                frame.ba_audio.resize(frame_audio_src->GetSampleFrameCount()*audio_channels*(16/8));

                memcpy(frame.ba_audio.data(), d_audio, frame.ba_audio.size());

                if(audio_channels==8)
                    channelsRemap(&frame.ba_audio);

                //

                frame_video_src->Release();
                frame_audio_src->Release();

                frame_video_src=nullptr;
                frame_audio_src=nullptr;
            }

            QMutexLocker ml(&mutex_subscription);

            foreach(FrameBuffer *buf, l_full)
                buf->appendFrame(frame);

            if(!l_audio.isEmpty()) {
                frame.ba_video.clear();

                foreach(FrameBuffer *buf, l_audio)
                    buf->appendFrame(frame);
            }

        }

        usleep(100);
    }
}

DlConvertThreadContainer::DlConvertThreadContainer(int thread_count)
{
    this->thread_count=thread_count;

    thread_num=0;

    thread.resize(thread_count);

    for(int i=0; i<thread_count; ++i)
        thread[i]=new DlConvertThread();
}

void DlConvertThreadContainer::addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet)
{
   thread[thread_num++]->addFrame(frame, audio_packet);

   if(thread_num>=thread_count)
       thread_num=0;
}

void DlConvertThreadContainer::subscribeForAll(FrameBuffer *obj)
{
     for(int i=0; i<thread_count; ++i)
         thread[i]->subscribeForAll(obj);
}

void DlConvertThreadContainer::subscribeForAudio(FrameBuffer *obj)
{
     for(int i=0; i<thread_count; ++i)
         thread[i]->subscribeForAudio(obj);
}

void DlConvertThreadContainer::unsubscribe(FrameBuffer *obj)
{
     for(int i=0; i<thread_count; ++i)
         thread[i]->unsubscribe(obj);
}

void DlConvertThreadContainer::setAudioChannels(int value)
{
    for(int i=0; i<thread_count; ++i) {
        QMutexLocker ml(&thread[i]->mutex);

        thread[i]->audio_channels=value;
    }
}

void DlConvertThreadContainer::init(IDeckLinkOutput *decklink_output)
{
    for(int i=0; i<thread_count; ++i) {
        if(!thread[i]->video_frame_converted_720p)
            decklink_output->CreateVideoFrame(1280, 720, 720*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &thread[i]->video_frame_converted_720p);

        if(!thread[i]->video_frame_converted_1080p)
            decklink_output->CreateVideoFrame(1920, 1080, 1920*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &thread[i]->video_frame_converted_1080p);

        if(!thread[i]->video_frame_converted_2160p)
            decklink_output->CreateVideoFrame(3840, 2160, 3840*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &thread[i]->video_frame_converted_2160p);
    }
}
