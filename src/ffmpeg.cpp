#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QFile>

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
#include <libavformat/avformat.h>
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

        next_pts=1;
        samples_count=0;

        frame=nullptr;
    }

    void free() {

    }

    AVStream *av_stream;
    AVCodecContext *av_codec_context;

    // pts of the next frame
    int64_t next_pts;
    int samples_count;
    QByteArray ba_audio_prev_part;

    AVFrame *frame;
    AVFrame *frame_converted;
};

class FFMpegContext
{
public:
    FFMpegContext() {
        out_stream_video;
        out_stream_audio;

        av_output_format=nullptr;
        av_format_context=nullptr;

        av_codec_audio=nullptr;
        av_codec_video=nullptr;

        opt=nullptr;

        i=0;
    }

    void free() {

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

    SwsContext *convert_context;

    AVDictionary *opt;

    int i;

    FFMpeg::Config cfg;
};

QString errString(int error)
{
    char buf[1024]={0};

    av_strerror(error, buf, 1024);

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

    // Write the compressed frame to the media file.
    log_packet(fmt_ctx, pkt);
    return av_interleaved_write_frame(fmt_ctx, pkt);
}

// Add an output stream
static void add_stream(OutputStream *ost, AVFormatContext *oc,
                       AVCodec **codec,
                       enum AVCodecID codec_id, FFMpeg::Config cfg)
{
    AVCodecContext *c;

    // find the encoder
    *codec=avcodec_find_encoder(codec_id);

    if(!(*codec)) {
        qCritical() << "Could not find encoder for" << codec_id;
        exit(1);
    }

    ost->av_stream=avformat_new_stream(oc, NULL);

    if(!ost->av_stream) {
        qCritical() << "Could not allocate stream";
        exit(1);
    }

    ost->av_stream->id=oc->nb_streams - 1;

    c=avcodec_alloc_context3(*codec);

    if(!c) {
        qCritical() << "Could not alloc an encoding context";
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
        case 2:
            c->channel_layout=AV_CH_LAYOUT_STEREO;
            break;

        case 6:
            c->channel_layout=AV_CH_LAYOUT_5POINT1;
            break;

        case 8:
            c->channel_layout=AV_CH_LAYOUT_7POINT1;
            break;

        default:
            c->channel_layout=AV_CH_LAYOUT_STEREO;
            break;
        }

        c->channels=av_get_channel_layout_nb_channels(c->channel_layout);

        ost->av_stream->time_base=(AVRational){ 1, c->sample_rate };

        break;

    case AVMEDIA_TYPE_VIDEO:
        c->codec_id=codec_id;

        c->bit_rate=400000;

        c->width=cfg.frame_resolution.width();
        c->height=cfg.frame_resolution.height();

        // timebase: This is the fundamental unit of time (in seconds) in terms
        // of which frame timestamps are represented. For fixed-fps content,
        // timebase should be 1/framerate and timestamp increments should be
        // identical to 1

        switch(cfg.framerate) {
        case FFMpeg::Framerate::half_50:
            ost->av_stream->time_base=(AVRational){ 1, 25 };
            break;

        case FFMpeg::Framerate::half_59:
            ost->av_stream->time_base=(AVRational){ 1001, 30000 };
            break;

        case FFMpeg::Framerate::half_60:
            ost->av_stream->time_base=(AVRational){ 1, 30 };
            break;

        case FFMpeg::Framerate::full_50:
            ost->av_stream->time_base=(AVRational){ 1, 50 };
            break;

        case FFMpeg::Framerate::full_59:
            ost->av_stream->time_base=(AVRational){ 1001, 60000 };
            break;

        case FFMpeg::Framerate::full_60:
            ost->av_stream->time_base=(AVRational){ 1, 60 };
            break;

        default:
            ost->av_stream->time_base=(AVRational){ 1, 30 };
            break;
        }

        c->time_base=ost->av_stream->time_base;

        c->gop_size=12; // emit one intra frame every twelve frames at most

        c->pix_fmt=AV_PIX_FMT_YUV444P;

        av_opt_set(c->priv_data, "preset",  "ultrafast",    0);
        av_opt_set(c->priv_data, "crf",     "10",           0);

        if(c->codec_id==AV_CODEC_ID_MPEG2VIDEO) {
            // just for testing, we also add B-frames
            c->max_b_frames=2;
        }

