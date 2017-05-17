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

    setTerminationEnabled();

    start(QThread::NormalPriority);
}

AudioOutputThread::~AudioOutputThread()
{
    terminate();
}

void AudioOutputThread::changeChannels(int size)
{
    AudioOutputInterface::changeChannels(size);

    return;

    QAudioFormat new_audio_format;

    new_audio_format.setSampleRate(48000);
    new_audio_format.setChannelCount(input_channels_size);
    new_audio_format.setSampleSize(16);
    new_audio_format.setCodec("audio/pcm");
    new_audio_format.setByteOrder(QAudioFormat::LittleEndian);
    new_audio_format.setSampleType(QAudioFormat::SignedInt);

    if(audio_format==new_audio_format)
        return;

    audio_format=new_audio_format;

    QAudioDeviceInfo audio_device_info(QAudioDeviceInfo::defaultOutputDevice());

    if(!audio_device_info.isFormatSupported(audio_format))
        audio_format=audio_device_info.nearestFormat(audio_format);

    qInfo() << audio_format;

    if(input_channels_size!=audio_format.channelCount()) {
        qWarning() << "input and device channels missmatch" << input_channels_size << audio_format.channelCount();
    }

    audio_output->stop();

    audio_output->deleteLater();

    audio_output=new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), audio_format);

    if(!audio_output) {
        qCritical() << "audio_output error";
        return;
    }

    audio_output->start(&dev_audio_output);
}

void AudioOutputThread::run()
{
    audio_format.setSampleRate(48000);
    audio_format.setChannelCount(2);
    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(QAudioFormat::LittleEndian);
    audio_format.setSampleType(QAudioFormat::SignedInt);


    QAudioDeviceInfo audio_device_info(QAudioDeviceInfo::defaultOutputDevice());

    if(!audio_device_info.isFormatSupported(audio_format)) {
        qCritical() << "audio format is not supported";

        return;
    }

    audio_output=new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), audio_format);
    // audio_output->setBufferSize(1024*20);

    audio_output->start(&dev_audio_output);

    //

    // int buf_trig_size=audio_output->bufferSize()*2;
    int buf_trig_size=audio_output->bufferSize();

    Frame::ptr frame;

    while(true) {
        if(dev_audio_output.size()<buf_trig_size)
            frame=frame_buffer->take();

        if(frame) {
            if(!frame->audio.raw.isEmpty()) {
                 buf_trig_size=frame->audio.raw.size()*.5;

                onInputFrameArrived(frame->audio.raw);

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

void AudioOutputThread::onInputFrameArrived(QByteArray ba_data)
{
    // if(!dev_audio_output)
    //     return;

    if(input_channels_size!=2) {
        QByteArray ba_tmp;

        mix8channelsTo2(&ba_data, &ba_tmp);

        ba_data=ba_tmp;
    }

    dev_audio_output.write(ba_data);
}
