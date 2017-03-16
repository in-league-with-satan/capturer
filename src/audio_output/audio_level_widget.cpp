#include <QDebug>
#include <QTimer>
#include <QMutexLocker>
#include <QPainter>

#include <algorithm>

#include "frame_buffer.h"

#include "audio_level_widget.h"

const int16_t max_value=std::numeric_limits<int16_t>::max();

const QString channel_name[]={ "L", "R", "C", "LFE", "BL", "BR", "SL", "SR" };

AudioLevelWidget::AudioLevelWidget(QWidget *parent) :
    QWidget(parent)
{
    setMinimumWidth(24);
    setMinimumHeight(48);

    memset(level, 0, sizeof(int16_t)*8);

    //

    frame_buffer=new FrameBuffer(QMutex::Recursive, this);
    frame_buffer->setMaxBufferSize(1);
    frame_buffer->setDropSkipped(true);

    timer=new QTimer(this);
    timer->setInterval(60);

    connect(timer, SIGNAL(timeout()), SLOT(checkBuffer()));

    timer->start();
}

FrameBuffer *AudioLevelWidget::frameBuffer()
{
    return frame_buffer;
}

void AudioLevelWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    int h=height();

    painter.fillRect(QRect(0, 0, width(), h), Qt::white);

    int graph_width=(width() - 18)/8;
    int graph_height;

    int text_height=20;

    for(int channel=0; channel<8; ++channel) {
        graph_height=level[channel]*(h - text_height - 2)/max_value;

        painter.fillRect(QRect(2 + channel*2 + channel*graph_width, h - graph_height - text_height, graph_width, graph_height), Qt::black);

        painter.drawText(QRect(2 + channel*2 + channel*graph_width, h - text_height, graph_width, text_height), Qt::AlignCenter, channel_name[channel]);
    }

    painter.end();
}

void AudioLevelWidget::checkBuffer()
{
    FrameBuffer::Frame frame;

    {
        QMutexLocker ml(frame_buffer->mutex_frame_buffer);

        if(frame_buffer->queue.isEmpty())
            return;

        frame=frame_buffer->queue.dequeue();
    }

    memset(level, 0, sizeof(int16_t)*8);

    int16_t *ptr_data=(int16_t*)frame.ba_audio.data();

    for(int pos=0, size=frame.ba_audio.size()/2; pos<size; pos+=8)
        for(int channel=0; channel<8; ++channel)
            level[channel]=std::max(level[channel], ptr_data[pos + channel]);

    emit levels(level[0], level[1], level[2], level[3], level[4], level[5], level[6], level[7]);

    update();
}
