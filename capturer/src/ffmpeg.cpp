#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QDir>

#include <x264_config.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libavutil/avassert.h>
#include <libavutil/timestamp.h>
#include "libswscale/swscale.h"
#include <libswresample/swresample.h>
}

#ifdef av_ts2str(ts)
#undef av_ts2str(ts)
#define av_ts2str(ts) av_ts_make_string((char*)(char[AV_TS_MAX_STRING_SIZE]){0}, ts)
#endif

#ifdef av_ts2timestr(ts, tb)
#undef av_ts2timestr(ts, tb)
#define av_ts2timestr(ts, tb) av_ts_make_time_string((char*)(char[AV_TS_MAX_STRING_SIZE]){0}, ts, tb)
#endif

#include "ffmpeg_tools.h"
#include "ffmpeg_format_converter.h"

#include "ffmpeg.h"

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

    FFMpeg::Config cfg;

    bool skip_frame;

    qint64 last_stats_update_time;
};

QString errString(int error)
{
    char buf[1024]={0};

    av_strerror(error, buf, 640);

    return QString(buf);
}

static void log_packet(const AVFormatContext *fmt_ctx, const AVPacket *pkt)
{
    AVRational *time_base=&fmt_ctx->streams[pkt->stream_index]->time_base;

    printf("pts:%s pts_time:%s dts:%s dts_time:%s duration:%s duration_time:%s stream_index:%d\n",
           av_ts2str(pkt->pts), av_ts2timestr(pkt->pts, time_base),
           av_ts2str(pkt->dts), av_ts2timestr(pkt->dts, time_base),
           av_ts2str(pkt->duration), av_ts2timestr(pkt->duration, time_base),
           pkt->stream_index);
}

