#ifndef SDL2_AUDIO_OUTPUT_THREAD_H
#define SDL2_AUDIO_OUTPUT_THREAD_H

#include "audio_output_interface.h"

class Sdl2AudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    Sdl2AudioOutputThread(QObject *parent=0);
    ~Sdl2AudioOutputThread();

    QByteArray ba_out_buffer;

public slots:

protected:
    virtual void run();

private:
    void onInputFrameArrived(QByteArray ba_data);
};

#endif // SDL2_AUDIO_OUTPUT_THREAD_H
