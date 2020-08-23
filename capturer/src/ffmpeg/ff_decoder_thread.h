/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef FF_DECODER_THREAD_H
#define FF_DECODER_THREAD_H

#include <QThread>
#include <QSize>

#include <atomic>

extern "C" {
#include <libavformat/avformat.h>
}

#include "protect.h"
#include "frame_buffer.h"
#include "ff_audio_converter.h"

class AVFormatContext;
class AVCodecContext;
class AVCodec;
class SwsContext;
class SwrContext;
class AVFrame;
class AVStream;

class FFDecoderThread : public QThread
{
    Q_OBJECT

public:
    FFDecoderThread(QObject *parent=0);
    ~FFDecoderThread();

    void subscribeVideo(FrameBuffer<Frame::ptr>::ptr obj);
    void subscribeAudio(FrameBuffer<Frame::ptr>::ptr obj);

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
            frame_cnv=nullptr;
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
        // AudioConverter audio_converter;

        int scale_filter=0;

        FrameBuffer<Frame::ptr>::ptr out_video_buffer;
        FrameBuffer<Frame::ptr>::ptr out_audio_buffer;

        AVFrame *frame_video;
        AVFrame *frame_audio;
        AVFrame *frame_cnv;

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

        bool reset_audio;

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
