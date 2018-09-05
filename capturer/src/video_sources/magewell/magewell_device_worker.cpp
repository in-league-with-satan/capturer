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
    MWCAP_PTR event_notify_buffering=0;
#else
    HANDLE event_capture=0;
    HANDLE event_notify_buffering=0;
#endif

    HNOTIFY notify_buffering=0;

    ULONGLONG status_bits=0;

    MWCAP_VIDEO_BUFFER_INFO video_buffer_info;


    DWORD fourcc=MWFOURCC_NV12;


    PixelFormat pixel_format=PixelFormat::nv12;

    QByteArray ba_buffer;
    DWORD min_stride;

    QSize framesize;
    AVRational framerate;

    //

    QByteArray ba_audio;
};


MagewellDeviceWorker::MagewellDeviceWorker(QObject *parent)
    : QObject(parent)
    , d(new MagewellDeviceWorkerContext())
    , a(new MagewellAudioThread())
{
    connect(a, SIGNAL(audioSampleSizeChnanged(SourceInterface::AudioSampleSize::T)), SIGNAL(audioSampleSizeChanged(SourceInterface::AudioSampleSize::T)), Qt::QueuedConnection);
    connect(a, SIGNAL(audioChannelsChnanged(SourceInterface::AudioChannels::T)), SIGNAL(audioChannelsChanged(SourceInterface::AudioChannels::T)), Qt::QueuedConnection);

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
        qDebug() << "d->hEventCapture nullptr";
        return false;
    }

    if(d->framesize.width()<=0) {
        updateVideoSignalInfo();
        qDebug() << "wrong d->frame_size.width";
        return false;
    }

    int ret;


    MWWaitEvent(d->event_notify_buffering, mw_timeout);
    // MWWaitEvent(d->event_notify_buffering, INFINITE);


    ret=MWGetNotifyStatus((HCHANNEL)current_channel, d->notify_buffering, &d->status_bits);

    if(ret!=MW_SUCCEEDED) {
        qCritical() << "MWGetNotifyStatus err";
        return true;
    }

    ret=MWGetVideoBufferInfo((HCHANNEL)current_channel, &d->video_buffer_info);

    if(ret!=MW_SUCCEEDED) {
        qCritical() << "MWGetVideoBufferInfo err";
        return true;
    }


    if(d->status_bits==MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE) {
        updateVideoSignalInfo();
    }


    if(d->status_bits==MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED) {
        HDMI_INFOFRAME_PACKET hdmi_infoframe_packet;

        int ret=MWGetHDMIInfoFramePacket((HCHANNEL)current_channel, MWCAP_HDMI_INFOFRAME_ID_HDR, &hdmi_infoframe_packet);

        // qDebug() << (ret==MW_SUCCEEDED) << (hdmi_infoframe_packet.hdrInfoFramePayload.maximum_content_light_level_lsb | (hdmi_infoframe_packet.hdrInfoFramePayload.maximum_content_light_level_lsb << 8));

        d->ba_audio=a->getData();

        if(d->ba_audio.isEmpty()) {
            qDebug() << "no audio";
            return true;
        }

#ifdef __linux__

        ret=MWCaptureVideoFrameToVirtualAddress((HCHANNEL)current_channel, d->video_buffer_info.iNewestBuffering,
                                                (MWCAP_PTR)(long)d->ba_buffer.constData(), d->ba_buffer.size(), d->min_stride,
                                                0, 0, d->fourcc, d->framesize.width(), d->framesize.height());

#else

        ret=MWCaptureVideoFrameToVirtualAddressEx(
                    (HCHANNEL)current_channel,
                    d->video_buffer_info.iNewestBuffering,
                    (LPBYTE)d->ba_buffer.constData(),
                    d->ba_buffer.size(),
                    d->min_stride,
                    0,
                    0, // ?
                    d->fourcc,
                    d->framesize.width(),
                    d->framesize.height(),
                    0,
                    64,
                    NULL,
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
                    (MWCAP_VIDEO_COLOR_FORMAT)color_format,
                    MWCAP_VIDEO_QUANTIZATION_UNKNOWN,
                    MWCAP_VIDEO_SATURATION_UNKNOWN
                    );

#endif

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWCaptureVideoFrameToVirtualAddressEx err";
            return true;
        }

        MWWaitEvent(d->event_capture, mw_timeout);

        MWCAP_VIDEO_CAPTURE_STATUS videoCaptureStatus;

        do {
            ret=MWGetVideoCaptureStatus((HCHANNEL)current_channel, &videoCaptureStatus);

        } while(ret==MW_SUCCEEDED && videoCaptureStatus.bFrameCompleted==false);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetVideoCaptureStatus";
            return true;
        }

        //

        LONGLONG device_time;

        ret=MWGetDeviceTime((HCHANNEL)current_channel, &device_time);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetDeviceTime err";
            return true;
        }

        Frame::ptr frame=Frame::make();

        frame->setDataVideo(d->ba_buffer, d->framesize);

        frame->video.pixel_format=d->pixel_format;

        //

        if(!d->ba_audio.isEmpty()) {
            frame->setDataAudio(d->ba_audio, a->channels(), a->sampleSize());
        }

        //

        if(signal_lost)
            emit signalLost(signal_lost=false);

        foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
            buf->append(frame);

    } else {
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

    d->event_notify_buffering=MWCreateEvent();

    if(d->event_notify_buffering==0) {
        qCritical() << "MWCreateEvent err";
        return;
    }

    // qInfo() << current_channel << d->event_notify_buffering;

    d->notify_buffering=MWRegisterNotify((HCHANNEL)current_channel, d->event_notify_buffering,
                                         MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED | MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE);

    if(d->notify_buffering==0) {
        qCritical() << "MWRegisterNotify err";
        return;
    }

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

    qDebug() << "ok";
}

