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

                dev_audio_output.write(convert(frame->audio.ptr_data, frame->audio.data_size, frame->audio.channels, frame->audio.sample_size));

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
