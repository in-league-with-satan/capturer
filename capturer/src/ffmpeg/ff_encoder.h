/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef FF_ENCODER_H
#define FF_ENCODER_H

#include <QObject>
#include <QImage>
#include <QDateTime>

#include "frame.h"
#include "ff_encoder_base_filename.h"

extern "C" {
#include <libavformat/avformat.h>
}

class FFMpegContext;
class FFFormatConverter;
class DecklinkFrameConverter;


class FFEncoder : public QObject
{
    Q_OBJECT

public:
    struct Mode {
        enum T {
            primary,
            webcam
        };
    };

    FFEncoder(FFEncoder::Mode::T mode, QObject *parent=0);
    ~FFEncoder();

    struct Framerate {
        enum T {
            full_11,
            full_12,
            full_12_5,
            full_14,
            full_15,
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
            nvenc_hevc,
            qsv_h264,
            ffvhuff
            // magicyuv
        };

        static QString toString(uint32_t enc);
        static QString toEncName(uint32_t enc);

        static uint64_t fromString(QString value);

        static QList <FFEncoder::VideoEncoder::T> list();
    };

    struct PixelFormat {
        enum T {
            RGB24=AV_PIX_FMT_RGB24,
            YUV420P=AV_PIX_FMT_YUV420P,
            YUV422P=AV_PIX_FMT_YUV422P,
            UYVY422=AV_PIX_FMT_UYVY422,
            YUV444P=AV_PIX_FMT_YUV444P,
            YUV420P10=AV_PIX_FMT_YUV420P10,
            // YUV422P10=AV_PIX_FMT_YUV422P10,
            V210=AV_PIX_FMT_YUV422P10LE,
            YUV444P10=AV_PIX_FMT_YUV444P10,
            R210=AV_PIX_FMT_RGB48LE,
            P010LE=AV_PIX_FMT_P010LE,
            NV12=AV_PIX_FMT_NV12
        };

        static QString toString(uint32_t value);

        static uint64_t fromString(QString value);

        static QList <FFEncoder::PixelFormat::T> compatiblePixelFormats(VideoEncoder::T encoder);

        static QList <FFEncoder::PixelFormat::T> list();
    };

    struct DownScale {
        enum T {
            Disabled,
            to720,
            to1080,
            to1440,
            to1800
        };

        static int toWidth(uint32_t value);
        static QString toString(uint32_t value);
    };

    struct ScaleFilter {
        enum T {
            FastBilinear,
            Bilinear,
            Bicubic,
            X,
            Point,
            Area,
            Bicublin,
            Gauss,
            Sinc,
            Lanczos,
            Spline
        };

        static int toSws(uint32_t value);
        static QString toString(uint32_t value);
    };

    struct Config {
        Config() {
            audio_channels_size=8;
            audio_sample_size=16;
            audio_dalay=0;
            downscale=DownScale::Disabled;
            scale_filter=ScaleFilter::FastBilinear;
            rgb_source=true;
            rgb_10bit=false;
        }

        QSize frame_resolution_src;
        QSize frame_resolution_dst;
        Framerate::T framerate;
        uint8_t audio_channels_size;
        uint8_t audio_sample_size;
        int audio_dalay;
        uint8_t crf;
        uint8_t downscale;
        int scale_filter;
        AVPixelFormat pixel_format;
        VideoEncoder::T video_encoder;
        QString preset;
        bool rgb_source;
        bool rgb_10bit;
    };

    struct Stats {
        QTime time;
        double avg_bitrate_audio;
        double avg_bitrate_video;
        size_t streams_size;
    };

    static Framerate::T calcFps(int64_t frame_duration, int64_t frame_scale, bool half_fps);
    static QString presetVisualNameToParamName(const QString &str);
    static QStringList compatiblePresets(VideoEncoder::T encoder);

    void setBaseFilename(FFEncoderBaseFilename *bf);

    QString lastErrorString() const;


public slots:
    bool setConfig(FFEncoder::Config cfg);

    bool appendFrame(Frame::ptr frame);

    bool stopCoder();

private:
    void calcStats();

    FFMpegContext *context;
    FFFormatConverter *format_converter_ff;
    DecklinkFrameConverter *format_converter_dl;

    QSize last_frame_size;

    QString last_error_string;

signals:
    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);
    void errorString(QString err_string);
};

Q_DECLARE_METATYPE(FFEncoder::Config)
Q_DECLARE_METATYPE(FFEncoder::Stats)

#endif // FF_ENCODER_H
