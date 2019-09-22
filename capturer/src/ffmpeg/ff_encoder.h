/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QMutex>
#include <QMutexLocker>

#include "frame.h"
#include "ff_encoder_base_filename.h"

struct FFMpegContext;
struct OutputStream;
class FFFormatConverter;
class FFFormatConverterMt;
class DecklinkFrameConverter;
class DecodeFrom210;

class FFEncStartSync
{
public:
    void setReady(int index) {
        QMutexLocker ml(&mutex);

        if(state[index]==ST_RESTART)
            return;

        state[index]=ST_READY;
    }

    bool isReady() {
        QMutexLocker ml(&mutex);

        foreach(const int &st, state.values()) {
            if(st!=ST_READY) {
                return false;
            }
        }

        return true;
    }

    void clear() {
        QMutexLocker ml(&mutex);
        state.clear();
    }

    void restart() {
        QMutexLocker ml(&mutex);

        for(int i=0; i<state.size(); ++i)
            state.values()[i]=ST_RESTART;
    }

    void add(int index) {
        QMutexLocker ml(&mutex);
        state[index]=ST_NOT_READY;
    }

private:
    enum {
        ST_NOT_READY,
        ST_READY,
        ST_RESTART
    };

    QMap <int, int> state;
    QMutex mutex;
};

class FFEncoder : public QObject
{
    Q_OBJECT

public:
    FFEncoder(int enc_num, FFEncStartSync *start_sync, QObject *parent=0);
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
            full_60,
            unknown
        };

        static AVRational toRational(Framerate::T value);
    };

    struct VideoEncoder {
        enum T {
            libx264,
            libx264rgb,
            nvenc_h264,
            nvenc_hevc,
            qsv_h264,
            qsv_hevc,
            vaapi_h264,
            vaapi_hevc,
            vaapi_vp8,
            vaapi_vp9,
            ffvhuff,
            magicyuv
        };

        static QString toString(uint32_t enc);
        static QString toEncName(uint32_t enc);

        static uint64_t fromString(QString value);

        static QList <FFEncoder::VideoEncoder::T> list();
    };

    struct AudioEncoder {
        enum T {
            pcm,
            flac,
            opus,
            vorbis,
            aac
        };

        static QString toString(uint32_t enc);
        static QString toEncName(uint32_t enc);

        static uint64_t fromString(QString value);

        static QList <FFEncoder::AudioEncoder::T> list();

        static AVSampleFormat sampleFmt(uint32_t enc);

        static bool setBitrate(uint32_t enc);
    };

    struct DownScale {
        enum T {
            Disabled,
            to720,
            to900,
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
        QSize frame_resolution_src;
        QSize frame_resolution_dst;
        Framerate::T framerate;
        uint8_t audio_channels_size=8;
        uint8_t audio_sample_size=16;
        int audio_dalay=0;
        AudioEncoder::T audio_encoder;
        int audio_bitrate=256;
        int audio_downmix_to_stereo=0;
        bool direct_stream_copy=false;
        bool fill_dropped_frames=false;
        uint8_t crf;
        int video_bitrate=0;
        uint8_t downscale=DownScale::Disabled;
        int scale_filter=ScaleFilter::FastBilinear;
        PixelFormat pixel_format_src;
        PixelFormat pixel_format_dst;
        VideoEncoder::T video_encoder;
        QString preset;
        AVRational framerate_force={ 0, 0 };
        uint32_t input_type_flags=0;

        uint32_t active_src_devices=0;

        int sws_color_space_src=SWS_CS_DEFAULT;
        int sws_color_space_dst=SWS_CS_DEFAULT;

        int sws_color_range_src=AVCOL_RANGE_UNSPECIFIED;
        int sws_color_range_dst=AVCOL_RANGE_UNSPECIFIED;

        int color_primaries=-1;
        int color_space=-1;
        int color_transfer_characteristic=-1;
        int color_range=-1;

        AVMasteringDisplayMetadata mastering_display_metadata={};

        struct NVEnc {
            int enabled=false;
            int device=0;
            int b_frames=0;
            int ref_frames=0;
            int b_ref_mode=0;
            int gop_size=12;
            int qp_i=0;
            int qp_p=0;
            int qp_b=0;
            int aq_mode=0;
            int aq_strength=0;
            int rc_lookahead=0;
            int surfaces=0;
            int no_scenecut=false;
            int forced_idr=false;
            int b_adapt=false;
            int nonref_p=false;
            int strict_gop=false;
            int weighted_pred=false;
            int bluray_compat=false;

        } nvenc;
    };

    struct Stats {
        QTime time;
        double avg_bitrate_audio=0.;
        double avg_bitrate_video=0.;
        size_t streams_size=0;
        int dropped_frames_counter=0;
        QMap <uint32_t, uint32_t> bitrate_video;
    };

    static Framerate::T calcFps(int64_t frame_duration, int64_t frame_scale, bool half_fps);
    static QString presetVisualNameToParamName(const QString &str);
    static QString presetParamNameToVisualName(const QString &str);
    static QStringList compatiblePresets(VideoEncoder::T encoder);

    static QList <int> availableColorPrimaries();
    static QList <int> availableColorSpaces();
    static QList <int> availableColorTransferCharacteristics();

    static QString colorPrimariesToString(int value);
    static QString colorSpaceToString(int value);
    static QString colorTransferCharacteristicToString(int value);

    void setEncodingToolName(const QString &encoding_tool);
    void setStoreDir(const QString &dir);
    void setBaseFilename(FFEncoderBaseFilename *bf);

    QString lastErrorString() const;

    enum {
        StreamingMode=-1
    };

public slots:
    bool setConfig(FFEncoder::Config cfg);

    bool appendFrame(Frame::ptr frame);

    bool stopCoder();

private slots:
    void processAudio(Frame::ptr frame);
    void flushAudio();
    void restartExt();

private:
    void calcStats();
    QTime duration();
    QString configString(const FFEncoder::Config &cfg);

    bool checkFrameParams(Frame::ptr frame) const;
    int64_t calcPts(int64_t pts, AVRational time_base_in, AVRational time_base_out);
    void restart(Frame::ptr frame);

    void fillDroppedFrames(int size);

    QString write_video_frame(AVFormatContext *format_context, OutputStream *output_stream, const int64_t &pts);

    FFMpegContext *context;
    FFFormatConverterMt *format_converter_ff;

    FFEncStartSync *start_sync;

    QSize last_frame_size;

    QString last_error_string;

    QString encoding_tool;

signals:
    void stats(FFEncoder::Stats s);
    void stateChanged(bool state);
    void errorString(QString err_string);

    void restartReq();
};

Q_DECLARE_METATYPE(FFEncoder::Config)
Q_DECLARE_METATYPE(FFEncoder::Stats)

#endif // FF_ENCODER_H
