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
#include <QAudioOutput>
#include <qcoreapplication.h>

#ifdef USE_SDL2
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#endif

#include "frame_buffer.h"
#include "audio_tools.h"

#include "sdl2_audio_output_thread.h"

void callbackSdlAudio(void *userdata, uint8_t *stream, int len)
{
    Sdl2AudioOutputThread *obj=(Sdl2AudioOutputThread*)userdata;

//    qInfo() << "callbackSdlAudio" << len << obj->ba_out_buffer.size();

    if(obj->ba_out_buffer.size()<len)
        return;

    QByteArray ba_tmp=obj->ba_out_buffer.left(len);

    obj->ba_out_buffer.remove(0, len);

    memcpy(stream, ba_tmp.constData(), len);
}

Sdl2AudioOutputThread::Sdl2AudioOutputThread(QObject *parent) :
    AudioOutputInterface(parent)
{
#ifdef USE_SDL2

    int ret=SDL_Init(SDL_INIT_AUDIO);

    qInfo() << "SDL_Init:" << ret;

    start(QThread::NormalPriority);

#endif
}

Sdl2AudioOutputThread::~Sdl2AudioOutputThread()
{
    terminate();
}

void Sdl2AudioOutputThread::run()
{
#ifdef USE_SDL2

    SDL_AudioSpec spec_deired;
    SDL_AudioSpec spec_obtained;

    spec_deired.freq=48000;
    spec_deired.channels=6;
    spec_deired.format=AUDIO_S16LSB;
    spec_deired.userdata=this;
    spec_deired.samples=2048;
    spec_deired.callback=callbackSdlAudio;

    if(SDL_OpenAudio(&spec_deired, &spec_obtained)<0) {
        qCritical() << "SDL_OpenAudio err:" << SDL_GetError();
        exit(1);
    }

    qInfo() << spec_deired.channels << spec_obtained.channels << spec_deired.samples << spec_obtained.samples;


    SDL_PauseAudio(0);

    Frame::ptr frame;

    while(true) {
        frame=frame_buffer->take();

        if(frame) {
            onInputFrameArrived(frame->audio.raw);

            frame.reset();
        }

        QCoreApplication::processEvents();

        usleep(1000);
    }

#endif
}

void Sdl2AudioOutputThread::onInputFrameArrived(QByteArray ba_data)
{
    /*
    if(input_channels_size!=2) {
        QByteArray ba_tmp;

        mix8channelsTo2(&ba_data, &ba_tmp);

        ba_data=ba_tmp;
    }
    */

    /*
    if(input_channels_size!=2) {
        QByteArray ba_tmp;

        mix8channelsTo6(&ba_data, &ba_tmp);

        ba_data=ba_tmp;
    }
    */

    if(ba_out_buffer.size()>ba_data.size()*4) {
//        qCritical() << "ba_out_buffer overflow";
        return;
    }

    ba_out_buffer+=ba_data;
}