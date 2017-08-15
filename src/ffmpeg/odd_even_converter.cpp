#include <QDebug>
#include <QDateTime>
#include <QElapsedTimer>
#include <QImage>
#include <QQueue>
#include <qcoreapplication.h>

#include <iostream>

#include "ff_tools.h"

#include "odd_even_converter.h"


OddEvenConverterThread::OddEvenConverterThread(QObject *parent)
    : QThread(parent)
{
    av_register_all();
}

OddEvenConverterThread::~OddEvenConverterThread()
{
}

bool OddEvenConverterThread::start(OddEvenConverterThread::Config cfg)
{
    if(isRunning())
        return false;

    config=cfg;

    QThread::start(QThread::NormalPriority);

    return true;
}

void OddEvenConverterThread::run()
{
    if(QString(config.filename_src).isEmpty()) {
        return;
    }

    int ret;

    AVFormatContext *in_format_context=nullptr;
    AVFormatContext *out_format_context=nullptr;

    AVStream *in_stream_video_odd=nullptr;
    AVStream *in_stream_video_even=nullptr;
    AVStream *in_stream_audio=nullptr;
    AVStream *out_stream_video=nullptr;
    AVStream *out_stream_audio=nullptr;

    AVCodec *in_codec_video_odd=nullptr;
    AVCodec *in_codec_video_even=nullptr;
    AVCodec *in_codec_audio=nullptr;
    AVCodec *out_codec_video=nullptr;
    AVCodec *out_codec_audio=nullptr;

    AVCodecContext *in_codec_context_odd=nullptr;
    AVCodecContext *in_codec_context_even=nullptr;
    AVCodecContext *in_codec_context_audio=nullptr;
    AVCodecContext *out_codec_context_video=nullptr;
    AVCodecContext *out_codec_context_audio=nullptr;

    SwsContext *convert_context=nullptr;

    AVFrame *in_frame=av_frame_alloc();
    AVFrame *out_frame=nullptr;
    AVFrame frame;

    QQueue <AVFrame> odd_frame_queue;
    QQueue <AVFrame> even_frame_queue;

    QQueue <double_t> frame_processing_time;
    QElapsedTimer timer;

    AVPacket packet;
    AVPacket *pkt;

    size_t pts_audio=0;
    size_t frame_counter=0;

    qint64 last_time_stats_out=0;

    bool odd_turn=true;
    bool frame_ready;

    int64_t duration;
    int max_string_size=0;
    int64_t total_frames=0;


    av_init_packet(&packet);

    ret=avformat_open_input(&in_format_context, QString(config.filename_src).toUtf8().constData(), nullptr, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: unable to open input file" << config.filename_src << ffErrorString(ret);
        goto end;
    }


    // Retrieve stream information
    ret=avformat_find_stream_info(in_format_context, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: unable to find stream info:" << ffErrorString(ret);
        goto end;
    }


    for(unsigned int i=0; i<in_format_context->nb_streams; ++i) {
        if(in_format_context->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            if(!(config.swap_the_frame_order ? in_stream_video_even : in_stream_video_odd ))
                (config.swap_the_frame_order ? in_stream_video_even : in_stream_video_odd)=in_format_context->streams[i];

            else if(!(config.swap_the_frame_order ? in_stream_video_odd : in_stream_video_even))
                (config.swap_the_frame_order ? in_stream_video_odd : in_stream_video_even)=in_format_context->streams[i];
        }

        if(in_format_context->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
            if(!in_stream_audio)
                in_stream_audio=in_format_context->streams[i];
        }
    }

    if(!in_stream_video_odd || !in_stream_video_even || !in_stream_audio) {
        qCritical() << "OddEvenConverter: unable to find stream";
        goto end;
    }

    in_codec_video_odd=avcodec_find_decoder(in_stream_video_odd->codecpar->codec_id);
    in_codec_video_even=avcodec_find_decoder(in_stream_video_even->codecpar->codec_id);
    in_codec_audio=avcodec_find_decoder(in_stream_audio->codecpar->codec_id);

    in_codec_context_odd=avcodec_alloc_context3(in_codec_video_odd);
    in_codec_context_even=avcodec_alloc_context3(in_codec_video_even);
    in_codec_context_audio=avcodec_alloc_context3(in_codec_audio);

    ret=avcodec_parameters_to_context(in_codec_context_odd, in_stream_video_odd->codecpar);

    if(ret<0) {
        qCritical() << "OddEvenConverter: avcodec_parameters_to_context err:" << ffErrorString(ret);
        goto end;
    }

    ret=avcodec_parameters_to_context(in_codec_context_even, in_stream_video_even->codecpar);

    if(ret<0) {
        qCritical() << "OddEvenConverter: avcodec_parameters_to_context err:" << ffErrorString(ret);
        goto end;
    }

    ret=avcodec_parameters_to_context(in_codec_context_audio, in_stream_audio->codecpar);

    if(ret<0) {
        qCritical() << "OddEvenConverter: avcodec_parameters_to_context err:" << ffErrorString(ret);
        goto end;
    }


    ret=avcodec_open2(in_codec_context_odd, in_codec_video_odd, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: unable to open codec:" << ffErrorString(ret);
        goto end;
    }

    ret=avcodec_open2(in_codec_context_even, in_codec_video_even, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: unable to open codec:" << ffErrorString(ret);
        goto end;
    }

    ret=avcodec_open2(in_codec_context_audio, in_codec_audio, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: unable to open codec:" << ffErrorString(ret);
        goto end;
    }


    // allocate the output media context
    avformat_alloc_output_context2(&out_format_context, nullptr, "matroska", nullptr);

    if(!out_format_context) {
        qCritical() << "OddEvenConverter: could not deduce output format";
        goto end;
    }

    // add the audio and video streams using the default format codecs
    // and initialize the codecs.

    switch(config.video_encoder) {
    case VideoEncoder::libx264:
        out_codec_video=avcodec_find_encoder_by_name("libx264");
        break;

    case VideoEncoder::libx265:
        out_codec_video=avcodec_find_encoder_by_name("libx265");
        break;

    case VideoEncoder::nvenc_h264:
        out_codec_video=avcodec_find_encoder_by_name("h264_nvenc");
        break;
    }

    if(!out_codec_video) {
        qCritical() << "OddEvenConverter: could not find encoder";
        goto end;
    }


    out_stream_video=avformat_new_stream(out_format_context, nullptr);

    if(!out_stream_video) {
        qCritical() << "OddEvenConverter: could not allocate stream";
        goto end;
    }

    out_stream_video->id=out_format_context->nb_streams - 1;


    out_codec_context_video=avcodec_alloc_context3(out_codec_video);

    if(!out_codec_context_video) {
        qCritical() << "OddEvenConverter: could not alloc an encoding context";
        goto end;
    }


    out_codec_context_video->codec_id=out_codec_video->id;

    out_codec_context_video->width=in_codec_context_odd->width;
    out_codec_context_video->height=in_codec_context_odd->height;


    out_frame=alloc_frame((AVPixelFormat)config.pixel_format, in_codec_context_odd->width, in_codec_context_odd->height);

    convert_context=sws_getContext(in_codec_context_odd->width, in_codec_context_odd->height,
                                   in_codec_context_odd->pix_fmt,
                                   in_codec_context_odd->width, in_codec_context_odd->height,
                                   (AVPixelFormat)config.pixel_format,
                                   SWS_AREA, nullptr, nullptr, nullptr);

    {
        AVRational framerate=av_guess_frame_rate(in_format_context, in_stream_video_odd, nullptr);

        out_stream_video->time_base={ framerate.den, framerate.num*2 };
    }

    out_codec_context_video->time_base=out_stream_video->time_base;

    out_codec_context_video->gop_size=12;

    out_codec_context_video->pix_fmt=(AVPixelFormat)config.pixel_format;


    // av_opt_set(enc_codec_context->priv_data, "preset", "ultrafast", 0);
    // av_opt_set(enc_codec_context->priv_data, "preset", "slow", 0);
    av_opt_set(out_codec_context_video->priv_data, "crf", QString::number(config.crf).toLatin1().data(), 0);

    if(config.video_encoder==VideoEncoder::nvenc_h264) {
        if(config.crf==0)
            av_opt_set(out_codec_context_video->priv_data, "preset", "lossless", 0);

        else
            out_codec_context_video->global_quality=config.crf;
    }

    if(out_codec_context_video->codec_id==AV_CODEC_ID_MPEG2VIDEO) {
        // just for testing, we also add B-frames
        out_codec_context_video->max_b_frames=2;
    }

    if(out_codec_context_video->codec_id==AV_CODEC_ID_MPEG1VIDEO) {
        // needed to avoid using macroblocks in which some coeffs overflow.
        // this does not happen with normal video, it just happens here as
        // the motion of the chroma plane does not match the luma plane
        out_codec_context_video->mb_decision=2;
    }

    // some formats want stream headers to be separate
    if(out_format_context->oformat->flags & AVFMT_GLOBALHEADER)
        out_codec_context_video->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;


    // open the codec
    ret=avcodec_open2(out_codec_context_video, out_codec_video, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: could not open video codec:" << ffErrorString(ret);
        goto end;
    }


    out_stream_audio=avformat_new_stream(out_format_context, nullptr);

    if(!out_stream_audio) {
        qCritical() << "OddEvenConverter: could not allocate stream";
        goto end;
    }

    out_stream_audio->id=out_format_context->nb_streams - 1;

    out_codec_audio=avcodec_find_encoder(in_stream_audio->codecpar->codec_id);

    if(!out_codec_audio) {
        qCritical() << "OddEvenConverter: could not find encoder";
        goto end;
    }


    out_codec_context_audio=avcodec_alloc_context3(out_codec_audio);

    if(!out_codec_context_audio) {
        qCritical() << "OddEvenConverter: could not alloc an encoding context";
        goto end;
    }


    out_codec_context_audio->bit_rate=in_stream_audio->codecpar->bit_rate;
    out_codec_context_audio->sample_fmt=(AVSampleFormat)in_stream_audio->codecpar->format;
    out_codec_context_audio->sample_rate=in_stream_audio->codecpar->sample_rate;
    out_codec_context_audio->channel_layout=in_stream_audio->codecpar->channel_layout;
    out_codec_context_audio->channels=in_stream_audio->codecpar->channels;

    out_stream_audio->time_base={ 1, in_codec_context_audio->sample_rate };


    // some formats want stream headers to be separate
    if(out_format_context->oformat->flags & AVFMT_GLOBALHEADER)
        out_codec_context_audio->flags|=AV_CODEC_FLAG_GLOBAL_HEADER;


    ret=avcodec_open2(out_codec_context_audio, out_codec_audio, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: could not open audio codec:" << ffErrorString(ret);
        goto end;
    }


    // copy the stream parameters to the muxer
    ret=avcodec_parameters_from_context(out_stream_video->codecpar, out_codec_context_video);

    if(ret<0) {
        qCritical() << "OddEvenConverter: could not copy the stream parameters:" << ffErrorString(ret);
        goto end;
    }

    ret=avcodec_parameters_from_context(out_stream_audio->codecpar, out_codec_context_audio);

    if(ret<0) {
        qCritical() << "OddEvenConverter: could not copy the stream parameters:" << ffErrorString(ret);
        goto end;
    }


    av_dump_format(out_format_context, 0, "", 1);

    // open the output file
    ret=avio_open(&out_format_context->pb, config.filename_dst.toLatin1().constData(), AVIO_FLAG_WRITE);


    if(ret<0) {
        qCritical() << "OddEvenConverter: could not open:" << config.filename_dst << ffErrorString(ret);
        goto end;
    }


    ret=avformat_write_header(out_format_context, nullptr);

    if(ret<0) {
        qCritical() << "OddEvenConverter: format_context_write err:" << config.filename_dst << ffErrorString(ret);
        goto end;
    }

    timer.restart();

    last_time_stats_out=QDateTime::currentMSecsSinceEpoch();

    duration=in_format_context->duration/AV_TIME_BASE;

    total_frames=av_q2d({ out_codec_context_video->time_base.den, out_codec_context_video->time_base.num })*duration;


    std::cout << std::endl;

    while(true) {
        if(av_read_frame(in_format_context, &packet)>=0) {
            if(packet.stream_index==in_stream_video_odd->index) {
                if(avcodec_send_packet(in_codec_context_odd, &packet)==0) {
                    ret=avcodec_receive_frame(in_codec_context_odd, in_frame);

                    if(ret>=0) {
                        odd_frame_queue.enqueue(*in_frame);
                    }
                }

            } else if(packet.stream_index==in_stream_video_even->index) {
                if(avcodec_send_packet(in_codec_context_even, &packet)==0) {
                    ret=avcodec_receive_frame(in_codec_context_even, in_frame);

                    if(ret>=0) {
                        even_frame_queue.enqueue(*in_frame);
                    }
                }

            } else if(packet.stream_index==in_stream_audio->index) {
                if(avcodec_send_packet(in_codec_context_audio, &packet)==0) {
                    ret=avcodec_receive_frame(in_codec_context_audio, in_frame);

                    if(ret>=0) {
                        ret=avcodec_send_frame(out_codec_context_audio, in_frame);

                        pts_audio+=in_frame->nb_samples;

                        out_frame->pts=pts_audio;

                        if(ret<0) {
                            qCritical() << "OddEvenConverter: error encoding audio frame:" << ffErrorString(ret);
                            goto end;
                        }

                        pkt=av_packet_alloc();

                        while(!ret) {
                            ret=avcodec_receive_packet(out_codec_context_audio, pkt);

                            if(!ret) {
                                pkt->stream_index=out_stream_audio->index;

                                // write the compressed frame to the media file
                                av_interleaved_write_frame(out_format_context, pkt);
                            }
                        }

                        av_packet_free(&pkt);
                    }
                }
            }

            av_packet_unref(&packet);

        } else {
            // qInfo() << "eof";

            if(odd_frame_queue.isEmpty() || even_frame_queue.isEmpty()) {
                break;
            }
        }

        if(!odd_frame_queue.isEmpty() || even_frame_queue.isEmpty()) {
            frame_ready=false;

            if(odd_turn && !odd_frame_queue.isEmpty()) {
                frame=odd_frame_queue.dequeue();
                frame_ready=true;
            }

            if(!odd_turn && !even_frame_queue.isEmpty()) {
                frame=even_frame_queue.dequeue();
                frame_ready=true;
            }

            if(frame_ready) {
                sws_scale(convert_context, frame.data, frame.linesize, 0, frame.height, out_frame->data, out_frame->linesize);

                out_frame->pts=frame_counter;

                ret=avcodec_send_frame(out_codec_context_video, out_frame);

                if(ret<0) {
                    qCritical() << "OddEvenConverter: encoding video frame err:" << ffErrorString(ret);
                    goto end;
                }

                pkt=av_packet_alloc();

                while(!ret) {
                    ret=avcodec_receive_packet(out_codec_context_video, pkt);

                    if(!ret) {
                        // rescale output packet timestamp values from codec to stream timebase
                        av_packet_rescale_ts(pkt, out_codec_context_video->time_base, out_stream_video->time_base);

                        pkt->stream_index=out_stream_video->index;

                        // write the compressed frame to the media file
                        ret=av_interleaved_write_frame(out_format_context, pkt);

                        if(ret<0) {
                            qCritical() << "OddEvenConverter: av_interleaved_write_frame err:" << ffErrorString(ret);
                        }
                    }
                }

                av_packet_free(&pkt);

                frame_processing_time.enqueue(timer.nsecsElapsed());

                if(frame_processing_time.size()>1000)
                    frame_processing_time.dequeue();

                timer.restart();

                frame_counter++;


                if(QDateTime::currentMSecsSinceEpoch() - last_time_stats_out>=1000) {
                    double_t avg_time=0.;

                    for(int i=0; i<frame_processing_time.size(); ++i)
                        avg_time+=frame_processing_time.at(i);


                    avg_time/=frame_processing_time.size();


                    double fps=(QDateTime::currentMSecsSinceEpoch() - last_time_stats_out)*1000000./avg_time;

                    last_time_stats_out=QDateTime::currentMSecsSinceEpoch();


                    QDateTime time_estimated=QDateTime(QDate(1, 1, 1), QTime(0, 0)).addSecs((total_frames - frame_counter)/fps);


                    QString str=QString("  completed: %1%   fps: %2   estimated time: %3")
                            .arg(QString::number((double)frame_counter/total_frames*100., 'f', 2))
                            .arg(QString::number(fps, 'f', 2))
                            .arg(time_estimated.date().day() - 1 >= 1
                                 ?
                                     QString("%1 days %2")
                                     .arg(time_estimated.date().day() - 1)
                                     .arg(time_estimated.toString("HH:mm:ss"))
                                   :
                                     time_estimated.toString("HH:mm:ss"));

                    max_string_size=qMax(max_string_size, str.size());

                    str.append(QString().fill(' ', (max_string_size>str.size() ? max_string_size - str.size() : 0)));


                    std::cout << str.toLatin1().data() << "\r";
                }

                odd_turn=!odd_turn;
            }
        }
    }

    av_write_trailer(out_format_context);

    std::cout << std::endl;


end:

    if(in_codec_context_odd)
        avcodec_free_context(&in_codec_context_odd);

    if(in_codec_context_even)
        avcodec_free_context(&in_codec_context_even);

    if(in_codec_context_audio)
        avcodec_free_context(&in_codec_context_audio);

    if(out_codec_context_video)
        avcodec_free_context(&out_codec_context_video);

    if(out_codec_context_video)
        avcodec_free_context(&out_codec_context_audio);

    if(in_frame)
        av_frame_free(&in_frame);

    if(out_frame)
        av_frame_free(&out_frame);

    if(convert_context)
        sws_freeContext(convert_context);

    if(in_format_context) {
        avio_closep(&in_format_context->pb);
        avformat_free_context(in_format_context);
    }

    if(out_format_context) {
        avio_closep(&out_format_context->pb);
        avformat_free_context(out_format_context);
    }
}
