#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QAudioFormat>

#include "audio_output_interface.h"

class QAudioOutput;
class QIODevice;

class AudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    AudioOutputThread(QObject *parent=0);
    ~AudioOutputThread();

public slots:
    virtual void changeChannels(int size);

protected:
    virtual void run();

private:
    void onInputFrameArrived(QByteArray ba_data);

    QAudioOutput *audio_output;
    QIODevice *dev_audio_output;

    QAudioFormat audio_format;
};

#endif // AUDIO_OUTPUT_THREAD_H
