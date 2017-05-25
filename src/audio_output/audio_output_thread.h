#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QAudioFormat>

#include "audio_output_interface.h"
#include "audio_io_device.h"
#include "ff_audio_converter.h"

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
    void onInputFrameArrived(QByteArray ba_data, int channels, int sample_size=16);

    QAudioOutput *audio_output;

    AudioIODevice dev_audio_output;

    QAudioFormat audio_format;

    AudioConverter audio_converter;
};

#endif // AUDIO_OUTPUT_THREAD_H
