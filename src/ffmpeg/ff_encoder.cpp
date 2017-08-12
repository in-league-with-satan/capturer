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

#include "ff_encoder.h"

class OutputStream
{
public:
    OutputStream() {
        stream=nullptr;
        codec_context=nullptr;

        next_pts=0;

        frame=nullptr;
        frame_converted=nullptr;

        convert_context=nullptr;
    }

    AVStream *stream;
    AVCodecContext *codec_context;

    // pts of the next frame
    int64_t next_pts;

    QByteArray ba_audio_prev_part;

    AVFrame *frame;
    AVFrame *frame_converted;

    SwsContext *convert_context;

    uint64_t size_total;
};

class FFMpegContext
{
public:
    FFMpegContext() {
        format_context=nullptr;

        codec_video_a=nullptr;
        codec_video_b=nullptr;
        codec_audio=nullptr;

        skip_frame=false;

        last_stats_update_time=0;
    }

    bool canAcceptFrame() {
        if(format_context)
            return true;

        return false;
    }

    QString filename;

    OutputStream out_stream_video_a;
    OutputStream out_stream_video_b;
    OutputStream out_stream_audio;

    AVFormatContext *format_context;

    AVCodec *codec_video_a;
    AVCodec *codec_video_b;
    AVCodec *codec_audio;

    FFEncoder::Config cfg;

    bool skip_frame;

    qint64 last_stats_update_time;
    size_t frame_counter;
};

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    // rescale output packet timestamp values from codec to stream timebase
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index=st->index;

    // write the compressed frame to the media file
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

