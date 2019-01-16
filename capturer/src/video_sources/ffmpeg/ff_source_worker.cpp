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
#include <QAudioInput>
#include <assert.h>

#include "framerate.h"
#include "ff_format_converter.h"
#include "ff_audio_converter.h"
#include "ff_source.h"

#include "ff_source_worker.h"


struct FFSourceContext
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

FFSourceWorker::FFSourceWorker(FFSource *parent_interface, QObject *parent)
    : QObject(parent)
    , parent_interface(parent_interface)
{
    default_format.setSampleRate(48000);
    default_format.setChannelCount(2);
    default_format.setSampleSize(16);
    default_format.setCodec("audio/pcm");
    default_format.setByteOrder(QAudioFormat::LittleEndian);
    // default_format.setSampleType(QAudioFormat::UnSignedInt);
    default_format.setSampleType(QAudioFormat::SignedInt);

    //

    d=new FFSourceContext();
}

FFSourceWorker::~FFSourceWorker()
{
    deviceStop();

    delete d;
}

QString FFSourceWorker::formatString(const QAudioFormat &format)
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

AVSampleFormat FFSourceWorker::qAudioFormatToAV(const int &depth, const QAudioFormat::SampleType &sample_format)
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
        if(depth==32)
            return AV_SAMPLE_FMT_FLT;

        if(depth==64)
            return AV_SAMPLE_FMT_DBL;
    }

    return AV_SAMPLE_FMT_NONE;
}

void FFSourceWorker::setVideoDevice(FFDevice::Dev video_device)
{
    this->video_device=video_device;
}

void FFSourceWorker::setAudioDevice(int index)
{
    index_device_audio=index;
}

void FFSourceWorker::setConfig(QSize size, AVRational framerate, int64_t pixel_format)
{
    assert(framerate.num>0);

    cfg.size=size;
    cfg.framerate=framerate;
    cfg.pixel_format=pixel_format;

    // qDebug() << size << framerate.den/framerate.num << pixel_format;
}

void FFSourceWorker::deviceStart()
{
    if(cfg.size.isNull())
        return;

    deviceStop();

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

            // qInfo() << "supportedSampleSizes" << dev_list[index_device_audio].supportedSampleSizes();

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

            d->audio_input=new QAudioInput(dev_list[index_device_audio], format);
            d->audio_device=d->audio_input->start();

            parent_interface->type_flags|=SourceInterface::TypeFlag::audio;
        }

    } else {
        parent_interface->type_flags&=~SourceInterface::TypeFlag::audio;
    }

    if((parent_interface->type_flags&SourceInterface::TypeFlag::video)==0)
        return;


    int ret;

    //

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
        qCritical() << "av_find_input_format nullptr";
        goto fail;
    }

    d->format_context=avformat_alloc_context();

    d->format_context->flags|=AVFMT_FLAG_NONBLOCK;


    if(cfg.pixel_format==PixelFormat::mjpeg) {
        d->format_context->video_codec_id=AV_CODEC_ID_MJPEG;

    } else {
        d->format_context->video_codec_id=AV_CODEC_ID_RAWVIDEO;
        av_dict_set(&d->dictionary, "pixel_format", cfg.pixel_format.toString().toLatin1().data(), 0);
    }


    d->format_context->video_codec=avcodec_find_decoder(d->format_context->video_codec_id);


    qDebug() << "dev name:" << video_device.name << video_device.dev;

#ifdef __linux__

    ret=avformat_open_input(&d->format_context, video_device.dev.toLatin1().constData(), d->input_format, &d->dictionary);

#else

    ret=avformat_open_input(&d->format_context, QString("video=%1").arg(video_device.name).toLatin1().constData(), d->input_format, &d->dictionary);

