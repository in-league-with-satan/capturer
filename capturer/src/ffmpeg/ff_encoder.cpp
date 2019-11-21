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

#include <QDebug>
#include <QThread>
#include <QFile>
#include <QDir>
#include <QJsonDocument>
#include <qcoreapplication.h>

#include <iostream>

#include "audio_buffer.h"
#include "audio_mixer.h"
#include "ff_tools.h"
#include "ff_format_converter_multithreaded.h"
#include "ff_audio_converter.h"
#include "decklink_frame_converter.h"
#include "source_interface.h"

#include "ff_encoder.h"


struct OutputStream
{
    AVStream *av_stream=nullptr;
    AVCodecContext *av_codec_context=nullptr;

    AVBufferRef *hw_device_ctx=nullptr;

    int64_t pts_start=0;
    int64_t pts_next=0;
    int64_t pts_last=AV_NOPTS_VALUE;
    int64_t pts_stats=0;

    QByteArray ba_audio_prev_part;

    AVPixelFormat frame_fmt=AV_PIX_FMT_NONE;

    AVFrame *frame=nullptr;
    AVFrame *frame_converted=nullptr;
    AVFrame *frame_hw=nullptr;

    AVFrameSP::ptr frame_current;

    AVPacket *pkt=nullptr;

    uint64_t size_total=0;
};

struct FFMpegContext
{
    bool canAcceptFrame() {
        if(av_format_context)
            return true;

        return false;
    }

    QString filename;

    AudioConverter audio_converter;
    AudioBuffer audio_buffer;
    AudioMixer audio_mixer;

    OutputStream out_stream_video;
    OutputStream out_stream_audio;

    AVOutputFormat *av_output_format=nullptr;
    AVFormatContext *av_format_context=nullptr;

    AVCodec *av_codec_audio=nullptr;
    AVCodec *av_codec_video=nullptr;

    AVDictionary *opt=nullptr;

    FFEncoder::Config cfg;

    bool skip_frame=false;

    qint64 last_stats_update_time=0;

    QString store_dir;
    FFEncoderBaseFilename *base_filename=nullptr;
    int enc_num=0;

    uint32_t dropped_frames_counter=0;
    uint32_t double_frames_counter=0;

    uint32_t counter_process_events=0;

    uint64_t prev_stream_size_total=0;
    QMap <uint64_t, uint64_t> bitrate_point;
};

static int interleaved_write_frame(AVFormatContext *format_context, AVRational *time_base, AVStream *stream, AVPacket *packet)
{
    packet->duration=0;

    av_packet_rescale_ts(packet, *time_base, stream->time_base);

    packet->stream_index=stream->index;

    return av_interleaved_write_frame(format_context, packet);
}

static QString add_stream_audio(OutputStream *output_stream, AVFormatContext *format_context, AVCodec **codec, AudioConverter *audio_converter, AudioBuffer *audio_buffer, AudioMixer *audio_mixer, const FFEncoder::Config &cfg)
{
    if(cfg.audio_encoder==FFEncoder::AudioEncoder::pcm) {
        AVCodecID codec_id=AV_CODEC_ID_PCM_S16LE;

        if(cfg.audio_sample_size!=16)
            codec_id=AV_CODEC_ID_PCM_S32LE;

        (*codec)=avcodec_find_encoder(codec_id);

    } else {
        (*codec)=avcodec_find_encoder_by_name(FFEncoder::AudioEncoder::toEncName(cfg.audio_encoder).toLatin1().data());
    }

    if(!(*codec)) {
        QString err("could not find encoder for " + FFEncoder::AudioEncoder::toString(cfg.audio_encoder));
        qCritical().noquote() << err;
        return err;
    }

    output_stream->av_stream=avformat_new_stream(format_context, nullptr);

    if(!output_stream->av_stream) {
        QString err("could not allocate stream");
        qCritical().noquote() << err;
        return err;
    }


    output_stream->av_stream->id=format_context->nb_streams - 1;

    output_stream->av_codec_context=avcodec_alloc_context3(*codec);

    if(!output_stream->av_codec_context) {
        QString err("could not alloc an encoding context");
        qCritical().noquote() << err;
        return err;
    }


    if(FFEncoder::AudioEncoder::setBitrate(cfg.audio_encoder))
        output_stream->av_codec_context->bit_rate=cfg.audio_bitrate*1000;

    else
        output_stream->av_codec_context->compression_level=12;


    int source_sample_fmt=-1;
    int source_channel_layout=-1;

    if(cfg.audio_sample_size==16)
        source_sample_fmt=output_stream->av_codec_context->sample_fmt=AV_SAMPLE_FMT_S16;

    else
        source_sample_fmt=output_stream->av_codec_context->sample_fmt=AV_SAMPLE_FMT_S32;


    if(cfg.audio_encoder!=FFEncoder::AudioEncoder::pcm && cfg.audio_encoder!=FFEncoder::AudioEncoder::flac)
        output_stream->av_codec_context->sample_fmt=FFEncoder::AudioEncoder::sampleFmt(cfg.audio_encoder);


    output_stream->av_codec_context->sample_rate=48000;


    switch(cfg.audio_channels_size) {
    case 6:
        source_channel_layout=AV_CH_LAYOUT_5POINT1;
        break;

    case 8:
        source_channel_layout=AV_CH_LAYOUT_7POINT1; // AV_CH_LAYOUT_7POINT1_WIDE_BACK
        break;

    case 2:
    default:
        source_channel_layout=AV_CH_LAYOUT_STEREO;
        break;
    }

    if(cfg.audio_downmix_to_stereo)
        output_stream->av_codec_context->channel_layout=AV_CH_LAYOUT_STEREO;

    else
        output_stream->av_codec_context->channel_layout=source_channel_layout;


    output_stream->av_codec_context->channels=av_get_channel_layout_nb_channels(output_stream->av_codec_context->channel_layout);


    output_stream->av_stream->time_base={ 1, output_stream->av_codec_context->sample_rate };


    if(format_context->oformat->flags&AVFMT_GLOBALHEADER)
        output_stream->av_codec_context->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;


    if(!audio_converter->init(source_channel_layout, output_stream->av_codec_context->sample_rate, source_sample_fmt,
                              output_stream->av_codec_context->channel_layout, output_stream->av_codec_context->sample_rate, output_stream->av_codec_context->sample_fmt)) {
        QString err("audio converter init error");
        qCritical().noquote() << err;
        return err;
    }

    audio_buffer->init(source_sample_fmt, av_get_channel_layout_nb_channels(source_channel_layout));

    audio_mixer->init(48000, AV_SAMPLE_FMT_S16, av_get_channel_layout_nb_channels(source_channel_layout), cfg.active_src_devices);

    return QStringLiteral("");
}

static QString add_stream_video_dsc(OutputStream *output_stream, AVFormatContext *format_context, AVCodec **codec, const FFEncoder::Config &cfg)
{
    if(cfg.pixel_format_src==PixelFormat::mjpeg)
        (*codec)=avcodec_find_encoder(AV_CODEC_ID_MJPEG);

    else if(cfg.pixel_format_src==PixelFormat::h264)
        (*codec)=avcodec_find_encoder(AV_CODEC_ID_H264);

    if(!(*codec)) {
        QString err("could not find encoder");
        qCritical().noquote() << err;
        return err;
    }

    output_stream->av_stream=avformat_new_stream(format_context, nullptr);

    if(!output_stream->av_stream) {
        QString err("could not allocate stream");
        qCritical().noquote() << err;
        return err;
    }


    output_stream->av_stream->id=format_context->nb_streams - 1;


    output_stream->av_codec_context=avcodec_alloc_context3(*codec);

    if(!output_stream->av_codec_context) {
        QString err("could not allocate an encoding context");
        qCritical().noquote() << err;
        return err;
    }

    output_stream->av_codec_context->codec_id=(*codec)->id;

    output_stream->av_codec_context->width=cfg.frame_resolution_dst.width();
    output_stream->av_codec_context->height=cfg.frame_resolution_dst.height();


    AVRational target_framerate;

    switch(cfg.framerate) {
    case FFEncoder::Framerate::unknown:
        target_framerate=cfg.framerate_force;
        break;

    default:
        target_framerate=FFEncoder::Framerate::toRational(cfg.framerate);
        break;
    }

    // tbr?
    // output_stream->av_stream->r_frame_rate;

    // tbn?
    // output_stream->av_stream->time_base;

    // tbc?
    output_stream->av_codec_context->time_base=target_framerate;

    output_stream->av_codec_context->framerate=av_inv_q(target_framerate);
    output_stream->av_codec_context->sample_aspect_ratio={ 1, 1 };

    output_stream->av_stream->avg_frame_rate=av_inv_q(target_framerate);

    output_stream->av_codec_context->pix_fmt=cfg.pixel_format_src.toAVPixelFormat();

    if(cfg.color_primaries>-1)
        output_stream->av_codec_context->color_primaries=(AVColorPrimaries)cfg.color_primaries;

    if(cfg.color_space>-1)
        output_stream->av_codec_context->colorspace=(AVColorSpace)cfg.color_space;

    if(cfg.color_transfer_characteristic>-1)
        output_stream->av_codec_context->color_trc=(AVColorTransferCharacteristic)cfg.color_transfer_characteristic;

    if(cfg.color_range>0)
        output_stream->av_codec_context->color_range=(AVColorRange)cfg.color_range;

    if(format_context->oformat->flags & AVFMT_GLOBALHEADER)
        output_stream->av_codec_context->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;


    return QStringLiteral("");
}

