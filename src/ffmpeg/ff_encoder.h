#ifndef FF_ENCODER_H
#define FF_ENCODER_H

#include <QObject>
#include <QImage>
#include <QDateTime>

#include "frame.h"

extern "C" {
#include <libavformat/avformat.h>
}

class FFMpegContext;

namespace FF {
    class FormatConverter;
}

class FFEncoder : public QObject
{
    Q_OBJECT

public:
    FFEncoder(QObject *parent=0);
    ~FFEncoder();

    static void init();

    static bool isLib_x264_10bit();

    struct Framerate {
        enum T {
            full_23,
            full_24,
            full_25,
            full_29,
            full_30,
            half_50,
            half_59,
            half_60,
            full_50,
            full_59,
            full_60
        };
    };

    struct VideoEncoder {
        enum T {
            libx264,
            libx264_10bit,
            libx264rgb,
            nvenc_h264,
            nvenc_hevc
        };
    };

    struct PixelFormat {
        enum T {
            RGB24=AV_PIX_FMT_RGB24,
            YUV420P=AV_PIX_FMT_YUV420P,
            YUV444P=AV_PIX_FMT_YUV444P,
            YUV420P10=AV_PIX_FMT_YUV420P10,
            YUV444P10=AV_PIX_FMT_YUV444P10
        };

        static QString toString(uint64_t format);

        static uint64_t fromString(QString format);

        static QList <T> compatiblePixelFormats(VideoEncoder::T encoder);
    };

    struct Config {
        Config() {
            audio_channels_size=8;
            audio_sample_size=16;
            audio_dalay=0;
        }

        QSize frame_resolution;
        Framerate::T framerate;
        uint8_t audio_channels_size;
        uint8_t audio_sample_size;
        int audio_dalay;
        uint8_t crf;
        AVPixelFormat pixel_format;
        VideoEncoder::T video_encoder;
    };

    struct Stats {
        QTime time;
        double avg_bitrate_audio;
        double avg_bitrate_video;
        size_t streams_size;
    };

    static Framerate::T calcFps(int64_t frame_duration, int64_t frame_scale, bool half_fps);

public slots:
    bool setConfig(FFEncoder::Config cfg);

    bool appendFrame(Frame::ptr frame);

    bool stopCoder();

private:
    void calcStats();

    FFMpegContext *context;
    FF::FormatConverter *converter;

    QSize last_frame_size;

signals:
    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);
};

Q_DECLARE_METATYPE(FFEncoder::Config)
Q_DECLARE_METATYPE(FFEncoder::Stats)

#endif // FF_ENCODER_H
