/******************************************************************************

Copyright © 2018-2021 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>

#include <assert.h>

#include "audio_tools.h"
#include "ff_tools.h"
#include "decklink_video_frame.h"
#include "decklink_audio_input_packet.h"

#include "ff_decoder_thread.h"

const int audio_buf_min_size=4096*2;


FFDecoderThread::FFDecoderThread(QObject *parent)
    : QThread(parent)
{
    prev_state=state=ST_STOPPED;

    start(QThread::NormalPriority);
}

FFDecoderThread::~FFDecoderThread()
{
    stop();

    term();
}

void FFDecoderThread::subscribeVideo(FrameBuffer<Frame::ptr>::ptr obj)
{
    context.out_video_buffer=obj;
}

void FFDecoderThread::subscribeAudio(FrameBuffer<Frame::ptr>::ptr obj)
{
    context.out_audio_buffer=obj;
}

void FFDecoderThread::open(const QString &filename)
{
    stop();

    this->filename=filename;

    state=ST_OPEN;
}

void FFDecoderThread::play()
{
    if(state==ST_STOPPED && !QString(filename).isEmpty()) {
        open(filename);
        return;
    }

    if(state==ST_IDLE) {
        seek(currentPos());

        return;
    }
}

void FFDecoderThread::pause()
{
    if(state!=ST_PLAY)
        return;

    state=ST_IDLE;
}

void FFDecoderThread::stop()
{
    state=ST_STOPPING;

    while(state!=ST_STOPPED)
        usleep(1);
}

void FFDecoderThread::seek(qint64 pos)
{
    if(state!=ST_PLAY && state!=ST_IDLE)
        return;

    position=pos;
    seek_pos=pos;

    state=ST_SEEK;
}

int FFDecoderThread::currentState() const
{
    return state;
}

int64_t FFDecoderThread::currentPos()  const
{
    return position;
}

int64_t FFDecoderThread::currentDuration() const
{
    return duration;
}

void FFDecoderThread::term()
{
     running=false;

     while(isRunning()) {
         msleep(30);
     }
}

int64_t FFDecoderThread::audioTS()
{
    return (context.pts_audio - context.start_time_audio)*av_q2d(context.stream_audio->time_base)*1000000000;
}

int64_t FFDecoderThread::videoTS()
{
    return (context.pts_video - context.start_time_video)*av_q2d(context.stream_video->time_base)*1000000000;
}

void FFDecoderThread::_open()
{
    assert(context.out_audio_buffer);
    assert(context.out_video_buffer);

    if(QString(filename).isEmpty()) {
        freeContext();
        return;
    }

    unsigned int stream_video_pos;
    unsigned int stream_audio_pos;

    qDebug() << "filename" << filename;

    int err=
            avformat_open_input(&context.format_context, QString(filename).toLatin1().constData(), nullptr, nullptr);

    if(err<0) {
        qCritical() << "avformat_open_input err:" << filename << ffErrorString(err);
        freeContext();
        return;
    }

    // Retrieve stream information
    err=avformat_find_stream_info(context.format_context, nullptr);

    if(err<0) {
        qCritical() << "avformat_find_stream_info err:" << ffErrorString(err);
        freeContext();
        return;
    }

    // { video

    for(stream_video_pos=0; stream_video_pos<context.format_context->nb_streams; ++stream_video_pos) {
        if(context.format_context->streams[stream_video_pos]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            break;
        }
    }

    if(stream_video_pos==context.format_context->nb_streams) {
        qCritical() << "unable to find video stream";
        freeContext();
        return;
    }

    context.stream_video=context.format_context->streams[stream_video_pos];

    bool hw_decoder=false;

    if(context.stream_video->codecpar->codec_id==AV_CODEC_ID_H264) {
        context.codec_video=(AVCodec*)avcodec_find_decoder_by_name("h264_cuvid");
        hw_decoder=true;

    } else if(context.stream_video->codecpar->codec_id==AV_CODEC_ID_HEVC) {
        context.codec_video=(AVCodec*)avcodec_find_decoder_by_name("hevc_cuvid");
        hw_decoder=true;

    } else {
        context.codec_video=(AVCodec*)avcodec_find_decoder(context.stream_video->codecpar->codec_id);
    }

    if(!context.codec_video) {
        qWarning() << "hw decoder err";

try_software_decoder:

        hw_decoder=false;
        context.codec_video=(AVCodec*)avcodec_find_decoder(context.stream_video->codecpar->codec_id);
    }

    if(!context.codec_video) {
        qCritical() << "decoder err";
        return;
    }

    context.codec_context_video=avcodec_alloc_context3(context.codec_video);

    err=avcodec_parameters_to_context(context.codec_context_video, context.stream_video->codecpar);

    if(err<0) {
        qCritical() << "avcodec_parameters_to_context err:" << ffErrorString(err);
        freeContext();
        state=ST_STOPPED;
    }

    context.codec_context_video->flags2|=AV_CODEC_FLAG2_FAST;

    context.codec_context_video->thread_count=qMin(QThread::idealThreadCount(), 4);

    av_opt_set(context.codec_context_video->priv_data, "tune", "fastdecode", 0);

    err=avcodec_open2(context.codec_context_video, context.codec_video, nullptr);

    if(err<0) {
        qCritical() << "avcodec_open2 err:" << ffErrorString(err);

        if(hw_decoder) {
            avcodec_free_context(&context.codec_context_video);
            goto try_software_decoder;
        }

        freeContext();
        return;
    }

    QSize target_size=QSize(context.codec_context_video->width, context.codec_context_video->height);

    target_size=target_size.width()<=1920 ? target_size : target_size.scaled(QSize(1920, 1080), Qt::KeepAspectRatio);

    context.scale_filter=SWS_POINT;

    if(context.codec_context_video->width!=target_size.width()) {
        context.target_size=QSize(context.codec_context_video->width, context.codec_context_video->height)
                .scaled(target_size, Qt::KeepAspectRatio);

        context.scale_filter=SWS_FAST_BILINEAR;

    } else
        context.target_size=QSize(context.codec_context_video->width, context.codec_context_video->height);

    qInfo() << "target_size" << context.target_size << av_get_pix_fmt_name((AVPixelFormat)context.stream_video->codecpar->format);

    // } video

    // { audio

    for(stream_audio_pos=0; stream_audio_pos<context.format_context->nb_streams; ++stream_audio_pos) {
        if(context.format_context->streams[stream_audio_pos]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
            break;
        }
    }

    if(stream_audio_pos==context.format_context->nb_streams) {
        qCritical() << "unable to find audio stream";
        freeContext();
        return;
    }

    context.stream_audio=context.format_context->streams[stream_audio_pos];


    context.codec_audio=(AVCodec*)avcodec_find_decoder(context.stream_audio->codecpar->codec_id);

    context.codec_context_audio=avcodec_alloc_context3(context.codec_audio);

    err=avcodec_parameters_to_context(context.codec_context_audio, context.stream_audio->codecpar);

    if(err<0) {
        qCritical() << "avcodec_parameters_to_context err:" << ffErrorString(err);
        freeContext();
        return;
    }

    err=avcodec_open2(context.codec_context_audio, context.codec_audio, nullptr);

    if(err<0) {
        qCritical() << "avcodec_open2 err:" << ffErrorString(err);
        freeContext();
        return;
    }

    context.convert_context_audio=swr_alloc();

    if(!context.convert_context_audio) {
        qCritical() << "swr_alloc nullptr";
        freeContext();
        return;
    }

    if(context.codec_context_audio->channel_layout==0)
        context.codec_context_audio->channel_layout=av_get_default_channel_layout(context.codec_context_audio->channels);

    // if(!context.audio_converter.init(context.codec_context_audio->channel_layout, context.codec_context_audio->sample_rate, context.codec_context_audio->sample_fmt,
    //                                  AV_CH_LAYOUT_STEREO, 48000, AV_SAMPLE_FMT_S16)) {
    //     qCritical() << "audio_converter.init err";
    //     freeContext();
    //     return;
    // }

    av_opt_set_channel_layout(context.convert_context_audio, "in_channel_layout", context.codec_context_audio->channel_layout, 0);
    av_opt_set_channel_layout(context.convert_context_audio, "out_channel_layout", AV_CH_LAYOUT_STEREO,  0);
    av_opt_set_int(context.convert_context_audio,"in_sample_rate", context.codec_context_audio->sample_rate, 0);
    av_opt_set_int(context.convert_context_audio, "out_sample_rate", 48000, 0);
    av_opt_set_sample_fmt(context.convert_context_audio, "in_sample_fmt", context.codec_context_audio->sample_fmt, 0);
    av_opt_set_sample_fmt(context.convert_context_audio, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

    err=swr_init(context.convert_context_audio);

    if(err<0) {
        qCritical() << "swr_init err:" << ffErrorString(err);
        freeContext();
        return;
    }

    // } audio

    emit durationChanged(duration=context.format_context->duration/1000.);

    context.pts_video=0;
    context.last_video_out_time=0;

    context.wait_video=false;
    context.wait_audio=false;


    context.frame_video=av_frame_alloc();
    context.frame_audio=av_frame_alloc();

    //

    context.start_time_video=0;
    context.start_time_audio=0;

    if(context.stream_video->start_time!=AV_NOPTS_VALUE)
        context.start_time_video=context.stream_video->start_time;

    if(context.stream_audio->start_time!=AV_NOPTS_VALUE)
        context.start_time_audio=context.stream_audio->start_time;

    qDebug() << "start_time" << context.start_time_video << context.start_time_audio << context.stream_video->start_time << context.stream_audio->start_time;

    //

    context.frame_timer=av_gettime();
    context.frame_last_pts=0;
    context.pict_pts=0;
    context.frame_last_delay=0;
    context.reset_audio=true;

    position=0;

    state=ST_PLAY;
}

void FFDecoderThread::_play()
{
    bool do_nothing=true;

    if(context.packets_audio.size()<320 || context.packets_video.isEmpty()) {
        if(av_read_frame(context.format_context, &context.packet)>=0) {
            if(context.packet.stream_index==context.stream_video->index) {
                context.packets_video.append(context.packet);

            } else if(context.packet.stream_index==context.stream_audio->index) {
                context.packets_audio.append(context.packet);

            } else {
                av_packet_unref(&context.packet);

            }
        }
    }


    { // video
        if(!context.wait_video) {
            if(!context.packets_video.isEmpty()) {
                AVPacket packet=context.packets_video.takeFirst();

                int frame_finished=0;

                if(avcodec_send_packet(context.codec_context_video, &packet)==0) {
                    int ret=avcodec_receive_frame(context.codec_context_video, context.frame_video);

                    if(ret<0 && ret!=AVERROR(EAGAIN)) {
                        qCritical() << "avcodec_receive_frame err:" << ffErrorString(ret);
                    }

                    if(ret>=0)
                        frame_finished=1;
                }

                if(frame_finished) {
                    int64_t ts=context.frame_video->pts;

                    if(ts!=AV_NOPTS_VALUE)
                        context.pts_video=ts;

                    int64_t pts=context.pts_video;

                    pts*=av_q2d(context.stream_video->time_base)*1000000;
                    pts=synchronizeVideo(context.frame_video, pts);

                    context.pict_pts=pts;

                    context.video_frame_duration=computeDelay();

                    if(context.frame_video->format!=AV_PIX_FMT_YUV420P && context.frame_video->format!=AV_PIX_FMT_NV12) {
                        if(!context.frame_cnv)
                            context.frame_cnv=alloc_frame(AV_PIX_FMT_NV12, context.target_size.width(), context.target_size.height());

                        if(!context.convert_context_video) {
                            context.convert_context_video=sws_getContext(context.codec_context_video->width, context.codec_context_video->height,
                                                                         (AVPixelFormat)context.frame_video->format,
                                                                         context.target_size.width(), context.target_size.height(),
                                                                         (AVPixelFormat)context.frame_cnv->format, context.scale_filter,
                                                                         nullptr, nullptr, nullptr);
                        }

                        sws_scale(context.convert_context_video,
                                  context.frame_video->data, context.frame_video->linesize,
                                  0, context.codec_context_video->height,
                                  context.frame_cnv->data, context.frame_cnv->linesize);

                        context.frame_cnv->pts=context.frame_video->pts;
                    }

                    context.wait_video=true;
                }

                av_packet_unref(&packet);

                do_nothing=false;
            }
        }


        if(context.wait_video) {
            if(av_gettime() - context.last_video_out_time>=context.video_frame_duration)
                context.wait_video=false;

            if(!context.wait_video) {
                Frame::ptr f=Frame::make();

                AVFrame *frame=context.frame_video;

                if(context.frame_cnv) {
                   frame=context.frame_cnv;
                }

                f->video.pixel_format.fromAVPixelFormat((AVPixelFormat)frame->format);
                f->video.size=QSize(frame->width, frame->height);
                f->video.data_size=frameBufSize(QSize(frame->width, frame->height), f->video.pixel_format);
                f->video.dummy.resize(f->video.data_size);
                f->video.data_ptr=(uint8_t*)f->video.dummy.constData();

                av_image_copy_to_buffer((uint8_t*)f->video.data_ptr, f->video.data_size, frame->data, frame->linesize,
                                        (AVPixelFormat)frame->format, frame->width, frame->height, alignment);

                context.last_video_out_time=av_gettime();


                if(context.out_video_buffer)
                    context.out_video_buffer->append(f);


                if(context.frame_video->pts!=AV_NOPTS_VALUE)
                    emit positionChanged(position=context.frame_video->pts*av_q2d(context.stream_video->time_base)*1000);

                f.reset();

                do_nothing=false;
            }
        }
    }


    { // audio
        if(!context.packets_audio.isEmpty()) {
            if(context.ba_audio.size()<audio_buf_min_size) {
                AVPacket packet;
                AVPacket packet_orig;

                packet_orig=packet=context.packets_audio.takeFirst();

                int frame_finished=0;

                if(avcodec_send_packet(context.codec_context_audio, &packet)==0) {
                    int ret=avcodec_receive_frame(context.codec_context_audio, context.frame_audio);

                    if(ret<0 && ret!=AVERROR(EAGAIN)) {
                        qCritical() << "avcodec_receive_frame err:" << ffErrorString(ret);
                    }

                    if(ret>=0)
                        frame_finished=1;
                }

                if(frame_finished) {
                    uint8_t *out_samples;

                    int out_num_samples=av_rescale_rnd(swr_get_delay(context.convert_context_audio, context.codec_context_audio->sample_rate)
                                                       + context.frame_audio->nb_samples, 48000, context.codec_context_audio->sample_rate, AV_ROUND_UP);

                    av_samples_alloc(&out_samples, nullptr, 2, out_num_samples, AV_SAMPLE_FMT_S16, 0);

                    out_num_samples=swr_convert(context.convert_context_audio, &out_samples, out_num_samples,
                                                (const uint8_t**)&context.frame_audio->extended_data[0], context.frame_audio->nb_samples);


                    int src_data_size=av_get_bytes_per_sample((AVSampleFormat)context.codec_context_audio->sample_fmt)*context.codec_context_audio->channels*context.frame_audio->nb_samples;

                    context.audio_buf_size+=src_data_size;

                    //

                    //int64_t ts=av_frame_get_best_effort_timestamp(context.frame_video); // ????????!
                    int64_t ts=context.frame_video->best_effort_timestamp;

                    if(context.ba_audio.isEmpty()) {
                        if(ts!=AV_NOPTS_VALUE)
                            context.pts_audio=ts;

                        // context.audio_clock=av_q2d(context.stream_audio->time_base)*context.pts_audio*1000000;
                        context.audio_clock=av_q2d(context.stream_video->time_base)*context.pts_audio*1000000;
                    }

                    context.ba_audio.append(QByteArray((char*)out_samples, av_samples_get_buffer_size(nullptr, 2, out_num_samples, AV_SAMPLE_FMT_S16, 0)));
                    // context.ba_audio.append(ba_dst);

                    av_freep(&out_samples);
                }

                context.wait_audio=true;


                av_packet_unref(&packet_orig);

                do_nothing=false;
            }
        }

        if(context.out_audio_buffer->isEmpty() && context.ba_audio.size()>=audio_buf_min_size) {
            Frame::ptr f=Frame::make();

            f->setDataAudio(context.ba_audio, 2, 16);

            context.ba_audio.clear();
            context.audio_buf_size=0;

            if(context.reset_audio) {
                f->reset_counter=true;
                context.reset_audio=false;
            }


            if(context.out_audio_buffer)
                context.out_audio_buffer->append(f);


            f.reset();

            do_nothing=false;
        }
    }

    if(do_nothing)
        usleep(1);
}

void FFDecoderThread::_seek()
{
    int ret;

    ret=av_seek_frame(context.format_context, context.stream_video->index, seek_pos/1000./av_q2d(context.stream_video->time_base), AVSEEK_FLAG_BACKWARD);

    if(ret<0) {
        qCritical() << "av_seek_frame err:" << ffErrorString(ret);
    }

    avcodec_flush_buffers(context.codec_context_video);

    while(!context.packets_video.isEmpty()) {
        AVPacket p=context.packets_video.takeFirst();
        av_packet_unref(&p);
    }

    while(!context.packets_audio.isEmpty()) {
        AVPacket p=context.packets_audio.takeFirst();
        av_packet_unref(&p);
    }


    context.frame_timer=av_gettime();
    context.frame_last_pts=0;
    context.pict_pts=0;
    context.frame_last_delay=0;

    context.ba_audio.clear();
    context.out_audio_buffer->clear();
    context.reset_audio=true;

    state=ST_PLAY;
}

void FFDecoderThread::freeContext()
{
    while(!context.packets_video.isEmpty()) {
        AVPacket p=context.packets_video.takeFirst();

        av_packet_unref(&p);
    }

    while(!context.packets_audio.isEmpty()) {
        AVPacket p=context.packets_audio.takeFirst();

        av_packet_unref(&p);
    }

    if(context.codec_context_video) {
        avcodec_free_context(&context.codec_context_video);

        context.codec_context_video=nullptr;
    }

    if(context.codec_context_audio) {
        avcodec_free_context(&context.codec_context_audio);

        context.codec_context_audio=nullptr;
    }

    if(context.format_context) {
        avformat_close_input(&context.format_context);

        context.format_context=nullptr;
    }

    if(context.convert_context_video) {
        sws_freeContext(context.convert_context_video);

        context.convert_context_video=nullptr;
    }

    if(context.convert_context_audio) {
        swr_free(&context.convert_context_audio);

        context.convert_context_audio=nullptr;
    }

    if(context.frame_video) {
        av_frame_free(&context.frame_video);

        context.frame_video=nullptr;
    }

    if(context.frame_audio) {
        av_frame_free(&context.frame_audio);

        context.frame_audio=nullptr;
    }

    if(context.frame_cnv) {
        av_frame_free(&context.frame_cnv);

        context.frame_cnv=nullptr;
    }

    // context.audio_converter.free();

    context.ba_audio.clear();

    state=ST_STOPPED;
}

#define AV_SYNC_THRESHOLD 0.01
#define AV_NOSYNC_THRESHOLD 10.0

int64_t FFDecoderThread::getAudioClock()
{
    int64_t pts=context.audio_clock;

    int64_t bytes_per_sec=context.codec_context_audio->sample_rate*
            context.codec_context_audio->channels*
            av_get_bytes_per_sample(context.codec_context_audio->sample_fmt);

    if(bytes_per_sec!=0)
        pts-=((double)context.audio_buf_size/(double)bytes_per_sec)*1000000;

    return pts;
}

int64_t FFDecoderThread::computeDelay()
{
    int64_t delay=context.pict_pts - context.frame_last_pts;

    if(delay<=0 || delay>=1000000) {
        // qInfo() << "delay out of range" << delay;
        // Delay incorrect - use previous one
        delay=context.frame_last_delay;
    }


    // Save for next time
    context.frame_last_pts=context.pict_pts;
    context.frame_last_delay=delay;

    // Update delay to sync to audio
    int64_t ref_clock=getAudioClock();
    int64_t diff=context.pict_pts - ref_clock;
    int64_t sync_threshold=FFMAX((int64_t)(AV_SYNC_THRESHOLD*1000000), delay);

    if(fabs(diff)<AV_NOSYNC_THRESHOLD*1000000) {
        // qWarning() << "! fabs(diff)<AV_NOSYNC_THRESHOLD" << diff << AV_NOSYNC_THRESHOLD*1000000 << context.pict_pts_i << ref_clock;

        if(diff<=-sync_threshold) {
            // qWarning() << "! diff<=-sync_threshold";

            delay=0;

        } else if(diff>=sync_threshold) {
            // qWarning() << "! diff>=sync_threshold";

            // delay=2*delay;
        }
    }


    context.frame_timer+=delay;


    int64_t actual_delay=context.frame_timer - av_gettime();

    // if(actual_delay<.0010*1000000)
    //     actual_delay=.0010*1000000;

    if(actual_delay<0)
        actual_delay=0;

    return actual_delay;
}

int64_t FFDecoderThread::synchronizeVideo(AVFrame *src_frame, int64_t pts)
{
    if(pts!=0) {
        // if we have pts, set video clock to it
        context.video_clock=pts;

    } else {
        // if we aren't given a pts, set it to the clock
        pts=context.video_clock;
    }

    // update the video clock
    int64_t frame_delay=av_q2d(context.codec_context_video->time_base)*1000000;

    // if we are repeating a frame, adjust clock accordingly
    frame_delay+=src_frame->repeat_pict*(frame_delay*.5);

    context.video_clock+=frame_delay;

    return pts;
}

void FFDecoderThread::run()
{
    av_init_packet(&context.packet);

    running=true;

    while(running) {
        if(state!=prev_state) {
            prev_state=(int)state;

            emit stateChanged(state);
        }

        switch(state) {
        case ST_OPEN:
            _open();
            break;

        case ST_PLAY:
            _play();
            break;

        case ST_SEEK:
            _seek();
            break;

        case ST_STOPPING: {
            freeContext();
            state=ST_STOPPED;

        } break;

        case ST_IDLE:
        case ST_STOPPED: {
            msleep(100);

        } break;

        default:
            break;
        }
    }
}