static QString add_stream_video(OutputStream *output_stream, AVFormatContext *format_context, AVCodec **codec, const FFEncoder::Config &cfg)
{
    (*codec)=avcodec_find_encoder_by_name(FFEncoder::VideoEncoder::toEncName(cfg.video_encoder).toLatin1().data());

    if(!(*codec)) {
        QString err("could not find encoder");
        qCritical().noquote() << err;
        return err;
    }


    output_stream->av_stream=avformat_new_stream(format_context, nullptr);

    if(!output_stream->av_stream) {
        QString err("could not allocate stream");
        qCritical().noquote() << err;
        return err;
    }


    output_stream->av_stream->id=format_context->nb_streams - 1;


    output_stream->av_codec_context=avcodec_alloc_context3(*codec);

    if(!output_stream->av_codec_context) {
        QString err("could not allocate an encoding context");
        qCritical().noquote() << err;
        return err;
    }

    output_stream->av_codec_context->codec_id=(*codec)->id;

    output_stream->av_codec_context->width=cfg.frame_resolution_dst.width();
    output_stream->av_codec_context->height=cfg.frame_resolution_dst.height();


    AVRational target_framerate;

    switch(cfg.framerate) {
    case FFEncoder::Framerate::unknown:
        target_framerate=cfg.framerate_force;
        break;

    default:
        target_framerate=FFEncoder::Framerate::toRational(cfg.framerate);
        break;
    }

    // tbr?
    // output_stream->av_stream->r_frame_rate;

    // tbn?
    // output_stream->av_stream->time_base;

    // tbc?
    output_stream->av_codec_context->time_base=target_framerate;

    output_stream->av_codec_context->framerate=av_inv_q(target_framerate);
    output_stream->av_codec_context->sample_aspect_ratio={ 1, 1 };

    output_stream->av_stream->avg_frame_rate=av_inv_q(target_framerate);

    output_stream->av_codec_context->pix_fmt=cfg.pixel_format_dst.toAVPixelFormat();


    if(cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_h264
            || cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_hevc
            || cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_vp8
            || cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_vp9) {
        int ret=av_hwdevice_ctx_create(&output_stream->hw_device_ctx, AV_HWDEVICE_TYPE_VAAPI, nullptr, nullptr, 0);

        if(ret<0)
            return QString("Failed to create a VAAPI device: %1").arg(ffErrorString(ret));

        output_stream->av_codec_context->pix_fmt=AV_PIX_FMT_VAAPI;

        AVBufferRef *hw_frames_ref;

        if(!(hw_frames_ref=av_hwframe_ctx_alloc(output_stream->hw_device_ctx)))
            return "Failed to create VAAPI frame context";

        AVHWFramesContext *frames_ctx=
                (AVHWFramesContext*)hw_frames_ref->data;

        frames_ctx->format=AV_PIX_FMT_VAAPI;
        frames_ctx->sw_format=cfg.pixel_format_dst.toAVPixelFormat();
        frames_ctx->width=cfg.frame_resolution_dst.width();
        frames_ctx->height=cfg.frame_resolution_dst.height();
        // frames_ctx->initial_pool_size=20; // ???

        if((ret=av_hwframe_ctx_init(hw_frames_ref))<0) {
            av_buffer_unref(&hw_frames_ref);

            QString err("Failed to initialize VAAPI frame context: " + ffErrorString(ret));
            qCritical().noquote() << err;
            return err;
        }

        output_stream->av_codec_context->hw_frames_ctx=av_buffer_ref(hw_frames_ref);

        av_buffer_unref(&hw_frames_ref);

        if(!output_stream->av_codec_context->hw_frames_ctx) {
            QString err=ffErrorString(AVERROR(ENOMEM));
            qCritical().noquote() << err;
            return err;
        }

        if(!(output_stream->frame_hw=av_frame_alloc())) {
            QString err=ffErrorString(AVERROR(ENOMEM));
            qCritical().noquote() << err;
            return err;
        }


        if((ret=av_hwframe_get_buffer(output_stream->av_codec_context->hw_frames_ctx, output_stream->frame_hw, 0))<0)  {
            QString err("av_hwframe_get_buffer err: " + ffErrorString(ret));
            qCritical().noquote() << err;
            return err;
        }

        if(!output_stream->frame_hw->hw_frames_ctx) {
            QString err=ffErrorString(AVERROR(ENOMEM));
            qCritical().noquote() << err;
            return err;
        }
    }

    if(cfg.video_encoder==FFEncoder::VideoEncoder::libx264 || cfg.video_encoder==FFEncoder::VideoEncoder::libx264rgb) {
        if(cfg.video_bitrate==0) {
            av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
            av_opt_set(output_stream->av_codec_context->priv_data, "crf", QString::number(cfg.crf).toLatin1().constData(), 0);
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_h264) {
        if(cfg.nvenc.enabled) {
            if(cfg.video_bitrate==0)
                av_opt_set(output_stream->av_codec_context->priv_data, "rc", "constqp", 0);

            else {
                // av_opt_set(output_stream->av_codec_context->priv_data, "rc", "vbr_hq", 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "rc", "cbr", 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "cbr", "1", 0);
            }

            av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

            if(cfg.nvenc.device!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "gpu", QString::number(cfg.nvenc.device - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.weighted_pred==0 && cfg.nvenc.b_frames>0)
                output_stream->av_codec_context->max_b_frames=cfg.nvenc.b_frames;

            if(cfg.nvenc.b_ref_mode>0)
                av_opt_set(output_stream->av_codec_context->priv_data, "b_ref_mode", QString::number(cfg.nvenc.b_ref_mode).toLatin1().constData(), 0);

            output_stream->av_codec_context->refs=cfg.nvenc.ref_frames;

            output_stream->av_codec_context->gop_size=cfg.nvenc.gop_size;

            if(cfg.video_bitrate==0) {
                if(cfg.nvenc.qp_i==0) {
                    av_opt_set(output_stream->av_codec_context->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);

                } else {
                    av_opt_set(output_stream->av_codec_context->priv_data, "init_qpI", QString::number(cfg.nvenc.qp_i - 1).toLatin1().constData(), 0);
                    av_opt_set(output_stream->av_codec_context->priv_data, "init_qpP", QString::number(cfg.nvenc.qp_p - 1).toLatin1().constData(), 0);
                    av_opt_set(output_stream->av_codec_context->priv_data, "init_qpB", QString::number(cfg.nvenc.qp_b - 1).toLatin1().constData(), 0);
                }
            }

            switch(cfg.nvenc.aq_mode) {
            case 1:
                av_opt_set(output_stream->av_codec_context->priv_data, "spatial-aq", "1", 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "aq-strength", QString::number(cfg.nvenc.aq_strength).toLatin1().constData(), 0);
                break;

            case 2:
                av_opt_set(output_stream->av_codec_context->priv_data, "temporal-aq", "1", 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "aq-strength", QString::number(cfg.nvenc.aq_strength).toLatin1().constData(), 0);
                break;

            case 0:
            default:
                break;
            }

            if(cfg.nvenc.rc_lookahead>0)
                av_opt_set(output_stream->av_codec_context->priv_data, "rc-lookahead", QString::number(cfg.nvenc.rc_lookahead - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.surfaces>0)
                av_opt_set(output_stream->av_codec_context->priv_data, "surfaces", QString::number(cfg.nvenc.surfaces - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.no_scenecut!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "no-scenecut", "1", 0);

            if(cfg.nvenc.forced_idr!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "forced-idr", "1", 0);

            if(cfg.nvenc.b_adapt!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "b_adapt", "1", 0);

            if(cfg.nvenc.nonref_p!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "nonref_p", "1", 0);

            if(cfg.nvenc.strict_gop!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "strict_gop", "1", 0);

            if(cfg.nvenc.weighted_pred!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "weighted_pred", "1", 0);

            if(cfg.nvenc.bluray_compat!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "bluray-compat", "1", 0);

        } else {
            if(cfg.video_bitrate==0) {
                av_opt_set(output_stream->av_codec_context->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "rc", "constqp", 0);
            }

            av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_hevc) {
        if(cfg.nvenc.enabled) {
            if(cfg.video_bitrate==0)
                av_opt_set(output_stream->av_codec_context->priv_data, "rc", "constqp", 0);

            else
                av_opt_set(output_stream->av_codec_context->priv_data, "rc", "vbr_hq", 0);


            av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

            if(cfg.nvenc.device!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "gpu", QString::number(cfg.nvenc.device - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.weighted_pred==0 && cfg.nvenc.b_frames>0)
                output_stream->av_codec_context->max_b_frames=cfg.nvenc.b_frames;

            if(cfg.nvenc.b_ref_mode>0)
                av_opt_set(output_stream->av_codec_context->priv_data, "b_ref_mode", QString::number(cfg.nvenc.b_ref_mode).toLatin1().constData(), 0);

            output_stream->av_codec_context->refs=cfg.nvenc.ref_frames;

            output_stream->av_codec_context->gop_size=cfg.nvenc.gop_size;
            output_stream->av_codec_context->keyint_min=6;
            // av_opt_set(output_stream->av_codec_context->priv_data, "sc_threshold", "40", 0);


            output_stream->av_codec_context->bit_rate=64*1024*1024;


            if(cfg.video_bitrate==0) {
                if(cfg.nvenc.qp_i==0) {
                    av_opt_set(output_stream->av_codec_context->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);

                    // output_stream->av_codec_context->qmin=cfg.crf;
                    // output_stream->av_codec_context->qmax=cfg.crf;
                    // output_stream->av_codec_context->max_qdiff=out_stream->av_codec_context->qmax - out_stream->av_codec_context->qmin;

                } else {
                    av_opt_set(output_stream->av_codec_context->priv_data, "init_qpI", QString::number(cfg.nvenc.qp_i - 1).toLatin1().constData(), 0);
                    av_opt_set(output_stream->av_codec_context->priv_data, "init_qpP", QString::number(cfg.nvenc.qp_p - 1).toLatin1().constData(), 0);
                    av_opt_set(output_stream->av_codec_context->priv_data, "init_qpB", QString::number(cfg.nvenc.qp_b - 1).toLatin1().constData(), 0);
                }
            }

            switch(cfg.nvenc.aq_mode) {
            case 1:
                av_opt_set(output_stream->av_codec_context->priv_data, "spatial_aq", "1", 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "aq-strength", QString::number(cfg.nvenc.aq_strength).toLatin1().constData(), 0);
                break;

            case 0:
            case 2:
            default:
                break;
            }

            if(cfg.nvenc.rc_lookahead>0)
                av_opt_set(output_stream->av_codec_context->priv_data, "rc-lookahead", QString::number(cfg.nvenc.rc_lookahead - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.surfaces>0)
                av_opt_set(output_stream->av_codec_context->priv_data, "surfaces", QString::number(cfg.nvenc.surfaces - 1).toLatin1().constData(), 0);

            if(cfg.nvenc.no_scenecut!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "no-scenecut", "1", 0);

            if(cfg.nvenc.forced_idr!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "forced-idr", "1", 0);

            if(cfg.nvenc.nonref_p!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "nonref_p", "1", 0);

            if(cfg.nvenc.strict_gop!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "strict_gop", "1", 0);

            if(cfg.nvenc.weighted_pred!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "weighted_pred", "1", 0);

            if(cfg.nvenc.bluray_compat!=0)
                av_opt_set(output_stream->av_codec_context->priv_data, "bluray-compat", "1", 0);

        } else {
            if(cfg.video_bitrate==0) {
                av_opt_set(output_stream->av_codec_context->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
                av_opt_set(output_stream->av_codec_context->priv_data, "rc", "constqp", 0);
            }

            av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::qsv_h264) {
        if(cfg.video_bitrate==0) {
            output_stream->av_codec_context->flags|=AV_CODEC_FLAG_QSCALE;
            output_stream->av_codec_context->global_quality=FF_QP2LAMBDA*cfg.crf;
        }

        av_opt_set(output_stream->av_codec_context->priv_data, "look_ahead", "0", 0);
        av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::qsv_hevc) {
        if(cfg.video_bitrate==0) {
            output_stream->av_codec_context->flags|=AV_CODEC_FLAG_QSCALE;
            output_stream->av_codec_context->global_quality=FF_QP2LAMBDA*cfg.crf;
        }

        av_opt_set(output_stream->av_codec_context->priv_data, "look_ahead", "1", 0);
        av_opt_set(output_stream->av_codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_h264 || cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_hevc) {
        if(cfg.video_bitrate==0) {
            av_opt_set(output_stream->av_codec_context->priv_data, "rc_mode", "CQP", 0);
            av_opt_set(output_stream->av_codec_context->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
            // output_stream->av_codec_context->global_quality=cfg.crf;
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_vp8 || cfg.video_encoder==FFEncoder::VideoEncoder::vaapi_vp9) {
        if(cfg.video_bitrate==0) {
            av_opt_set(output_stream->av_codec_context->priv_data, "rc_mode", "CQP", 0);
            output_stream->av_codec_context->global_quality=cfg.crf;
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::ffvhuff) {
        output_stream->av_codec_context->thread_count=QThread::idealThreadCount() - 1;

        if(output_stream->av_codec_context->thread_count<=0)
            output_stream->av_codec_context->thread_count=1;
    }

    if(cfg.video_bitrate!=0) {
        // output_stream->av_codec_context->bit_rate_tolerance=0;
        output_stream->av_codec_context->bit_rate=cfg.video_bitrate*1000;
        output_stream->av_codec_context->rc_max_rate=cfg.video_bitrate*1000;
        output_stream->av_codec_context->rc_buffer_size=output_stream->av_codec_context->rc_max_rate*2;
    }

    if(cfg.color_primaries>-1)
        output_stream->av_codec_context->color_primaries=(AVColorPrimaries)cfg.color_primaries;

    if(cfg.color_space>-1)
        output_stream->av_codec_context->colorspace=(AVColorSpace)cfg.color_space;

    if(cfg.color_transfer_characteristic>-1)
        output_stream->av_codec_context->color_trc=(AVColorTransferCharacteristic)cfg.color_transfer_characteristic;

    if(cfg.color_range>0)
        output_stream->av_codec_context->color_range=(AVColorRange)cfg.color_range;

    if(cfg.mastering_display_metadata.has_luminance || cfg.mastering_display_metadata.has_luminance) {
        AVMasteringDisplayMetadata *mastering_display_metadata=(AVMasteringDisplayMetadata*)av_malloc(sizeof(AVMasteringDisplayMetadata));
        memcpy(mastering_display_metadata, &cfg.mastering_display_metadata, sizeof(cfg.mastering_display_metadata));
        av_stream_add_side_data(output_stream->av_stream, AV_PKT_DATA_MASTERING_DISPLAY_METADATA, (uint8_t*)mastering_display_metadata, sizeof(AVMasteringDisplayMetadata));
    }

    if(cfg.aspect_ratio_4_3) {
        output_stream->av_codec_context->sample_aspect_ratio={ 3, 4 };
        output_stream->av_stream->sample_aspect_ratio={ 3, 4 };
    }

    if(format_context->oformat->flags & AVFMT_GLOBALHEADER)
        output_stream->av_codec_context->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;

    return QStringLiteral("");
}

static QString alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples, AVFrame **frame)
{
    (*frame)=av_frame_alloc();

    int ret;

    if(!(*frame)) {
        QString err("error allocating an audio frame");
        qCritical().noquote() << err;
        return err;
    }

    (*frame)->format=sample_fmt;
    (*frame)->channel_layout=channel_layout;
    (*frame)->sample_rate=sample_rate;
    (*frame)->nb_samples=nb_samples;

    if(nb_samples) {
        ret=av_frame_get_buffer(*frame, 0);

        if(ret<0) {
            av_frame_free(&(*frame));
            (*frame)=nullptr;

            QString err("error allocating an audio buffer");
            qCritical().noquote() << err;
            return err;
        }
    }

    return QStringLiteral("");
}

static QString open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    Q_UNUSED(oc)

    AVCodecContext *c;

    int ret;

    AVDictionary *opt=nullptr;

    c=ost->av_codec_context;

    av_dict_copy(&opt, opt_arg, 0);

    av_dict_set(&opt, "strict", "experimental", 0);

    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        QString err("could not open audio codec: " + ffErrorString(ret));
        qCritical().noquote() << err;
        return err;
    }

    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0) {
        QString err("could not copy the stream parameters");
        qCritical().noquote() << err;
        return err;
    }

    if(c->frame_size<1)
        c->frame_size=1024;

    if(!ost->pkt)
        ost->pkt=av_packet_alloc();

    return QStringLiteral("");
}

static QString write_audio_frame(AVFormatContext *oc, OutputStream *ost, AVFrame *frame)
{
    int ret=avcodec_send_frame(ost->av_codec_context, frame);

    if(ret<0) {
        QString err("error encoding audio frame: " + ffErrorString(ret));
        qCritical().noquote() << err;
        return err;
    }

    while(!ret) {
        ret=avcodec_receive_packet(ost->av_codec_context, ost->pkt);

        if(ret<0 && ret!=AVERROR(EAGAIN) && ret!=AVERROR_EOF) {
            QString err=ffErrorString(ret);
            qCritical().noquote() << err;
            return err;
        }

        if(ret==0) {
            ost->size_total+=ost->pkt->size;

            interleaved_write_frame(oc, &ost->av_codec_context->time_base, ost->av_stream, ost->pkt);
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

    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        QString err("could not open video codec: " + ffErrorString(ret));
        qCritical().noquote() << err;
        return err;
    }

    ost->frame=alloc_frame(ost->frame_fmt, cfg.frame_resolution_src.width(), cfg.frame_resolution_src.height());

    if(!ost->frame) {
        QString err("could not allocate video frame");
        qCritical().noquote() << err;
        return err;
    }

    ost->frame_converted=alloc_frame(cfg.pixel_format_dst.toAVPixelFormat(), cfg.frame_resolution_dst.width(), cfg.frame_resolution_dst.height());

    if(!ost->frame_converted) {
        QString err("Could not allocate video frame");
        qCritical().noquote() << err;
        return err;
    }

    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0) {
        QString err("could not copy the stream parameters");
        qCritical().noquote() << err;
        return err;
    }

    if(!ost->pkt)
        ost->pkt=av_packet_alloc();


    return QStringLiteral("");
}

static QString write_video_packet(AVFormatContext *format_context, OutputStream *output_stream, AVPacket *packet, const int64_t &pts)
{
    output_stream->size_total+=packet->size;

    if(pts!=AV_NOPTS_VALUE) {
        packet->dts=
                packet->pts=
                pts;

    } else {
        av_packet_rescale_ts(packet, output_stream->av_codec_context->time_base, output_stream->av_stream->time_base);
    }

    int ret=interleaved_write_frame(format_context, &output_stream->av_stream->time_base, output_stream->av_stream, packet);

    if(ret!=0) {
        qCritical() << "write_video_packet err:" << ffErrorString(ret);
        return ffErrorString(ret);
    }

    return QStringLiteral("");
}

QString FFEncoder::write_video_frame(AVFormatContext *format_context, OutputStream *output_stream, const int64_t &pts)
{
    int ret;
    QString err_string;

    if(output_stream->frame_hw) {
        output_stream->frame_hw->pts=pts;

        if((ret=av_hwframe_transfer_data(output_stream->frame_hw, output_stream->frame_converted, 0))<0) {
            return QString("error while transferring frame data to surface: %1").arg(ffErrorString(ret));
        }

        ret=avcodec_send_frame(output_stream->av_codec_context, output_stream->frame_hw);

    } else {
        ret=avcodec_send_frame(output_stream->av_codec_context, output_stream->frame_converted);
    }

    if(ret<0) {
        qCritical() << "write_video_frame err:" << ffErrorString(ret);
        return QString("error encoding video frame: %1").arg(ffErrorString(ret));
    }

    while(ret>=0) {
        ret=avcodec_receive_packet(output_stream->av_codec_context, output_stream->pkt);

        if(ret<0 && ret!=AVERROR(EAGAIN)) {
            err_string=ffErrorString(ret);
            return err_string;
        }

        if(ret>=0) {
            if((context->cfg.video_encoder==VideoEncoder::nvenc_h264 || context->cfg.video_encoder==VideoEncoder::nvenc_hevc) && context->cfg.nvenc.b_ref_mode>0)
                output_stream->pkt->pts=output_stream->pkt->dts;

            err_string=write_video_packet(format_context, output_stream, output_stream->pkt, AV_NOPTS_VALUE);

            av_packet_unref(output_stream->pkt);

            if(!err_string.isEmpty()) {
                return err_string;
            }
        }
    }

    return QStringLiteral("");
}

static void close_stream(OutputStream *ost)
{
    if(ost->hw_device_ctx) {
        av_buffer_unref(&ost->hw_device_ctx);
        ost->hw_device_ctx=nullptr;
    }

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

    if(ost->frame_hw) {
        av_frame_free(&ost->frame_hw);
        ost->frame_hw=nullptr;
    }
}

// ------------------------------

FFEncoder::FFEncoder(int enc_num, FFEncStartSync *start_sync, QObject *parent) :
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

    context->base_filename=nullptr;
    context->enc_num=enc_num;
    this->start_sync=start_sync;
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

QString FFEncoder::presetParamNameToVisualName(const QString &str)
{
    if(str==QLatin1String("hq"))
        return QLatin1String("high quality");

    if(str==QLatin1String("hp"))
        return QLatin1String("high performance");

    if(str==QLatin1String("bd"))
        return QLatin1String("bluray disk");

    if(str==QLatin1String("ll"))
        return QLatin1String("low latency");

    if(str==QLatin1String("llhq"))
        return QLatin1String("low latency high quality");

    if(str==QLatin1String("llhp"))
        return QLatin1String("low latency high performance");

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
    case VideoEncoder::qsv_hevc:
        return QStringList() << QLatin1String("veryfast") << QLatin1String("faster") << QLatin1String("fast")
                             << QLatin1String("medium")
                             << QLatin1String("slow") << QLatin1String("slower") << QLatin1String("veryslow");

    default: return QStringList() << QLatin1String("--");
    }

    return QStringList() << QLatin1String("--");
}

QList <int> FFEncoder::availableColorPrimaries()
{
    static QList <int> list=QList<int>() << AVCOL_PRI_BT709 << AVCOL_PRI_BT470M << AVCOL_PRI_BT470BG << AVCOL_PRI_SMPTE170M << AVCOL_PRI_SMPTE240M
                                         << AVCOL_PRI_FILM << AVCOL_PRI_BT2020 << AVCOL_PRI_SMPTE428 << AVCOL_PRI_SMPTE431 << AVCOL_PRI_SMPTE432 << AVCOL_PRI_JEDEC_P22;

    return list;
}

QList <int> FFEncoder::availableColorSpaces()
{
    static QList <int> list=QList<int>() << AVCOL_SPC_RGB << AVCOL_SPC_BT709 << AVCOL_SPC_FCC << AVCOL_SPC_BT470BG << AVCOL_SPC_SMPTE170M << AVCOL_SPC_SMPTE240M
                                         << AVCOL_SPC_YCGCO << AVCOL_SPC_BT2020_NCL << AVCOL_SPC_BT2020_CL << AVCOL_SPC_SMPTE2085 << AVCOL_SPC_CHROMA_DERIVED_NCL
                                         << AVCOL_SPC_CHROMA_DERIVED_CL << AVCOL_SPC_ICTCP;

    return list;
}

QList <int> FFEncoder::availableColorTransferCharacteristics()
{
    static QList <int> list=QList<int>() << AVCOL_TRC_BT709 << AVCOL_TRC_GAMMA22 << AVCOL_TRC_GAMMA28 << AVCOL_TRC_SMPTE170M << AVCOL_TRC_SMPTE240M << AVCOL_TRC_LINEAR
                                         << AVCOL_TRC_LOG << AVCOL_TRC_LOG_SQRT << AVCOL_TRC_IEC61966_2_4 << AVCOL_TRC_BT1361_ECG << AVCOL_TRC_IEC61966_2_1
                                         << AVCOL_TRC_BT2020_10 << AVCOL_TRC_BT2020_12 << AVCOL_TRC_SMPTE2084 << AVCOL_TRC_SMPTE428 << AVCOL_TRC_ARIB_STD_B67;

    return list;
}

QString FFEncoder::colorPrimariesToString(int value)
{
    switch(value) {
    case AVCOL_PRI_BT709:
        return QStringLiteral("BT709");

    case AVCOL_PRI_BT470M:
        return QStringLiteral("BT470M");

    case AVCOL_PRI_BT470BG:
        return QStringLiteral("BT470BG");

    case AVCOL_PRI_SMPTE170M:
        return QStringLiteral("SMPTE170M");

    case AVCOL_PRI_SMPTE240M:
        return QStringLiteral("SMPTE240M");

    case AVCOL_PRI_FILM:
        return QStringLiteral("FILM");

    case AVCOL_PRI_BT2020:
        return QStringLiteral("BT2020");

    case AVCOL_PRI_SMPTE428:
        return QStringLiteral("SMPTE428");

    case AVCOL_PRI_SMPTE431:
        return QStringLiteral("SMPTE431");

    case AVCOL_PRI_SMPTE432:
        return QStringLiteral("SMPTE432");

    case AVCOL_PRI_JEDEC_P22:
        return QStringLiteral("JEDEC_P22");
    }

    return QStringLiteral("undefined");
}

QString FFEncoder::colorSpaceToString(int value)
{
    switch(value) {
    case AVCOL_SPC_RGB:
        return QStringLiteral("RGB");

    case AVCOL_SPC_BT709:
        return QStringLiteral("BT709");

    case AVCOL_SPC_FCC:
        return QStringLiteral("FCC");

    case AVCOL_SPC_BT470BG:
        return QStringLiteral("BT470BG");

    case AVCOL_SPC_SMPTE170M:
        return QStringLiteral("SMPTE170M");

    case AVCOL_SPC_SMPTE240M:
        return QStringLiteral("SMPTE240M");

    case AVCOL_SPC_YCGCO:
        return QStringLiteral("YCGCO");

    case AVCOL_SPC_BT2020_NCL:
        return QStringLiteral("BT2020_NCL");

    case AVCOL_SPC_BT2020_CL:
        return QStringLiteral("BT2020_CL");

    case AVCOL_SPC_SMPTE2085:
        return QStringLiteral("SMPTE2085");

    case AVCOL_SPC_CHROMA_DERIVED_NCL:
        return QStringLiteral("CHROMA_DERIVED_NCL");

    case AVCOL_SPC_CHROMA_DERIVED_CL:
        return QStringLiteral("CHROMA_DERIVED_CL");

    case AVCOL_SPC_ICTCP:
        return QStringLiteral("ICTCP");
    }

    return QStringLiteral("undefined");
}

QString FFEncoder::colorTransferCharacteristicToString(int value)
{
    switch(value) {
    case AVCOL_TRC_BT709:
        return QStringLiteral("BT709");

    case AVCOL_TRC_GAMMA22:
        return QStringLiteral("GAMMA22");

    case AVCOL_TRC_GAMMA28:
        return QStringLiteral("GAMMA28");

    case AVCOL_TRC_SMPTE170M:
        return QStringLiteral("SMPTE170M");

    case AVCOL_TRC_SMPTE240M:
        return QStringLiteral("SMPTE240M");

    case AVCOL_TRC_LINEAR:
        return QStringLiteral("LINEAR");

    case AVCOL_TRC_LOG:
        return QStringLiteral("LOG");

    case AVCOL_TRC_LOG_SQRT:
        return QStringLiteral("LOG_SQRT");

    case AVCOL_TRC_IEC61966_2_4:
        return QStringLiteral("IEC61966_2_4");

    case AVCOL_TRC_BT1361_ECG:
        return QStringLiteral("BT1361_ECG");

    case AVCOL_TRC_IEC61966_2_1:
        return QStringLiteral("IEC61966_2_1");

    case AVCOL_TRC_BT2020_10:
        return QStringLiteral("BT2020_10");

    case AVCOL_TRC_BT2020_12:
        return QStringLiteral("BT2020_12");

    case AVCOL_TRC_SMPTE2084:
        return QStringLiteral("SMPTE2084");

    case AVCOL_TRC_SMPTE428:
        return QStringLiteral("SMPTE428");

    case AVCOL_TRC_ARIB_STD_B67:
        return QStringLiteral("ARIB_STD_B67");
    }

    return QStringLiteral("undefined");
}

void FFEncoder::setEncodingToolName(const QString &encoding_tool)
{
    this->encoding_tool=encoding_tool;
}

void FFEncoder::setStoreDir(const QString &dir)
{
    context->store_dir=dir;
}

void FFEncoder::setBaseFilename(FFEncoderBaseFilename *bf)
{
    context->base_filename=bf;
}

QString FFEncoder::configString(const FFEncoder::Config &cfg)
{
    QVariantMap map;

    if(cfg.direct_stream_copy) {
        return "direct stream copy mode";

    } else {
        map.insert("src_pix_fmt", cfg.pixel_format_src.toString());
        map.insert("dst_pix_fmt", cfg.pixel_format_dst.toString());

        if(cfg.frame_resolution_src==cfg.frame_resolution_dst) {
            map.insert("resolution", QString("%1x%2").arg(cfg.frame_resolution_src.width()).arg(cfg.frame_resolution_src.height()));

        } else {
            map.insert("src_resolution", QString("%1x%2").arg(cfg.frame_resolution_src.width()).arg(cfg.frame_resolution_src.height()));
            map.insert("dst_resolution", QString("%1x%2").arg(cfg.frame_resolution_dst.width()).arg(cfg.frame_resolution_dst.height()));
            map.insert("scale_filter", ScaleFilter::toString(cfg.scale_filter));
        }

        if(cfg.crf!=0xff && cfg.video_bitrate==0)
            map.insert("crf", cfg.crf);

        if(cfg.preset!="--")
            map.insert("preset", presetParamNameToVisualName(cfg.preset));

        map.insert("video_encoder", VideoEncoder::toString(cfg.video_encoder));


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

            if((cfg.video_encoder==VideoEncoder::nvenc_h264 || cfg.video_encoder==VideoEncoder::nvenc_hevc) && cfg.nvenc.b_frames>0)
                map.insert("b_frames", cfg.nvenc.b_frames);

            map.insert("ref_frames", cfg.nvenc.ref_frames);
            map.insert("gop_size", cfg.nvenc.gop_size);

            if(cfg.nvenc.b_ref_mode>0)
                map.insert("b_ref_mode", cfg.nvenc.b_ref_mode==1 ? "each" : "middle");

            if(cfg.nvenc.rc_lookahead>0)
                map.insert("rc_lookahead", cfg.nvenc.rc_lookahead - 1);

            if(cfg.nvenc.surfaces>0)
                map.insert("surfaces", cfg.nvenc.surfaces - 1);

            if(cfg.nvenc.no_scenecut!=0)
                map.insert("no-scenecut", QVariant());

            if(cfg.nvenc.forced_idr!=0)
                map.insert("forced-idr", QVariant());

            if(cfg.nvenc.b_adapt!=0)
                map.insert("b_adapt", QVariant());

            if(cfg.nvenc.nonref_p!=0)
                map.insert("nonref_p", QVariant());

            if(cfg.nvenc.strict_gop!=0)
                map.insert("strict_gop", QVariant());

            if(cfg.nvenc.weighted_pred!=0)
                map.insert("weighted_pred", QVariant());

            if(cfg.nvenc.bluray_compat!=0)
                map.insert("bluray_compat", QVariant());


            if(cfg.video_bitrate==0) {
                if(cfg.nvenc.qp_i>0) {
                    map.insert("qp_i", cfg.nvenc.qp_i - 1);
                    map.insert("qp_p", cfg.nvenc.qp_p - 1);

                    if(cfg.video_encoder==VideoEncoder::nvenc_h264 || cfg.video_encoder==VideoEncoder::nvenc_hevc)
                        map.insert("qp_b", cfg.nvenc.qp_b - 1);

                } else {
                    map.insert("qp", cfg.crf);
                }
            }
        }
    }


    return QString(QJsonDocument::fromVariant(map).toJson(QJsonDocument::Compact))
            .remove("{")
            .remove("}")
            .remove("\"")
            .remove(":null")
            .replace(":", "=")
            .replace(",", ", ");
}

bool FFEncoder::checkFrameParams(Frame::ptr frame) const
{
    if(frame->video.data_ptr) {
        if(context->cfg.direct_stream_copy) {
            if(frame->video.size!=context->cfg.frame_resolution_src || frame->video.pixel_format_pkt!=context->cfg.pixel_format_src) {
                qWarning().noquote() << "video params changed:" << frame->video.size << context->cfg.frame_resolution_src << PixelFormat::toString(frame->video.pixel_format_pkt) << PixelFormat::toString(context->cfg.pixel_format_src);
                return false;
            }

        } else if(frame->video.size!=context->cfg.frame_resolution_src || frame->video.pixel_format!=context->cfg.pixel_format_src) {
            qWarning().noquote() << "video params changed:" << frame->video.size << context->cfg.frame_resolution_src << PixelFormat::toString(frame->video.pixel_format) << PixelFormat::toString(context->cfg.pixel_format_src);
            return false;
        }
    }

    if(frame->audio.data_ptr) {
        if(frame->audio.channels!=context->cfg.audio_channels_size || frame->audio.sample_size!=context->cfg.audio_sample_size) {
            qWarning() << "audio params changed:" << frame->audio.channels << context->cfg.audio_channels_size << frame->audio.sample_size << context->cfg.audio_sample_size;
            return false;
        }
    }

    return true;
}

void FFEncoder::restart(Frame::ptr frame)
{
    stopCoder();

    Config cfg=context->cfg;

    if(start_sync && context->enc_num!=StreamingMode)
        start_sync->restart();

    if(frame->video.data_ptr) {
        cfg.frame_resolution_src=frame->video.size;

        if(context->cfg.direct_stream_copy && frame->video.pixel_format_pkt.isValid())
            cfg.pixel_format_src=frame->video.pixel_format_pkt;

        else
            cfg.pixel_format_src=frame->video.pixel_format;
    }

    if(frame->audio.data_ptr) {
        cfg.audio_channels_size=frame->audio.channels;
        cfg.audio_sample_size=frame->audio.sample_size;
    }

    context->base_filename->reset();

    setConfig(cfg);

    emit restartReq();
}

void FFEncoder::fillDroppedFrames(int size)
{
    if(!context->cfg.fill_dropped_frames)
        return;

    if(context->cfg.direct_stream_copy)
        return;

    if(!context->out_stream_video.frame_current)
        return;

    switch(context->cfg.framerate) {
    case Framerate::half_50:
    case Framerate::half_59:
    case Framerate::half_60:
        return;

    default:
        break;
    }

    AVFrame *frame_orig=context->out_stream_video.frame_converted;

    context->out_stream_video.frame_converted=context->out_stream_video.frame_current->d;

    int framenum=1;

    while(size--) {
        context->out_stream_video.frame_converted->pts=
                context->out_stream_video.pts_last + framenum++;

        last_error_string=write_video_frame(context->av_format_context, &context->out_stream_video, context->out_stream_video.frame_converted->pts);

        if(!last_error_string.isEmpty()) {
            qCritical() << "write_video_frame error: " + last_error_string;
        }
    }

    context->out_stream_video.frame_converted=frame_orig;
}

void checkCrfValue(FFEncoder::Config *cfg)
{
    if(cfg->video_encoder==FFEncoder::VideoEncoder::ffvhuff || cfg->video_encoder==FFEncoder::VideoEncoder::magicyuv) {
        cfg->crf=0xff;
        return;
    }

    if(cfg->video_encoder==FFEncoder::VideoEncoder::nvenc_h264 || cfg->video_encoder==FFEncoder::VideoEncoder::nvenc_hevc) {
        if(cfg->crf>51)
            cfg->crf=51;

        return;
    }

    if(cfg->video_encoder==FFEncoder::VideoEncoder::vaapi_h264 || cfg->video_encoder==FFEncoder::VideoEncoder::vaapi_hevc) {
        if(cfg->crf>52)
            cfg->crf=52;

        return;
    }
}

bool FFEncoder::setConfig(FFEncoder::Config cfg)
{
    if(cfg.input_type_flags&SourceInterface::TypeFlag::video && (!cfg.pixel_format_src.isValid() || !cfg.pixel_format_dst.isValid()))
        return false;

    last_error_string.clear();

    checkCrfValue(&cfg);

    int ret;
    // int sws_flags=0;

    cfg.direct_stream_copy=cfg.direct_stream_copy && cfg.pixel_format_src.isCompressed();

    if(!cfg.direct_stream_copy)
        cfg.pixel_format_src=PixelFormat::normalizeFormat(cfg.pixel_format_src);

    context->out_stream_video.frame_fmt=cfg.pixel_format_dst.toAVPixelFormat();

    cfg.frame_resolution_dst=cfg.frame_resolution_src;

    if(!cfg.direct_stream_copy && cfg.downscale!=DownScale::Disabled) {
        if(cfg.frame_resolution_dst.height()>DownScale::toWidth(cfg.downscale)) {
            cfg.frame_resolution_dst.setHeight(DownScale::toWidth(cfg.downscale));
            cfg.frame_resolution_dst.setWidth(cfg.frame_resolution_dst.height()*(16./9.));
            // sws_flags|=ScaleFilter::toSws(cfg.scale_filter);
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

    if(!cfg.direct_stream_copy && cfg.input_type_flags&SourceInterface::TypeFlag::video && !format_converter_ff->setup(
                PixelFormat::normalizeFormat(cfg.pixel_format_src).toAVPixelFormat(), cfg.frame_resolution_src, cfg.pixel_format_dst.toAVPixelFormat(), cfg.frame_resolution_dst,
                cfg.sws_color_space_src, cfg.sws_color_space_dst, cfg.sws_color_range_src, cfg.sws_color_range_dst,
                cfg.downscale==DownScale::Disabled ? FFFormatConverter::Filter::cNull : (FFFormatConverter::Filter::T)ScaleFilter::toSws(cfg.scale_filter),
                cfg.pixel_format_src.is210() ? (cfg.pixel_format_src.isRgb() ? DecodeFrom210::Format::R210 : (cfg.pixel_format_src==PixelFormat::yuv444p10 ? DecodeFrom210::Format::V410 : DecodeFrom210::Format::V210)) : DecodeFrom210::Format::Disabled)) {
        emit errorString(last_error_string=QStringLiteral("err init format converter"));
        goto fail;
    }

    format_converter_ff->resetQueues();

    if(context->enc_num==StreamingMode) {
        context->filename=(*context->base_filename);

        avformat_alloc_output_context2(&context->av_format_context, nullptr, "flv", nullptr);

    } else {
        QString name;

        if(!context->base_filename->isEmpty())
            name=(*context->base_filename);

        else
            name=QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");


        if(context->enc_num>0)
            name+=QString("_%1").arg(context->enc_num + 1);


        context->filename=QString(QLatin1String("%1/%2.mkv"))
                .arg(context->store_dir).arg(name);

        avformat_alloc_output_context2(&context->av_format_context, nullptr, "matroska", nullptr);
    }

    if(!context->av_format_context) {
        emit errorString(last_error_string=QStringLiteral("could not deduce output format"));
        goto fail;
    }

    context->av_output_format=context->av_format_context->oformat;

    if(cfg.direct_stream_copy && cfg.input_type_flags&SourceInterface::TypeFlag::video) {
        last_error_string=add_stream_video_dsc(&context->out_stream_video, context->av_format_context, &context->av_codec_video, cfg);

        if(!last_error_string.isEmpty()) {
            emit errorString(last_error_string);
            goto fail;
        }

        last_error_string=open_video(context->av_format_context, context->av_codec_video, &context->out_stream_video, context->opt, cfg);

        if(!last_error_string.isEmpty()) {
            emit errorString(last_error_string);
            goto fail;
        }

    } else if(cfg.input_type_flags&SourceInterface::TypeFlag::video) {
        last_error_string=add_stream_video(&context->out_stream_video, context->av_format_context, &context->av_codec_video, cfg);

        if(!last_error_string.isEmpty()) {
            emit errorString(last_error_string);
            goto fail;
        }

        last_error_string=open_video(context->av_format_context, context->av_codec_video, &context->out_stream_video, context->opt, cfg);

        if(!last_error_string.isEmpty()) {
            emit errorString(last_error_string);
            goto fail;
        }
    }


    if(cfg.input_type_flags&SourceInterface::TypeFlag::audio && cfg.audio_sample_size!=0) {
        last_error_string=add_stream_audio(&context->out_stream_audio, context->av_format_context, &context->av_codec_audio, &context->audio_converter, &context->audio_buffer, &context->audio_mixer, cfg);

        if(!last_error_string.isEmpty()) {
            emit errorString(last_error_string);
            goto fail;
        }


        last_error_string=open_audio(context->av_format_context, context->av_codec_audio, &context->out_stream_audio, context->opt);

        if(!last_error_string.isEmpty()) {
            emit errorString(last_error_string);
            goto fail;
        }
    }


    av_dump_format(context->av_format_context, 0, "", 1);


    ret=avio_open(&context->av_format_context->pb, context->filename.toLatin1().constData(), AVIO_FLAG_WRITE);

    if(ret<0) {
        emit errorString(last_error_string=QString(QStringLiteral("could not open %1: %2")).arg(context->filename).arg(ffErrorString(ret)));
        goto fail;
    }

    if(cfg.input_type_flags&SourceInterface::TypeFlag::video)
        av_dict_set(&context->av_format_context->metadata, "encoding_params", configString(cfg).toLatin1().constData(), 0);

    if(!encoding_tool.isEmpty())
        av_dict_set(&context->av_format_context->metadata, "encoding_tool", encoding_tool.toLatin1().constData(), 0);

    if(context->enc_num==StreamingMode)
        av_dict_set(&context->av_format_context->metadata, "writing_date", QDateTime::currentDateTimeUtc().toString("UTC yyyy-MM-dd hh:mm:ss").toLatin1().data(), 0);

    else
        av_dict_set(&context->av_format_context->metadata, "writing_date", QDateTime::fromString(*context->base_filename, "yyyy-MM-dd_hh-mm-ss")
                    .toUTC().toString("UTC yyyy-MM-dd hh:mm:ss").toLatin1().data(), 0);


    ret=avformat_write_header(context->av_format_context, &context->opt);

    if(ret<0) {
        emit errorString(last_error_string=QStringLiteral("error occurred when opening output file: ") + ffErrorString(ret));
        goto fail;
    }

    if(cfg.input_type_flags&SourceInterface::TypeFlag::video) {
        qInfo().noquote() << "video params:" << configString(cfg);
    }


    context->cfg=cfg;

    context->skip_frame=false;

    context->dropped_frames_counter=0;
    context->double_frames_counter=0;

    context->out_stream_video.pts_next=0;
    context->out_stream_video.size_total=0;

    context->out_stream_video.pts_last=AV_NOPTS_VALUE;
    context->out_stream_audio.pts_last=AV_NOPTS_VALUE;
    context->out_stream_video.pts_start=AV_NOPTS_VALUE;
    context->out_stream_audio.pts_start=AV_NOPTS_VALUE;

    context->out_stream_video.pts_stats=0;

    context->out_stream_audio.pts_next=0;
    context->out_stream_audio.size_total=0;

    context->out_stream_audio.ba_audio_prev_part.clear();

    context->prev_stream_size_total=0;
    context->bitrate_point.clear();


    if(cfg.audio_sample_size!=0) {
        if(cfg.audio_dalay!=0)
            context->out_stream_audio.pts_next=cfg.audio_dalay/1000.*context->out_stream_audio.av_codec_context->sample_rate;
    }

    if(start_sync && context->enc_num!=StreamingMode)
        start_sync->add(context->enc_num);

    emit stateChanged(true);

    return true;

fail:

    stopCoder();

    return false;
}

int64_t FFEncoder::calcPts(int64_t pts, AVRational time_base_in, AVRational time_base_out)
{
    if(pts==AV_NOPTS_VALUE) {
        return context->out_stream_video.pts_stats=
                context->out_stream_video.pts_last=
                context->out_stream_video.pts_next++;
    }

    if(context->out_stream_video.pts_start==AV_NOPTS_VALUE) {
        context->out_stream_video.pts_start=pts;
    }

    context->out_stream_video.pts_next=pts - context->out_stream_video.pts_start;

    // context->out_stream_video.pts_next+=context->double_frames_counter;

    if(context->out_stream_video.pts_next<0) {
        qWarning() << "wrong pts" << context->out_stream_video.pts_next;
        return AV_NOPTS_VALUE;
    }

    int64_t pts_res=av_rescale_q(context->out_stream_video.pts_next,
                                 time_base_in,
                                 time_base_out);

    context->out_stream_video.pts_stats=av_rescale_q(context->out_stream_video.pts_next,
                                                     time_base_in,
                                                     context->out_stream_video.av_codec_context->time_base);

    if(context->out_stream_video.pts_last==AV_NOPTS_VALUE)
        context->out_stream_video.pts_last=pts_res;

    if(context->out_stream_video.pts_next==context->out_stream_video.pts_last && context->out_stream_video.pts_next!=0) {
        qWarning() << "double pts" << context->out_stream_video.pts_next - 1 << context->out_stream_video.pts_next;
        context->double_frames_counter++;
        context->out_stream_video.pts_next++;
    }

    const int64_t pts_dif=av_rescale_q(pts_res - context->out_stream_video.pts_last,
                                       time_base_out,
                                       context->out_stream_video.av_codec_context->time_base);

    if(pts_dif>1) {
        context->dropped_frames_counter+=pts_dif - 1;
        qWarning().noquote() << "frames dropped" << duration().toString(QStringLiteral("hh:mm:ss.zzz")) << context->dropped_frames_counter << context->out_stream_video.pts_last << pts_res << pts;

        fillDroppedFrames(pts_dif - 1);
    }

    context->out_stream_video.pts_last=pts_res;

    return pts_res;
}

bool FFEncoder::appendFrame(Frame::ptr frame)
{
    if(!context->canAcceptFrame())
        return false;

    if(start_sync) {
        if(context->enc_num==StreamingMode && !start_sync->isReady()) {
            return false;

        } else if(!start_sync->isReady()) {
            start_sync->setReady(context->enc_num);
            return false;
        }
    }


    if(frame->device_index==context->enc_num && !checkFrameParams(frame)) {
        restart(frame);
        return false;
    }


    if(context->cfg.input_type_flags&SourceInterface::TypeFlag::audio)
        if((!(context->cfg.pixel_format_src==PixelFormat::h264
             && context->out_stream_video.pts_last==AV_NOPTS_VALUE
             && frame->video.av_packet->flags!=AV_PICTURE_TYPE_I) && context->cfg.direct_stream_copy) || !context->cfg.direct_stream_copy)
            processAudio(frame);


    if(context->enc_num==StreamingMode && frame->device_index!=0)
        return true;


    if((context->cfg.input_type_flags&SourceInterface::TypeFlag::video)==0)
        return true;


    if(context->cfg.direct_stream_copy) {
        if(!frame->video.av_packet) {
            emit errorString("frame->video.av_packet null pointer");
            stopCoder();
            return false;
        }

        if(context->cfg.pixel_format_src==PixelFormat::h264
                && context->out_stream_video.pts_last==AV_NOPTS_VALUE
                && frame->video.av_packet->flags!=AV_PICTURE_TYPE_I) {
            return false;
        }

        last_error_string=write_video_packet(context->av_format_context, &context->out_stream_video, frame->video.av_packet,
                                             calcPts(frame->video.pts, frame->video.time_base, context->out_stream_video.av_stream->time_base));

        if(!last_error_string.isEmpty()) {
            emit errorString("write_video_packet error: " + last_error_string);
            stopCoder();
            return false;
        }

    } else {
        if(frame->video.data_ptr==nullptr) {
            // qDebug() << "frame->video.data_ptr nullptr";
            // context->out_stream_video.pts_next++;
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
            if(!context->canAcceptFrame())
                continue;


            context->out_stream_video.frame_current=frame_out;

            frame_out->d->pts=calcPts(frame_out->d->pts, frame_out->time_base, context->out_stream_video.av_codec_context->time_base);

            context->out_stream_video.frame_current.reset();


            AVFrame *frame_orig=context->out_stream_video.frame_converted;

            context->out_stream_video.frame_converted=frame_out->d;

            last_error_string=write_video_frame(context->av_format_context, &context->out_stream_video, frame_out->d->pts);
            context->out_stream_video.frame_converted=frame_orig;

            if(!last_error_string.isEmpty()) {
                qCritical() << "write_video_frame error: " + last_error_string;

                emit errorString("write_video_frame error: " + last_error_string);

                stopCoder();
                // restart(frame);

                return false;
            }

            if(++context->counter_process_events>10) {
                context->counter_process_events=0;
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents | QEventLoop::ExcludeSocketNotifiers | QEventLoop::X11ExcludeTimers);
            }
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
    if(context->cfg.audio_sample_size==0)
        return;


    if(frame->audio.data_size) {
        if(context->enc_num==StreamingMode) {
            context->audio_mixer.processFrame(frame);

            AudioBuffer::AudioData dtmp=context->audio_mixer.get();

            context->audio_buffer.put((uint8_t*)dtmp.data.constData(), dtmp.data.size(), dtmp.pts, dtmp.time_base);

        } else {
            context->audio_buffer.put(frame->audio.data_ptr, frame->audio.data_size, frame->audio.pts, frame->audio.time_base);
        }


        int frame_size=context->out_stream_audio.av_codec_context->frame_size;

        // if(context->cfg.audio_encoder==AudioEncoder::flac || context->cfg.audio_encoder==AudioEncoder::pcm)
        //     frame_size=context->audio_buffer.sizeSamples();

        while(context->audio_buffer.sizeSamples()>=frame_size) {
            const AudioBuffer::AudioData dtmp=context->audio_buffer.getSamples(frame_size);

            if(dtmp.data.isEmpty()) {
                return;
            }



            if(dtmp.pts!=AV_NOPTS_VALUE) {
                if(context->out_stream_audio.pts_start==AV_NOPTS_VALUE) {
                    context->out_stream_audio.pts_start=dtmp.pts;

                    context->out_stream_audio.pts_last=av_rescale_q(dtmp.pts - context->out_stream_audio.pts_start,
                                                                    dtmp.time_base,
                                                                    context->out_stream_audio.av_codec_context->time_base);
                }

                context->out_stream_audio.pts_next=av_rescale_q(dtmp.pts - context->out_stream_audio.pts_start,
                                                                dtmp.time_base,
                                                                context->out_stream_audio.av_codec_context->time_base);
            }


            AVFrame *frame=context->audio_converter.convert((void*)dtmp.data.constData(), dtmp.data.size());

            if(frame) {
                context->out_stream_audio.pts_last=frame->pts=context->out_stream_audio.pts_next;

                context->out_stream_audio.pts_next+=frame->nb_samples;

                const QString last_error_string=write_audio_frame(context->av_format_context, &context->out_stream_audio, frame);

                av_frame_free(&frame);

                if(!last_error_string.isEmpty()) {
                    stopCoder();
                    emit errorString("write_audio_frame error: " + last_error_string);
                }
            }
        }
    }
}

void FFEncoder::flushAudio()
{
    const AudioBuffer::AudioData dtmp=context->audio_buffer.get(context->audio_buffer.size());

    if(!dtmp.data.isEmpty()) {
        AVFrame *frame=context->audio_converter.convert((void*)dtmp.data.constData(), dtmp.data.size());

        if(frame) {
            context->out_stream_audio.pts_last=frame->pts=context->out_stream_audio.pts_next;

            write_audio_frame(context->av_format_context, &context->out_stream_audio, frame);

            av_frame_free(&frame);
        }
    }

    write_audio_frame(context->av_format_context, &context->out_stream_audio, nullptr);
}

void FFEncoder::restartExt()
{
    if(!context->canAcceptFrame())
        return;

    stopCoder();

    setConfig(context->cfg);
}

bool FFEncoder::stopCoder()
{
    if(context->av_format_context) {
        if(context->cfg.input_type_flags&SourceInterface::TypeFlag::audio)
            flushAudio();

        if(context->av_format_context->pb && (context->cfg.input_type_flags&SourceInterface::TypeFlag::audio
                                              || context->cfg.input_type_flags&SourceInterface::TypeFlag::video))
            av_write_trailer(context->av_format_context);
    }

    close_stream(&context->out_stream_video);
    close_stream(&context->out_stream_audio);

    if(context->av_format_context) {
        if(context->av_format_context->pb)
            avio_closep(&context->av_format_context->pb);

        avformat_free_context(context->av_format_context);
        context->av_format_context=nullptr;
    }

    emit stateChanged(false);

    return true;
}

double bitrateForLastSec(const QMap <uint64_t, uint64_t> &bitrate_point, const int &duration)
{
    if(bitrate_point.size()<2)
        return 0.;

    int index_start=bitrate_point.size() - duration - 1;

    if(index_start<0)
        index_start=0;

    const double time=(bitrate_point.keys().last() - bitrate_point.keys()[index_start])*.001;
    const double size=bitrate_point.last() - bitrate_point.values()[index_start];

    return size/time;
}

void FFEncoder::calcStats()
{
    if(!context->canAcceptFrame())
        return;

    Stats s;

    s.enc_num=context->enc_num;

    if(context->out_stream_audio.av_stream) {
        double cf_a=av_stream_get_end_pts(context->out_stream_audio.av_stream)*av_q2d(context->out_stream_audio.av_stream->time_base);

        if(cf_a<.01)
            cf_a=.01;

        if(context->out_stream_audio.pts_last!=AV_NOPTS_VALUE)
            s.time=QTime(0, 0).addMSecs((double)context->out_stream_audio.pts_last/(double)context->out_stream_audio.av_codec_context->sample_rate*1000);

        else if(context->out_stream_video.av_codec_context)
            s.time=QTime(0, 0).addMSecs((double)context->out_stream_video.pts_stats*av_q2d(context->out_stream_video.av_codec_context->time_base)*1000);

        s.avg_bitrate_audio=(double)(context->out_stream_audio.size_total*8)/cf_a;
        s.streams_size=context->out_stream_audio.size_total;

    } else {
        s.time=QTime(0, 0).addMSecs((double)context->out_stream_video.pts_stats*av_q2d(context->out_stream_video.av_codec_context->time_base)*1000);
    }

    if(context->out_stream_video.av_stream) {
        double cf_v=av_stream_get_end_pts(context->out_stream_video.av_stream)*av_q2d(context->out_stream_video.av_stream->time_base);

        if(cf_v<.01)
            cf_v=.01;

        uint64_t t=av_stream_get_end_pts(context->out_stream_video.av_stream)*av_q2d(context->out_stream_video.av_stream->time_base)*1000;

        static QList <int> point=QList<int>() << 1 << 2 << 10 << 30 << 60;

        static uint64_t max_time=(*std::max_element(point.begin(), point.end()) + 1)*1000;

        foreach(const uint64_t &p, context->bitrate_point.keys()) {
            if((t - p)>max_time)
                context->bitrate_point.remove(p);

            else
                break;
        }

        context->bitrate_point[t]=context->out_stream_video.size_total;
        context->prev_stream_size_total=context->out_stream_video.size_total;

        foreach(int p, point) {
            s.bitrate_video[p]=bitrateForLastSec(context->bitrate_point, p);
        }

        s.avg_bitrate_video=(double)(context->out_stream_video.size_total*8)/cf_v;
        s.streams_size+=context->out_stream_video.size_total;
        s.dropped_frames_counter=context->dropped_frames_counter;
    }

    emit stats(s);
}

QTime FFEncoder::duration()
{
    if(context->out_stream_audio.av_stream && context->cfg.audio_sample_size!=0)
        return QTime(0, 0).addMSecs((double)context->out_stream_audio.pts_last/(double)context->out_stream_audio.av_codec_context->sample_rate*1000);

    return QTime(0, 0).addMSecs((double)av_stream_get_end_pts(context->out_stream_video.av_stream)*av_q2d(context->out_stream_video.av_stream->time_base)*1000);
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

    case qsv_hevc:
        return QLatin1String("qsv_hevc");

    case vaapi_h264:
        return QLatin1String("vaapi_h264");

    case vaapi_hevc:
        return QLatin1String("vaapi_hevc");

    case vaapi_vp8:
        return QLatin1String("vaapi_vp8");

    case vaapi_vp9:
        return QLatin1String("vaapi_vp9");

    case ffvhuff:
        return QLatin1String("ffvhuff");

    case magicyuv:
        return QLatin1String("magicyuv");
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

    case qsv_hevc:
        return QLatin1String("hevc_qsv");

    case vaapi_h264:
        return QLatin1String("h264_vaapi");

    case vaapi_hevc:
        return QLatin1String("hevc_vaapi");

    case vaapi_vp8:
        return QLatin1String("vp8_vaapi");

    case vaapi_vp9:
        return QLatin1String("vp9_vaapi");

    case ffvhuff:
        return QLatin1String("ffvhuff");

    case magicyuv:
        return QLatin1String("magicyuv");
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

    else if(value==QLatin1String("qsv_hevc"))
        return qsv_hevc;

    else if(value==QLatin1String("vaapi_h264"))
        return vaapi_h264;

    else if(value==QLatin1String("vaapi_hevc"))
        return vaapi_hevc;

    else if(value==QLatin1String("vaapi_vp8"))
        return vaapi_vp8;

    else if(value==QLatin1String("vaapi_vp9"))
        return vaapi_vp9;

    else if(value==QLatin1String("ffvhuff"))
        return ffvhuff;

    else if(value==QLatin1String("magicyuv"))
        return magicyuv;

    return 0;
}

QList <FFEncoder::VideoEncoder::T> FFEncoder::VideoEncoder::list()
{
    static QList <FFEncoder::VideoEncoder::T> res=
            QList <FFEncoder::VideoEncoder::T>() << libx264 << libx264rgb
                                                 << nvenc_h264 << nvenc_hevc
                                                 << qsv_h264 << qsv_hevc
                                                 << vaapi_h264 << vaapi_hevc << vaapi_vp8 << vaapi_vp9
                                                 << ffvhuff << magicyuv;

    return res;
}

QString FFEncoder::AudioEncoder::toString(uint32_t enc)
{
    switch(enc) {
    case pcm:
        return QLatin1String("pcm");

    case flac:
        return QLatin1String("flac");

    case opus:
        return QLatin1String("opus");

    case vorbis:
        return QLatin1String("vorbis");

    case aac:
        return QLatin1String("aac");
    }

    return QLatin1String("");
}

QString FFEncoder::AudioEncoder::toEncName(uint32_t enc)
{
    switch(enc) {
    case opus:
        return QLatin1String("libopus");

    case vorbis:
        return QLatin1String("libvorbis");

    case aac:
        // return QLatin1String("libfdk_aac");
        break;
    }

    return toString(enc);
}

uint64_t FFEncoder::AudioEncoder::fromString(QString value)
{
    if(value==QLatin1String("pcm"))
        return pcm;

    else if(value==QLatin1String("flac"))
        return flac;

    else if(value==QLatin1String("opus"))
        return opus;

    else if(value==QLatin1String("vorbis"))
        return vorbis;

    else if(value==QLatin1String("aac"))
        return aac;

    return pcm;
}

QList <FFEncoder::AudioEncoder::T> FFEncoder::AudioEncoder::list()
{
    static QList <FFEncoder::AudioEncoder::T> res=
            QList <FFEncoder::AudioEncoder::T>() << pcm << flac
                                                 << opus << vorbis
                                                 << aac;

    return res;
}

AVSampleFormat FFEncoder::AudioEncoder::sampleFmt(uint32_t enc)
{
    switch(enc) {
    case opus:
        return AV_SAMPLE_FMT_S16;

    case vorbis:
    case aac:
        return AV_SAMPLE_FMT_FLTP;
    }

    return AV_SAMPLE_FMT_S16;
}

bool FFEncoder::AudioEncoder::setBitrate(uint32_t enc)
{
    switch(enc) {
    case opus:
        return true;

    case vorbis:
        return true;

    case aac:
        return true;
    }

    return false;
}

int FFEncoder::DownScale::toWidth(uint32_t value)
{
    switch(value) {
    case to720:
        return 720;

    case to900:
        return 900;

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

    case to900:
        return QLatin1String("900p");

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

