#ifndef ODD_EVEN_CONVERTER_H
#define ODD_EVEN_CONVERTER_H

#include <QThread>
#include <QSize>

#include <atomic>

extern "C" {
#include <libavformat/avformat.h>
}


class OddEvenConverterThread : public QThread
{
    Q_OBJECT

public:
    OddEvenConverterThread(QObject *parent=0);
    ~OddEvenConverterThread();

    struct VideoEncoder {
        enum T {
            libx264,
            libx265,
            nvenc_h264
        };
    };

    struct PixelFormat {
        enum T {
            YUV420P=AV_PIX_FMT_YUV420P,
            YUV444P=AV_PIX_FMT_YUV444P,
            YUV420P10=AV_PIX_FMT_YUV420P10,
            YUV444P10=AV_PIX_FMT_YUV444P10
        };
    };

    struct Config {
        Config() {
            video_encoder=VideoEncoder::nvenc_h264;
            pixel_format=PixelFormat::YUV420P;
            crf=4;
            swap_the_frame_order=false;
        }

        QString filename_src;
        QString filename_dst;
        VideoEncoder::T video_encoder;
        PixelFormat::T pixel_format;
        uint16_t crf;

        bool swap_the_frame_order;

    } config;

    bool start(Config cfg);

public slots:

private:

protected:
    void run();

signals:

};

#endif // ODD_EVEN_CONVERTER_H
