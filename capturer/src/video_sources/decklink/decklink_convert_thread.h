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

#ifndef DL_CONVERT_THREAD_H
#define DL_CONVERT_THREAD_H

#include <QThread>
#include <QMutex>
#include <QVector>

#include <atomic>

#include "frame_buffer.h"

#include "event_waiting.h"

class DeckLinkThread;

class IDeckLinkVideoConversion;
class IDeckLinkVideoFrame;
class IDeckLinkAudioInputPacket;
class IDeckLinkMutableVideoFrame;

class DlConvertThread;
class DlConvertThreadContainer;

typedef void (*FrameCompletedCallback)(Frame::ptr);

class DlConvertThreadContainer : public QObject
{
    Q_OBJECT

public:
    explicit DlConvertThreadContainer(int thread_count, QObject *parent=0);
    ~DlConvertThreadContainer();

    void stopThreads();

    void addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet, uint8_t counter, bool reset_counter);

    void subscribe(FrameBuffer<Frame::ptr>::ptr obj);
    void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj);

    void setAudioChannels(int value);
    void setSampleSize(int value);

    void frameCompleted(Frame::ptr frame);

    QVector <DlConvertThread*> thread;

private:
    void queueClear();

    int thread_count;
    int thread_num;

    QMutex mutex_subscription;

    uint8_t last_frame_counter;

    QList <Frame::ptr> queue;

    QList <FrameBuffer<Frame::ptr>::ptr> subscription_list;

signals:
    void frameSkipped();
};

class DlConvertThread : public QThread
{
    Q_OBJECT

    friend class DlConvertThreadContainer;

public:
    explicit DlConvertThread(FrameCompletedCallback func_frame_completed, QObject *parent=0);
    ~DlConvertThread();

    void term();

    void addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet, uint8_t frame_counter, bool reset_counter);

private:
    IDeckLinkVideoConversion *video_converter;

    IDeckLinkVideoFrame *frame_video_src;
    IDeckLinkAudioInputPacket *frame_audio_src;

    uint8_t frame_counter;
    bool reset_counter;

    FrameCompletedCallback func_frame_completed;

    QMutex mutex;

    EventWaiting event;

    int audio_channels;
    int sample_size;

    std::atomic <bool> running;

protected:
    void run();
};

#endif // DL_CONVERT_THREAD_H
