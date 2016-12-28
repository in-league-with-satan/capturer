#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QThread>
#include <QAudioFormat>

class QAudioOutput;
class QIODevice;

class FrameBuffer;

class AudioOutputThread : public QThread
{
    Q_OBJECT

public:
    AudioOutputThread(QWidget *parent=0);
    ~AudioOutputThread();

    FrameBuffer *frameBuffer();

public slots:
    void changeChannels(int size);


protected:
    void run();

private:
    void onInputFrameArrived(QByteArray ba_data);

    QAudioOutput *audio_output;
    QIODevice *dev_audio_output;

    QAudioFormat audio_format;

    FrameBuffer *frame_buffer;

    int input_channels_size;
};

#endif // AUDIO_OUTPUT_THREAD_H