static void add_stream_audio(OutputStream *out_stream, AVFormatContext *format_context, AVCodec **codec, const FFEncoder::Config &cfg)
{
    AVCodecContext *c;

    // find the encoder
    if(cfg.audio_sample_size==16)
        *codec=avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);

    else
        *codec=avcodec_find_encoder(AV_CODEC_ID_PCM_S32LE);

    if(!(*codec)) {
        qCritical() << "could not find encoder for PCM_S16LE";
        exit(1);
    }

    out_stream->stream=avformat_new_stream(format_context, nullptr);

    if(!out_stream->stream) {
        qCritical() << "could not allocate stream";
        exit(1);
    }

    out_stream->stream->id=format_context->nb_streams - 1;

    c=avcodec_alloc_context3(*codec);

    if(!c) {
        qCritical() << "could not alloc an encoding context";
        exit(1);
    }

    out_stream->codec_context=c;

    c->sample_fmt=(*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

    // c->bit_rate=480000;

    c->sample_rate=48000;

    switch(cfg.audio_channels_size) {
    case 6:
    case 8:
        c->channel_layout=AV_CH_LAYOUT_7POINT1;
        // c->channel_layout=AV_CH_LAYOUT_7POINT1_WIDE_BACK;
        break;

    case 2:
    default:
        c->channel_layout=AV_CH_LAYOUT_STEREO;
        break;
    }

    c->channels=av_get_channel_layout_nb_channels(c->channel_layout);


    out_stream->stream->time_base={ 1, c->sample_rate };


    // some formats want stream headers to be separate
    if(format_context->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;
}

static void add_stream_video(OutputStream *out_stream, AVFormatContext *format_context, AVCodec **codec, const FFEncoder::Config &cfg)
{
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

    default:
        break;
    }

    if(!(*codec)) {
        qCritical() << "could not find encoder";
        exit(1);
    }

    out_stream->stream=avformat_new_stream(format_context, nullptr);

    if(!out_stream->stream) {
        qCritical() << "could not allocate stream";
        exit(1);
    }

    out_stream->stream->id=format_context->nb_streams - 1;

    out_stream->codec_context=avcodec_alloc_context3(*codec);

    if(!out_stream->codec_context) {
        qCritical() << "could not alloc an encoding context";
        exit(1);
    }

    out_stream->codec_context->codec_id=(*codec)->id;

    out_stream->codec_context->width=cfg.frame_resolution.width();
    out_stream->codec_context->height=cfg.frame_resolution.height();

    // timebase: This is the fundamental unit of time (in seconds) in terms
    // of which frame timestamps are represented. For fixed-fps content,
    // timebase should be 1/framerate and timestamp increments should be
    // identical to 1

    if(cfg.split_odd_even_frames) {
        switch(cfg.framerate) {
        case FFEncoder::Framerate::full_23:
            out_stream->stream->time_base= { 1001, 12000 };
            break;

        case FFEncoder::Framerate::full_24:
            out_stream->stream->time_base={ 1000, 12000 };
            break;

        case FFEncoder::Framerate::full_25:
        case FFEncoder::Framerate::half_50:
            out_stream->stream->time_base={ 1000, 12500 };
            break;

        case FFEncoder::Framerate::full_29:
        case FFEncoder::Framerate::half_59:
            out_stream->stream->time_base={ 1001, 15000 };
            break;

        case FFEncoder::Framerate::full_30:
        case FFEncoder::Framerate::half_60:
            out_stream->stream->time_base={ 1000, 15000 };
            break;

        case FFEncoder::Framerate::full_50:
            out_stream->stream->time_base={ 1000, 25000 };
            break;

        case FFEncoder::Framerate::full_59:
            out_stream->stream->time_base={ 1001, 30000 };
            break;

        case FFEncoder::Framerate::full_60:
            out_stream->stream->time_base={ 1000, 30000 };
            break;

        default:
            out_stream->stream->time_base={ 1000, 15000 };
            break;
        }

    } else {
        switch(cfg.framerate) {
        case FFEncoder::Framerate::full_23:
            out_stream->stream->time_base= { 1001, 24000 };
            break;

        case FFEncoder::Framerate::full_24:
            out_stream->stream->time_base={ 1000, 24000 };
            break;

        case FFEncoder::Framerate::full_25:
        case FFEncoder::Framerate::half_50:
            out_stream->stream->time_base={ 1000, 25000 };
            break;

        case FFEncoder::Framerate::full_29:
        case FFEncoder::Framerate::half_59:
            out_stream->stream->time_base={ 1001, 30000 };
            break;

        case FFEncoder::Framerate::full_30:
        case FFEncoder::Framerate::half_60:
            out_stream->stream->time_base={ 1000, 30000 };
            break;

        case FFEncoder::Framerate::full_50:
            out_stream->stream->time_base={ 1000, 50000 };
            break;

        case FFEncoder::Framerate::full_59:
            out_stream->stream->time_base={ 1001, 60000 };
            break;

        case FFEncoder::Framerate::full_60:
            out_stream->stream->time_base={ 1000, 60000 };
            break;

        default:
            out_stream->stream->time_base={ 1000, 30000 };
            break;
        }
    }


    out_stream->codec_context->framerate=
            out_stream->stream->time_base;

    out_stream->codec_context->time_base=out_stream->stream->time_base;

    out_stream->codec_context->gop_size=12; // emit one intra frame every twelve frames at most

    out_stream->codec_context->pix_fmt=cfg.pixel_format;

    if(cfg.video_encoder==FFEncoder::VideoEncoder::libx264 || cfg.video_encoder==FFEncoder::VideoEncoder::libx264rgb) {
        // av_opt_set(c->priv_data, "preset", "ultrafast", 0);
        av_opt_set(out_stream->codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        // av_opt_set(c->priv_data, "tune", "zerolatency", 0);
        av_opt_set(out_stream->codec_context->priv_data, "crf", QString::number(cfg.crf).toLatin1().constData(), 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_h264) {
        out_stream->codec_context->bit_rate=0;

        if(cfg.crf==0) {
            av_opt_set(out_stream->codec_context->priv_data, "preset", "lossless", 0);

        } else {
            out_stream->codec_context->global_quality=cfg.crf;

            // av_opt_set(c->priv_data, "preset", "fast", 0); // HP
            // av_opt_set(c->priv_data, "preset", "slow", 0); // HQ
            av_opt_set(out_stream->codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);
        }

        // av_opt_set(c->priv_data, "tune", "zerolatency", 0);

    } else if(cfg.video_encoder==FFEncoder::VideoEncoder::nvenc_hevc) {
        out_stream->codec_context->bit_rate=0;
        out_stream->codec_context->global_quality=cfg.crf;

        // av_opt_set(c->priv_data, "preset", "fast", 0); // HP
        // av_opt_set(c->priv_data, "preset", "slow", 0); // HQ
        av_opt_set(out_stream->codec_context->priv_data, "preset", cfg.preset.toLatin1().constData(), 0);

        // av_opt_set(c->priv_data, "tune", "zerolatency", 0);
    }

    // c->thread_count=8;

    if(out_stream->codec_context->codec_id==AV_CODEC_ID_MPEG2VIDEO) {
        // just for testing, we also add B-frames
        out_stream->codec_context->max_b_frames=2;
    }

    if(out_stream->codec_context->codec_id==AV_CODEC_ID_MPEG1VIDEO) {
        // needed to avoid using macroblocks in which some coeffs overflow.
        // this does not happen with normal video, it just happens here as
        // the motion of the chroma plane does not match the luma plane
        out_stream->codec_context->mb_decision=2;
    }

    // some formats want stream headers to be separate
    if(format_context->oformat->flags & AVFMT_GLOBALHEADER)
        out_stream->codec_context->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;
}

// audio output
static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
    AVFrame *frame=av_frame_alloc();

    int ret;

    if(!frame) {
        qCritical() << "error allocating an audio frame";
        exit(1);
    }

    frame->format=sample_fmt;
    frame->channel_layout=channel_layout;
    frame->sample_rate=sample_rate;
    frame->nb_samples=nb_samples;

    if(nb_samples) {
        ret=av_frame_get_buffer(frame, 0);

        if(ret<0) {
            qCritical() << "error allocating an audio buffer";
            exit(1);
        }
    }

    return frame;
}

static void open_audio(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    Q_UNUSED(oc)

    AVCodecContext *c;

    int nb_samples;
    int ret;

    AVDictionary *opt=nullptr;

    c=ost->codec_context;

    // open it
    av_dict_copy(&opt, opt_arg, 0);

    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        qCritical() << "could not open audio codec:" << ffErrorString(ret);
        return;
    }


    nb_samples=10000;

    ost->frame=alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);

    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->stream->codecpar, c);

    if(ret<0) {
        qCritical() << "could not copy the stream parameters";
        exit(1);
    }
}

