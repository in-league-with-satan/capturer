#include <QDebug>
#include <QAudioOutput>
#include <QMutexLocker>
#include <qcoreapplication.h>

#include "frame_buffer.h"
#include "audio_tools.h"

#include "audio_output_thread.h"

AudioOutputThread::AudioOutputThread(QObject *parent) :
    AudioOutputInterface(parent)
{
    audio_output=nullptr;
    dev_audio_output=nullptr;

    start(QThread::NormalPriority);
}

AudioOutputThread::~AudioOutputThread()
{
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
    new_audio_format.setSampleType(QAudioFormat::UnSignedInt);

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

    dev_audio_output=nullptr;

    audio_output->stop();

    audio_output->deleteLater();

    audio_output=new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), audio_format);

    if(!audio_output) {
        qCritical() << "audio_output error";
        return;
    }

    dev_audio_output=audio_output->start();

    if(!dev_audio_output) {
        qCritical() << "audio_output->start err";
        return;
    }
}

void AudioOutputThread::run()
{
    audio_format.setSampleRate(48000);
    audio_format.setChannelCount(2);
    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(QAudioFormat::LittleEndian);
    audio_format.setSampleType(QAudioFormat::UnSignedInt);


    QAudioDeviceInfo audio_device_info(QAudioDeviceInfo::defaultOutputDevice());

    if(!audio_device_info.isFormatSupported(audio_format)) {
        qCritical() << "audio format is not supported";

        return;
    }

    audio_output=new QAudioOutput(QAudioDeviceInfo::defaultOutputDevice(), audio_format);
    audio_output->setBufferSize(1024*20);


    dev_audio_output=audio_output->start();

    if(!dev_audio_output) {
        qCritical() << "audio_output->start err";
        return;
    }

    //

    FrameBuffer::Frame frame;

    while(true) {
        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty())
                goto end;

            frame=frame_buffer->queue.dequeue();
        }

        onInputFrameArrived(frame.ba_audio);

end:
        QCoreApplication::processEvents();

        usleep(1000);
    }
}

void AudioOutputThread::onInputFrameArrived(QByteArray ba_data)
{
    if(!dev_audio_output)
        return;

    if(input_channels_size!=2) {
        QByteArray ba_tmp;

        mix8channelsTo2(&ba_data, &ba_tmp);

        ba_data=ba_tmp;
    }

    dev_audio_output->write(ba_data);
}
