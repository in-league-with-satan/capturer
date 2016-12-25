#include <QDebug>
#include <QAudioOutput>

#include "audio_output_thread.h"

AudioOutputThread::AudioOutputThread(QWidget *parent)
{
    audio_output=nullptr;
    dev_audio_output=nullptr;

    start(QThread::NormalPriority);
}

AudioOutputThread::~AudioOutputThread()
{

}

void AudioOutputThread::run()
{
    QAudioFormat audio_format;

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
//    audio_output->setBufferSize(1024*10);
    audio_output->setBufferSize(1024*20);


    dev_audio_output=audio_output->start();

    if(!dev_audio_output) {
        qCritical() << "audio_output->start err";
        return;
    }

    //

    exec();
}

void AudioOutputThread::onInputFrameArrived(QByteArray ba_data)
{
//    qDebug() << ba_data.size();

    if(!dev_audio_output)
        return;

    dev_audio_output->write(ba_data);
}
