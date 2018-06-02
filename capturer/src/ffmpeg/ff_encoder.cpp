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

#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QJsonDocument>

#include <iostream>

#include "ff_tools.h"
#include "ff_format_converter_multithreaded.h"
#include "decklink_frame_converter.h"

#include "ff_encoder.h"

class OutputStream
{
public:
    OutputStream() {
        av_stream=nullptr;
        av_codec_context=nullptr;

        next_pts=0;

        frame=nullptr;
        frame_converted=nullptr;

        pkt=nullptr;

        convert_context=nullptr;
    }

    AVStream *av_stream;
    AVCodecContext *av_codec_context;

    int64_t next_pts;

    QByteArray ba_audio_prev_part;

    AVPixelFormat frame_fmt;

    AVFrame *frame;
    AVFrame *frame_converted;

    AVPacket *pkt;

    SwsContext *convert_context;

    uint64_t size_total;
};

class FFMpegContext
{
public:
    FFMpegContext() {
        av_output_format=nullptr;
        av_format_context=nullptr;

        av_codec_audio=nullptr;
        av_codec_video=nullptr;

        opt=nullptr;

        skip_frame=false;

        last_stats_update_time=0;
    }

    bool canAcceptFrame() {
        if(av_format_context)
            return true;

        return false;
    }

    QString filename;

    int64_t in_start_pts;

    OutputStream out_stream_video;
    OutputStream out_stream_audio;

    AVOutputFormat *av_output_format;
    AVFormatContext *av_format_context;

    AVCodec *av_codec_audio;
    AVCodec *av_codec_video;

    AVDictionary *opt;

    FFEncoder::Config cfg;

    bool skip_frame;

    qint64 last_stats_update_time;

    FFEncoderBaseFilename *base_filename;
    FFEncoder::Mode::T mode;
};

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    // rescale output packet timestamp values from codec to stream timebase
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index=st->index;

    // write the compressed frame to the media file
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