#endif

    if(ret!=0) {
        qCritical() << "avformat_open_input err:" << ffErrorString(ret) << video_device.name;
        goto fail;
    }


    av_dump_format(d->format_context, 0, "", 0);

    for(unsigned int i=0; i<d->format_context->nb_streams; ++i) {
        if(d->format_context->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {

            d->stream=d->format_context->streams[i];
            qDebug() << "avcodec name:" << i << d->format_context->nb_streams << avcodec_get_name(d->format_context->streams[i]->codecpar->codec_id);

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

    qInfo() << "avcodec name:" << avcodec_get_name(d->stream->codecpar->codec_id);

    d->codec_context=avcodec_alloc_context3(d->codec);


    if(!d->codec_context) {
        qCritical() << "avcodec_alloc_context3 nullptr";
        goto fail;
    }

    d->codec_context->pix_fmt=cfg.pixel_format.toAVPixelFormat();
    d->codec_context->width=cfg.size.width();
    d->codec_context->height=cfg.size.height();

    ret=avcodec_open2(d->codec_context, d->codec, nullptr);

    if(ret<0) {
        qCritical() << "avcodec_open2 err:" << ffErrorString(ret);
        goto fail;
    }

    if(!d->frame)
        d->frame=av_frame_alloc();

    {
        PixelFormat pf;

        pf.fromAVPixelFormat(d->codec_context->pix_fmt);

        parent_interface->setPixelFormat(pf);
        parent_interface->setFramerate(cfg.framerate);
        parent_interface->setFramesize(QSize(d->codec_context->width, d->codec_context->height));

        emit signalLost(false);
        emit formatChanged(QString("%1p@%2 %3").arg(d->codec_context->height).arg(Framerate::fromRational(cfg.framerate)).arg(pf.toStringView()));
    }

    return;

fail:
    deviceStop();
}

void FFSourceWorker::deviceStop()
{
    if(d->audio_input) {
        d->audio_input->stop();
        d->audio_input->deleteLater();
        d->audio_input=nullptr;
    }

    d->audio_device=nullptr;


    if(d->frame) {
        av_frame_free(&d->frame);
        d->frame=nullptr;
    }


    if(d->codec_context) {
        avcodec_free_context(&d->codec_context);
        d->codec_context=nullptr;
    }


    if(d->format_context) {
        avformat_close_input(&d->format_context);
        avformat_free_context(d->format_context);
        d->format_context=nullptr;
    }


    d->codec=nullptr;
    d->stream=nullptr;

    emit signalLost(true);
    emit formatChanged("NONE");
}

void FFSourceWorker::subscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void FFSourceWorker::unsubscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    subscription_list.removeAll(obj);
}

bool FFSourceWorker::isActive()
{
    if(parent_interface->type_flags&SourceInterface::TypeFlag::video) {
        if(d->format_context)
            return true;

    } else if(d->audio_device) {
        return true;
    }

    return false;
}

AVRational FFSourceWorker::currentFrameRate() const
{
    return cfg.framerate;
}

PixelFormat FFSourceWorker::pixelFormat() const
{
    if(cfg.pixel_format==PixelFormat::mjpeg)
        return PixelFormat(AV_PIX_FMT_YUV422P);

    return cfg.pixel_format;
}

bool FFSourceWorker::step()
{
    if((parent_interface->type_flags&SourceInterface::TypeFlag::video)==0 && d->audio_device) {
        QByteArray ba_audio=d->audio_device->readAll();

        if(!ba_audio.isEmpty()) {
            if(d->audio_input->format()!=default_format) {
                QByteArray ba_audio_conv;

                d->audio_converter.convert(&ba_audio, &ba_audio_conv);
                ba_audio=ba_audio_conv;
            }

            Frame::ptr frame=Frame::make();

            frame->setDataAudio(ba_audio, d->audio_input->format().channelCount(), d->audio_input->format().sampleSize());

            foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
                buf->append(frame);
        }

        return true;
    }


    if(d->format_context && d->codec_context) {
        AVPacket packet;

        if(av_read_frame(d->format_context, &packet)>=0) {
            if(packet.stream_index==d->stream->index) {
                int frame_finished=0;

                if(avcodec_send_packet(d->codec_context, &packet)==0) {
                    int ret=avcodec_receive_frame(d->codec_context, d->frame);

                    if(ret<0 && ret!=AVERROR(EAGAIN)) {
                        qCritical() << "avcodec_receive_frame err:" << ffErrorString(ret);
                    }

                    if(ret>=0)
                        frame_finished=1;
                }

                if(frame_finished) {
                    Frame::ptr frame=Frame::make();

                    PixelFormat tmp_fmt;
                    tmp_fmt.fromAVPixelFormat((AVPixelFormat)d->frame->format);

                    if(tmp_fmt.isDirect()) {
                        QByteArray ba_frame;

                        const int buf_size=
                                av_image_get_buffer_size((AVPixelFormat)d->frame->format,
                                                         d->frame->width,
                                                         d->frame->height,
                                                         alignment);

                        ba_frame.resize(buf_size);

                        av_image_copy_to_buffer((uint8_t*)ba_frame.constData(), buf_size,
                                                d->frame->data, d->frame->linesize,
                                                (AVPixelFormat)d->frame->format,
                                                d->frame->width, d->frame->height, alignment);


                        if(d->audio_device) {
                            QByteArray ba_audio=d->audio_device->readAll();

                            if(!ba_audio.isEmpty()) {
                                if(d->audio_input->format()!=default_format) {
                                    QByteArray ba_audio_conv;

                                    d->audio_converter.convert(&ba_audio, &ba_audio_conv);
                                    ba_audio=ba_audio_conv;
                                }

                                frame->setData(ba_frame, QSize(d->frame->width, d->frame->height), ba_audio, d->audio_input->format().channelCount(), d->audio_input->format().sampleSize());
                            }

                        } else
                            frame->setData(ba_frame, QSize(d->frame->width, d->frame->height), QByteArray(), 0, 0);


                        frame->video.pts=d->frame->pts - d->stream->start_time;
                        frame->video.time_base=d->stream->time_base;

                        if(!frame->video.pixel_format.fromAVPixelFormat((AVPixelFormat)d->frame->format)) {
                            frame.reset();
                        }

                    } else {
                        d->converter.setup((AVPixelFormat)d->frame->format, QSize(d->frame->width, d->frame->height),
                                           AV_PIX_FMT_BGRA, QSize(d->frame->width, d->frame->height));

                        AVFrameSP::ptr ptr_frame_rgb=
                                d->converter.convert(d->frame);

                        if(ptr_frame_rgb) {
                            QByteArray ba_frame_rgb;

                            const int buf_size=
                                    av_image_get_buffer_size((AVPixelFormat)ptr_frame_rgb->d->format,
                                                             ptr_frame_rgb->d->width,
                                                             ptr_frame_rgb->d->height,
                                                             alignment);

                            ba_frame_rgb.resize(buf_size);

                            av_image_copy_to_buffer((uint8_t*)ba_frame_rgb.constData(), buf_size,
                                                    ptr_frame_rgb->d->data, ptr_frame_rgb->d->linesize,
                                                    (AVPixelFormat)ptr_frame_rgb->d->format,
                                                    ptr_frame_rgb->d->width, ptr_frame_rgb->d->height, alignment);

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
                        }

                        frame->video.pts=d->frame->pts - d->stream->start_time;
                        frame->video.time_base=d->stream->time_base;
                        frame->video.pixel_format.fromAVPixelFormat(AV_PIX_FMT_BGRA);
                    }

                    if(frame) {
                        foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
                            buf->append(frame);
                    }
                }
            }

            av_packet_unref(&packet);
        }

        return true;
    }

    return false;
}
