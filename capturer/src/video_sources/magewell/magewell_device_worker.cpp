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
#include <QElapsedTimer>
#include <qcoreapplication.h>

#include "MWCapture.h"
#include "MWFOURCC.h"

#include "framerate.h"

#include "magewell_audio_thread.h"

#include "magewell_device_worker.h"

const int mw_timeout=300;

PixelFormat fromMagewellPixelFormat(uint32_t fmt)
{
    switch(fmt) {
    case MWFOURCC_RGB24:
        return PixelFormat::rgb24;

    case MWFOURCC_BGR24:
        return PixelFormat::bgr24;

    case MWFOURCC_RGBA:
        return PixelFormat::rgb0;

    case MWFOURCC_BGRA:
        return PixelFormat::bgra;

    case MWFOURCC_BGR10:
        return PixelFormat::gbrp10le;

    case MWFOURCC_I420:
        return PixelFormat::yuv420p;

    case MWFOURCC_YUYV:
        return PixelFormat::yuyv422;

    case MWFOURCC_UYVY:
        return PixelFormat::uyvy422;

    case MWFOURCC_P210:
        return PixelFormat::yuv422p10le;

    case MWFOURCC_P010:
        return PixelFormat::p010le;

    case MWFOURCC_NV12:
        return PixelFormat::nv12;

    default:
        break;
    }

    return PixelFormat::undefined;
}

uint32_t toMagewellPixelFormat(PixelFormat fmt)
{
    switch((int)fmt) {
    case PixelFormat::rgb24:
        return MWFOURCC_RGB24;

    case PixelFormat::bgr24:
        return MWFOURCC_BGR24;

    case PixelFormat::rgb0:
        return MWFOURCC_RGBA;

    case PixelFormat::bgr0:
    case PixelFormat::bgra:
        return MWFOURCC_BGRA;

    case PixelFormat::gbrp10le:
        return MWFOURCC_BGR10;

    case PixelFormat::yuv420p:
        return MWFOURCC_I420;

    case PixelFormat::yuyv422:
        return MWFOURCC_YUYV;

    case PixelFormat::uyvy422:
        return MWFOURCC_UYVY;

    case PixelFormat::yuv422p10le:
        return MWFOURCC_P210;

    case PixelFormat::p010le:
        return MWFOURCC_P010;

    case PixelFormat::nv12:
        return MWFOURCC_NV12;

    default:
        break;
    }

    return MWFOURCC_UNK;
}

struct MagewellDeviceWorkerContext {
#ifdef __linux__
    MWCAP_PTR event_capture=0;
    MWCAP_PTR event_buf=0;
#else
    HANDLE event_capture=0;
    HANDLE event_buf=0;
#endif

    HNOTIFY notify_event_buf=0;

    ULONGLONG status_bits=0;

    MWCAP_VIDEO_BUFFER_INFO video_buffer_info;

    MWCAP_VIDEO_SIGNAL_STATUS signal_status;
    MWCAP_INPUT_SPECIFIC_STATUS specific_status;

    DWORD fourcc=MWFOURCC_NV12;


    PixelFormat pixel_format=PixelFormat::nv12;

    size_t frame_buffer_size;
    DWORD min_stride;

    QSize framesize;
    AVRational framerate;

    int color_format=0;
    int quantization_range=0;

    //

    long long audio_timestamp;
    QByteArray ba_audio;

    //

    LONGLONG device_time;

    struct Fps {
        int frame_counter=0;
        double value=0.;
        LONGLONG device_time_last=0;

    } fps;
};


MagewellDeviceWorker::MagewellDeviceWorker(QObject *parent)
    : QObject(parent)
    , d(new MagewellDeviceWorkerContext())
    , a(new MagewellAudioThread())
{
    connect(a, SIGNAL(audioSampleSizeChanged(SourceInterface::AudioSampleSize::T)), SIGNAL(audioSampleSizeChanged(SourceInterface::AudioSampleSize::T)), Qt::QueuedConnection);
    connect(a, SIGNAL(audioChannelsChanged(SourceInterface::AudioChannels::T)), SIGNAL(audioChannelsChanged(SourceInterface::AudioChannels::T)), Qt::QueuedConnection);

    connect(this, SIGNAL(channelChanged(MGHCHANNEL)), a, SLOT(setChannel(MGHCHANNEL)), Qt::QueuedConnection);
}

