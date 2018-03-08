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
#include <QDateTime>

#include "decklink_dummy.h"

DeckLinkDummy::DeckLinkDummy(bool frame_counter, int frame_height, QObject *parent)
    : DeckLinkCapture(parent)
    , frame_counter(frame_counter)
{
    switch(frame_height) {
    case 480:
        frame_size=QSize(640, 480);
        break;

    case 720:
        frame_size=QSize(1280, 720);
        break;

    case 1080:
        frame_size=QSize(1920, 1080);
        break;

    case 2160:
        frame_size=QSize(3840, 2160);
        break;

    default:
        frame_size=QSize(1920, 1080);
    }

    running=false;

    start(QThread::NormalPriority);
}

DeckLinkDummy::~DeckLinkDummy()
{
    running_thread=false;

    while(isRunning()) {
        msleep(30);
    }
}

bool DeckLinkDummy::isRunning() const
{
    return running;
}

bool DeckLinkDummy::gotSignal() const
{
    return true;
}

bool DeckLinkDummy::sourceRGB() const
{
    return true;
}

bool DeckLinkDummy::source10Bit() const
{
    return false;
}

void DeckLinkDummy::run()
{
    std::srand(std::time(nullptr));

    glazing_ribbon.init(frame_size, 24, 1, true, QPainter::CompositionMode_SourceOver);
    glazing_ribbon.showFrameCounter(frame_counter);

    time_last_frame=QDateTime::currentMSecsSinceEpoch();

    running_thread=true;

    qint64 t;

    while(running_thread) {
        if(running) {
            QImage img=glazing_ribbon.next();

            if(!img.isNull() && !subscription_list.isEmpty()) {
                Frame::ptr frame=Frame::make();

                frame->video.size=img.size();
                frame->video.data_size=DeckLinkVideoFrame::frameSize(frame->video.size, bmdFormat8BitBGRA);
                frame->video.dummy.resize(frame->video.data_size);
                frame->video.data_ptr=(uint8_t*)frame->video.dummy.constData();
                frame->video.source_rgb=true;
                frame->video.source_10bit=false;

                memcpy(frame->video.data_ptr, img.bits(), frame->video.data_size);

                t=16 - (QDateTime::currentMSecsSinceEpoch() - time_last_frame);

                if(t>0)
                    msleep(t);

                foreach(FrameBuffer::ptr buf, subscription_list)
                    buf->append(frame);

                time_last_frame=QDateTime::currentMSecsSinceEpoch();

            } else {
                msleep(30);
            }

        } else {
            msleep(30);
        }
    }
}

void DeckLinkDummy::captureStart()
{
    running=true;

    emit formatChanged(frame_size.width(), frame_size.height(), 1000, 60000, true, "rgb32");
    emit signalLost(false);
}

void DeckLinkDummy::captureStop()
{
    running=false;
}
