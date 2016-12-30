#include <QDebug>
#include <QAudioOutput>
#include <QMutexLocker>
#include <qcoreapplication.h>

#include "frame_buffer.h"

#include "audio_output_interface.h"

AudioOutputInterface::AudioOutputInterface(QObject *parent) :
    QThread(parent)
{
    frame_buffer=new FrameBuffer(QMutex::Recursive, this);
    frame_buffer->setMaxBufferSize(2);
    frame_buffer->setDropSkipped(true);
}

AudioOutputInterface::~AudioOutputInterface()
{
    delete frame_buffer;
}

FrameBuffer *AudioOutputInterface::frameBuffer()
{
    return frame_buffer;
}

void AudioOutputInterface::changeChannels(int size)
{
    input_channels_size=size;
}
