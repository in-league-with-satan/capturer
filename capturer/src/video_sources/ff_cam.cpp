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
#include <QTimer>
#include <QAudioInput>
#include <qcoreapplication.h>

#include <assert.h>
#include <cmath>

#include "ff_format_converter.h"
#include "ff_audio_converter.h"

#include "tools_cam.h"

#include "ff_cam.h"


QList <Cam::Dev> dev_list;


QAudioFormat default_format;

struct FFCamPrivate
{
    AVInputFormat *input_format=nullptr;
    AVCodec *codec=nullptr;

    AVCodecContext *codec_context=nullptr;

    AVFormatContext *format_context=nullptr;

    AVStream *stream=nullptr;

    AVDictionary *dictionary=nullptr;

    AVFrame *frame=nullptr;

    FFFormatConverter converter;

    //

    QAudioInput *audio_input=nullptr;
    QIODevice *audio_device=nullptr;

    AudioConverter audio_converter;
};

FFCam::FFCam(QObject *parent)
    : QThread(parent)
{
    default_format.setSampleRate(48000);
    default_format.setChannelCount(2);
    default_format.setSampleSize(16);
    default_format.setCodec("audio/pcm");
    default_format.setByteOrder(QAudioFormat::LittleEndian);
    default_format.setSampleType(QAudioFormat::UnSignedInt);
    // default_format.setSampleType(QAudioFormat::SignedInt);

    //

    d=new FFCamPrivate();

    start();
}

FFCam::~FFCam()
{
    stop();

    quit();

    running=false;

    while(isRunning()) {
        msleep(30);
    }

    delete d;
}

QString FFCam::formatString(const QAudioFormat &format)
{
    QString str=QString("rate: %1hz; channels: %2; depth: %3; %4")
            .arg(format.sampleRate())
            .arg(format.channelCount())
            .arg(format.sampleSize())
            .arg(format.byteOrder()==QAudioFormat::LittleEndian ? "le" : "be")
            ;

    switch(format.sampleType()) {
    case QAudioFormat::Unknown:
        str+="; wtf?!!!!";
        break;

    case QAudioFormat::SignedInt:
        str+="; signed";
        break;

    case QAudioFormat::UnSignedInt:
        str+="; unsigned";
        break;

    case QAudioFormat::Float:
        str+="; float";
        break;
    }

    return str;
}

AVSampleFormat FFCam::qAudioFormatToAV(const int &depth, const QAudioFormat::SampleType &sample_format)
{
    switch(sample_format) {
    case QAudioFormat::Unknown:
        return AV_SAMPLE_FMT_NONE;

    case QAudioFormat::SignedInt:
        if(depth==8)
            return AV_SAMPLE_FMT_NONE;

        if(depth==16)
            return AV_SAMPLE_FMT_S16;

        if(depth==32)
            return AV_SAMPLE_FMT_S32;

        return AV_SAMPLE_FMT_NONE;

    case QAudioFormat::UnSignedInt:
        if(depth==8)
            return AV_SAMPLE_FMT_U8;

        return AV_SAMPLE_FMT_NONE;

    case QAudioFormat::Float:
        return AV_SAMPLE_FMT_FLT;
    }

    return AV_SAMPLE_FMT_NONE;
}

QStringList FFCam::availableCameras()
{
    QStringList list;

    if(dev_list.isEmpty())
        updateDevList();

    foreach(Cam::Dev dev, dev_list) {
        list << dev.name;
    }

    return list;
}

QStringList FFCam::availableAudioInput()
{
    QStringList list;

    foreach(QAudioDeviceInfo di, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
        list << di.deviceName();
    }

    return list;
}

bool FFCam::setVideoDevice(int index)
{
    if(index<0 || index>=dev_list.size())
        return false;

    index_device_video=index;

    return true;
}

void FFCam::setAudioDevice(int index)
{
    index_device_audio=index;
}

QList <QSize> FFCam::supportedResolutions()
{
    if(dev_list.size() - 1<index_device_video)
        return QList<QSize>();

    QList <QSize> lst;

    QMap <int64_t, QList <QSize>> res_per_format;

    foreach(Cam::Format format, dev_list.at(index_device_video).format) {
        foreach(Cam::Resolution res, format.resolution) {
            res_per_format[format.pixel_format].append(res.size);

            if(!lst.contains(res.size)) {
                lst << res.size;
            }
        }
    }

    for(int i_resolution=0; i_resolution<lst.size(); ++i_resolution) {
        for(int i_fmt=0; i_fmt<res_per_format.size(); ++i_fmt) {
            const int64_t key=res_per_format.keys()[i_fmt];

            if(!res_per_format[key].contains(lst[i_resolution])) {
                lst.removeAt(i_resolution--);
            }
        }
    }

    std::sort(lst.begin(), lst.end(),
              [](const QSize &l, const QSize &r) { return (l.width()==r.width() ? l.height()<r.height() : l.width()<r.width()); }
    );

    return lst;
}

