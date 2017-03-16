#include <QDebug>
#include <QApplication>
#include <QAudioOutput>
#include <QMutexLocker>
#include <qcoreapplication.h>

#ifdef USE_PULSE_AUDIO
#include <pulse/error.h>
#endif

//#define SAVE_STREAM

#include "frame_buffer.h"
#include "audio_tools.h"

#include "pulse_audio_output_thread.h"

PulseAudioOutputThread::PulseAudioOutputThread(QObject *parent) :
    AudioOutputInterface(parent)
{
#ifdef USE_PULSE_AUDIO

    s=nullptr;

#endif

    output_channels_size=2;

    start(QThread::NormalPriority);
}

PulseAudioOutputThread::~PulseAudioOutputThread()
{
    terminate();
}

void PulseAudioOutputThread::changeChannels(int size)
{
    AudioOutputInterface::changeChannels(size);

    init();
}

void PulseAudioOutputThread::run()
{
#ifdef SAVE_STREAM

    f_src.setFileName(QApplication::applicationDirPath() + "/aud_src.raw");
    f_conv.setFileName(QApplication::applicationDirPath() + "/aud_conv.raw");

    f_src.open(QFile::ReadWrite | QFile::Truncate | QFile::Unbuffered);
    f_conv.open(QFile::ReadWrite | QFile::Truncate | QFile::Unbuffered);

#endif


#ifdef USE_PULSE_AUDIO

    init();

    FrameBuffer::Frame frame;

    while(true) {
        frame_buffer->event.wait();

        {
            QMutexLocker ml(frame_buffer->mutex_frame_buffer);

            if(frame_buffer->queue.isEmpty())
                goto end;

            frame=frame_buffer->queue.dequeue();
        }

        onInputFrameArrived(frame.ba_audio);

end:
        QCoreApplication::processEvents();
    }

#endif
}

void PulseAudioOutputThread::onInputFrameArrived(QByteArray ba_data)
{
#ifdef USE_PULSE_AUDIO

    if(!s) {
        return;
    }

    if(input_channels_size!=output_channels_size) {
        QByteArray ba_tmp;

        if(input_channels_size==8 && output_channels_size==2)
            mix8channelsTo2(&ba_data, &ba_tmp);

        else if(input_channels_size==8 && output_channels_size==6)
            mix8channelsTo6(&ba_data, &ba_tmp);

#ifdef SAVE_STREAM

        f_src.write(ba_data);
        f_conv.write(ba_tmp);

#endif

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

void PulseAudioOutputThread::init()
{
#ifdef USE_PULSE_AUDIO

    ss.rate=48000;
    ss.format=PA_SAMPLE_S16LE;

    pa_channel_map map;

    ss.channels=map.channels=output_channels_size;

    if(output_channels_size==2) {
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;

    } else if(output_channels_size==6) {
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
        map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
        map.map[3]=PA_CHANNEL_POSITION_LFE;
        map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
        map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;

    } else {
        map.map[0]=PA_CHANNEL_POSITION_FRONT_LEFT;
        map.map[1]=PA_CHANNEL_POSITION_FRONT_RIGHT;
        map.map[2]=PA_CHANNEL_POSITION_FRONT_CENTER;
        map.map[3]=PA_CHANNEL_POSITION_LFE;
        map.map[4]=PA_CHANNEL_POSITION_REAR_LEFT;
        map.map[5]=PA_CHANNEL_POSITION_REAR_RIGHT;
        map.map[6]=PA_CHANNEL_POSITION_SIDE_LEFT;
        map.map[7]=PA_CHANNEL_POSITION_SIDE_RIGHT;

    }

    int error;

    if(s) {
        pa_simple_free(s);
        s=nullptr;
    }

    s=pa_simple_new(nullptr, "capturer", PA_STREAM_PLAYBACK, nullptr, "audio output", &ss, &map, nullptr, &error);

    if(!s) {
        qCritical() << "pa_simple_new err:" << pa_strerror(error);
        exit(1);
        // return;
    }

#endif
}
