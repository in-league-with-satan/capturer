#include <QDebug>
#include <QElapsedTimer>
#include <qcoreapplication.h>

#include <algorithm>

#include "audio_level.h"

#ifdef _MSC_VER
#undef max
#endif

const int16_t max_value=std::numeric_limits<int16_t>::max();

AudioLevel::AudioLevel(QObject *parent) :
    QThread(parent)
{
    memset(level, 0, sizeof(int16_t)*8);

    //

    frame_buffer=FrameBuffer::make();
    frame_buffer->setMaxSize(1);

    start();
}

AudioLevel::~AudioLevel()
{
    running=false;

    frame_buffer->append(nullptr);

    while(isRunning()) {
        msleep(30);
    }
}

FrameBuffer::ptr AudioLevel::frameBuffer()
{
    return frame_buffer;
}

void AudioLevel::run()
{
    Frame::ptr frame;

    QElapsedTimer last_emit_timer;
    last_emit_timer.start();

    const int64_t emit_interval=33*1000*1000; // 33 ms, ~30 fps

    running=true;

    while(running) {
        frame_buffer->wait();

        frame=frame_buffer->take();

        if(frame) {
            int16_t *ptr_data=(int16_t*)frame->audio.raw.data();

            for(int pos=0, size=frame->audio.raw.size()/2; pos<size; pos+=8)
                for(int channel=0; channel<8; ++channel)
                    level[channel]=std::max(level[channel], ptr_data[pos + channel]);

            if(last_emit_timer.nsecsElapsed()>=emit_interval) {
                emit levels(level[0], level[1], level[2], level[3], level[4], level[5], level[6], level[7]);

                memset(level, 0, sizeof(int16_t)*8);

                last_emit_timer.restart();
            }

            frame.reset();

            qApp->processEvents();
        }
    }
}