static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    AVPacket *pkt=av_packet_alloc();

    ret=avcodec_send_frame(ost->codec_context, ost->frame);

    if(ret<0) {
        qCritical() << "error encoding audio frame" << ffErrorString(ret);
        exit(1);
    }

    while(!ret) {
        ret=avcodec_receive_packet(ost->codec_context, pkt);

        if(!ret) {
            ost->size_total+=pkt->size;

            write_frame(oc, &ost->codec_context->time_base, ost->stream, pkt);
        }
    }

    av_packet_free(&pkt);

    return 0;
}

void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg, FFEncoder::Config cfg)
{
    Q_UNUSED(oc)

    int ret;

    AVCodecContext *c=ost->codec_context;
    AVDictionary *opt=nullptr;

    av_dict_copy(&opt, opt_arg, 0);

    // open the codec
    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        qCritical() << "could not open video codec:" << ffErrorString(ret);
        exit(1);
    }

    // allocate and init a re-usable frame
    ost->frame=alloc_frame(AV_PIX_FMT_BGRA, c->width, c->height);

    if(!ost->frame) {
        qCritical() << "could not allocate video frame";
        exit(1);
    }

    // allocate and init a re-usable frame
    ost->frame_converted=alloc_frame(cfg.pixel_format, c->width, c->height);

    if(!ost->frame_converted) {
        qCritical() << "Could not allocate video frame";
        exit(1);
    }

    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->stream->codecpar, c);

    if(ret<0) {
        qCritical() << "could not copy the stream parameters";
        exit(1);
    }
}