static int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
{
    // rescale output packet timestamp values from codec to stream timebase
    av_packet_rescale_ts(pkt, *time_base, st->time_base);
    pkt->stream_index=st->index;

    // write the compressed frame to the media file
    log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

// add an output stream
static void add_stream(OutputStream *ost, AVFormatContext *oc,
                       AVCodec **codec,
                       enum AVCodecID codec_id, FFMpeg::Config cfg)
{
    AVCodecContext *c;

    // find the encoder
    if(codec_id!=AV_CODEC_ID_H264) {
        *codec=avcodec_find_encoder(codec_id);

    } else {
        switch(cfg.video_encoder) {
        case FFMpeg::VideoEncoder::libx264: {
            *codec=avcodec_find_encoder(codec_id);

        } break;

        case FFMpeg::VideoEncoder::libx264rgb: {
            *codec=avcodec_find_encoder_by_name("libx264rgb");

        } break;

        case FFMpeg::VideoEncoder::nvenc_h264: {
            *codec=avcodec_find_encoder_by_name("h264_nvenc");

        } break;

        case FFMpeg::VideoEncoder::nvenc_hevc: {
            *codec=avcodec_find_encoder_by_name("hevc_nvenc");

            codec_id=AV_CODEC_ID_H265;

        } break;

        default:
            break;
        }
    }

    if(!(*codec)) {
        qCritical() << "could not find encoder for" << codec_id;
        exit(1);
    }

    ost->av_stream=avformat_new_stream(oc, nullptr);

    if(!ost->av_stream) {
        qCritical() << "could not allocate stream";
        exit(1);
    }

    ost->av_stream->id=oc->nb_streams - 1;

    c=avcodec_alloc_context3(*codec);

    if(!c) {
        qCritical() << "could not alloc an encoding context";
        exit(1);
    }

    ost->av_codec_context=c;

    switch((*codec)->type) {
    case AVMEDIA_TYPE_AUDIO:
        c->sample_fmt=(*codec)->sample_fmts ?
                    (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;

        c->bit_rate=480000;

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

        ost->av_stream->time_base=(AVRational){ 1, c->sample_rate };

        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id=codec_id;

        c->width=cfg.frame_resolution.width();
        c->height=cfg.frame_resolution.height();

        // timebase: This is the fundamental unit of time (in seconds) in terms
        // of which frame timestamps are represented. For fixed-fps content,
        // timebase should be 1/framerate and timestamp increments should be
        // identical to 1

        switch(cfg.framerate) {
        case FFMpeg::Framerate::full_23:
            ost->av_stream->time_base=(AVRational){ 1001, 24000 };
            break;

        case FFMpeg::Framerate::full_24:
            ost->av_stream->time_base=(AVRational){ 1000, 24000 };
            break;

        case FFMpeg::Framerate::full_25:
        case FFMpeg::Framerate::half_50:
            ost->av_stream->time_base=(AVRational){ 1000, 25000 };
            break;

        case FFMpeg::Framerate::full_29:
        case FFMpeg::Framerate::half_59:
            ost->av_stream->time_base=(AVRational){ 1001, 30000 };
            break;

        case FFMpeg::Framerate::full_30:
        case FFMpeg::Framerate::half_60:
            ost->av_stream->time_base=(AVRational){ 1000, 30000 };
            break;

        case FFMpeg::Framerate::full_50:
            ost->av_stream->time_base=(AVRational){ 1000, 50000 };
            break;

        case FFMpeg::Framerate::full_59:
            ost->av_stream->time_base=(AVRational){ 1001, 60000 };
            break;

        case FFMpeg::Framerate::full_60:
            ost->av_stream->time_base=(AVRational){ 1000, 60000 };
            break;

        default:
            ost->av_stream->time_base=(AVRational){ 1000, 30000 };
            break;
        }

        c->time_base=ost->av_stream->time_base;

        c->gop_size=12; // emit one intra frame every twelve frames at most

        c->pix_fmt=cfg.pixel_format;

        if(cfg.video_encoder==FFMpeg::VideoEncoder::libx264 || cfg.video_encoder==FFMpeg::VideoEncoder::libx264rgb) {
            av_opt_set(c->priv_data, "preset", "ultrafast", 0);
            // av_opt_set(c->priv_data, "tune", "zerolatency", 0);
            av_opt_set(c->priv_data, "crf", QString::number(cfg.crf).toLatin1().data(), 0);

        } else if(cfg.video_encoder==FFMpeg::VideoEncoder::nvenc_h264) {
            c->bit_rate=0;

            if(cfg.crf==0) {
                av_opt_set(c->priv_data, "preset", "lossless", 0);

            } else {
                c->global_quality=cfg.crf;

                // av_opt_set(c->priv_data, "preset", "fast", 0); // HP
                av_opt_set(c->priv_data, "preset", "slow", 0); // HQ
            }

            // av_opt_set(c->priv_data, "tune", "zerolatency", 0);

        } else if(cfg.video_encoder==FFMpeg::VideoEncoder::nvenc_hevc) {
            c->bit_rate=0;
            c->global_quality=cfg.crf;

            // av_opt_set(c->priv_data, "preset", "fast", 0); // HP
            av_opt_set(c->priv_data, "preset", "slow", 0); // HQ

            // av_opt_set(c->priv_data, "tune", "zerolatency", 0);
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

        break;

    default:
        break;
    }

    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;
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
    AVCodecContext *c;

    int nb_samples;
    int ret;

    AVDictionary *opt=nullptr;

    c=ost->av_codec_context;

    // open it
    av_dict_copy(&opt, opt_arg, 0);

    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        qCritical() << "could not open audio codec:" << errString(ret);
        return;
    }


    nb_samples=10000;

    ost->frame=alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);

    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0) {
        qCritical() << "could not copy the stream parameters";
        exit(1);
    }
}

