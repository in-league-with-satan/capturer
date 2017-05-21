#ifndef DL_CONVERT_THREAD_H
#define DL_CONVERT_THREAD_H

#include <QThread>
#include <QMutex>
#include <QVector>

#include <atomic>

#include "frame_buffer.h"

#include "event_waiting.h"

class FrameBuffer;
class DeckLinkCapture;

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

    void subscribe(FrameBuffer *obj);
    void unsubscribe(FrameBuffer *obj);

    void setAudioChannels(int value);

    void frameCompleted(Frame::ptr frame);

    QVector <DlConvertThread*> thread;

private:
    void queueClear();

    int thread_count;
    int thread_num;

    QMutex mutex_subscription;

    uint8_t last_frame_counter;

    QList <Frame::ptr> queue;

    QList <FrameBuffer*> subscription_list;

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

    FrameBuffer *frameBuffer();

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

    std::atomic <bool> running;

protected:
    void run();
};

#endif // DL_CONVERT_THREAD_H
