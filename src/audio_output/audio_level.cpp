#include <QDebug>
#include <QElapsedTimer>
#include <qcoreapplication.h>

#include <algorithm>

#include "audio_level.h"

#ifdef _MSC_VER
#undef max
#endif

const double max_value_16=std::numeric_limits<int16_t>::max();
const double max_value_32=std::numeric_limits<int32_t>::max();

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
            if(frame->audio.sample_size==16) {
                int16_t *ptr_data=(int16_t*)frame->audio.raw.constData();

                for(int pos=0, size=frame->audio.raw.size()/2; pos<size; pos+=8)
                    for(int channel=0; channel<8; ++channel)
                        level[channel]=std::max(level[channel], (int32_t)ptr_data[pos + channel]);

            } else {
                int32_t *ptr_data=(int32_t*)frame->audio.raw.constData();

                for(int pos=0, size=frame->audio.raw.size()/4; pos<size; pos+=8)
                    for(int channel=0; channel<8; ++channel)
                        level[channel]=std::max(level[channel], ptr_data[pos + channel]);
            }

            if(last_emit_timer.nsecsElapsed()>=emit_interval) {
                if(frame->audio.sample_size==16) {
                    emit levels(level[0]/max_value_16, level[1]/max_value_16, level[2]/max_value_16, level[3]/max_value_16,
                            level[4]/max_value_16, level[5]/max_value_16, level[6]/max_value_16, level[7]/max_value_16);

                } else {
                    emit levels(level[0]/max_value_32, level[1]/max_value_32, level[2]/max_value_32, level[3]/max_value_32,
                            level[4]/max_value_32, level[5]/max_value_32, level[6]/max_value_32, level[7]/max_value_32);
                }

                memset(level, 0, sizeof(int32_t)*8);

                last_emit_timer.restart();
            }

            frame.reset();

            qApp->processEvents();
        }
    }
}
