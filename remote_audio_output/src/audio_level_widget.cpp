/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QTimer>
#include <QPainter>

#include <algorithm>

#include "audio_level_widget.h"

#ifdef _MSC_VER
#undef max
#endif

const int16_t max_value_16=std::numeric_limits<int16_t>::max();
const int32_t max_value_32=std::numeric_limits<int32_t>::max();

const QString channel_name[]={ "L", "R", "C", "LFE", "BL", "BR", "SL", "SR" };

AudioLevelWidget::AudioLevelWidget(QWidget *parent) :
    QWidget(parent)
{
    setMinimumWidth(24);
    setMinimumHeight(48);

    memset(level, 0, sizeof(int32_t)*8);
}

void AudioLevelWidget::write(const QByteArray &data, int channels, int sample_size)
{
    if(!isVisible())
        return;

    if(channels<1 || channels>8)
        return;

    if(sample_size!=16 && sample_size!=32)
        return;

    memset(level, 0, sizeof(int32_t)*8);

    if(sample_size==16) {
        sample_size_16=true;

        int16_t *ptr_data=(int16_t*)data.constData();

        for(int pos=0, size=data.size()/2; pos<size; pos+=channels)
            for(int channel=0; channel<channels; ++channel)
                level[channel]=std::max(level[channel], (int32_t)ptr_data[pos + channel]);

    } else {
        sample_size_16=false;

        int32_t *ptr_data=(int32_t*)data.constData();

        for(int pos=0, size=data.size()/4; pos<size; pos+=channels)
            for(int channel=0; channel<channels; ++channel)
                level[channel]=std::max(level[channel], ptr_data[pos + channel]);
    }

    update();
}

void AudioLevelWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    int h=height();

    painter.fillRect(QRect(0, 0, width(), h), Qt::white);

    int graph_width=(width() - 18)/8;
    int graph_height;

    int text_height=20;

    int32_t max_value=sample_size_16 ? max_value_16 : max_value_32;

    for(int channel=0; channel<8; ++channel) {
        graph_height=(double)level[channel]/(double)max_value*(double)(h - text_height - 2);

        painter.fillRect(QRect(2 + channel*2 + channel*graph_width, h - graph_height - text_height, graph_width, graph_height), Qt::black);

        painter.drawText(QRect(2 + channel*2 + channel*graph_width, h - text_height, graph_width, text_height), Qt::AlignCenter, channel_name[channel]);
    }

    painter.end();
}