MagewellDeviceWorker::~MagewellDeviceWorker()
{
    deviceStop();

    delete d;
    delete a;
}

void MagewellDeviceWorker::subscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void MagewellDeviceWorker::unsubscribe(FrameBuffer<Frame::ptr>::ptr obj)
{
    subscription_list.removeAll(obj);
}

bool MagewellDeviceWorker::isActive()
{
    return d->event_capture!=0;
}

bool MagewellDeviceWorker::gotSignal()
{
    return !signal_lost;
}

AVRational MagewellDeviceWorker::currentFramerate()
{
    return d->framerate;
}

PixelFormat MagewellDeviceWorker::currentPixelFormat()
{
    return d->pixel_format;
}

QSize MagewellDeviceWorker::currentFramesize()
{
    return d->framesize;
}

SourceInterface::AudioSampleSize::T MagewellDeviceWorker::currentAudioSampleSize()
{
    return (a->sampleSize()==SourceInterface::AudioSampleSize::bitdepth_16 ? SourceInterface::AudioSampleSize::bitdepth_16 : SourceInterface::AudioSampleSize::bitdepth_32);
}

SourceInterface::AudioChannels::T MagewellDeviceWorker::currentAudioChannels()
{
    return (SourceInterface::AudioChannels::T)a->channels();
}

bool MagewellDeviceWorker::step()
{
    if(d->event_capture==0) {
        // qDebug() << "d->hEventCapture nullptr";
        return false;
    }

    if(d->framesize.width()<=0) {
        updateVideoSignalInfo();
        // qDebug() << "wrong d->frame_size.width";
        return false;
    }

    int ret;


    MWWaitEvent(d->event_buf, mw_timeout);


    ret=MWGetNotifyStatus((HCHANNEL)current_channel, d->notify_event_buf, &d->status_bits);

    if(ret!=MW_SUCCEEDED) {
        qCritical() << "MWGetNotifyStatus err";
        return true;
    }


    // if(d->status_bits&MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE) {
    //     updateVideoSignalInfo();
    // }


    if(d->status_bits&MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING) {
        MWGetDeviceTime((HCHANNEL)current_channel, &d->device_time);


        d->fps.frame_counter++;

        if(d->fps.frame_counter%10==0) {
            d->fps.value=(double)d->fps.frame_counter*10000000LL/(d->device_time - d->fps.device_time_last);

            if(d->device_time - d->fps.device_time_last>30000000LL) {
                d->fps.device_time_last=d->device_time;
                d->fps.frame_counter=0;
                qInfo() << "fps:" << d->fps.value << QString::number(d->fps.value, 'f', 1).toLatin1().data();
            }
        }


        //

        updateVideoSignalInfo();

        //

        ret=MWGetVideoBufferInfo((HCHANNEL)current_channel, &d->video_buffer_info);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetVideoBufferInfo err";
            return true;
        }


        HDMI_INFOFRAME_PACKET hdmi_infoframe_packet;

        int ret=MWGetHDMIInfoFramePacket((HCHANNEL)current_channel, MWCAP_HDMI_INFOFRAME_ID_HDR, &hdmi_infoframe_packet);

        // qDebug() << (ret==MW_SUCCEEDED) << (hdmi_infoframe_packet.hdrInfoFramePayload.maximum_content_light_level_lsb | (hdmi_infoframe_packet.hdrInfoFramePayload.maximum_content_light_level_lsb << 8));


        a->getData(&d->ba_audio, &d->audio_timestamp);

        if(d->ba_audio.isEmpty()) {
            qDebug() << "no audio";
            return true;
        }


        Frame::ptr frame=Frame::make();

        frame->video.dummy.resize(d->frame_buffer_size);
        frame->video.data_ptr=(uint8_t*)frame->video.dummy.constData();
        frame->video.data_size=d->frame_buffer_size;
        frame->video.size=d->framesize;


        ret=MWCaptureVideoFrameToVirtualAddressEx(
                    (HCHANNEL)current_channel,
                    d->video_buffer_info.iNewestBufferedFullFrame,
                    (LPBYTE)frame->video.data_ptr,
                    frame->video.data_size,
                    d->min_stride,
                    0,
                    0,
                    d->fourcc,
                    d->framesize.width(),
                    d->framesize.height(),
                    0,
                    0,
                    0,
                    NULL,
                    0,
                    100,
                    0,
                    100,
                    0,
                    MWCAP_VIDEO_DEINTERLACE_WEAVE,
                    MWCAP_VIDEO_ASPECT_RATIO_IGNORE,
                    NULL,
                    NULL,
                    0,
                    0,
                    (MWCAP_VIDEO_COLOR_FORMAT)d->color_format,
                    (MWCAP_VIDEO_QUANTIZATION_RANGE)d->quantization_range,
                    MWCAP_VIDEO_SATURATION_UNKNOWN
                    );

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWCaptureVideoFrameToVirtualAddressEx err";
            return true;
        }

        MWWaitEvent(d->event_capture, mw_timeout);

        //

        MWCAP_VIDEO_CAPTURE_STATUS videoCaptureStatus;

        do {
            ret=MWGetVideoCaptureStatus((HCHANNEL)current_channel, &videoCaptureStatus);

        } while(ret==MW_SUCCEEDED && videoCaptureStatus.bFrameCompleted==false);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetVideoCaptureStatus";
            return true;
        }

        //

        frame->video.pixel_format=d->pixel_format;

        //

        if(!d->ba_audio.isEmpty()) {
            // int sample_size_bytes=a->sampleSize()==16 ? 2 : 4;
            // int expected_audio_size=(1000.*av_q2d(d->framerate))/(1000./48000)*sample_size_bytes*a->channels();
            // qInfo() << expected_audio_size << d->ba_audio.size();

            frame->setDataAudio(d->ba_audio, a->channels(), a->sampleSize());
        }


        if(pts_enabled) {
            frame->video.pts=d->device_time;
            frame->video.time_base={ 1, 10000000 };

            frame->audio.pts=d->audio_timestamp;
            frame->audio.time_base={ 1, 10000000 };
        }

        //

        if(signal_lost)
            emit signalLost(signal_lost=false);

        foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
            buf->append(frame);

    } else {
        updateVideoSignalInfo();
        qDebug() << "!MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED";
    }

    return true;
}

