#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include <QImage>

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

    struct Framerate {
        enum T {
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

    struct Config {
        QSize frame_resolution;
        Framerate::T framerate;
        uint8_t audio_channels_size;
        uint8_t crf;       
        AVPixelFormat pixel_format;
    };

public slots:
    bool setConfig(FFMpeg::Config cfg);

    bool appendFrame(QByteArray *ba_video, QSize *size, QByteArray *ba_audio);

    bool stopCoder();

private:
    FFMpegContext *context;
    FF::FormatConverter *converter;

    QSize last_frame_size;
};

Q_DECLARE_METATYPE(FFMpeg::Config)

#endif // FFMPEG_H
