#include <QDebug>
#include <QAudioOutput>
#include <qcoreapplication.h>

#include "audio_output_interface.h"

AudioOutputInterface::AudioOutputInterface(QObject *parent) :
    QThread(parent)
{
    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(2);

    input_channels_size=2;
}

AudioOutputInterface::~AudioOutputInterface()
{
    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer::ptr AudioOutputInterface::frameBuffer()
{
    return frame_buffer;
}

void AudioOutputInterface::changeChannels(int size)
{
    input_channels_size=size;
}
