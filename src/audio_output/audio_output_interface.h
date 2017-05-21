#ifndef AUDIO_OUTPUT_INTERFACE_H
#define AUDIO_OUTPUT_INTERFACE_H

#include <QThread>

class FrameBuffer;

class AudioOutputInterface : public QThread
{
    Q_OBJECT

public:
    AudioOutputInterface(QObject *parent=0);
    ~AudioOutputInterface();

    FrameBuffer *frameBuffer();

public slots:
    virtual void changeChannels(int size);

protected:
    FrameBuffer *frame_buffer;

    int input_channels_size;

    std::atomic <bool> running;
};

#endif // AUDIO_OUTPUT_THREAD_H