void MagewellDeviceWorker::setDevice(QSize board_channel)
{
    if(current_channel)
        MWCloseChannel((HCHANNEL)current_channel);

    current_channel=MWOpenChannel(board_channel.width(), board_channel.height());

    if(current_channel==0) {
        qCritical() << "MWOpenChannelByPath err";
        return;
    }

    emit channelChanged(current_channel);
}

void MagewellDeviceWorker::deviceStart()
{
    deviceStop();

    qDebug() << "current_channel" << current_channel;

    d->event_capture=MWCreateEvent();

    if(d->event_capture==0) {
        qCritical() << "MWCreateEvent err";
        return;
    }

    d->event_buf=MWCreateEvent();

    if(d->event_buf==0) {
        qCritical() << "MWCreateEvent err";
        return;
    }

    // qInfo() << current_channel << d->event_notify_buffering;

    d->notify_event_buf=MWRegisterNotify((HCHANNEL)current_channel, d->event_buf,
                                         MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING | MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE);

    if(d->notify_event_buf==0) {
        qCritical() << "MWRegisterNotify err";
        return;
    }

    //

    MWSetDeviceTime((HCHANNEL)current_channel, 0ll);

    //

    int ret=MWStartVideoCapture((HCHANNEL)current_channel, d->event_capture);

    if(ret!=MW_SUCCEEDED) {
        qCritical() << "MWStartVideoCapture err";
        return;
    }

    emit signalLost(signal_lost=false);


    updateVideoSignalInfo();

    //

    QMetaObject::invokeMethod(a, "deviceStart", Qt::QueuedConnection);

    //

    d->fps.device_time_last=0;
    d->fps.frame_counter=0;

    //

    qDebug() << "ok";
}

void MagewellDeviceWorker::deviceStop()
{
    if(current_channel) {
        MWStopVideoCapture((HCHANNEL)current_channel);
    }

    if(d->event_buf) {
        MWUnregisterNotify((HCHANNEL)current_channel, d->notify_event_buf);
        d->notify_event_buf=0;
    }

    if(d->event_capture) {
        MWCloseEvent(d->event_capture);
        d->event_capture=0;
    }

    emit signalLost(signal_lost=true);

    QMetaObject::invokeMethod(a, "deviceStop", Qt::QueuedConnection);
}

