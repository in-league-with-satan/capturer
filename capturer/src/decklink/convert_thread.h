#ifndef DL_CONVERT_THREAD_H
#define DL_CONVERT_THREAD_H

#include <QThread>
#include <QMutex>
#include <QVector>

#include "frame_buffer.h"

class FrameBuffer;
class DeckLinkCapture;

class IDeckLinkVideoConversion;
class IDeckLinkOutput;
class IDeckLinkVideoFrame;
class IDeckLinkAudioInputPacket;
class IDeckLinkMutableVideoFrame;

class DlConvertThread;
class DlConvertThreadContainer;

typedef void (*FrameCompletedCallback)(FrameBuffer::Frame);

class DlConvertThreadContainer : public QObject
{
    Q_OBJECT

public:
    explicit DlConvertThreadContainer(int thread_count, QObject *parent=0);
    ~DlConvertThreadContainer();

    void addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet, uint8_t counter, bool reset_counter);

    void subscribeForAll(FrameBuffer *obj);
    void subscribeForAudio(FrameBuffer *obj);
    void unsubscribe(FrameBuffer *obj);

    void setAudioChannels(int value);

    void init(IDeckLinkOutput *decklink_output);

    void frameCompleted(FrameBuffer::Frame frame);

    QVector <DlConvertThread*> thread;

private:
    void queueClear();

    int thread_count;
    int thread_num;

    QMutex mutex_subscription;

    uint8_t last_frame_counter;

    QList <FrameBuffer::Frame> queue;

    QList <FrameBuffer*> l_full;
    QList <FrameBuffer*> l_audio;

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

    FrameBuffer *frameBuffer();

    void addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet, uint8_t frame_counter, bool reset_counter);

private:
    IDeckLinkVideoConversion *video_converter;
    IDeckLinkMutableVideoFrame *video_frame_converted_720p;
    IDeckLinkMutableVideoFrame *video_frame_converted_1080p;
    IDeckLinkMutableVideoFrame *video_frame_converted_2160p;

    IDeckLinkVideoFrame *frame_video_src;
    IDeckLinkAudioInputPacket *frame_audio_src;

    uint8_t frame_counter;
    bool reset_counter;

    FrameCompletedCallback func_frame_completed;

    QMutex mutex;

    int audio_channels;

protected:
    void run();
};

#endif // DL_CONVERT_THREAD_H
