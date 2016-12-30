#include <QDebug>
#include <QAudioOutput>
#include <QMutexLocker>
#include <qcoreapplication.h>

#ifdef USE_PULSE_AUDIO
#include <pulse/error.h>
#endif

#include "frame_buffer.h"
#include "audio_tools.h"

#include "pulse_audio_output_thread.h"

PulseAudioOutputThread::PulseAudioOutputThread(QObject *parent) :
    AudioOutputInterface(parent)
{
    start(QThread::NormalPriority);
}

PulseAudioOutputThread::~PulseAudioOutputThread()
{
}

void PulseAudioOutputThread::run()
{
#ifdef USE_PULSE_AUDIO

    ss.rate=48000;
    ss.format=PA_SAMPLE_S16LE;
//    ss.channels=2;
    ss.channels=6;

    int error;

    pa_channel_map map;
    map.channels=6;
    map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
    map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
    map.map[2]=PA_CHANNEL_POSITION_LFE;
    map.map[3]=PA_CHANNEL_POSITION_FRONT_CENTER;
    map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
    map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;

    s=pa_simple_new(nullptr, "capturer", PA_STREAM_PLAYBACK, nullptr, "audio output", &ss, &map, nullptr, &error);

    if(!s) {
        qCritical() << "pa_simple_new err:" << pa_strerror(error);
        exit(1);
        // return;
    }


    FrameBuffer::Frame frame;

    while(true) {
        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty())
                goto end;

            frame=frame_buffer->queue.dequeue();
        }

        onInputFrameArrived(frame.ba_audio);

end:
        QCoreApplication::processEvents();

        usleep(1000);
    }

#endif
}

void PulseAudioOutputThread::onInputFrameArrived(QByteArray ba_data)
{
#ifdef USE_PULSE_AUDIO

    if(!s) {
        return;
    }

    /*
    if(input_channels_size!=2) {
        QByteArray ba_tmp;

        mix8channelsTo2(&ba_data, &ba_tmp);

        ba_data=ba_tmp;
    }
    */

    if(input_channels_size!=2) {
        QByteArray ba_tmp;

        mix8channelsTo6(&ba_data, &ba_tmp);

        ba_data=ba_tmp;
    }

    int error=0;

    if(pa_simple_write(s, ba_data.data(), ba_data.size(), &error)<0) {
        qCritical() << "pa_simple_write err" << pa_strerror(error);

//        pa_simple_free(s);
//        s=nullptr;
    }

//    pa_simple_drain(s, nullptr);

#endif
}