void MagewellDeviceWorker::setPixelFormat(PixelFormat fmt)
{
    d->pixel_format=fmt;
    d->fourcc=toMagewellPixelFormat(fmt);

    updateVideoSignalInfo();
}

void MagewellDeviceWorker::setColorFormat(int value)
{
    d->color_format=value;
}

void MagewellDeviceWorker::setQuantizationRange(int value)
{
    d->quantization_range=value;
}

void MagewellDeviceWorker::setPtsEnabled(bool value)
{
    pts_enabled=value;
}

void MagewellDeviceWorker::updateVideoSignalInfo()
{
    MWCAP_VIDEO_SIGNAL_STATUS signal_status;

    if(MWGetVideoSignalStatus((HCHANNEL)current_channel, &signal_status)!=MW_SUCCEEDED) {
        qWarning() << "MWGetVideoSignalStatus err";
        signal_status.state=MWCAP_VIDEO_SIGNAL_UNSUPPORTED;
    }


    switch(signal_status.state) {
    case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
        if(!signal_lost || d->signal_status.state!=signal_status.state) {
            qWarning() << "MWCAP_VIDEO_SIGNAL_UNSUPPORTED";
            // emit errorString(QStringLiteral("video signal status not valid"));
            emit formatChanged("UNSUPPORTED");
            emit signalLost(signal_lost=true);
            d->signal_status=signal_status;
            d->framesize=QSize();
        }
        return;

    case MWCAP_VIDEO_SIGNAL_NONE:
        if(!signal_lost || d->signal_status.state!=signal_status.state) {
            emit formatChanged("NONE");
            emit signalLost(signal_lost=true);
            d->signal_status=signal_status;
            d->framesize=QSize();
        }
        return;

    case MWCAP_VIDEO_SIGNAL_LOCKING:
    case MWCAP_VIDEO_SIGNAL_LOCKED:
        break;
    }

    d->signal_status=signal_status;

    MWCAP_INPUT_SPECIFIC_STATUS specific_status;

    MWGetInputSpecificStatus((HCHANNEL)current_channel, &specific_status);


    QSize framesize=QSize(signal_status.cx, signal_status.cy);

    d->min_stride=FOURCC_CalcMinStride(d->fourcc, d->framesize.width(), 4);
    d->frame_buffer_size=FOURCC_CalcImageSize(d->fourcc, d->framesize.width(), d->framesize.height(), d->min_stride);

    AVRational framerate=Framerate::toRational(10000000./signal_status.dwFrameDuration);


    if(av_cmp_q(d->framerate, framerate)!=0 || d->framesize!=framesize || d->specific_status.hdmiStatus.pixelEncoding!=specific_status.hdmiStatus.pixelEncoding) {
        d->framerate=framerate;
        d->framesize=framesize;
        d->specific_status.hdmiStatus.pixelEncoding=specific_status.hdmiStatus.pixelEncoding;

        //

        QString str_pixel_format;

        switch(specific_status.hdmiStatus.pixelEncoding) {
        case HDMI_ENCODING_RGB_444:
            str_pixel_format=QStringLiteral("RGB");
            break;

        case HDMI_ENCODING_YUV_422:
            str_pixel_format=QStringLiteral("YUV422");
            break;

        case HDMI_ENCODING_YUV_444:
            str_pixel_format=QStringLiteral("YUV444");
            break;

        case HDMI_ENCODING_YUV_420:
            str_pixel_format=QStringLiteral("YUV420");
            break;

        default:
            break;
        }

        str_pixel_format.insert(0, QString("%1Bit").arg(specific_status.hdmiStatus.byBitDepth));

        //

        emit framerateChanged(d->framerate);
        emit framesizeChanged(d->framesize);

        emit formatChanged(QString("%1%2@%3 %4").arg(d->framesize.height()).arg(signal_status.bInterlaced ? 'i' : 'p')
                           .arg(Framerate::rnd2(Framerate::fromRational(d->framerate))).arg(str_pixel_format));

        qInfo() << d->framesize << Framerate::fromRational(d->framerate) << str_pixel_format;
    }
}