static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    AVPacket *pkt=av_packet_alloc();

    ret=avcodec_send_frame(ost->codec_context, ost->frame_converted);

    if(ret<0) {
        qCritical() << "error encoding video frame:" << ffErrorString(ret);
        exit(1);
    }

    while(!ret) {
        ret=avcodec_receive_packet(ost->codec_context, pkt);

        if(!ret) {
            ost->size_total+=pkt->size;

            write_frame(oc, &ost->codec_context->time_base, ost->stream, pkt);
        }
    }

    av_packet_free(&pkt);

    return 0;
}

static void close_stream(OutputStream *ost)
{
    if(ost->codec_context)
        avcodec_free_context(&ost->codec_context);

    if(ost->frame)
        av_frame_free(&ost->frame);

    if(ost->frame_converted)
        av_frame_free(&ost->frame_converted);

    if(ost->codec_context)
        sws_freeContext(ost->convert_context);
}

// ------------------------------

FFEncoder::FFEncoder(QObject *parent) :
    QObject(parent)
{
    context=new FFMpegContext();

    converter=new FF::FormatConverter();
}

FFEncoder::~FFEncoder()
{
    stopCoder();

    delete context;

    delete converter;
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

int FFEncoder::calcFrameDuration(FFEncoder::Framerate::T framerate)
{
    switch(framerate) {
    case FFEncoder::Framerate::full_23:
        return 1001;

    case FFEncoder::Framerate::full_24:
        return 1000;

    case FFEncoder::Framerate::full_25:
        return 1000;

    case FFEncoder::Framerate::full_29:
        return 1001;

    case FFEncoder::Framerate::full_30:
        return 1000;

    case FFEncoder::Framerate::half_50:
        return 1000;

    case FFEncoder::Framerate::half_59:
        return 1001;

    case FFEncoder::Framerate::half_60:
        return 1000;

    case FFEncoder::Framerate::full_50:
        return 1000;

    case FFEncoder::Framerate::full_59:
        return 1001;

    case FFEncoder::Framerate::full_60:
        return 1000;
    }

    return 1000;
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
    }

    return QStringList() << QLatin1String("fast");
}

