#include <QDebug>
#include <QAudioOutput>
#include <qcoreapplication.h>

#include "ff_audio_converter.h"
#include "ff_tools.h"

#include "audio_output_interface.h"


AudioOutputInterface::AudioOutputInterface(QObject *parent) :
    QThread(parent)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(2);

    audio_converter=new AudioConverter();
}

AudioOutputInterface::~AudioOutputInterface()
{
    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }

    //

    delete audio_converter;
}

FrameBuffer::ptr AudioOutputInterface::frameBuffer()
{
    return frame_buffer;
}

QByteArray AudioOutputInterface::convert(QByteArray *in, const int channels, int sample_size)
{
    if(channels==2 && sample_size==16)
        return *in;

    if((uint64_t)channels!=audio_converter->inChannels() || (sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32)!=audio_converter->inSampleFormat()) {
        if(!audio_converter->init(av_get_default_channel_layout(channels), 48000, sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32,
                                  AV_CH_LAYOUT_STEREO, 48000, AV_SAMPLE_FMT_S16)) {
            qCritical() << "AudioConverter init err";

            running=false;

            return QByteArray();
        }
    }

    QByteArray ba_res;

    audio_converter->convert(in, &ba_res);

    return ba_res;
}
