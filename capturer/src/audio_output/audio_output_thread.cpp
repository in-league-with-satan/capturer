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

#include "frame_buffer.h"
#include "audio_tools.h"

#include "audio_output_thread.h"

AudioOutputThread::AudioOutputThread(QObject *parent) :
    AudioOutputInterface(parent)
{
    audio_output=nullptr;

    dev_audio_output.open(AudioIODevice::ReadWrite);

    frame_buffer->setMaxSize(2);


    audio_format.setSampleRate(48000);
    audio_format.setChannelCount(6);
    audio_format.setChannelCount(2);
    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(QAudioFormat::LittleEndian);
    audio_format.setSampleType(QAudioFormat::SignedInt);


    start(QThread::NormalPriority);
}

AudioOutputThread::~AudioOutputThread()
{
}

void AudioOutputThread::run()
{
    QAudioDeviceInfo audio_device_info(QAudioDeviceInfo::defaultOutputDevice());

    if(!audio_device_info.isFormatSupported(audio_format)) {
        qCritical() << "audio format is not supported";

        return;
    }

    audio_output=new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), audio_format);
    audio_output->setBufferSize(1024*10);


    audio_output->start(&dev_audio_output);

    //

    int buf_trig_size=audio_output->bufferSize()*1.5;
    // int buf_trig_size=audio_output->bufferSize();
    // int buf_trig_size=4096*2;


    Frame::ptr frame;

    running=true;

    while(running) {
        if(dev_audio_output.size()<buf_trig_size) {
            frame=frame_buffer->take();
        }

        if(frame) {
            if(frame->reset_counter) {
                audio_output->reset();
                dev_audio_output.clear();
            }

            if(frame->audio.data_size) {
                // buf_trig_size=frame->audio.raw.size()*.5;

                dev_audio_output.write(convert(frame->audio.data_ptr, frame->audio.data_size, frame->audio.channels, frame->audio.sample_size, audio_format.channelCount()));

                if(audio_output->state()!=QAudio::ActiveState) {
                    audio_output->start(&dev_audio_output);
                }
            }

            frame.reset();
        }

        qApp->processEvents();

        usleep(1);
    }
}
