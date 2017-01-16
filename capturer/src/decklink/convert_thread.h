#ifndef DL_CONVERT_THREAD_H
#define DL_CONVERT_THREAD_H

#include <QThread>
#include <QMutex>
#include <QVector>

//#include "ffmpeg.h"

class FrameBuffer;
class DeckLinkCapture;

class IDeckLinkVideoConversion;
class IDeckLinkOutput;
class IDeckLinkVideoFrame;
class IDeckLinkAudioInputPacket;
class IDeckLinkMutableVideoFrame;


class DlConvertThread;
class DlConvertThreadContainer;

class DlConvertThreadContainer
{
public:
    explicit DlConvertThreadContainer(int thread_count);

    void addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet);

    void subscribeForAll(FrameBuffer *obj);
    void subscribeForAudio(FrameBuffer *obj);
    void unsubscribe(FrameBuffer *obj);

    void setAudioChannels(int value);

    void init(IDeckLinkOutput *decklink_output);

    QVector <DlConvertThread*> thread;

private:
    int thread_count;
    int thread_num;
};


class DlConvertThread : public QThread
{
    Q_OBJECT

    friend class DlConvertThreadContainer;

public:
    explicit DlConvertThread(QObject *parent=0);
    ~DlConvertThread();

    FrameBuffer *frameBuffer();

    void addFrame(IDeckLinkVideoFrame *frame, IDeckLinkAudioInputPacket *audio_packet);


    void subscribeForAll(FrameBuffer *obj);
    void subscribeForAudio(FrameBuffer *obj);
    void unsubscribe(FrameBuffer *obj);

public slots:

private:
    IDeckLinkVideoConversion *video_converter;
    IDeckLinkMutableVideoFrame *video_frame_converted_720p;
    IDeckLinkMutableVideoFrame *video_frame_converted_1080p;
    IDeckLinkMutableVideoFrame *video_frame_converted_2160p;

    IDeckLinkVideoFrame *frame_video_src;
    IDeckLinkAudioInputPacket *frame_audio_src;

    QList <FrameBuffer*> l_full;
    QList <FrameBuffer*> l_audio;

    QMutex mutex;
    QMutex mutex_subscription;

    int audio_channels;

protected:
    void run();

signals:

};

#endif // DL_CONVERT_THREAD_H
