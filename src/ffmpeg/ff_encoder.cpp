#include <QApplication>
#include <QDebug>
#include <QThread>
#include <QFile>
#include <QDir>

#ifdef __linux__

#include <x264_config.h>

#endif

#include "ff_tools.h"
#include "ff_format_converter.h"
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

        convert_context=nullptr;
    }

    AVStream *av_stream;
    AVCodecContext *av_codec_context;

    // pts of the next frame
    int64_t next_pts;

    QByteArray ba_audio_prev_part;

    AVPixelFormat frame_fmt;

    AVFrame *frame;
    AVFrame *frame_converted;

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
    case FFEncoder::VideoEncoder::libx264_10bit:
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

    // timebase: This is the fundamental unit of time (in seconds) in terms
    // of which frame timestamps are represented. For fixed-fps content,
    // timebase should be 1/framerate and timestamp increments should be
    // identical to 1

    switch(cfg.framerate) {
    case FFEncoder::Framerate::full_11:
        out_stream->av_stream->time_base={ 1001, 12000 };
        break;

    case FFEncoder::Framerate::full_12:
        out_stream->av_stream->time_base={ 1000, 12000 };
        break;

    case FFEncoder::Framerate::full_14:
        out_stream->av_stream->time_base={ 1001, 15000 };
        break;

    case FFEncoder::Framerate::full_15:
        out_stream->av_stream->time_base={ 1000, 15000 };
        break;

    case FFEncoder::Framerate::full_23:
        out_stream->av_stream->time_base={ 1001, 24000 };
        break;

    case FFEncoder::Framerate::full_24:
        out_stream->av_stream->time_base={ 1000, 24000 };
        break;

    case FFEncoder::Framerate::full_25:
    case FFEncoder::Framerate::half_50:
        out_stream->av_stream->time_base={ 1000, 25000 };
        break;

    case FFEncoder::Framerate::full_29:
    case FFEncoder::Framerate::half_59:
        out_stream->av_stream->time_base={ 1001, 30000 };
        break;

    case FFEncoder::Framerate::full_30:
    case FFEncoder::Framerate::half_60:
        out_stream->av_stream->time_base={ 1000, 30000 };
        break;

    case FFEncoder::Framerate::full_50:
        out_stream->av_stream->time_base={ 1000, 50000 };
        break;

    case FFEncoder::Framerate::full_59:
        out_stream->av_stream->time_base={ 1001, 60000 };
        break;

    case FFEncoder::Framerate::full_60:
        out_stream->av_stream->time_base={ 1000, 60000 };
        break;

    default:
        out_stream->av_stream->time_base={ 1000, 30000 };
        break;
    }



    c->time_base=out_stream->av_stream->time_base;

    c->gop_size=12; // emit one intra frame every twelve frames at most

    c->pix_fmt=cfg.pixel_format;

    if(cfg.video_encoder==FFEncoder::VideoEncoder::libx264 || cfg.video_encoder==FFEncoder::VideoEncoder::libx264rgb) {
        av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        av_opt_set(c->priv_data, "crf", QString::number(cfg.crf).toLatin1().constData(), 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_h264) {
        if(cfg.crf==0) {
            av_opt_set(c->priv_data, "preset", "lossless", 0);

        } else {
            av_opt_set(c->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
            av_opt_set(c->priv_data, "rc", "constqp", 0);
            av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        }

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_hevc) {
        av_opt_set(c->priv_data, "qp", QString::number(cfg.crf).toLatin1().constData(), 0);
        av_opt_set(c->priv_data, "rc", "constqp", 0);
        av_opt_set(c->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

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

    // c->thread_count=8;


    if(c->codec_id==AV_CODEC_ID_MPEG2VIDEO) {
        // just for testing, we also add B-frames
        c->max_b_frames=2;
    }

    if(c->codec_id==AV_CODEC_ID_MPEG1VIDEO) {
        // needed to avoid using macroblocks in which some coeffs overflow.
        // this does not happen with normal video, it just happens here as
        // the motion of the chroma plane does not match the luma plane
        c->mb_decision=2;
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

    return QStringLiteral("");
}

static QString write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    AVPacket *pkt=av_packet_alloc();

    ret=avcodec_send_frame(ost->av_codec_context, ost->frame);

    if(ret<0)
        return QStringLiteral("error encoding audio frame: ") + ffErrorString(ret);

    while(!ret) {
        ret=avcodec_receive_packet(ost->av_codec_context, pkt);

        if(!ret) {
            ost->size_total+=pkt->size;

            write_frame(oc, &ost->av_codec_context->time_base, ost->av_stream, pkt);
        }
    }

    av_packet_free(&pkt);

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
        return QStringLiteral("could not open video codec: ") + ffErrorString(ret) + QStringLiteral("\nunsupported pixel format?");
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


    return QStringLiteral("");
}

static QString write_video_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;
    int ret_2;

    AVPacket *pkt=av_packet_alloc();

    ret=avcodec_send_frame(ost->av_codec_context, ost->frame_converted);

    if(ret<0)
       return QStringLiteral("error encoding video frame: ") + ffErrorString(ret);


    while(!ret) {
        ret=avcodec_receive_packet(ost->av_codec_context, pkt);

        if(!ret) {
            ost->size_total+=pkt->size;

            ret_2=write_frame(oc, &ost->av_codec_context->time_base, ost->av_stream, pkt);

            if(ret_2!=0) {
                av_packet_free(&pkt);

                return ffErrorString(ret_2);
            }
        }
    }

    av_packet_free(&pkt);

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

FFEncoder::FFEncoder(QObject *parent) :
    QObject(parent)
{
    context=new FFMpegContext();

    format_converter_ff=new FFFormatConverter();
    format_converter_dl=new DecklinkFrameConverter();
}

FFEncoder::~FFEncoder()
{
    stopCoder();

    delete context;

    delete format_converter_ff;
    delete format_converter_dl;
}

void FFEncoder::init()
{
    qRegisterMetaType<FFEncoder::Config>("FFEncoder::Config");
    qRegisterMetaType<FFEncoder::Stats>("FFEncoder::Stats");

    av_register_all();
}

bool FFEncoder::isLib_x264_10bit()
{
#ifdef __linux__

    return X264_BIT_DEPTH==10;

#endif

    return false;
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

        case 50000:
            return Framerate::half_50;

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

        case 50000:
            return Framerate::full_50;

        case 60000:
            return frame_duration==1000 ? Framerate::full_60: Framerate::full_59;
        }
    }

    return Framerate::full_30;
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
    case VideoEncoder::libx264_10bit:
        return QStringList() << QLatin1String("ultrafast") << QLatin1String("superfast") << QLatin1String("veryfast") << QLatin1String("faster")
                             << QLatin1String("fast") << QLatin1String("medium") << QLatin1String("slow");

    case VideoEncoder::nvenc_h264:
    case VideoEncoder::nvenc_hevc:
        return QStringList() << QLatin1String("high quality") << QLatin1String("high performance") << QLatin1String("bluray disk") << QLatin1String("low latency")
                             << QLatin1String("low latency high quality") << QLatin1String("low latency high performance")
                             << QLatin1String("slow") << QLatin1String("medium") << QLatin1String("fast") << QLatin1String("default");

    case VideoEncoder::qsv_h264:
        return QStringList() << QLatin1String("veryfast") << QLatin1String("faster") << QLatin1String("fast")
                             << QLatin1String("medium")
                             << QLatin1String("slow") << QLatin1String("slower") << QLatin1String("veryslow");
    }

    return QStringList() << QLatin1String("--");
}

