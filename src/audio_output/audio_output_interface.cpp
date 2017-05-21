#include <QDebug>
#include <QAudioOutput>
#include <qcoreapplication.h>

#include "frame_buffer.h"

#include "audio_output_interface.h"

AudioOutputInterface::AudioOutputInterface(QObject *parent) :
    QThread(parent)
{
    frame_buffer=new FrameBuffer(this);
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

FrameBuffer *AudioOutputInterface::frameBuffer()
{
    return frame_buffer;
}

void AudioOutputInterface::changeChannels(int size)
{
    input_channels_size=size;
}
