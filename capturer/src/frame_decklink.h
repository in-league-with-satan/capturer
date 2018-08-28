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

#ifndef FRAME_DECKLINK_H
#define FRAME_DECKLINK_H

#include "frame.h"
#include "decklink_video_frame.h"

struct FrameDecklink : public Frame
{
    FrameDecklink(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet, int audio_channels, int audio_sample_size) {
        if(video_frame) {
            this->video_frame=video_frame;

            video_frame->AddRef();

            video.size=QSize(video_frame->GetWidth(), video_frame->GetHeight());
            video.pixel_format.fromBMDPixelFormat(video_frame->GetPixelFormat());

            video.data_size=DeckLinkVideoFrame::frameSize(video.size, video_frame->GetPixelFormat());
            video_frame->GetBytes((void**)&video.data_ptr);
        }

        if(audio_packet) {
            this->audio_packet=audio_packet;

            audio_packet->AddRef();

            audio.channels=audio_channels;
            audio.sample_size=audio_sample_size;

            audio.data_size=audio_packet->GetSampleFrameCount()*audio_channels*(audio_sample_size/8);
            audio_packet->GetBytes((void**)&audio.data_ptr);
        }
    }

    ~FrameDecklink() {
        if(video_frame)
            video_frame->Release();

        if(audio_packet)
            audio_packet->Release();
    }

    static ptr make(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet, int audio_channels, int audio_sample_size) {
        return ptr((Frame*)new FrameDecklink(video_frame, audio_packet, audio_channels, audio_sample_size));
    }

    IDeckLinkVideoInputFrame *video_frame=nullptr;
    IDeckLinkAudioInputPacket *audio_packet=nullptr;
};

#endif // FRAME_DECKLINK_H
