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

QByteArray AudioOutputInterface::convert(void *data, size_t size, const int in_channels, int in_sample_size, int out_channels)
{
    if(in_channels==2 && in_sample_size==16)
        return QByteArray((char*)data, size);

    if((uint64_t)in_channels!=audio_converter->inChannels() || (in_sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32)!=audio_converter->inSampleFormat()) {
        if(!audio_converter->init(av_get_default_channel_layout(in_channels), 48000, in_sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32,
                                  // AV_CH_LAYOUT_STEREO
                                  av_get_default_channel_layout(out_channels)
                                  , 48000, AV_SAMPLE_FMT_S16)) {
            qCritical() << "AudioConverter init err";

            running=false;

            return QByteArray();
        }
    }

    QByteArray ba_res;

    audio_converter->convert(data, size, &ba_res);

    return ba_res;
}
