#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QAudioFormat>

#include "audio_output_interface.h"
#include "audio_io_device.h"

class QAudioOutput;
class QIODevice;

class AudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    AudioOutputThread(QObject *parent=0);
    ~AudioOutputThread();

protected:
    virtual void run();

private:
    QAudioOutput *audio_output;

    AudioIODevice dev_audio_output;

    QAudioFormat audio_format;

};

#endif // AUDIO_OUTPUT_THREAD_H
