#ifndef AUDIO_OUTPUT_INTERFACE_H
#define AUDIO_OUTPUT_INTERFACE_H

#include <QThread>

#include <atomic>

#include "frame_buffer.h"

class AudioConverter;

class AudioOutputInterface : public QThread
{
    Q_OBJECT

public:
    AudioOutputInterface(QObject *parent=0);
    ~AudioOutputInterface();

    FrameBuffer::ptr frameBuffer();

protected:
    QByteArray convert(void *data, size_t size, const int in_channels, int in_sample_size, int out_channels);

    FrameBuffer::ptr frame_buffer;

    std::atomic <bool> running;

    AudioConverter *audio_converter;
};

#endif // AUDIO_OUTPUT_THREAD_H