bool FFEncoder::setConfig(FFEncoder::Config cfg)
{
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



    if(!format_converter_ff->setup(context->out_stream_video.frame_fmt, cfg.frame_resolution_src, cfg.pixel_format, cfg.frame_resolution_src, false)) {
        emit errorString(last_error_string=QStringLiteral("err init format converter"));
        goto fail;
    }


    if(cfg.rgb_source && cfg.rgb_10bit) {
        format_converter_dl->init(bmdFormat10BitRGB, cfg.frame_resolution_src, bmdFormat8BitBGRA, cfg.frame_resolution_src);
    }


    {
        QDir dir(QApplication::applicationDirPath() + "/videos");

        if(!dir.exists())
            dir.mkdir(dir.dirName());
    }

    context->filename=QString(QLatin1String("%1/videos/%2.mkv"))
            .arg(QApplication::applicationDirPath(), QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));

    // context->filename="/dev/null";


    // allocate the output media context
    // avformat_alloc_output_context2(&context->av_format_context, nullptr, nullptr, context->filename.toLatin1().constData());
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
//        close_stream(context->av_format_context, &context->out_stream_video);
//        context->av_format_context=nullptr;
        emit errorString(last_error_string);
        goto fail;
//        return false;
    }


    last_error_string=add_stream_audio(&context->out_stream_audio, context->av_format_context, &context->av_codec_audio, cfg);

    if(!last_error_string.isEmpty()) {
//        close_stream(context->av_format_context, &context->out_stream_video);
//        close_stream(context->av_format_context, &context->out_stream_audio);
//        context->av_format_context=nullptr;
        emit errorString(last_error_string);
        goto fail;
//        return false;
    }

    // now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers
    last_error_string=open_video(context->av_format_context, context->av_codec_video, &context->out_stream_video, context->opt, cfg);

    if(!last_error_string.isEmpty()) {
//        close_stream(context->av_format_context, &context->out_stream_video);
//        close_stream(context->av_format_context, &context->out_stream_audio);
//        context->av_format_context=nullptr;
        emit errorString(last_error_string);
        goto fail;
//        return false;
    }


    last_error_string=open_audio(context->av_format_context, context->av_codec_audio, &context->out_stream_audio, context->opt);

    if(!last_error_string.isEmpty()) {
//        close_stream(context->av_format_context, &context->out_stream_video);
//        close_stream(context->av_format_context, &context->out_stream_audio);
//        context->av_format_context=nullptr;
        emit errorString(last_error_string);
        goto fail;
//        return false;
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
//        close_stream(context->av_format_context, &context->out_stream_video);
//        close_stream(context->av_format_context, &context->out_stream_audio);
//        context->av_format_context=nullptr;
        emit errorString(last_error_string=QString(QStringLiteral("could not open %1: %2")).arg(context->filename).arg(ffErrorString(ret)));
        goto fail;
//        return false;
    }


    if(cfg.video_encoder!=VideoEncoder::ffvhuff)
        av_dict_set(&context->av_format_context->metadata, "crf/quality", QString::number(cfg.crf).toLatin1().constData(), 0);

    if(cfg.downscale!=DownScale::Disabled)
        av_dict_set(&context->av_format_context->metadata, "scale filter", ScaleFilter::toString(cfg.scale_filter).toLatin1().constData(), 0);


    // write the stream header, if any
    ret=avformat_write_header(context->av_format_context, &context->opt);

    if(ret<0) {
//        close_stream(context->av_format_context, &context->out_stream_video);
//        close_stream(context->av_format_context, &context->out_stream_audio);
//        context->av_format_context=nullptr;
        emit errorString(last_error_string=QStringLiteral("error occurred when opening output file: ") + ffErrorString(ret));
        goto fail;
//        return false;
    }


    context->cfg=cfg;

    context->skip_frame=false;

    context->out_stream_audio.next_pts=0;

    context->out_stream_video.next_pts=0;

    context->out_stream_audio.size_total=0;

    context->out_stream_video.size_total=0;


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


    // video
    {
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
            uint8_t *ptr_orig=context->out_stream_video.frame->data[0];

            if(frame->video.rgb && frame->video.rgb_10bit) {
                format_converter_dl->convert(frame->video.ptr_data, context->out_stream_video.frame->data[0]);

            } else {
                context->out_stream_video.frame->data[0]=(uint8_t*)frame->video.ptr_data;

            }


            sws_scale(context->out_stream_video.convert_context, context->out_stream_video.frame->data, context->out_stream_video.frame->linesize, 0, context->out_stream_video.frame->height, context->out_stream_video.frame_converted->data, context->out_stream_video.frame_converted->linesize);

            context->out_stream_video.frame_converted->pts=context->out_stream_video.next_pts++;

            last_error_string=write_video_frame(context->av_format_context, &context->out_stream_video);

            context->out_stream_video.frame->data[0]=ptr_orig;

            if(!last_error_string.isEmpty()) {
                stopCoder();
                emit errorString(last_error_string);
                return false;
            }
        }
    }

    // audio
    {
        QByteArray ba_audio=QByteArray(frame->audio.ptr_data, frame->audio.data_size);

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
            emit errorString(last_error_string=QStringLiteral("could not setup audio frame"));
            return false;
        }

        context->out_stream_audio.frame->pts=context->out_stream_audio.next_pts;

        context->out_stream_audio.next_pts+=context->out_stream_audio.frame->nb_samples;

        write_audio_frame(context->av_format_context, &context->out_stream_audio);

        context->out_stream_audio.frame->nb_samples=default_nb_samples;
    }

    if(QDateTime::currentMSecsSinceEpoch() - context->last_stats_update_time>1000) {
        context->last_stats_update_time=QDateTime::currentMSecsSinceEpoch();

        calcStats();
    }

    return true;
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

    case YUV420P:
        return QLatin1String("yuv420p");

    case YUV422P:
        return QLatin1String("yuv422p");

    case UYVY422:
        return QLatin1String("uyvy422p");

    case YUV444P:
        return QLatin1String("yuv444p");

    case YUV420P10:
        return QLatin1String("yuv420p10");

    // case YUV422P10:
    //     return QLatin1String("yuv422p10");

    case YUV444P10:
        return QLatin1String("yuv444p10");

    case V210:
        return QLatin1String("yuv422p10le (v210)");

    case R210:
        return QLatin1String("r210");

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

    else if(value==QLatin1String("yuv420p"))
        return YUV420P;

    else if(value==QLatin1String("yuv422p"))
        return YUV422P;

    else if(value==QLatin1String("uyvy422p"))
        return UYVY422;

    else if(value==QLatin1String("yuv444p"))
        return YUV444P;

    else if(value==QLatin1String("yuv420p10"))
        return YUV420P10;

    // else if(value==QLatin1String("yuv422p10"))
    //     return YUV422P10;

    else if(value==QLatin1String("yuv444p10"))
        return YUV444P10;

    else if(value==QLatin1String("yuv422p10le (v210)"))
        return V210;

    else if(value==QLatin1String("r210"))
        return R210;

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

    case VideoEncoder::libx264_10bit:
        return QList<T>() << YUV420P10 << YUV444P10;

    case VideoEncoder::libx264rgb:
        return QList<T>() << RGB24;

    case VideoEncoder::nvenc_h264:
        return QList<T>() << YUV420P << YUV444P;

    case VideoEncoder::nvenc_hevc:
        return QList<T>() << YUV420P;

    case VideoEncoder::qsv_h264:
        return QList<T>() /*<< P010LE*/ << NV12;

    case VideoEncoder::ffvhuff:
        return QList<T>() << RGB24 << YUV420P << YUV422P << YUV444P << YUV420P10 /*<< YUV422P10*/ << YUV444P10 << V210;
    }

    return QList<T>() << RGB24 << YUV420P << YUV444P << YUV420P10 << YUV444P10;
}

QString FFEncoder::VideoEncoder::toString(uint32_t enc)
{
    switch(enc) {
    case libx264:
        return QLatin1String("libx264");

    case libx264_10bit:
        return QLatin1String("libx264_10bit");

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
