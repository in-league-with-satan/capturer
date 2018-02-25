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

#include <QDebug>
#include <QApplication>
#include <QImage>
#include <QBuffer>
#include <QTimer>
#include <QElapsedTimer>
#include <QMutexLocker>

#include "database.h"
#include "ff_tools.h"

#include "ff_snapshot.h"

QByteArray imgToJpegByteArray(const QImage &img, int jpeg_quality=85)
{
    QByteArray ba;

    if(img.isNull())
        return ba;

    QBuffer buffer(&ba);

    buffer.open(QIODevice::WriteOnly);

    img.save(&buffer, "JPG", jpeg_quality);

    buffer.close();

    return ba;
}

QImage imgFromJpegByteArray(QByteArray &data)
{
    QBuffer buffer(&data);

    buffer.open(QIODevice::ReadWrite);

    QImage img;

    img.load(&buffer, "JPG");

    return img;
}

QByteArray imageListToByteArray(const QList <QImage> &images)
{
    QByteArray ba;

    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);

    QDataStream stream(&buffer);

    foreach(const QImage &img, images) {
        QByteArray ba_tmp=imgToJpegByteArray(img);

        stream << (qint32)ba_tmp.size();
        buffer.write(ba_tmp);
    }

    buffer.close();

    return ba;
}

QList <QImage> imageListFromByteArray(QByteArray &ba)
{
    QBuffer buffer(&ba);
    buffer.open(QIODevice::ReadWrite);

    QDataStream stream(&buffer);

    qint32 size;
    QList <QImage> list;
    QByteArray ba_tmp;

    while(true) {
        if(buffer.atEnd())
            break;

        stream >> size;

        ba_tmp=buffer.read(size);

        list.append(imgFromJpegByteArray(ba_tmp));
    }

    return list;
}

FFSnapshot::FFSnapshot(QObject *paretn)
    : QThread(paretn)
{
    accurate_seek=false;
    shots_per_10_min=4;
    on_pause=true;
    viewer_visible=false;

    start();
}

void FFSnapshot::enqueue(const QString &filename)
{
    QMutexLocker ml(&mutex_queue);

    queue.enqueue(filename);

    if(viewer_visible)
        on_pause=false;
}

void FFSnapshot::pause(bool state)
{
    on_pause=state;
}

void FFSnapshot::viewerVisible(bool value)
{
    viewer_visible=value;

    if(viewer_visible)
        on_pause=false;
}

void FFSnapshot::run()
{
    database=new Database();
    database->moveToThread(this);

    timer=new QTimer();
    timer->moveToThread(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), SLOT(checkQueue()), Qt::DirectConnection);

    timer->start();

    exec();

    delete database;
}

void FFSnapshot::checkQueue()
{
    if(on_pause)
        return;

    timer->stop();

    QString filename;

    while(true) {
        {
            QMutexLocker ml(&mutex_queue);

            if(queue.isEmpty()) {
                on_pause=true;
                timer->start();
                return;
            }

            filename=queue.dequeue();
        }

        // qInfo() << "FFSnapshot:" << filename;

        QList <QImage> images;

        {
            QByteArray ba_images;

            database->snapshot(filename, &ba_images);

            if(!ba_images.isEmpty()) {
                QElapsedTimer t;
                t.start();

                images=imageListFromByteArray(ba_images);

                foreach(const QImage &img, images) {
                    emit ready(filename, img.convertToFormat(QImage::Format_RGB16));
                }

                QApplication::processEvents();

                // qInfo().noquote() << QStringLiteral("snapshots from db time:") << t.elapsed() << QStringLiteral("ms");

                continue;
            }
        }

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

        ret=avformat_open_input(&format_context, filename.toUtf8().constData(), nullptr, nullptr);

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


        // if(codec_video->capabilities & CODEC_CAP_TRUNCATED)
        //     codec_context_video->flags|=CODEC_FLAG_TRUNCATED;

        // codec_context_video->flags2|=CODEC_FLAG2_FAST;

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
            while(on_pause)
                sleep(1);

            qInfo().nospace() << "frame: " << i_frame << "/" << shots_count - 1;

            timestamp=av_rescale_q(i_frame*step*AV_TIME_BASE, av_get_time_base_q(), stream_video->time_base);

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

                                int data_size=av_image_get_buffer_size(AV_PIX_FMT_BGRA, frame_rgb->width, frame_rgb->height, alignment);

                                ba_frame.resize(data_size);

                                av_image_copy_to_buffer((uint8_t*)ba_frame.constData(), data_size, frame_rgb->data, frame_rgb->linesize, AV_PIX_FMT_BGRA, frame_rgb->width, frame_rgb->height, alignment);
                            }
                        }
                    }

                    av_packet_unref(&packet);

                    if(!ba_frame.isEmpty()) {
                        QImage img=QImage((uchar*)ba_frame.constData(),
                                          frame_rgb->width,
                                          frame_rgb->height,
                                          QImage::Format_ARGB32).scaledToWidth(640, Qt::SmoothTransformation);

                        images.append(img);

                        emit ready(filename, img.convertToFormat(QImage::Format_RGB16));

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

        if(!images.isEmpty()) {
            QByteArray ba_images=imageListToByteArray(images);

            database->addSnapshot(filename, ba_images);
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
