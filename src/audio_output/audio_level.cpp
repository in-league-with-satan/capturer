#include <QDebug>
#include <QTimer>
#include <qcoreapplication.h>

#include <algorithm>

#include "frame_buffer.h"

#include "audio_level.h"

#ifdef _MSC_VER
#undef max
#endif

const int16_t max_value=std::numeric_limits<int16_t>::max();

const QString channel_name[]={ "L", "R", "C", "LFE", "BL", "BR", "SL", "SR" };

AudioLevel::AudioLevel(QObject *parent) :
    QThread(parent)
{
    memset(level, 0, sizeof(int16_t)*8);

    //

    frame_buffer=new FrameBuffer(this);
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

FrameBuffer *AudioLevel::frameBuffer()
{
    return frame_buffer;
}

void AudioLevel::run()
{
    Frame::ptr frame;

    running=true;

    while(running) {
        frame=frame_buffer->take();

        if(frame) {
            memset(level, 0, sizeof(int16_t)*8);

            int16_t *ptr_data=(int16_t*)frame->audio.raw.data();

            for(int pos=0, size=frame->audio.raw.size()/2; pos<size; pos+=8)
                for(int channel=0; channel<8; ++channel)
                    level[channel]=std::max(level[channel], ptr_data[pos + channel]);

            emit levels(level[0], level[1], level[2], level[3], level[4], level[5], level[6], level[7]);

            frame.reset();

            qApp->processEvents();
        }
    }
}
