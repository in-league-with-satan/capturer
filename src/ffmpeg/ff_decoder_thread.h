#ifndef FF_DECODER_THREAD_H
#define FF_DECODER_THREAD_H

#include <QThread>
#include <QSize>

extern "C" {
#include <libavformat/avformat.h>
}

#include "protect.h"

class AVFormatContext;
class AVCodecContext;
class AVCodec;
class SwsContext;
class SwrContext;
class AVFrame;
class AVStream;

class FrameBuffer;


class FFDecoderThread : public QThread
{
    Q_OBJECT

public:
    FFDecoderThread(QObject *parent=0);
    ~FFDecoderThread();

    void subscribeVideo(FrameBuffer *obj);
    void subscribeAudio(FrameBuffer *obj);

    enum States {
        ST_IDLE,
        ST_OPEN,
        ST_PLAY,
        ST_SEEK,
        ST_STOPPING,
        ST_STOPPED
    };

public slots:
    void open(const QString &filename);

    void play();
    void pause();

    void stop();
    void seek(qint64 position);

    int currentState() const;
    int64_t currentPos() const;
    int64_t currentDuration() const;

    void term();

private:
    struct Context {
        Context() {
            format_context=nullptr;

            codec_context_video=nullptr;
            codec_context_audio=nullptr;

            codec_video=nullptr;
            codec_audio=nullptr;

            stream_video=nullptr;
            stream_audio=nullptr;

            convert_context_video=nullptr;
            convert_context_audio=nullptr;

            out_video_buffer=nullptr;
            out_audio_buffer=nullptr;

            frame_video=nullptr;
            frame_audio=nullptr;
            frame_rgb=nullptr;

            //skip_pkt=-10;
            skip_pkt=0;
        }

        AVFormatContext *format_context;

        AVCodecContext *codec_context_video;
        AVCodecContext *codec_context_audio;

        AVCodec *codec_video;
        AVCodec *codec_audio;

        AVStream *stream_video;
        AVStream *stream_audio;

        SwsContext *convert_context_video;
        SwrContext *convert_context_audio;

        FrameBuffer *out_video_buffer;
        FrameBuffer *out_audio_buffer;

        AVFrame *frame_video;
        AVFrame *frame_audio;
        AVFrame *frame_rgb;

        QSize target_size;

        int64_t last_video_out_time;

        int64_t start_time_video;
        int64_t start_time_audio;

        QList <AVPacket> packets_video;
        QList <AVPacket> packets_audio;

        AVPacket packet;

        int64_t pts_video;
        int64_t pts_audio;

        int64_t audio_buf_size;

        bool wait_video;
        bool wait_audio;

        QByteArray ba_audio;

        int64_t video_frame_duration;

        int64_t audio_clock;
        int64_t video_clock;

        int64_t pict_pts;
        int64_t frame_last_pts;
        int64_t frame_timer;
        int64_t frame_last_delay;

        int skip_pkt;
        int skipped_pkt_video;
        int skipped_pkt_audio;

    } context;

    int64_t getAudioClock();
    int64_t computeDelay();
    int64_t synchronizeVideo(AVFrame *src_frame, int64_t pts);

    int64_t audioTS();
    int64_t videoTS();

    void _open();
    inline void _play();
    void _seek();

    void freeContext();

    std::atomic <int> prev_state;
    std::atomic <int> state;
    std::atomic <bool> running;
    std::atomic <int64_t> seek_pos;

    std::atomic <int64_t> position;
    std::atomic <int64_t> duration;

    Protect <QString> filename;

protected:
    void run();

signals:
    void durationChanged(qint64 value);
    void positionChanged(qint64 value);
    void stateChanged(int state);
};

#endif // FF_DECODER_THREAD_H
