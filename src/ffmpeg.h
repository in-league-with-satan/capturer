#ifndef FFMPEG_H
#define FFMPEG_H

#include <QObject>
#include <QImage>

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
    };

public slots:
    bool initCoder(Config cfg);

    bool appendFrame(QByteArray ba_video, QSize size, QByteArray ba_audio);

    bool stopCoder();

private:
    FFMpegContext *context;
    FF::FormatConverter *converter;
};

#endif // FFMPEG_H
