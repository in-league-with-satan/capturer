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

#ifndef MAGEWELL_DEVICE_WORKER_H
#define MAGEWELL_DEVICE_WORKER_H

#include <QObject>
#include <QSize>

#include "magewell_global.h"
#include "source_interface.h"
#include "frame_buffer.h"

class AudioConverter;

class MagewellAudioThread;
class MagewellDeviceWorkerContext;

class MagewellDeviceWorker : public QObject
{
    Q_OBJECT

public:
    explicit MagewellDeviceWorker(QObject *parent=0);
    virtual ~MagewellDeviceWorker();

    void subscribe(FrameBuffer<Frame::ptr>::ptr obj);
    void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj);

    bool isActive();
    bool gotSignal();

    AVRational currentFramerate();
    PixelFormat currentPixelFormat();
    QSize currentFramesize();

    SourceInterface::AudioSampleSize::T currentAudioSampleSize();
    SourceInterface::AudioChannels::T currentAudioChannels();

    bool step();

public slots:
    void setDevice(QSize board_channel);
    void deviceStart();
    void deviceStop();

    void setPixelFormat(PixelFormat fmt);

    void setColorFormat(int value);
    void setQuantizationRange(int value);

    void setPtsEnabled(bool value);

    void setAudioRemapMode(int value);

private:
    bool updateVideoSignalInfo();
    void setState(int value);

    MGHCHANNEL current_channel=0;

    MagewellDeviceWorkerContext *d;
    MagewellAudioThread *a;

    struct State {
        enum T {
            running,
            no_signal,
            unsupported
        };
    };

    int state=State::no_signal;

    bool pts_enabled=false;

protected:
    QList <FrameBuffer<Frame::ptr>::ptr> subscription_list;

signals:
    void framerateChanged(AVRational fr);
    void framesizeChanged(QSize r);

    void audioSampleSizeChanged(SourceInterface::AudioSampleSize::T value);
    void audioChannelsChanged(SourceInterface::AudioChannels::T value);

    void signalLost(bool value);
    void formatChanged(QString format);
    void channelChanged(MGHCHANNEL channel);
    void errorString(QString err_string);
};

#endif // MAGEWELL_DEVICE_WORKER_H