static QString add_stream_audio(OutputStream *out_stream, AVFormatContext *format_context, AVCodec **codec, const FFEncoder::Config &cfg)
{
    AVCodecID codec_id=AV_CODEC_ID_PCM_S16LE;

    // find the encoder
    if(cfg.audio_sample_size!=16)
        codec_id=AV_CODEC_ID_PCM_S32LE;

    (*codec)=avcodec_find_encoder(codec_id);


    if(!(*codec))
        return QStringLiteral("could not find encoder for ") + avcodec_get_name(codec_id);


    out_stream->av_stream=avformat_new_stream(format_context, nullptr);

    if(!out_stream->av_stream)
        return QStringLiteral("could not allocate stream");


    out_stream->av_stream->id=format_context->nb_streams - 1;

    out_stream->av_codec_context=avcodec_alloc_context3(*codec);

    if(!out_stream->av_codec_context)
        return QStringLiteral("could not alloc an encoding context");


    out_stream->av_codec_context->sample_fmt=(*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

    out_stream->av_codec_context->sample_rate=48000;

    switch(cfg.audio_channels_size) {
    case 6:
    case 8:
        out_stream->av_codec_context->channel_layout=AV_CH_LAYOUT_7POINT1;
        // c->channel_layout=AV_CH_LAYOUT_7POINT1_WIDE_BACK;
        break;

    case 2:
    default:
        out_stream->av_codec_context->channel_layout=AV_CH_LAYOUT_STEREO;
        break;
    }

    out_stream->av_codec_context->channels=av_get_channel_layout_nb_channels(out_stream->av_codec_context->channel_layout);


    out_stream->av_stream->time_base={ 1, out_stream->av_codec_context->sample_rate };


    // some formats want stream headers to be separate
    if(format_context->oformat->flags & AVFMT_GLOBALHEADER)
        out_stream->av_codec_context->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    return QStringLiteral("");
}

static QString add_stream_video(OutputStream *out_stream, AVFormatContext *format_context, AVCodec **codec, const FFEncoder::Config &cfg)
{
    AVCodecContext *c;

    switch(cfg.video_encoder) {
    case FFEncoder::VideoEncoder::libx264:
        *codec=avcodec_find_encoder_by_name("libx264");
        break;

    case FFEncoder::VideoEncoder::libx264rgb:
        *codec=avcodec_find_encoder_by_name("libx264rgb");
        break;

    case FFEncoder::VideoEncoder::nvenc_h264:
        *codec=avcodec_find_encoder_by_name("h264_nvenc");
        break;

    case FFEncoder::VideoEncoder::nvenc_hevc:
        *codec=avcodec_find_encoder_by_name("hevc_nvenc");
        break;

    case FFEncoder::VideoEncoder::qsv_h264:
        *codec=avcodec_find_encoder_by_name("h264_qsv");
        break;

    case FFEncoder::VideoEncoder::ffvhuff:
        *codec=avcodec_find_encoder_by_name("ffvhuff");
        break;

    default:
        break;
    }

    if(!(*codec))
        return QStringLiteral("could not find encoder");


    out_stream->av_stream=avformat_new_stream(format_context, nullptr);

    if(!out_stream->av_stream)
        return QStringLiteral("could not allocate stream");


    out_stream->av_stream->id=format_context->nb_streams - 1;

    c=avcodec_alloc_context3(*codec);

    if(!c)
        return QStringLiteral("could not allocate an encoding context");

    out_stream->av_codec_context=c;

    c->codec_id=(*codec)->id;

    c->width=cfg.frame_resolution_dst.width();
    c->height=cfg.frame_resolution_dst.height();


    switch(cfg.framerate) {
    case FFEncoder::Framerate::unknown:
        out_stream->av_stream->time_base=cfg.framerate_force;
        break;

    default:
        out_stream->av_stream->time_base=FFEncoder::Framerate::toRational(cfg.framerate);
        break;
    }


    c->time_base=out_stream->av_stream->time_base;

    c->pix_fmt=cfg.pixel_format;

    if(cfg.video_encoder==FFEncoder::VideoEncoder::libx264 || cfg.video_encoder==FFEncoder::VideoEncoder::libx264rgb) {
        av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        av_opt_set(c->priv_data, "crf", QString::number(cfg.crf).toLatin1().constData(), 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_h264) {
        if(cfg.nvenc.enabled) {
            av_opt_set(c->priv_data, "rc", "constqp", 0);
            av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

            if(cfg.nvenc.device!=0)
                av_opt_set(c->priv_data, "gpu", QString::number(cfg.nvenc.device - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.weighted_pred==0)
                c->max_b_frames=cfg.nvenc.b_frames;

            c->refs=cfg.nvenc.ref_frames;

            c->gop_size=cfg.nvenc.gop_size;

            if(cfg.nvenc.qp_i==0) {
                av_opt_set(c->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);

            } else {
                av_opt_set(c->priv_data, "init_qpI", QString::number(cfg.nvenc.qp_i - 1).toLatin1().constData(), 0);
                av_opt_set(c->priv_data, "init_qpP", QString::number(cfg.nvenc.qp_p - 1).toLatin1().constData(), 0);
                av_opt_set(c->priv_data, "init_qpB", QString::number(cfg.nvenc.qp_b - 1).toLatin1().constData(), 0);
            }

            switch(cfg.nvenc.aq_mode) {
            case 1:
                av_opt_set(c->priv_data, "spatial-aq", "1", 0);
                av_opt_set(c->priv_data, "aq-strength", QString::number(cfg.nvenc.aq_strength).toLatin1().constData(), 0);
                break;

            case 2:
                av_opt_set(c->priv_data, "temporal-aq", "1", 0);
                av_opt_set(c->priv_data, "aq-strength", QString::number(cfg.nvenc.aq_strength).toLatin1().constData(), 0);
                break;

            case 0:
            default:
                break;
            }

            if(cfg.nvenc.rc_lookahead>0)
                av_opt_set(c->priv_data, "rc-lookahead", QString::number(cfg.nvenc.rc_lookahead - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.surfaces>0)
                av_opt_set(c->priv_data, "surfaces", QString::number(cfg.nvenc.surfaces - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.no_scenecut!=0)
                av_opt_set(c->priv_data, "no-scenecut", "1", 0);

            if(cfg.nvenc.forced_idr!=0)
                av_opt_set(c->priv_data, "forced-idr", "1", 0);

            if(cfg.nvenc.b_adapt!=0)
                av_opt_set(c->priv_data, "b_adapt", "1", 0);

            if(cfg.nvenc.nonref_p!=0)
                av_opt_set(c->priv_data, "nonref_p", "1", 0);

            if(cfg.nvenc.strict_gop!=0)
                av_opt_set(c->priv_data, "strict_gop", "1", 0);

            if(cfg.nvenc.weighted_pred!=0)
                av_opt_set(c->priv_data, "weighted_pred", "1", 0);

            if(cfg.nvenc.bluray_compat!=0)
                av_opt_set(c->priv_data, "bluray-compat", "1", 0);

        } else {
            av_opt_set(c->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
            av_opt_set(c->priv_data, "rc", "constqp", 0);
            av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_hevc) {
        if(cfg.nvenc.enabled) {
            av_opt_set(c->priv_data, "rc", "constqp", 0);
            av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

            if(cfg.nvenc.device!=0)
                av_opt_set(c->priv_data, "gpu", QString::number(cfg.nvenc.device - 1).toLatin1().constData(), 0);

            c->refs=cfg.nvenc.ref_frames;

            c->gop_size=cfg.nvenc.gop_size;

            if(cfg.nvenc.qp_i==0) {
                av_opt_set(c->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);

            } else {
                av_opt_set(c->priv_data, "init_qpI", QString::number(cfg.nvenc.qp_i - 1).toLatin1().constData(), 0);
                av_opt_set(c->priv_data, "init_qpP", QString::number(cfg.nvenc.qp_p - 1).toLatin1().constData(), 0);
            }

            switch(cfg.nvenc.aq_mode) {
            case 1:
                av_opt_set(c->priv_data, "spatial_aq", "1", 0);
                av_opt_set(c->priv_data, "aq-strength", QString::number(cfg.nvenc.aq_strength).toLatin1().constData(), 0);
                break;

            case 0:
            case 2:
            default:
                break;
            }

            if(cfg.nvenc.rc_lookahead>0)
                av_opt_set(c->priv_data, "rc-lookahead", QString::number(cfg.nvenc.rc_lookahead - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.surfaces>0)
                av_opt_set(c->priv_data, "surfaces", QString::number(cfg.nvenc.surfaces - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.no_scenecut!=0)
                av_opt_set(c->priv_data, "no-scenecut", "1", 0);

            if(cfg.nvenc.forced_idr!=0)
                av_opt_set(c->priv_data, "forced-idr", "1", 0);

            if(cfg.nvenc.nonref_p!=0)
                av_opt_set(c->priv_data, "nonref_p", "1", 0);

            if(cfg.nvenc.strict_gop!=0)
                av_opt_set(c->priv_data, "strict_gop", "1", 0);

            if(cfg.nvenc.weighted_pred!=0)
                av_opt_set(c->priv_data, "weighted_pred", "1", 0);

            if(cfg.nvenc.bluray_compat!=0)
                av_opt_set(c->priv_data, "bluray-compat", "1", 0);

        } else {
            av_opt_set(c->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
            av_opt_set(c->priv_data, "rc", "constqp", 0);
            av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::qsv_h264) {
        c->flags|=AV_CODEC_FLAG_QSCALE;
        c->global_quality=FF_QP2LAMBDA*cfg.crf;

        av_opt_set(c->priv_data, "look_ahead", "0", 0);
        av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::ffvhuff) {
        c->thread_count=QThread::idealThreadCount() - 1;

        if(c->thread_count<=0)
            c->thread_count=1;
    }

    // some formats want stream headers to be separate
    if(format_context->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    return QStringLiteral("");
}

// audio output
static QString alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples, AVFrame **frame)
{
    (*frame)=av_frame_alloc();

    int ret;

    if(!(*frame))
        return QStringLiteral("error allocating an audio frame");

    (*frame)->format=sample_fmt;
    (*frame)->channel_layout=channel_layout;
    (*frame)->sample_rate=sample_rate;
    (*frame)->nb_samples=nb_samples;

    if(nb_samples) {
        ret=av_frame_get_buffer(*frame, 0);

        if(ret<0) {
            av_frame_free(&(*frame));
            (*frame)=nullptr;
            return QStringLiteral("error allocating an audio buffer");
        }
    }

    return QStringLiteral("");
}

static QString open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    Q_UNUSED(oc)

    AVCodecContext *c;

    int nb_samples;
    int ret;

    AVDictionary *opt=nullptr;

    c=ost->av_codec_context;

    // open it
    av_dict_copy(&opt, opt_arg, 0);

    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0)
        return QStringLiteral("could not open audio codec: ") + ffErrorString(ret);

    nb_samples=10000;

    QString err=alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples, &ost->frame);

    if(!err.isEmpty())
        return err;

    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0)
        return QStringLiteral("could not copy the stream parameters");

   if(!ost->pkt)
        ost->pkt=av_packet_alloc();

    return QStringLiteral("");
}

static QString write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    ret=avcodec_send_frame(ost->av_codec_context, ost->frame);

    if(ret<0)
        return QStringLiteral("error encoding audio frame: ") + ffErrorString(ret);

    while(!ret) {
        ret=avcodec_receive_packet(ost->av_codec_context, ost->pkt);

        if(!ret) {
            ost->size_total+=ost->pkt->size;

            write_frame(oc, &ost->av_codec_context->time_base, ost->av_stream, ost->pkt);
        }
    }

    return QStringLiteral("");
}

QString open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg, FFEncoder::Config cfg)
{
    Q_UNUSED(oc)

    int ret;

    AVCodecContext *c=ost->av_codec_context;
    AVDictionary *opt=nullptr;

    av_dict_copy(&opt, opt_arg, 0);

    // av_dict_set(&ost->av_stream->metadata, "framerate", QString("%1/%2")
    //             .arg(ost->av_codec_context->time_base.den)
    //             .arg(ost->av_codec_context->time_base.num).toLatin1().constData(), 0);

    // open the codec
    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        return QStringLiteral("could not open video codec: ") + ffErrorString(ret);
    }


    // allocate and init a re-usable frame
    ost->frame=alloc_frame(ost->frame_fmt, cfg.frame_resolution_src.width(), cfg.frame_resolution_src.height());

    if(!ost->frame)
        return QStringLiteral("could not allocate video frame");


    // allocate and init a re-usable frame
    ost->frame_converted=alloc_frame(cfg.pixel_format, cfg.frame_resolution_dst.width(), cfg.frame_resolution_dst.height());

    if(!ost->frame_converted)
        return QStringLiteral("Could not allocate video frame");


    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0)
        return QStringLiteral("could not copy the stream parameters");

    if(!ost->pkt)
        ost->pkt=av_packet_alloc();

    return QStringLiteral("");
}

static QString write_video_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;
    int ret_2;

    ret=avcodec_send_frame(ost->av_codec_context, ost->frame_converted);

    if(ret<0) {
        qCritical() << "write_video_frame err" << ret << ffErrorString(ret);
        return QStringLiteral("error encoding video frame: ") + ffErrorString(ret);
    }


    while(!ret) {
        ret=avcodec_receive_packet(ost->av_codec_context, ost->pkt);

        if(!ret) {
            ost->size_total+=ost->pkt->size;

            ret_2=write_frame(oc, &ost->av_codec_context->time_base, ost->av_stream, ost->pkt);

            if(ret_2!=0) {
                qCritical() << "write_video_frame err2";
                return ffErrorString(ret_2);
            }
        }
    }

    return QStringLiteral("");
}

static void close_stream(OutputStream *ost)
{
    if(ost->av_codec_context) {
        avcodec_free_context(&ost->av_codec_context);
        ost->av_codec_context=nullptr;
    }

    if(ost->frame) {
        av_frame_free(&ost->frame);
        ost->frame=nullptr;
    }

    if(ost->frame_converted) {
        av_frame_free(&ost->frame_converted);
        ost->frame_converted=nullptr;
    }

    if(ost->convert_context) {
        sws_freeContext(ost->convert_context);
        ost->convert_context=nullptr;
    }
}

// ------------------------------

FFEncoder::FFEncoder(FFEncoder::Mode::T mode, QObject *parent) :
    QObject(parent)
{
    context=new FFMpegContext();

    int thread_count=QThread::idealThreadCount()/2;
    // thread_count=QThread::idealThreadCount();

    if(thread_count<1)
        thread_count=2;

    if(thread_count>4)
        thread_count=4;

    format_converter_ff=new FFFormatConverterMt(thread_count);
    // format_converter_ff->useMultithreading(false);
    format_converter_ff->useMultithreading(true);

    // connect(format_converter_ff, SIGNAL(frameSkipped()), SLOT(converterFrameSkip()), Qt::QueuedConnection);

    context->base_filename=nullptr;
    context->mode=mode;
}

FFEncoder::~FFEncoder()
{
    stopCoder();

    delete context;
    delete format_converter_ff;
}

FFEncoder::Framerate::T FFEncoder::calcFps(int64_t frame_duration, int64_t frame_scale, bool half_fps)
{
    if(half_fps) {
        switch(frame_scale) {
        case 12000:
            return frame_duration==1000 ? Framerate::full_12 : Framerate::full_12;

        case 12500:
            return Framerate::full_12_5;

        case 15000:
            return frame_duration==1000 ? Framerate::full_15 : Framerate::full_14;

        case 24000:
            return frame_duration==1000 ? Framerate::full_24 : Framerate::full_23;

        case 25000:
            return Framerate::full_25;

        case 30000:
            return frame_duration==1000 ? Framerate::full_30 : Framerate::full_29;

        case 50:
        case 50000:
            return Framerate::half_50;

        case 60:
            return Framerate::half_60;

        case 4975:
            return Framerate::half_59;

        case 60000:
            return frame_duration==1000 ? Framerate::half_60: Framerate::half_59;
        }

    } else {
        switch(frame_scale) {
        case 12000:
            return frame_duration==1000 ? Framerate::full_12 : Framerate::full_11;

        case 12500:
            return Framerate::full_12_5;

        case 15000:
            return frame_duration==1000 ? Framerate::full_15 : Framerate::full_14;

        case 24000:
            return frame_duration==1000 ? Framerate::full_24 : Framerate::full_23;

        case 25000:
            return Framerate::full_25;

        case 30000:
            return frame_duration==1000 ? Framerate::full_30 : Framerate::full_29;

        case 50:
        case 50000:
            return Framerate::full_50;

        case 60:
            return Framerate::full_60;

        case 4975:
            return Framerate::half_59;

        case 60000:
            return frame_duration==1000 ? Framerate::full_60: Framerate::full_59;
        }
    }

    return Framerate::unknown;
}

QString FFEncoder::presetVisualNameToParamName(const QString &str)
{
    if(str==QLatin1String("high quality"))
        return QLatin1String("hq");

    if(str==QLatin1String("high performance"))
        return QLatin1String("hp");

    if(str==QLatin1String("bluray disk"))
        return QLatin1String("bd");

    if(str==QLatin1String("low latency"))
        return QLatin1String("ll");

    if(str==QLatin1String("low latency high quality"))
        return QLatin1String("llhq");

    if(str==QLatin1String("low latency high performance"))
        return QLatin1String("llhp");

    return str;
}

QStringList FFEncoder::compatiblePresets(FFEncoder::VideoEncoder::T encoder)
{
    switch(encoder) {
    case VideoEncoder::libx264:
    case VideoEncoder::libx264rgb:
        return QStringList() << QLatin1String("ultrafast") << QLatin1String("superfast") << QLatin1String("veryfast") << QLatin1String("faster")
                             << QLatin1String("fast") << QLatin1String("medium") << QLatin1String("slow");

    case VideoEncoder::nvenc_h264:
    case VideoEncoder::nvenc_hevc:
        return QStringList() << QLatin1String("high quality") << QLatin1String("high performance") << QLatin1String("bluray disk") << QLatin1String("low latency")
                             << QLatin1String("low latency high quality") << QLatin1String("low latency high performance")
                             << QLatin1String("slow") << QLatin1String("medium") << QLatin1String("fast") << QLatin1String("default")
                             << QLatin1String("lossless");

    case VideoEncoder::qsv_h264:
        return QStringList() << QLatin1String("veryfast") << QLatin1String("faster") << QLatin1String("fast")
                             << QLatin1String("medium")
                             << QLatin1String("slow") << QLatin1String("slower") << QLatin1String("veryslow");

    default: return QStringList() << QLatin1String("--");
    }

    return QStringList() << QLatin1String("--");
}

void FFEncoder::setEncodingToolName(const QString &encoding_tool)
{
    this->encoding_tool=encoding_tool;
}

void FFEncoder::setBaseFilename(FFEncoderBaseFilename *bf)
{
    context->base_filename=bf;
}

QString FFEncoder::configString(const FFEncoder::Config &cfg)
{
    QVariantMap map;

    if(cfg.rgb_source) {
        if(cfg.depth_10bit)
            map.insert("src_pix_fmt", PixelFormat::toString(PixelFormat::GBRP10LE));

        else
            map.insert("src_pix_fmt", PixelFormat::toString(PixelFormat::BGRA));

    } else {
        if(cfg.depth_10bit)
            map.insert("src_pix_fmt", PixelFormat::toString(PixelFormat::YUV422P10LE));

        else
            map.insert("src_pix_fmt", PixelFormat::toString(PixelFormat::UYVY422));
    }

    map.insert("dst_pix_fmt", PixelFormat::toString(cfg.pixel_format));


    if(cfg.frame_resolution_src==cfg.frame_resolution_dst) {
        map.insert("resolution", QString("%1x%2").arg(cfg.frame_resolution_src.width()).arg(cfg.frame_resolution_src.height()));

    } else {
        map.insert("src_resolution", QString("%1x%2").arg(cfg.frame_resolution_src.width()).arg(cfg.frame_resolution_src.height()));
        map.insert("dst_resolution", QString("%1x%2").arg(cfg.frame_resolution_dst.width()).arg(cfg.frame_resolution_dst.height()));
        map.insert("scale_filter", ScaleFilter::toString(cfg.scale_filter));
    }

    map.insert("crf", cfg.crf);
    map.insert("video_encoder", VideoEncoder::toString(cfg.video_encoder));
    map.insert("preset", cfg.preset);


    AVRational fr=Framerate::toRational(cfg.framerate);

    if(cfg.framerate==Framerate::unknown)
        fr=cfg.framerate_force;

    map.insert("framerate", QString("%1/%2").arg(fr.den).arg(fr.num));


    if((cfg.video_encoder==VideoEncoder::nvenc_h264 || cfg.video_encoder==VideoEncoder::nvenc_hevc)
            && cfg.nvenc.enabled) {
        switch(cfg.nvenc.aq_mode) {
        case 1:
            map.insert("aq_mode", "spatial");
            map.insert("aq_strength", cfg.nvenc.aq_strength);
            break;

        case 2:
            map.insert("aq_mode", "temporal");
            map.insert("aq_strength", cfg.nvenc.aq_strength);
            break;

        default:
            break;
        }

        map.remove("crf");

        if(cfg.video_encoder==VideoEncoder::nvenc_h264)
            map.insert("b_frames", cfg.nvenc.b_frames);

        map.insert("ref_frames", cfg.nvenc.ref_frames);
        map.insert("gop_size", cfg.nvenc.gop_size);

        if(cfg.nvenc.rc_lookahead>0)
            map.insert("rc_lookahead", cfg.nvenc.rc_lookahead - 1);

        if(cfg.nvenc.surfaces>0)
            map.insert("surfaces", cfg.nvenc.surfaces - 1);

        if(cfg.nvenc.qp_i>0) {
            map.insert("qp_i", cfg.nvenc.qp_i - 1);
            map.insert("qp_p", cfg.nvenc.qp_p - 1);

            if(cfg.video_encoder==VideoEncoder::nvenc_h264)
                map.insert("qp_b", cfg.nvenc.qp_b - 1);

        } else {
            map.insert("qp", cfg.crf);
        }
    }

    return QString(QJsonDocument::fromVariant(map).toJson(QJsonDocument::Compact)).remove("{").remove("}").remove("\"").replace(":", "=").replace(",", ", ");
}

bool FFEncoder::setConfig(FFEncoder::Config cfg)
{
    if(context->mode==FFEncoder::Mode::webcam)
        cfg.audio_channels_size=2;

    last_error_string.clear();

    int ret;
    int sws_flags=0;


    if(cfg.rgb_source)
        context->out_stream_video.frame_fmt=AV_PIX_FMT_BGRA;

    else
        context->out_stream_video.frame_fmt=AV_PIX_FMT_UYVY422;


    cfg.frame_resolution_dst=cfg.frame_resolution_src;

    if(cfg.downscale!=DownScale::Disabled) {
        if(cfg.frame_resolution_dst.height()>DownScale::toWidth(cfg.downscale)) {
            cfg.frame_resolution_dst.setHeight(DownScale::toWidth(cfg.downscale));
            cfg.frame_resolution_dst.setWidth(cfg.frame_resolution_dst.height()*(16./9.));
            sws_flags|=ScaleFilter::toSws(cfg.scale_filter);
        }
    }

    if(cfg.video_encoder==VideoEncoder::nvenc_h264) {
        if(cfg.nvenc.ref_frames>5)
            cfg.nvenc.ref_frames=5;
    }

    if(cfg.video_encoder==VideoEncoder::nvenc_hevc) {
        if(cfg.nvenc.aq_mode==2)
            cfg.nvenc.aq_mode=0;
    }

    if(cfg.depth_10bit) {
        bool ret;

        if(cfg.rgb_source)
            ret=format_converter_ff->setup(DecodeFrom210::r210PixelFormat(), cfg.frame_resolution_src, cfg.pixel_format, cfg.frame_resolution_dst,
                                           cfg.downscale==DownScale::Disabled ? FFFormatConverter::Filter::cNull : (FFFormatConverter::Filter::T)ScaleFilter::toSws(cfg.scale_filter),
                                           DecodeFrom210::Format::R210);

        else
            ret=format_converter_ff->setup(DecodeFrom210::v210PixelFormat(), cfg.frame_resolution_src, cfg.pixel_format, cfg.frame_resolution_dst,
                                           cfg.downscale==DownScale::Disabled ? FFFormatConverter::Filter::cNull : (FFFormatConverter::Filter::T)ScaleFilter::toSws(cfg.scale_filter),
                                           DecodeFrom210::Format::V210);

        if(!ret) {
            emit errorString(last_error_string=QStringLiteral("err init format converter"));
            goto fail;
        }

    } else {
        if(!format_converter_ff->setup(context->out_stream_video.frame_fmt, cfg.frame_resolution_src, cfg.pixel_format, cfg.frame_resolution_dst,
                                       cfg.downscale==DownScale::Disabled ? FFFormatConverter::Filter::cNull : (FFFormatConverter::Filter::T)ScaleFilter::toSws(cfg.scale_filter),
                                       DecodeFrom210::Format::Disabled)) {
            emit errorString(last_error_string=QStringLiteral("err init format converter"));
            goto fail;
        }
    }


    format_converter_ff->resetQueues();


    {
        QDir dir(QApplication::applicationDirPath() + "/videos");

        if(!dir.exists())
            dir.mkdir(dir.dirName());
    }

    {
        QString name;

        if(!context->base_filename->isEmpty())
            name=(*context->base_filename);

        else
            name=QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");


        if(context->mode==Mode::webcam)
            name+=QLatin1String("_cam");


        context->filename=QString(QLatin1String("%1/videos/%2.mkv"))
                .arg(QApplication::applicationDirPath()).arg(name);
    }

    // context->filename="/dev/null";


    // allocate the output media context
    avformat_alloc_output_context2(&context->av_format_context, nullptr, "matroska", nullptr);

    if(!context->av_format_context) {
        emit errorString(last_error_string=QStringLiteral("could not deduce output format"));
        goto fail;
    }

    context->av_output_format=context->av_format_context->oformat;


    // add the audio and video streams using the default format codecs
    // and initialize the codecs
    last_error_string=add_stream_video(&context->out_stream_video, context->av_format_context, &context->av_codec_video, cfg);

    if(!last_error_string.isEmpty()) {
        emit errorString(last_error_string);
        goto fail;
    }


    last_error_string=add_stream_audio(&context->out_stream_audio, context->av_format_context, &context->av_codec_audio, cfg);

    if(!last_error_string.isEmpty()) {
        emit errorString(last_error_string);
        goto fail;
    }

    // now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers
    last_error_string=open_video(context->av_format_context, context->av_codec_video, &context->out_stream_video, context->opt, cfg);

    if(!last_error_string.isEmpty()) {
        emit errorString(last_error_string);
        goto fail;
    }


    last_error_string=open_audio(context->av_format_context, context->av_codec_audio, &context->out_stream_audio, context->opt);

    if(!last_error_string.isEmpty()) {
        emit errorString(last_error_string);
        goto fail;
    }


    context->out_stream_video.convert_context=sws_getContext(cfg.frame_resolution_src.width(), cfg.frame_resolution_src.height(),
                                                             context->out_stream_video.frame_fmt,
                                                             cfg.frame_resolution_dst.width(), cfg.frame_resolution_dst.height(),
                                                             cfg.pixel_format,
                                                             sws_flags, nullptr, nullptr, nullptr);


    av_dump_format(context->av_format_context, 0, "", 1);


    // open the output file
    ret=avio_open(&context->av_format_context->pb, context->filename.toLatin1().constData(), AVIO_FLAG_WRITE);

    if(ret<0) {
        emit errorString(last_error_string=QString(QStringLiteral("could not open %1: %2")).arg(context->filename).arg(ffErrorString(ret)));
        goto fail;
    }


    av_dict_set(&context->av_format_context->metadata, "encoding_params", configString(cfg).toLatin1().constData(), 0);

    if(!encoding_tool.isEmpty())
        av_dict_set(&context->av_format_context->metadata, "encoding_tool", encoding_tool.toLatin1().constData(), 0);


    // write the stream header, if any
    ret=avformat_write_header(context->av_format_context, &context->opt);

    if(ret<0) {
        emit errorString(last_error_string=QStringLiteral("error occurred when opening output file: ") + ffErrorString(ret));
        goto fail;
    }


    context->cfg=cfg;

    context->skip_frame=false;

    context->out_stream_audio.next_pts=0;

    context->out_stream_video.next_pts=0;

    context->out_stream_audio.size_total=0;

    context->out_stream_video.size_total=0;

    context->in_start_pts=AV_NOPTS_VALUE;


    if(cfg.audio_dalay!=0)
        context->out_stream_audio.next_pts=cfg.audio_dalay/1000.*context->out_stream_audio.av_codec_context->sample_rate;


    emit stateChanged(true);

    return true;

fail:

    stopCoder();

    return false;
}

bool FFEncoder::appendFrame(Frame::ptr frame)
{
    if(!context->canAcceptFrame())
        return false;


    processAudio(frame);


    if(frame->video.data_ptr==nullptr) {
        context->out_stream_video.next_pts++;
        return true;
    }

    switch(context->cfg.framerate) {
    case Framerate::half_50:
    case Framerate::half_59:
    case Framerate::half_60:
        context->skip_frame=!context->skip_frame;
        break;

    default:
        break;
    }

    if(!context->skip_frame) {
        format_converter_ff->convert(frame);
    }

    AVFrameSP::ptr frame_out;

    while(frame_out=format_converter_ff->result()) {
        if(frame_out->d->pts==AV_NOPTS_VALUE) {
            frame_out->d->pts=context->out_stream_video.next_pts++;

        } else {
            if(context->in_start_pts==AV_NOPTS_VALUE)
                context->in_start_pts=frame_out->d->pts;

            frame_out->d->pts=
                    context->out_stream_video.next_pts=
                    av_rescale_q(frame_out->d->pts - context->in_start_pts,
                                 frame_out->time_base,
                                 context->out_stream_video.av_codec_context->time_base);
        }

        AVFrame *frame_orig=context->out_stream_video.frame_converted;

        context->out_stream_video.frame_converted=frame_out->d;

        last_error_string=write_video_frame(context->av_format_context, &context->out_stream_video);
        context->out_stream_video.frame_converted=frame_orig;

        if(!last_error_string.isEmpty()) {
            stopCoder();
            emit errorString("write_video_frame error: " + last_error_string);
            return false;
        }
    }

    if(QDateTime::currentMSecsSinceEpoch() - context->last_stats_update_time>1000) {
        context->last_stats_update_time=QDateTime::currentMSecsSinceEpoch();

        calcStats();
    }

    return true;
}

void FFEncoder::processAudio(Frame::ptr frame)
{
    if(frame->audio.data_size) {
        QByteArray ba_audio=QByteArray((char*)frame->audio.data_ptr, frame->audio.data_size);

        ba_audio.insert(0, context->out_stream_audio.ba_audio_prev_part);

        AVSampleFormat sample_format=context->cfg.audio_sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32;

        int buffer_size=0;

        int default_nb_samples=context->out_stream_audio.frame->nb_samples;

        while(true) {
            buffer_size=av_samples_get_buffer_size(nullptr, context->out_stream_audio.frame->channels, context->out_stream_audio.frame->nb_samples,
                                                   sample_format, 0);

            if(ba_audio.size()>=buffer_size) {
                break;
            }

            context->out_stream_audio.frame->nb_samples--;
        }

        QByteArray ba_audio_tmp=ba_audio.left(buffer_size);

        context->out_stream_audio.ba_audio_prev_part=ba_audio.remove(0, buffer_size);

        int ret=avcodec_fill_audio_frame(context->out_stream_audio.frame, context->out_stream_audio.frame->channels, sample_format,
                                         (const uint8_t*)ba_audio_tmp.constData(), buffer_size, 0);

        if(ret<0) {
            stopCoder();
            emit errorString(last_error_string=QStringLiteral("avcodec_fill_audio_frame error: could not setup audio frame"));
            return;
        }

        context->out_stream_audio.frame->pts=context->out_stream_audio.next_pts;

        context->out_stream_audio.next_pts+=context->out_stream_audio.frame->nb_samples;

        write_audio_frame(context->av_format_context, &context->out_stream_audio);

        context->out_stream_audio.frame->nb_samples=default_nb_samples;
    }
}

bool FFEncoder::stopCoder()
{
    if(!context->canAcceptFrame())
        return false;

    if(context->av_format_context->pb)
        av_write_trailer(context->av_format_context);

    // close each codec.
    close_stream(&context->out_stream_video);
    close_stream(&context->out_stream_audio);

    // close the output file.
    if(context->av_format_context->pb)
        avio_closep(&context->av_format_context->pb);

    // free the stream
    if(context->av_format_context) {
        avformat_free_context(context->av_format_context);
        context->av_format_context=nullptr;
    }

    emit stateChanged(false);

    return true;
}

void FFEncoder::converterFrameSkip()
{
    // context->out_stream_video.next_pts++;
}

void FFEncoder::calcStats()
{
    double cf_a=av_stream_get_end_pts(context->out_stream_audio.av_stream) * av_q2d(context->out_stream_audio.av_stream->time_base);

    if(cf_a<.01)
        cf_a=.01;


    double cf_v=av_stream_get_end_pts(context->out_stream_video.av_stream) * av_q2d(context->out_stream_video.av_stream->time_base);

    if(cf_v<.01)
        cf_v=.01;


    Stats s;

    s.avg_bitrate_audio=(double)(context->out_stream_audio.size_total*8)/cf_a;
    s.avg_bitrate_video=(double)(context->out_stream_video.size_total*8)/cf_v;

    s.time=QTime(0, 0).addMSecs((double)context->out_stream_audio.frame->pts/(double)context->out_stream_audio.av_codec_context->sample_rate*1000);

    s.streams_size=context->out_stream_audio.size_total + context->out_stream_video.size_total;

    emit stats(s);
}

//

QString FFEncoder::PixelFormat::toString(uint32_t value)
{
    switch(value) {
    case RGB24:
        return QLatin1String("rgb24");

    case BGR0:
        return QLatin1String("bgr0");

    case RGB0:
        return QLatin1String("rgb0");

    case BGRA:
        return QLatin1String("bgra");

    case GBRP10LE:
        return QLatin1String("gbrp10le");

    case YUV420P:
        return QLatin1String("yuv420p");

    case YUV420P10:
        return QLatin1String("yuv420p10");

    case YUV422P:
        return QLatin1String("yuv422p");

    case UYVY422:
        return QLatin1String("uyvy422p");

    case YUV422P10LE:
        return QLatin1String("yuv422p10le (v210)");

    case YUV444P:
        return QLatin1String("yuv444p");

    case YUV444P10:
        return QLatin1String("yuv444p10");

    case YUV444P16LE:
        return QLatin1String("yuv444p16le");

    case RGB48LE:
        return QLatin1String("rgb48le (r210)");

    case P010LE:
        return QLatin1String("p010le");

    case NV12:
        return QLatin1String("nv12");
    }

    return QLatin1String("unknown");
}

uint64_t FFEncoder::PixelFormat::fromString(QString value)
{
    if(value==QLatin1String("rgb24"))
        return RGB24;

    else if(value==QLatin1String("bgr0"))
        return BGR0;

    else if(value==QLatin1String("rgb0"))
        return RGB0;

    else if(value==QLatin1String("bgra"))
        return BGRA;

    else if(value==QLatin1String("gbrp10le"))
        return GBRP10LE;

    else if(value==QLatin1String("yuv420p"))
        return YUV420P;

    else if(value==QLatin1String("yuv420p10"))
        return YUV420P10;

    else if(value==QLatin1String("yuv422p"))
        return YUV422P;

    else if(value==QLatin1String("uyvy422p"))
        return UYVY422;

    else if(value==QLatin1String("yuv422p10le (v210)"))
        return YUV422P10LE;

    else if(value==QLatin1String("yuv444p"))
        return YUV444P;

    else if(value==QLatin1String("yuv444p10"))
        return YUV444P10;

    else if(value==QLatin1String("yuv444p16le"))
        return YUV444P16LE;

    else if(value==QLatin1String("rgb48le (r210)"))
        return RGB48LE;

    else if(value==QLatin1String("p010le"))
        return P010LE;

    else if(value==QLatin1String("nv12"))
        return NV12;

    return 0;
}

QList <FFEncoder::PixelFormat::T> FFEncoder::PixelFormat::compatiblePixelFormats(FFEncoder::VideoEncoder::T encoder)
{
    switch(encoder) {
    case VideoEncoder::libx264:
        return QList<T>() << YUV420P << YUV422P << YUV444P;

    // case VideoEncoder::libx264_10bit:
    //     return QList<T>() << YUV420P10 << YUV444P10;

    case VideoEncoder::libx264rgb:
        return QList<T>() << RGB24;

    case VideoEncoder::nvenc_h264:
        return QList<T>() << YUV420P << YUV444P;

    case VideoEncoder::nvenc_hevc:
        return QList<T>() << YUV420P << NV12 << P010LE << YUV444P << YUV444P16LE << BGR0 << RGB0;

    case VideoEncoder::qsv_h264:
        return QList<T>() /*<< P010LE*/ << NV12;

    case VideoEncoder::ffvhuff:
        return QList<T>() << RGB24 << YUV420P << YUV422P << YUV444P << YUV420P10 /*<< YUV422P10*/ << YUV444P10 << YUV422P10LE;
    }

    return QList<T>() << RGB24 << YUV420P << YUV444P << YUV420P10 << YUV444P10;
}

QList <FFEncoder::PixelFormat::T> FFEncoder::PixelFormat::list()
{
    static QList <FFEncoder::PixelFormat::T> res=
            QList <FFEncoder::PixelFormat::T>() << RGB24 << BGR0 << RGB0 << YUV420P << YUV420P10 << YUV422P << UYVY422 << YUV444P
                                                << YUV422P10LE << YUV444P10 << YUV444P16LE << RGB48LE << P010LE << NV12;

    return res;
}

QString FFEncoder::VideoEncoder::toString(uint32_t enc)
{
    switch(enc) {
    case libx264:
        return QLatin1String("libx264");

    case libx264rgb:
        return QLatin1String("libx264rgb");

    case nvenc_h264:
        return QLatin1String("nvenc_h264");

    case nvenc_hevc:
        return QLatin1String("nvenc_hevc");

    case qsv_h264:
        return QLatin1String("qsv_h264");

    case ffvhuff:
        return QLatin1String("ffvhuff");
    }

    return QLatin1String("");
}

QString FFEncoder::VideoEncoder::toEncName(uint32_t enc)
{
    switch(enc) {
    case libx264:
        return QLatin1String("libx264");

    case libx264rgb:
        return QLatin1String("libx264rgb");

    case nvenc_h264:
        return QLatin1String("h264_nvenc");

    case nvenc_hevc:
        return QLatin1String("hevc_nvenc");

    case qsv_h264:
        return QLatin1String("h264_qsv");

    case ffvhuff:
        return QLatin1String("ffvhuff");
    }

    return QLatin1String("");
}

uint64_t FFEncoder::VideoEncoder::fromString(QString value)
{
    if(value==QLatin1String("libx264"))
        return libx264;

    else if(value==QLatin1String("libx264rgb"))
        return libx264rgb;

    else if(value==QLatin1String("nvenc_h264"))
        return nvenc_h264;

    else if(value==QLatin1String("nvenc_hevc"))
        return nvenc_hevc;

    else if(value==QLatin1String("qsv_h264"))
        return qsv_h264;

    else if(value==QLatin1String("ffvhuff"))
        return ffvhuff;

    return 0;
}

QList <FFEncoder::VideoEncoder::T> FFEncoder::VideoEncoder::list()
{
    QList <FFEncoder::VideoEncoder::T> res=
            QList <FFEncoder::VideoEncoder::T>() << libx264 << libx264rgb << nvenc_h264 << nvenc_hevc << qsv_h264 << ffvhuff;

    return res;
}

int FFEncoder::DownScale::toWidth(uint32_t value)
{
    switch(value) {
    case to720:
        return 720;

    case to1080:
        return 1080;

    case to1440:
        return 1440;

    case to1800:
        return 1800;
    }

    return 1080;
}

QString FFEncoder::DownScale::toString(uint32_t value)
{
    switch(value) {
    case to720:
        return QLatin1String("720p");

    case to1080:
        return QLatin1String("1080p");

    case to1440:
        return QLatin1String("1440p");

    case to1800:
        return QLatin1String("1800p");
    }

    return QLatin1String("disabled");
}

int FFEncoder::ScaleFilter::toSws(uint32_t value)
{
    switch(value) {
    case FastBilinear:
        return SWS_FAST_BILINEAR;

    case Bilinear:
        return SWS_BILINEAR;

    case Bicubic:
        return SWS_BICUBIC;

    case X:
        return SWS_X;

    case Point:
        return SWS_POINT;

    case Area:
        return SWS_AREA;

    case Bicublin:
        return SWS_BICUBLIN;

    case Gauss:
        return SWS_GAUSS;

    case Sinc:
        return SWS_SINC;

    case Lanczos:
        return SWS_LANCZOS;

    case Spline:
        return SWS_SPLINE;
    }

    return SWS_FAST_BILINEAR;
}

QString FFEncoder::ScaleFilter::toString(uint32_t value)
{
    switch(value) {
    case FastBilinear:
        return QLatin1String("fast bilinear");

    case Bilinear:
        return QLatin1String("bilinear");

    case Bicubic:
        return QLatin1String("bicubic");

    case X:
        return QLatin1String("x (experimental)");

    case Point:
        return QLatin1String("point (nearest neighbor)");

    case Area:
        return QLatin1String("averaging area");

    case Bicublin:
        return QLatin1String("luma bicubic, chroma bilinear");

    case Gauss:
        return QLatin1String("gaussian");

    case Sinc:
        return QLatin1String("sinc");

    case Lanczos:
        return QLatin1String("lanczos");

    case Spline:
        return QLatin1String("natural bicubic spline");
    }

    return QLatin1String("unknown?!!");
}

AVRational FFEncoder::Framerate::toRational(FFEncoder::Framerate::T value)
{
    switch(value) {
    case full_11:
        return { 1001, 12000 };

    case full_12:
        return { 1000, 12000 };

    case full_12_5:
        return { 1000, 12500 };

    case full_14:
        return { 1001, 15000 };

    case full_15:
        return { 1000, 15000 };

    case full_23:
        return { 1001, 24000 };

    case full_24:
        return { 1000, 24000 };

    case full_25:
    case half_50:
        return { 1000, 25000 };

    case full_29:
    case half_59:
        return { 1001, 30000 };

    case full_30:
    case half_60:
        return { 1000, 30000 };

    case full_50:
        return { 1000, 50000 };

    case full_59:
        return { 1001, 60000 };

    case full_60:
        return { 1000, 60000 };

    case unknown:
    default:
        break;
    }

    return { 1000, 30000 };
}