// encode one audio frame and send it to the muxer
// return 1 when encoding is finished, 0 otherwise
static int write_audio_frame(AVFormatContext *oc, OutputStream *ost)
{
    AVCodecContext *c;
    AVPacket pkt={ 0 }; // data and size must be 0;
    AVFrame *frame;

    int ret;
    int got_packet;

    av_init_packet(&pkt);

    c=ost->av_codec_context;

    frame=ost->frame;

    ret=avcodec_encode_audio2(c, &pkt, frame, &got_packet);

    if(ret<0) {
        qCritical() << "error encoding audio frame" << errString(ret);
        exit(1);
    }

    if(got_packet) {
        ost->size_total+=pkt.size;

        ret=write_frame(oc, &c->time_base, ost->av_stream, &pkt);

        if(ret<0) {
            qCritical() << "error while writing audio frame:" << errString(ret);
            exit(1);
        }
    }

    return (frame || got_packet) ? 0 : 1;
}

void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg, FFMpeg::Config cfg)
{
    int ret;

    AVCodecContext *c=ost->av_codec_context;
    AVDictionary *opt=nullptr;

    av_dict_copy(&opt, opt_arg, 0);

    // open the codec
    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        qCritical() << "could not open video codec:" << errString(ret);
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
    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0) {
        qCritical() << "could not copy the stream parameters";
        exit(1);
    }
}

// encode one video frame and send it to the muxer
// return 1 when encoding is finished, 0 otherwise
static int write_video_frame(AVFormatContext *oc, OutputStream *ost)
{
    int ret;

    AVCodecContext *c;

    int got_packet=0;
    AVPacket pkt={ 0 };

    c=ost->av_codec_context;

    av_init_packet(&pkt);

    // encode the image
    ret=avcodec_encode_video2(c, &pkt, ost->frame_converted, &got_packet);

    if(ret<0) {
        qCritical() << "error encoding video frame:" << errString(ret);
        exit(1);
    }

    if(got_packet) {
        ost->size_total+=pkt.size;

        ret=write_frame(oc, &c->time_base, ost->av_stream, &pkt);

        if(ret<0) {
            qCritical() << "error while writing video frame:" << errString(ret);
            exit(1);
        }
    }

    return (got_packet ? 0 : 1);
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->av_codec_context);
    av_frame_free(&ost->frame);
    av_frame_free(&ost->frame_converted);

    sws_freeContext(ost->convert_context);
}

// ------------------------------

FFMpeg::FFMpeg(QObject *parent) :
    QObject(parent)
{
    context=new FFMpegContext();

    converter=new FF::FormatConverter();
}

FFMpeg::~FFMpeg()
{
    stopCoder();

    delete context;

    delete converter;
}

void FFMpeg::init()
{
    qRegisterMetaType<FFMpeg::Config>("FFMpeg::Config");
    qRegisterMetaType<FFMpeg::Stats>("FFMpeg::Stats");

    av_register_all();
}

bool FFMpeg::isLib_x264_10bit()
{
    return X264_BIT_DEPTH==10;
}

FFMpeg::Framerate::T FFMpeg::calcFps(int64_t frame_duration, int64_t frame_scale, bool half_fps)
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