bool FFEncoder::setConfig(FFEncoder::Config cfg)
{
    int ret;

    if(!converter->setup(AV_PIX_FMT_BGRA, cfg.frame_resolution, cfg.pixel_format, cfg.frame_resolution)) {
        qCritical() << "err init format converter" << cfg.frame_resolution;
        return false;
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
    avformat_alloc_output_context2(&context->format_context, nullptr, "matroska", nullptr);

    if(!context->format_context) {
        qCritical() << "could not deduce output format";
        return false;
    }


    // add the audio and video streams using the default format codecs
    // and initialize the codecs.
    add_stream_video(&context->out_stream_video_a, context->format_context, &context->codec_video_a, cfg);

    if(cfg.split_odd_even_frames) {
        add_stream_video(&context->out_stream_video_b, context->format_context, &context->codec_video_b, cfg);
    }

    add_stream_audio(&context->out_stream_audio, context->format_context, &context->codec_audio, cfg);


    // now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers
    open_video(context->format_context, context->codec_video_a, &context->out_stream_video_a, nullptr, cfg);

    if(cfg.split_odd_even_frames) {
        open_video(context->format_context, context->codec_video_b, &context->out_stream_video_b, nullptr, cfg);

        av_dict_set(&context->out_stream_video_a.stream->metadata, "title", "odd", 0);
        av_dict_set(&context->out_stream_video_b.stream->metadata, "title", "even", 0);
    }

    open_audio(context->format_context, context->codec_audio, &context->out_stream_audio, nullptr);



    context->out_stream_video_a.convert_context=sws_getContext(cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                                               AV_PIX_FMT_BGRA,
                                                               cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                                               cfg.pixel_format,
                                                               SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);


    context->out_stream_video_b.convert_context=sws_getContext(cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                                               AV_PIX_FMT_BGRA,
                                                               cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                                               cfg.pixel_format,
                                                               SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);


    av_dump_format(context->format_context, 0, "", 1);


    // open the output file
    ret=avio_open(&context->format_context->pb, context->filename.toLatin1().constData(), AVIO_FLAG_WRITE);

    if(ret<0) {
        qCritical() << "could not open" << context->filename << ffErrorString(ret);
        return false;
    }


    // av_dict_set(&context->format_context->metadata, "title", "hello world", 0);

    // write the stream header, if any
    ret=avformat_write_header(context->format_context, nullptr);

    if(ret<0) {
        qCritical() << "error occurred when opening output file:" << ffErrorString(ret);
        return false;
    }


    context->cfg=cfg;

    context->skip_frame=false;
    context->frame_counter=0;

    context->out_stream_audio.next_pts=0;
    context->out_stream_audio.size_total=0;
    context->out_stream_video_a.next_pts=0;
    context->out_stream_video_a.size_total=0;

    if(cfg.split_odd_even_frames) {
        context->out_stream_video_b.next_pts=0;
        context->out_stream_video_b.size_total=0;
    }


    if(cfg.audio_dalay!=0)
        context->out_stream_audio.next_pts=cfg.audio_dalay/1000.*context->out_stream_audio.codec_context->sample_rate;


    emit stateChanged(true);

    return true;
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
            if(context->cfg.split_odd_even_frames) {
                if(size_t(context->frame_counter + 1)%size_t(2)) {
                    uint8_t *ptr_orig=context->out_stream_video_a.frame->data[0];

                    context->out_stream_video_a.frame->data[0]=(uint8_t*)frame->video.raw->constData();

                    sws_scale(context->out_stream_video_a.convert_context, context->out_stream_video_a.frame->data, context->out_stream_video_a.frame->linesize, 0, context->out_stream_video_a.frame->height, context->out_stream_video_a.frame_converted->data, context->out_stream_video_a.frame_converted->linesize);

                    context->out_stream_video_a.frame_converted->pts=context->out_stream_video_a.next_pts++;

                    write_video_frame(context->format_context, &context->out_stream_video_a);

                    context->out_stream_video_a.frame->data[0]=ptr_orig;

                } else {
                    uint8_t *ptr_orig=context->out_stream_video_b.frame->data[0];

                    context->out_stream_video_b.frame->data[0]=(uint8_t*)frame->video.raw->constData();

                    sws_scale(context->out_stream_video_b.convert_context, context->out_stream_video_b.frame->data, context->out_stream_video_b.frame->linesize, 0, context->out_stream_video_b.frame->height, context->out_stream_video_b.frame_converted->data, context->out_stream_video_b.frame_converted->linesize);

                    context->out_stream_video_b.frame_converted->pts=context->out_stream_video_b.next_pts++;

                    write_video_frame(context->format_context, &context->out_stream_video_b);

                    context->out_stream_video_b.frame->data[0]=ptr_orig;
                }

            } else {
                uint8_t *ptr_orig=context->out_stream_video_a.frame->data[0];

                context->out_stream_video_a.frame->data[0]=(uint8_t*)frame->video.raw->constData();

                sws_scale(context->out_stream_video_a.convert_context, context->out_stream_video_a.frame->data, context->out_stream_video_a.frame->linesize, 0, context->out_stream_video_a.frame->height, context->out_stream_video_a.frame_converted->data, context->out_stream_video_a.frame_converted->linesize);

                context->out_stream_video_a.frame_converted->pts=context->out_stream_video_a.next_pts++;

                write_video_frame(context->format_context, &context->out_stream_video_a);

                context->out_stream_video_a.frame->data[0]=ptr_orig;
            }

            context->frame_counter++;
        }
    }

    // audio
    {
        QByteArray ba_audio=frame->audio.raw;

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
            qCritical() << "could not setup audio frame";
            exit(1);
        }

        context->out_stream_audio.frame->pts=context->out_stream_audio.next_pts;

        context->out_stream_audio.next_pts+=context->out_stream_audio.frame->nb_samples;

        write_audio_frame(context->format_context, &context->out_stream_audio);

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

    av_write_trailer(context->format_context);

    // close each codec.

    close_stream(&context->out_stream_video_a);
    close_stream(&context->out_stream_video_b);

    close_stream(&context->out_stream_audio);

    // close the output file.
    avio_closep(&context->format_context->pb);

    // free the stream
    avformat_free_context(context->format_context);

    context->format_context=nullptr;


    emit stateChanged(false);

    return true;
}

