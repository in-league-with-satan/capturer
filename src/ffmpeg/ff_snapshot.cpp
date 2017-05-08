#include <QDebug>
#include <QApplication>
#include <QImage>
#include <QBuffer>
#include <QTimer>
#include <QMutexLocker>

#include "ff_tools.h"

#include "ff_snapshot.h"


FFSnapshot::FFSnapshot(QObject *paretn)
    : QThread(paretn)
{
    accurate_seek=false;
    shots_per_10_min=4;

    start();
}

void FFSnapshot::enqueue(QString file)
{
    QMutexLocker ml(&mutex_queue);

    queue.enqueue(file);
}

void FFSnapshot::run()
{
    timer=new QTimer();
    timer->moveToThread(this);
    timer->setInterval(10);
    connect(timer, SIGNAL(timeout()), SLOT(checkQueue()), Qt::DirectConnection);

    timer->start();

    exec();
}

void FFSnapshot::checkQueue()
{
    timer->stop();

    QString filename;

    while(true) {
        {
            QMutexLocker ml(&mutex_queue);

            if(queue.isEmpty()) {
                timer->start();
                return;
            }

            filename=queue.dequeue();
        }

        qInfo() << "FFSnapshot:" << filename;

        int ret;

        AVFormatContext *format_context=nullptr;
        AVCodecContext *codec_context_video=nullptr;
        AVCodec *codec_video=nullptr;
        AVStream *stream_video=nullptr;
        SwsContext *convert_context=nullptr;
        AVFrame *frame=av_frame_alloc();
        AVFrame *frame_rgb=nullptr;
        AVPacket packet;

        unsigned int stream_video_index;

        QByteArray ba_frame;

        double per_10_min_count;
        int shots_count;
        int64_t duration;
        int64_t step;

        int64_t timestamp;

        ret=avformat_open_input(&format_context, filename.toUtf8().data(), nullptr, nullptr);

        if(ret<0) {
            qCritical() << "ffmpeg: Unable to open input file" << ffErrorString(ret);
            goto end;
        }

        // Retrieve stream information
        ret=avformat_find_stream_info(format_context, nullptr);

        if(ret<0) {
            qCritical() << "ffmpeg: Unable to find stream info" << ffErrorString(ret);
            goto end;
        }

        // { video init

        for(stream_video_index=0; stream_video_index<format_context->nb_streams; ++stream_video_index) {
            if(format_context->streams[stream_video_index]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
                break;
            }
        }

        if(stream_video_index==format_context->nb_streams) {
            qCritical() << "ffmpeg: Unable to find video stream";
            goto end;
        }

        stream_video=format_context->streams[stream_video_index];

        codec_video=avcodec_find_decoder(stream_video->codecpar->codec_id);

        codec_context_video=avcodec_alloc_context3(codec_video);

        if(!codec_context_video) {
            qCritical() << "Could not allocate a decoding context";
            goto end;
        }

        ret=avcodec_parameters_to_context(codec_context_video, format_context->streams[stream_video_index]->codecpar);

        if(ret<0) {
            qCritical() << "ffmpeg: avcodec_parameters_to_context" << ffErrorString(ret);
            goto end;
        }

        if(codec_context_video->pix_fmt==AV_PIX_FMT_NONE) {
            qCritical() << "pix format err";
            goto end;
        }


        if(codec_video->capabilities & CODEC_CAP_TRUNCATED)
            codec_context_video->flags|=CODEC_FLAG_TRUNCATED;

        codec_context_video->flags2|=CODEC_FLAG2_FAST;

        codec_context_video->thread_count=QThread::idealThreadCount();

        av_opt_set(codec_context_video->priv_data, "tune", "fastdecode", 0);


        ret=avcodec_open2(codec_context_video, codec_video, nullptr);

        if(ret<0) {
            qCritical() << "ffmpeg: Unable to open codec" << ffErrorString(ret);
            goto end;
        }


        convert_context=sws_getContext(codec_context_video->width, codec_context_video->height,
                                       correctPixelFormat(codec_context_video->pix_fmt),
                                       codec_context_video->width, codec_context_video->height,
                                       AV_PIX_FMT_BGRA, SWS_FAST_BILINEAR,
                                       nullptr, nullptr, nullptr);

        if(convert_context==nullptr) {
            qCritical() << "Cannot initialize the conversion context";
            goto end;
        }


        if(codec_context_video->width%8!=0 || codec_context_video->height%8!=0)
            frame_rgb=alloc_frame(AV_PIX_FMT_BGRA, ceil(codec_context_video->width/8.)*8, ceil(codec_context_video->height/8.)*8);

        else
            frame_rgb=alloc_frame(AV_PIX_FMT_BGRA, codec_context_video->width, codec_context_video->height);


        // } video init

        accurate_seek=codec_video->id!=AV_CODEC_ID_H264;

        //

        duration=format_context->duration/AV_TIME_BASE;

        per_10_min_count=duration/600.;

        if(per_10_min_count<1.)
            per_10_min_count=1.;

        shots_count=per_10_min_count*shots_per_10_min + 1;

        if(shots_count>33)
            shots_count=33;

        step=duration/shots_count;

        //


        for(int i_frame=1; i_frame<shots_count; i_frame++) {
            qInfo().nospace() << "frame: " << i_frame << "/" << shots_count - 1;

            timestamp=av_rescale_q(i_frame*step*AV_TIME_BASE, AV_TIME_BASE_Q, stream_video->time_base);

            if(stream_video->start_time!=AV_NOPTS_VALUE)
                timestamp+=stream_video->start_time;

            if(accurate_seek)
                ret=av_seek_frame(format_context, stream_video_index, timestamp, AVSEEK_FLAG_BACKWARD);

            else
                ret=av_seek_frame(format_context, stream_video_index, timestamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);

            if(ret<0) {
                qCritical() << "ffmpeg: av_seek_frame err";
                goto end;
            }

            avcodec_flush_buffers(codec_context_video);

            while(true) {
                ret=av_read_frame(format_context, &packet);

                if(ret>=0) {
                    if(packet.stream_index==stream_video->index) {
                        ret=avcodec_send_packet(codec_context_video, &packet);

                        if(ret<0) {
                            qCritical() << "ffmpeg: avcodec_send_packet" << ffErrorString(ret);
                            goto end;
                        }

                        ret=avcodec_receive_frame(codec_context_video, frame);

                        if(ret>=0) {
                            int64_t pts=packet.pts;

                            if(pts==AV_NOPTS_VALUE)
                                pts=packet.dts;

                            // qInfo() << i_frame << shots_count << timestamp << pts << packet.duration;

                            if(i_frame>1 && pts<1) {
                                qCritical() << "seek err";
                                goto end;
                            }

                            if(abs(timestamp - pts)<=packet.duration || pts>timestamp) {
                                sws_scale(convert_context,
                                          frame->data, frame->linesize,
                                          0, codec_context_video->height,
                                          frame_rgb->data, frame_rgb->linesize);

                                //

                                int data_size=av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame_rgb->width, frame_rgb->height, 32);

                                ba_frame.resize(data_size);

                                av_image_copy_to_buffer((uint8_t*)ba_frame.data(), data_size, frame_rgb->data, frame_rgb->linesize, AV_PIX_FMT_BGRA, frame_rgb->width, frame_rgb->height, 32);
                            }
                        }
                    }

                    av_packet_unref(&packet);

                    if(!ba_frame.isEmpty()) {
                        QImage img((uchar*)ba_frame.data(),
                                   frame_rgb->width,
                                   frame_rgb->height,
                                   QImage::Format_ARGB32);

                        // emit ready(filename, img.scaledToWidth(384, Qt::SmoothTransformation));
                        // emit ready(filename, img.scaledToWidth(384, Qt::SmoothTransformation).convertToFormat(QImage::Format_Indexed8));
                        emit ready(filename, img.scaledToWidth(640, Qt::SmoothTransformation).convertToFormat(QImage::Format_RGB16));

                        QApplication::processEvents();

                        ba_frame.clear();

                        break;
                    }

                } else {
                    qCritical() << "av_read_frame" << ffErrorString(ret);

                    goto end;
                }

                QApplication::processEvents();
            }
        }

end:

        //

        if(frame)
            av_frame_unref(frame);

        if(frame_rgb)
            av_frame_unref(frame_rgb);

        if(codec_context_video)
            avcodec_free_context(&codec_context_video);

        if(format_context)
            avformat_close_input(&format_context);

        if(convert_context)
            sws_freeContext(convert_context);
    }
}