bool FFMpeg::setConfig(FFMpeg::Config cfg)
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

    context->filename=QString("%1/videos/%2.mkv")
            .arg(QApplication::applicationDirPath())
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));


    // allocate the output media context
    // avformat_alloc_output_context2(&context->av_format_context, nullptr, nullptr, context->filename.toLatin1().data());
    avformat_alloc_output_context2(&context->av_format_context, nullptr, "matroska", nullptr);

    if(!context->av_format_context) {
        qCritical() << "could not deduce output format";
        return false;
    }

    context->av_output_format=context->av_format_context->oformat;

    // add the audio and video streams using the default format codecs
    // and initialize the codecs.
    add_stream(&context->out_stream_video, context->av_format_context, &context->av_codec_video, AV_CODEC_ID_H264, cfg);
    add_stream(&context->out_stream_audio, context->av_format_context, &context->av_codec_audio, AV_CODEC_ID_PCM_S16LE, cfg);

    // now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers
    open_video(context->av_format_context, context->av_codec_video, &context->out_stream_video, context->opt, cfg);

    open_audio(context->av_format_context, context->av_codec_audio, &context->out_stream_audio, context->opt);


    context->out_stream_video.convert_context=sws_getContext(cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                                             AV_PIX_FMT_BGRA,
                                                             cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                                             cfg.pixel_format,
                                                             SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    av_dump_format(context->av_format_context, 0, "", 1);


    // open the output file
    ret=avio_open(&context->av_format_context->pb, context->filename.toLatin1().data(), AVIO_FLAG_WRITE);

    if(ret<0) {
        qCritical() << "could not open" << context->filename << errString(ret);
        return false;
    }


    // write the stream header, if any
    ret=avformat_write_header(context->av_format_context, &context->opt);

    if(ret<0) {
        qCritical() << "error occurred when opening output file:" << errString(ret);
        return false;
    }


    context->cfg=cfg;

    context->skip_frame=false;

    context->out_stream_audio.next_pts=0;

    context->out_stream_video.next_pts=0;

    context->out_stream_audio.size_total=0;

    context->out_stream_video.size_total=0;


    if(cfg.audio_dalay!=0)
        context->out_stream_audio.next_pts=cfg.audio_dalay/1000.*context->out_stream_audio.av_codec_context->sample_rate;

    return true;
}

bool FFMpeg::appendFrame(QByteArray *ba_video, QSize *size, QByteArray *ba_audio)
{
    if(!context->canAcceptFrame())
        return false;


    // video
    {
        if(ba_video->isEmpty()) {
            context->out_stream_video.next_pts++;

        } else {
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
                byteArrayToAvFrame(ba_video, context->out_stream_video.frame);

                sws_scale(context->out_stream_video.convert_context, context->out_stream_video.frame->data, context->out_stream_video.frame->linesize, 0, context->out_stream_video.frame->height, context->out_stream_video.frame_converted->data, context->out_stream_video.frame_converted->linesize);

                context->out_stream_video.frame_converted->pts=context->out_stream_video.next_pts++;

                write_video_frame(context->av_format_context, &context->out_stream_video);
            }
        }
    }

    // audio
    {
        ba_audio->insert(0, context->out_stream_audio.ba_audio_prev_part);

        int buffer_size=0;

        int default_nb_samples=context->out_stream_audio.frame->nb_samples;

        while(true) {
            buffer_size=av_samples_get_buffer_size(nullptr, context->out_stream_audio.frame->channels, context->out_stream_audio.frame->nb_samples,
                                                   AV_SAMPLE_FMT_S16, 0);

            if(ba_audio->size()>=buffer_size) {

                break;
            }

            context->out_stream_audio.frame->nb_samples--;
        }

        QByteArray ba_audio_tmp=ba_audio->left(buffer_size);

        context->out_stream_audio.ba_audio_prev_part=ba_audio->remove(0, buffer_size);

        int ret=avcodec_fill_audio_frame(context->out_stream_audio.frame, context->out_stream_audio.frame->channels, AV_SAMPLE_FMT_S16,
                                         (const uint8_t*)ba_audio_tmp.data(), buffer_size, 0);

        if(ret<0) {
            qCritical() << "could not setup audio frame";
            exit(1);
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

bool FFMpeg::stopCoder()
{
    if(!context->canAcceptFrame())
        return false;

    av_write_trailer(context->av_format_context);

    // close each codec.

    close_stream(context->av_format_context, &context->out_stream_video);

    close_stream(context->av_format_context, &context->out_stream_audio);

    // close the output file.
    avio_closep(&context->av_format_context->pb);

    // free the stream
    avformat_free_context(context->av_format_context);

    context->av_format_context=nullptr;

    return true;
}

void FFMpeg::calcStats()
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
