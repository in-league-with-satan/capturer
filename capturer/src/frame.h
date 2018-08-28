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

#ifndef FRAME_H
#define FRAME_H

#include <QDebug>
#include <QtGlobal>
#include <QSize>

#include <memory>

#include "ff_tools.h"
#include "pixel_format.h"

struct Frame
{
    typedef std::shared_ptr<Frame> ptr;

    virtual ~Frame() {
    }

    static ptr make() {
        return ptr(new Frame());
    }

    ptr copyFrameSoundOnly() {
        ptr p=make();

        if(audio.data_ptr) {
            p->audio.dummy=QByteArray((char*)audio.data_ptr, audio.data_size);
            p->audio.data_ptr=(uint8_t*)p->audio.dummy.constData();
            p->audio.data_size=audio.data_size;

            p->audio.channels=audio.channels;
            p->audio.sample_size=audio.sample_size;
        }

        return p;
    }

    void setData(const QByteArray &video_frame, QSize size, const QByteArray &audio_packet, int audio_channels, int audio_sample_size) {
        setDataVideo(video_frame, size);
        setDataAudio(audio_packet, audio_channels, audio_sample_size);
    }

    void setDataVideo(const QByteArray &video_frame, QSize size) {
        if(!video_frame.isEmpty()) {
            video.dummy=video_frame;
            video.data_ptr=(uint8_t*)video.dummy.constData();
            video.data_size=video.dummy.size();
            video.size=size;
        }
    }

    void setDataAudio(const QByteArray &audio_packet, int audio_channels, int audio_sample_size) {
        if(!audio_packet.isEmpty()) {
            audio.dummy=audio_packet;
            audio.data_ptr=(uint8_t*)audio.dummy.constData();
            audio.data_size=audio_packet.size();
            audio.channels=audio_channels;
            audio.sample_size=audio_sample_size;
        }
    }

    struct DataVideo {
        uint8_t *data_ptr=nullptr;
        size_t data_size=0;
        QSize size;
        AVRational time_base={};
        int64_t pts=AV_NOPTS_VALUE;
        PixelFormat pixel_format;
        QByteArray dummy;

    } video;

    struct DataAudio {
        uint8_t *data_ptr=nullptr;
        size_t data_size=0;

        int channels=0;
        int sample_size=0;

        QByteArray dummy;

    } audio;

    uint16_t counter=0;
    bool reset_counter=false;
};

#endif // FRAME_H