        if(c->codec_id==AV_CODEC_ID_MPEG1VIDEO) {
            // Needed to avoid using macroblocks in which some coeffs overflow.
            // This does not happen with normal video, it just happens here as
            // the motion of the chroma plane does not match the luma plane.
            c->mb_decision=2;
        }

        break;

    default:
        break;
    }

    // Some formats want stream headers to be separate.
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;
}


// audio output
static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
{
    AVFrame *frame=av_frame_alloc();

    int ret;

    if(!frame) {
        qCritical() << "Error allocating an audio frame";
        exit(1);
    }

    frame->format=sample_fmt;
    frame->channel_layout=channel_layout;
    frame->sample_rate=sample_rate;
    frame->nb_samples=nb_samples;

    if(nb_samples) {
        ret=av_frame_get_buffer(frame, 0);

        if(ret<0) {
            qCritical() << "Error allocating an audio buffer";
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
        qCritical() << "Could not open audio codec:" << errString(ret);
        return;
    }


    nb_samples=10000;

    ost->frame=alloc_audio_frame(c->sample_fmt, c->channel_layout, c->sample_rate, nb_samples);

    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0) {
        qCritical() << "Could not copy the stream parameters";
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
        qCritical() << "Error encoding audio frame" << errString(ret);
        exit(1);
    }

    if(got_packet) {
        ret=write_frame(oc, &c->time_base, ost->av_stream, &pkt);

        if(ret<0) {
            qCritical() << "Error while writing audio frame:" << errString(ret);
            exit(1);
        }
    }

    return (frame || got_packet) ? 0 : 1;
}

void open_video(AVFormatContext *oc, AVCodec *codec, OutputStream *ost, AVDictionary *opt_arg)
{
    int ret;

    AVCodecContext *c=ost->av_codec_context;
    AVDictionary *opt=nullptr;

    av_dict_copy(&opt, opt_arg, 0);

    // open the codec
    ret=avcodec_open2(c, codec, &opt);

    av_dict_free(&opt);

    if(ret<0) {
        qCritical() << "Could not open video codec:" << errString(ret);
        exit(1);
    }

    // allocate and init a re-usable frame
    ost->frame=alloc_frame(AV_PIX_FMT_BGRA, c->width, c->height);

    if(!ost->frame) {
        qCritical() << "Could not allocate video frame";
        exit(1);
    }

    // allocate and init a re-usable frame
    ost->frame_converted=alloc_frame(AV_PIX_FMT_YUV444P, c->width, c->height);

    if(!ost->frame_converted) {
        qCritical() << "Could not allocate video frame";
        exit(1);
    }

    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(ost->av_stream->codecpar, c);

    if(ret<0) {
        qCritical() << "Could not copy the stream parameters";
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
        qCritical() << "Error encoding video frame:" << errString(ret);
        exit(1);
    }

    if(got_packet) {
        ret=write_frame(oc, &c->time_base, ost->av_stream, &pkt);

        if(ret<0) {
            qCritical() << "Error while writing video frame:" << errString(ret);
            exit(1);
        }
    }

    return (got_packet ? 0 : 1);
}

static void close_stream(AVFormatContext *oc, OutputStream *ost)
{
    avcodec_free_context(&ost->av_codec_context);
    av_frame_free(&ost->frame);
    //    sws_freeContext(ost->sws_ctx);
    //    swr_free(&ost->swr_ctx);
}

/////////////////

FFMpeg::FFMpeg(QObject *parent) :
    QObject(parent)
{
    context=new FFMpegContext();

    converter=new FF::FormatConverter();
}

FFMpeg::~FFMpeg()
{
    delete context;

    delete converter;

    stopCoder();
}

void FFMpeg::init()
{
    av_register_all();
}