QList <int64_t> FFCam::supportedPixelFormats(QSize size)
{
    if(dev_list.size() - 1<index_device_video)
        return QList<int64_t>();

    QSet <int64_t> set;

    foreach(Cam::Format format, dev_list.at(index_device_video).format) {
        foreach(Cam::Resolution res, format.resolution) {
            if(res.size==size) {
                set << format.pixel_format;
            }
        }
    }

    return set.toList();
}

QList <AVRational> FFCam::supportedFramerates(QSize size, int64_t fmt)
{
    if(dev_list.size() - 1<index_device_video)
        return QList<AVRational>();

    foreach(Cam::Format format, dev_list.at(index_device_video).format) {
        if(format.pixel_format==fmt) {
            foreach(Cam::Resolution res, format.resolution) {
                if(res.size==size) {
                    return res.framerate;
                }
            }
        }
    }

    return QList<AVRational>();
}

void FFCam::setConfig(QSize size, AVRational framerate, int64_t pixel_format)
{
    cfg.size=size;
    cfg.framerate=framerate;
    cfg.pixel_format=pixel_format;
}

void FFCam::startCam()
{
    if(cfg.size.isNull())
        return;

    stop();

    if(index_device_audio>=0) {
        QList <QAudioDeviceInfo> dev_list=QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

        if(dev_list.size()>index_device_audio) {
            QAudioFormat format=default_format;

            if(!dev_list[index_device_audio].supportedSampleSizes().contains(format.sampleSize()))
                format.setSampleSize(dev_list[index_device_audio].supportedSampleSizes().first());

            if(!dev_list[index_device_audio].supportedByteOrders().contains(format.byteOrder()))
                format.setByteOrder(dev_list[index_device_audio].supportedByteOrders().first());

            if(!dev_list[index_device_audio].supportedSampleTypes().contains(format.sampleType()))
                format.setSampleType(dev_list[index_device_audio].supportedSampleTypes().first());

            dev_list[index_device_audio].supportedByteOrders();

            if(!dev_list[index_device_audio].isFormatSupported(format)) {
                qWarning() << "Default format not supported, trying to use the nearest.";
                qWarning().noquote() << formatString(format);
                format=dev_list[index_device_audio].nearestFormat(format);
                qWarning().noquote() << formatString(format);

                d->audio_converter.init(av_get_default_channel_layout(format.channelCount()),
                                        format.sampleRate(), qAudioFormatToAV(format.sampleSize(), format.sampleType()),
                                        av_get_default_channel_layout(default_format.channelCount()),
                                        default_format.sampleRate(), AV_SAMPLE_FMT_S16);
            }

            d->audio_input=new QAudioInput(dev_list[index_device_audio], format, this);
            d->audio_device=d->audio_input->start();
        }
    }

    int ret;

    //

#ifdef __linux__

    if(cfg.pixel_format==1196444237) // mjpeg
        av_dict_set(&d->dictionary, "pixel_format", "mjpeg", 0);

    else if(cfg.pixel_format==1448695129) // yuv
        av_dict_set(&d->dictionary, "pixel_format", "yuyv422", 0);

    else
        av_dict_set(&d->dictionary, "pixel_format", "yuyv422", 0);

#endif

    av_dict_set(&d->dictionary, "video_size", QString("%1x%2").arg(cfg.size.width()).arg(cfg.size.height()).toLatin1().constData(), 0);
    av_dict_set(&d->dictionary, "framerate", QString("%1/%2").arg(cfg.framerate.den).arg(cfg.framerate.num).toLatin1().constData(), 0);
    av_dict_set(&d->dictionary, "rtbufsize", "1000M", 0);

    qDebug() << "video_size:" << QString("%1x%2").arg(cfg.size.width()).arg(cfg.size.height()) << "framerate:" << QString("%1/%2").arg(cfg.framerate.den).arg(cfg.framerate.num);


    if(!d->input_format) {
#ifdef __linux__

        d->input_format=av_find_input_format("video4linux2");

#else

        d->input_format=av_find_input_format("dshow");

#endif
    }

    if(!d->input_format) {
        qCritical() << "av_find_input_format return null";
        goto fail;
    }

    d->format_context=avformat_alloc_context();

    d->format_context->flags|=AVFMT_FLAG_NONBLOCK;

#ifdef __linux__

    if(avformat_open_input(&d->format_context, QString("/dev/video%1").arg(index_device_video).toLatin1().constData(), d->input_format, &d->dictionary)!=0) {
        qCritical() << "Couldn't open input stream";
        goto fail;
    }

#else

    ret=avformat_open_input(&d->format_context, QString("video=%1").arg(dev_list[index_device_video].name).toLatin1().constData(), d->input_format, &d->dictionary);

    if(ret!=0) {
        qCritical() << "Couldn't open input stream" << dev_list[index_device_video].name << ffErrorString(ret);
        goto fail;
    }

#endif

    av_dump_format(d->format_context, 0, "", 0);

    for(unsigned int i=0; i<d->format_context->nb_streams; ++i) {
        if(d->format_context->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            d->stream=d->format_context->streams[i];
            break;
        }
    }

    if(!d->stream) {
        qCritical() << "find video stream err";
        goto fail;
    }


    d->codec=avcodec_find_decoder(d->stream->codecpar->codec_id);

    if(!d->codec) {
        qCritical() << "avcodec_find_decoder err";
        goto fail;
    }

    qDebug() << "avcodec name:" << avcodec_get_name(d->stream->codecpar->codec_id);

    d->codec_context=avcodec_alloc_context3(d->codec);

    if(!d->codec_context) {
        qCritical() << "avcodec_alloc_context3 err";
        goto fail;
    }

    ret=avcodec_open2(d->codec_context, d->codec, nullptr);

    if(ret<0) {
        qCritical() << "Cannot open video decoder for webcam" << ffErrorString(ret);
        goto fail;
    }

    if(!d->frame)
        d->frame=av_frame_alloc();

    return;

fail:
    stop();
}

