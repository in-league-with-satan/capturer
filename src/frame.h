#ifndef FRAME_H
#define FRAME_H

#include <QtGlobal>
#include <QSize>

#include <memory>

#include "decklink_video_frame.h"

struct Frame
{
    typedef std::shared_ptr<Frame> ptr;

    Frame() {
        reset_counter=false;

        video_frame=nullptr;
        audio_packet=nullptr;
    }

    ~Frame() {
        if(video_frame)
            video_frame->Release();

        if(audio_packet)
            audio_packet->Release();
    }

    static ptr make() {
        return ptr(new Frame());
    }

    void setData(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet, int audio_channels, int audio_sample_size) {
        if(video_frame) {
            video_frame->AddRef();

            this->video_frame=video_frame;

            video.rgb=video_frame->GetPixelFormat()!=bmdFormat8BitYUV;
            video.rgb_10bit=video_frame->GetPixelFormat()==bmdFormat10BitRGB;
            video.size=QSize(video_frame->GetWidth(), video_frame->GetHeight());
            video.data_size=DeckLinkVideoFrame::frameSize(video.size, video_frame->GetPixelFormat());

            video_frame->GetBytes((void**)&video.ptr_data);
        }

        //

        if(audio_packet) {
            audio_packet->AddRef();
            this->audio_packet=audio_packet;

            audio.channels=audio_channels;
            audio.sample_size=audio_sample_size;
            audio.data_size=audio_packet->GetSampleFrameCount()*audio_channels*(audio_sample_size/8);

            audio_packet->GetBytes((void**)&audio.ptr_data);
        }
    }

    struct DataVideo {
        DataVideo() {
            ptr_data=nullptr;
            data_size=0;
            rgb=true;
            rgb_10bit=false;
        }

        char *ptr_data;
        size_t data_size;
        QSize size;

        bool rgb;
        bool rgb_10bit;

    } video;

    struct DataAudio {
        DataAudio() {
            ptr_data=nullptr;
            data_size=0;
            channels=0;
            sample_size=0;
        }

        char *ptr_data;
        size_t data_size;

        int channels;
        int sample_size;

    } audio;

    IDeckLinkVideoInputFrame *video_frame;
    IDeckLinkAudioInputPacket *audio_packet;

    uint8_t counter;
    bool reset_counter;
};

#endif // FRAME_H
