#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include <QImage>
#include <QDateTime>

extern "C" {
#include <libavformat/avformat.h>
}

class FFMpegContext;

namespace FF {
    class FormatConverter;
}

class FFMpeg : public QObject
{
    Q_OBJECT

public:
    FFMpeg(QObject *parent=0);
    ~FFMpeg();

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
            libx264rgb,
            nvenc_h264,
            nvenc_hevc
        };
    };

    struct Config {
        Config() {
            audio_channels_size=8;
            audio_dalay=0;
        }

        QSize frame_resolution;
        Framerate::T framerate;
        uint8_t audio_channels_size;
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
    bool setConfig(FFMpeg::Config cfg);

    bool appendFrame(QByteArray *ba_video, QSize *size, QByteArray *ba_audio);

    bool stopCoder();

private:
    void calcStats();

    FFMpegContext *context;
    FF::FormatConverter *converter;

    QSize last_frame_size;

signals:
    void stats(FFMpeg::Stats s);

};

Q_DECLARE_METATYPE(FFMpeg::Config)
Q_DECLARE_METATYPE(FFMpeg::Stats)

#endif // FFMPEG_H