bool FFMpeg::initCoder(Config cfg)
{
    int ret;

    if(!converter->setup(AV_PIX_FMT_BGRA, cfg.frame_resolution, AV_PIX_FMT_YUV444P, cfg.frame_resolution)) {
        qCritical() << "err init format converter";
        return false;
    }


    context->filename=QString("%1/%2.mkv")
            .arg(QApplication::applicationDirPath())
            .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss"));


    // allocate the output media context
    avformat_alloc_output_context2(&context->av_format_context, nullptr, "matroska", nullptr);

    if(!context->av_format_context) {
        qCritical() << "Could not deduce output format";
        return false;
    }

    context->av_output_format=context->av_format_context->oformat;

    // Add the audio and video streams using the default format codecs
    // and initialize the codecs.
    add_stream(&context->out_stream_video, context->av_format_context, &context->av_codec_video, AV_CODEC_ID_H264, cfg);
    add_stream(&context->out_stream_audio, context->av_format_context, &context->av_codec_audio, AV_CODEC_ID_PCM_S16LE, cfg);

    // Now that all the parameters are set, we can open the audio and
    // video codecs and allocate the necessary encode buffers.
    open_video(context->av_format_context, context->av_codec_video, &context->out_stream_video, context->opt);

    open_audio(context->av_format_context, context->av_codec_audio, &context->out_stream_audio, context->opt);


    context->convert_context=sws_getContext(cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                            AV_PIX_FMT_BGRA,
                                            cfg.frame_resolution.width(), cfg.frame_resolution.height(),
                                            AV_PIX_FMT_YUV444P,
                                            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);

    av_dump_format(context->av_format_context, 0, "", 1);


    // open the output file
    ret=avio_open(&context->av_format_context->pb, context->filename.toLatin1().data(), AVIO_FLAG_WRITE);

    if(ret<0) {
        qCritical() << "Could not open" << context->filename << errString(ret);
        return false;
    }


    // Write the stream header, if any
    ret=avformat_write_header(context->av_format_context, &context->opt);

    if(ret<0) {
        qCritical() << "Error occurred when opening output file:" << errString(ret);
        return false;
    }

    return true;
}

bool FFMpeg::appendFrame(QByteArray ba_video, QSize size, QByteArray ba_audio)
{
    if(!context->canAcceptFrame())
        return false;


    // video
    {
        byteArrayToAvFrame(&ba_video, context->out_stream_video.frame);

        sws_scale(context->convert_context, context->out_stream_video.frame->data, context->out_stream_video.frame->linesize, 0, context->out_stream_video.frame->height, context->out_stream_video.frame_converted->data, context->out_stream_video.frame_converted->linesize);

        context->out_stream_video.frame_converted->pts=context->out_stream_video.next_pts++;

        write_video_frame(context->av_format_context, &context->out_stream_video);

    }

    // audio
    {
        ba_audio.insert(0, context->out_stream_audio.ba_audio_prev_part);

        int buffer_size=0;

        int default_nb_samples=context->out_stream_audio.frame->nb_samples;

        while(true) {
            buffer_size=av_samples_get_buffer_size(nullptr, context->out_stream_audio.frame->channels, context->out_stream_audio.frame->nb_samples,
                                                   AV_SAMPLE_FMT_S16, 0);

            if(ba_audio.size()>=buffer_size) {

                break;
            }

            context->out_stream_audio.frame->nb_samples--;
        }

        // qDebug() << "audio size a" << buffer_size << ba_audio.size() << context->out_stream_audio.frame->channels << context->out_stream_audio.frame->nb_samples;


        QByteArray ba_audio_tmp=ba_audio.left(buffer_size);

        context->out_stream_audio.ba_audio_prev_part=ba_audio.remove(0, buffer_size);


        // qDebug() << "audio size b" << ba_audio_tmp.size() << context->out_stream_audio.ba_audio_prev_part.size();
        \
        int ret=avcodec_fill_audio_frame(context->out_stream_audio.frame, context->out_stream_audio.frame->channels, AV_SAMPLE_FMT_S16,
                                         (const uint8_t*)ba_audio_tmp.data(), buffer_size, 0);

        if(ret<0) {
            qCritical() << "Could not setup audio frame";
            exit(1);
        }

        context->out_stream_audio.frame->pts=context->out_stream_audio.next_pts;

        context->out_stream_audio.next_pts  += context->out_stream_audio.frame->nb_samples;

        write_audio_frame(context->av_format_context, &context->out_stream_audio);

        context->out_stream_audio.frame->nb_samples=default_nb_samples;
    }

    return true;
}

bool FFMpeg::stopCoder()
{
    if(!context->canAcceptFrame())
        return false;

    av_write_trailer(context->av_format_context);

    // Close each codec.

    close_stream(context->av_format_context, &context->out_stream_video);

    close_stream(context->av_format_context, &context->out_stream_audio);


    // Close the output file.
    avio_closep(&context->av_format_context->pb);

    // free the stream
    avformat_free_context(context->av_format_context);

    context->av_format_context=nullptr;

    return true;
}