void FFEncoder::calcStats()
{
    double cf_a=av_stream_get_end_pts(context->out_stream_audio.stream) * av_q2d(context->out_stream_audio.stream->time_base);

    if(cf_a<.01)
        cf_a=.01;


    double cf_v=av_stream_get_end_pts(context->out_stream_video_a.stream) * av_q2d(context->out_stream_video_a.stream->time_base);

    if(cf_v<.01)
        cf_v=.01;


    Stats s;

    s.avg_bitrate_audio=(double)(context->out_stream_audio.size_total*8)/cf_a;
    s.avg_bitrate_video=(double)(context->out_stream_video_a.size_total*8)/cf_v;

    s.time=QTime(0, 0).addMSecs((double)context->out_stream_audio.frame->pts/(double)context->out_stream_audio.codec_context->sample_rate*1000);

    s.streams_size=context->out_stream_audio.size_total + context->out_stream_video_a.size_total;

    emit stats(s);
}

//

QString FFEncoder::PixelFormat::toString(uint64_t format)
{
    switch(format) {
    case RGB24:
        return QLatin1String("rgb24");

    case YUV420P:
        return QLatin1String("yuv420p");

    case YUV444P:
        return QLatin1String("yuv444p");

    case YUV420P10:
        return QLatin1String("yuv420p10");

    case YUV444P10:
        return QLatin1String("yuv444p10");
    }

    return QLatin1String("unknown");
}

uint64_t FFEncoder::PixelFormat::fromString(QString format)
{
    if(format==QLatin1String("rgb24"))
        return RGB24;

    else if(format==QLatin1String("yuv420p"))
        return YUV420P;

    else if(format==QLatin1String("yuv444p"))
        return YUV444P;

    else if(format==QLatin1String("yuv420p10"))
        return YUV420P10;

    else if(format==QLatin1String("yuv444p10"))
        return YUV444P10;

    return 0;
}

QList <FFEncoder::PixelFormat::T> FFEncoder::PixelFormat::compatiblePixelFormats(FFEncoder::VideoEncoder::T encoder)
{
    switch(encoder) {
    case VideoEncoder::libx264:
        return QList<T>() << YUV420P << YUV444P;

    case VideoEncoder::libx264_10bit:
        return QList<T>() << YUV420P10 << YUV444P10;

    case VideoEncoder::libx264rgb:
        return QList<T>() << RGB24;

    case VideoEncoder::nvenc_h264:
        return QList<T>() << YUV420P << YUV444P;

    case VideoEncoder::nvenc_hevc:
        return QList<T>() << YUV420P;
    }

    return QList<T>() << RGB24 << YUV420P << YUV444P << YUV420P10 << YUV444P10;
}