void MagewellDeviceWorker::deviceStop()
{
    if(current_channel) {
        MWStopVideoCapture((HCHANNEL)current_channel);
    }

    if(d->event_notify_buffering) {
        MWUnregisterNotify((HCHANNEL)current_channel, d->notify_buffering);
        d->notify_buffering=0;
    }

    if(d->event_capture) {
        MWCloseEvent(d->event_capture);
        d->event_capture=0;
    }

    emit signalLost(signal_lost=true);
    // QMetaObject::invokeMethod(a, "deviceStop", Qt::QueuedConnection);
}

void MagewellDeviceWorker::setPixelFormat(PixelFormat fmt)
{
    d->pixel_format=fmt;
    d->fourcc=toMagewellPixelFormat(fmt);

    updateVideoSignalInfo();
}

void MagewellDeviceWorker::updateVideoSignalInfo()
{
    color_format=MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;

    MWCAP_VIDEO_SIGNAL_STATUS signal_status;

    if(MWGetVideoSignalStatus((HCHANNEL)current_channel, &signal_status)!=MW_SUCCEEDED)
        return;

    switch(signal_status.state) {
    case MWCAP_VIDEO_SIGNAL_UNSUPPORTED:
        if(!signal_lost) {
            qWarning() << "MWCAP_VIDEO_SIGNAL_UNSUPPORTED";
            emit errorString(QStringLiteral("video signal status not valid"));
            emit formatChanged("UNSUPPORTED");
            emit signalLost(signal_lost=true);
            d->framesize=QSize();
        }
        return;

    case MWCAP_VIDEO_SIGNAL_NONE:
        if(!signal_lost) {
            emit formatChanged("NONE");
            emit signalLost(signal_lost=true);
            d->framesize=QSize();
        }
        return;

    case MWCAP_VIDEO_SIGNAL_LOCKING:
    case MWCAP_VIDEO_SIGNAL_LOCKED:
        break;
    }


    MWCAP_INPUT_SPECIFIC_STATUS specific_status;

    QString str_pixel_format;

    if(MWGetInputSpecificStatus((HCHANNEL)current_channel, &specific_status)==MW_SUCCEEDED) {
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

//        MWCAP_VIDEO_COLOR_FORMAT tmp_color_format;

//        if(MWGetVideoInputColorFormat((HCHANNEL)current_channel, &tmp_color_format)==MW_SUCCEEDED) {
//            if(tmp_color_format==MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN)
//                color_format=(specific_status.hdmiStatus.pixelEncoding==HDMI_ENCODING_RGB_444 ? MWCAP_VIDEO_COLOR_FORMAT_RGB :  MWCAP_VIDEO_COLOR_FORMAT_YUV709);

//            else
//                color_format=tmp_color_format;

//        } else {
            color_format=(specific_status.hdmiStatus.pixelEncoding==HDMI_ENCODING_RGB_444 ? MWCAP_VIDEO_COLOR_FORMAT_YUV709 :  MWCAP_VIDEO_COLOR_FORMAT_RGB); // o_O wtf?
//        }

        str_pixel_format.insert(0, QString("%1Bit").arg(specific_status.hdmiStatus.byBitDepth));
    }


    d->framesize=QSize(signal_status.cx, signal_status.cy);

    d->min_stride=FOURCC_CalcMinStride(d->fourcc, d->framesize.width(), 4);
    int buf_size=FOURCC_CalcImageSize(d->fourcc, d->framesize.width(), d->framesize.height(), d->min_stride);

    d->ba_buffer.resize(buf_size);

    d->framerate=Framerate::toRational(10000000./signal_status.dwFrameDuration);

    //

    emit framerateChanged(d->framerate);
    emit framesizeChanged(d->framesize);

    emit formatChanged(QString("%1%2@%3 %4").arg(d->framesize.height()).arg(signal_status.bInterlaced ? 'i' : 'p')
                       .arg(Framerate::rnd2(Framerate::fromRational(d->framerate))).arg(str_pixel_format));

    qDebug() << d->framesize << buf_size << Framerate::fromRational(d->framerate);
}