void FFCam::stop()
{
    if(d->audio_input) {
        d->audio_input->stop();
        d->audio_input->deleteLater();
        d->audio_input=nullptr;
    }

    d->audio_device=nullptr;


    if(d->codec_context) {
        avcodec_free_context(&d->codec_context);
        d->codec_context=nullptr;
    }


    if(d->format_context) {
        avformat_close_input(&d->format_context);
        avformat_free_context(d->format_context);
        d->format_context=nullptr;
    }


    if(d->frame) {
        av_frame_free(&d->frame);
        d->frame=nullptr;
    }

    d->codec=nullptr;
    d->stream=nullptr;
}

void FFCam::subscribe(FrameBuffer::ptr obj)
{
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void FFCam::unsubscribe(FrameBuffer::ptr obj)
{
    subscription_list.removeAll(obj);
}

bool FFCam::isActive()
{
    if(d->format_context)
        return true;

    return false;
}

void FFCam::updateDevList()
{
    dev_list=ToolsCam::devList();

    ToolsCam::testDevList(dev_list);
}

void FFCam::run()
{
    running=true;

    while(running) {
        if(d->format_context && d->codec_context) {
            AVPacket packet;

            if(av_read_frame(d->format_context, &packet)>=0) {
                if(packet.stream_index==d->stream->index) {
                    int frame_finished=0;

                    if(avcodec_send_packet(d->codec_context, &packet)==0) {
                        int ret=avcodec_receive_frame(d->codec_context, d->frame);

                        if(ret<0 && ret!=AVERROR(EAGAIN)) {
                            qCritical() << "avcodec_receive_frame";
                        }

                        if(ret>=0)
                            frame_finished=1;
                    }

                    if(frame_finished) {
                        Frame::ptr frame=Frame::make();
                        int buf_size=av_image_get_buffer_size((AVPixelFormat)d->frame->format, d->frame->width, d->frame->height, alignment);


                        QByteArray ba_frame_yuv;
                        QByteArray ba_frame_rgb;

                        ba_frame_yuv.resize(buf_size);

                        av_image_copy_to_buffer((uint8_t*)ba_frame_yuv.constData(), buf_size, d->frame->data, d->frame->linesize,
                                                (AVPixelFormat)d->frame->format, d->frame->width, d->frame->height, alignment);


                        d->converter.setup((AVPixelFormat)d->frame->format, QSize(d->frame->width, d->frame->height),
                                           AV_PIX_FMT_BGRA, QSize(d->frame->width, d->frame->height));

                        d->converter.convert(&ba_frame_yuv, &ba_frame_rgb);

                        if(d->audio_device) {
                            QByteArray ba_audio=d->audio_device->readAll();

                            if(d->audio_input->format()!=default_format) {
                                QByteArray ba_audio_conv;

                                d->audio_converter.convert(&ba_audio, &ba_audio_conv);
                                ba_audio=ba_audio_conv;
                            }

                            frame->setData(ba_frame_rgb, QSize(d->frame->width, d->frame->height), ba_audio, d->audio_input->format().channelCount(), d->audio_input->format().sampleSize());

                        } else
                            frame->setData(ba_frame_rgb, QSize(d->frame->width, d->frame->height), QByteArray(), 0, 0);

                        frame->video.pts=d->frame->pts - d->stream->start_time;
                        frame->video.time_base=d->stream->time_base;

                        foreach(FrameBuffer::ptr buf, subscription_list)
                            buf->append(frame);
                    }
                }

                av_packet_unref(&packet);
            }
        }


        qApp->processEvents();

        if(d->format_context && d->codec_context)
            usleep(1);

        else
            msleep(100);
    }
}
