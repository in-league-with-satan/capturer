#ifndef PULSE_AUDIO_OUTPUT_THREAD_H
#define PULSE_AUDIO_OUTPUT_THREAD_H

#include "audio_output_interface.h"

#ifdef USE_PULSE_AUDIO
#include <pulse/simple.h>
#endif

#include <QFile>

class PulseAudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    PulseAudioOutputThread(QObject *parent=0);
    ~PulseAudioOutputThread();

public slots:
    virtual void changeChannels(int size);

protected:
    virtual void run();

    void onInputFrameArrived(QByteArray ba_data);

    void init();

#ifdef USE_PULSE_AUDIO
    pa_simple *s;
    pa_sample_spec ss;
#endif

    QFile f_src;
    QFile f_conv;
};

#endif // PULSE_AUDIO_OUTPUT_THREAD_H
