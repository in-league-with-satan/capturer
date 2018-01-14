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
#include <QApplication>
#include <QAudioOutput>
#include <qcoreapplication.h>

#ifdef USE_PULSE_AUDIO
#include <pulse/error.h>
#include <pulse/thread-mainloop.h>
#include <pulse/stream.h>
#endif

//#define SAVE_STREAM

#include "frame_buffer.h"
#include "audio_tools.h"

#include "pulse_audio_output_thread.h"

/*
struct pa_simple {
    pa_threaded_mainloop *mainloop;
    pa_context *context;
    pa_stream *stream;
    pa_stream_direction_t direction;
    const void *read_data;
    size_t read_index, read_length;
    int operation_success;
};
*/

PulseAudioOutputThread::PulseAudioOutputThread(QObject *parent) :
    AudioOutputInterface(parent)
{
#ifdef USE_PULSE_AUDIO

    s=nullptr;

#endif

    start(QThread::NormalPriority);
}

PulseAudioOutputThread::~PulseAudioOutputThread()
{
}

void PulseAudioOutputThread::run()
{
#ifdef SAVE_STREAM

    f_src.setFileName(QApplication::applicationDirPath() + "/aud_src.raw");
    f_conv.setFileName(QApplication::applicationDirPath() + "/aud_conv.raw");

    f_src.open(QFile::ReadWrite | QFile::Truncate | QFile::Unbuffered);
    f_conv.open(QFile::ReadWrite | QFile::Truncate | QFile::Unbuffered);

#endif


#ifdef USE_PULSE_AUDIO

    init();

    Frame::ptr frame;

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            onInputFrameArrived(frame->audio.ptr_data, frame->audio.data_size, frame->audio.channels, frame->audio.sample_size);

            frame.reset();
        }

        QCoreApplication::processEvents();

        // usleep(1);
    }

#endif
}

void PulseAudioOutputThread::onInputFrameArrived(void *data, size_t size, int channels, int sample_size)
{
#ifdef USE_PULSE_AUDIO

    if(!s) {
        return;
    }

    QByteArray ba_tmp=convert(data, size, channels, sample_size, ss.channels);

    if(ba_tmp.isEmpty()) {
        qCritical() << "onInputFrameArrived conv empty";
        return;
    }

#ifdef SAVE_STREAM

    f_src.write(*ba_data);
    f_conv.write(ba_tmp);

#endif

    int error=0;

    // if(pa_simple_drain(s, &error)<0) {
    //     qCritical() << "pa_simple_drain err" << pa_strerror(error);

    //     init();
    // }

    if(pa_simple_write(s, ba_tmp.constData(), ba_tmp.size(), &error)<0) {
        qCritical() << "pa_simple_write err" << pa_strerror(error) << ba_tmp.size();

        init();
    }

    // pa_simple_drain(s, nullptr);

#endif
}

void PulseAudioOutputThread::init()
{
#ifdef USE_PULSE_AUDIO

    if(s) {
        pa_simple_free(s);
        s=nullptr;
    }

    ss.rate=48000;
    ss.format=PA_SAMPLE_S16LE;

    pa_channel_map map;

    ss.channels=map.channels=2;

    if(ss.channels==2) {
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;

    } else if(ss.channels==6) {
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
        map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
        map.map[3]=PA_CHANNEL_POSITION_LFE;
        map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
        map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;

    } else {
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
        map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
        map.map[3]=PA_CHANNEL_POSITION_LFE;
        map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
        map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;
        map.map[6]=PA_CHANNEL_POSITION_SIDE_LEFT;
        map.map[7]=PA_CHANNEL_POSITION_SIDE_RIGHT;

    }

    int error;

    if(s) {
        pa_simple_free(s);
        s=nullptr;
    }

    s=pa_simple_new(nullptr, "capturer", PA_STREAM_PLAYBACK, nullptr, "audio output", &ss, &map, nullptr, &error);

    if(!s) {
        qCritical() << "pa_simple_new err:" << pa_strerror(error);

        running=false;
    }

#endif
}
