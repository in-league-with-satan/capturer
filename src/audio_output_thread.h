#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QThread>
#include <QAudioFormat>

class QAudioOutput;
class QIODevice;

class AudioOutputThread : public QThread
{
    Q_OBJECT

public:
    AudioOutputThread(QWidget *parent=0);
    ~AudioOutputThread();

public slots:
    void changeChannels(int size);

    void onInputFrameArrived(QByteArray ba_data);

protected:
    void run();

private:
    QAudioOutput *audio_output;
    QIODevice *dev_audio_output;

    QAudioFormat audio_format;

    int input_channels_size;
};

#endif // AUDIO_OUTPUT_THREAD_H
