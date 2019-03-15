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
#include <QElapsedTimer>
#include <qcoreapplication.h>

#ifdef LIB_MWCAPTURE

#include "MWCapture.h"
#include "MWFOURCC.h"

#endif

#include "framerate.h"

#include "magewell_audio_thread.h"

#include "magewell_device_worker.h"

const int mw_timeout=300;

PixelFormat fromMagewellPixelFormat(uint32_t fmt)
{
#ifdef LIB_MWCAPTURE

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
        return PixelFormat::gbrp10;

    case MWFOURCC_I420:
        return PixelFormat::yuv420p;

    case MWFOURCC_YUYV:
        return PixelFormat::yuyv422;

    case MWFOURCC_UYVY:
        return PixelFormat::uyvy422;

    case MWFOURCC_P210:
        return PixelFormat::yuv422p10;

    case MWFOURCC_Y410:
        return PixelFormat::yuv444p10;

    case MWFOURCC_NV12:
        return PixelFormat::nv12;

    case MWFOURCC_P010:
        return PixelFormat::p010;

    default:
        break;
    }

#endif

    return PixelFormat::undefined;
}

uint32_t toMagewellPixelFormat(PixelFormat fmt)
{
#ifdef LIB_MWCAPTURE

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

    case PixelFormat::gbrp10:
        return MWFOURCC_BGR10;

    case PixelFormat::yuv420p:
        return MWFOURCC_I420;

    case PixelFormat::yuyv422:
        return MWFOURCC_YUYV;

    case PixelFormat::uyvy422:
        return MWFOURCC_UYVY;

    case PixelFormat::yuv422p10:
        return MWFOURCC_P210;

    case PixelFormat::yuv444p10:
        return MWFOURCC_V410;

    case PixelFormat::nv12:
        return MWFOURCC_NV12;

    case PixelFormat::p010:
        return MWFOURCC_P010;

    default:
        break;
    }

    return MWFOURCC_UNK;

#endif

    return 0;
}

struct MagewellDeviceWorkerContext
{
#ifdef LIB_MWCAPTURE

#ifdef __linux__
    MWCAP_PTR event_capture=0;
    MWCAP_PTR event_buf=0;
#else
    HANDLE event_capture=0;
    HANDLE event_buf=0;
#endif

    HNOTIFY notify_event_buf=0;

    ULONGLONG status_bits=0;

    MWCAP_VIDEO_FRAME_INFO video_frame_info;
    MWCAP_VIDEO_BUFFER_INFO video_buffer_info;

    MWCAP_INPUT_SPECIFIC_STATUS specific_status;

    uint64_t notify_flag=0;

    DWORD fourcc=MWFOURCC_NV12;


    PixelFormat pixel_format=PixelFormat::nv12;

    bool half_fps=false;
    bool skip_frame=false;

    bool low_latency=false;

    size_t frame_buffer_size;
    DWORD min_stride;


    QSize framesize;
    QSize frameresize;
    AVRational framerate;

    int color_format_in=0;
    int color_format_out=0;
    int quantization_range_in=0;
    int quantization_range_out=0;

    int pts_mode=0;

    //

    int64_t audio_timestamp;
    QByteArray ba_audio;

    //

    LONGLONG device_time;

    struct Fps {
        int frame_counter=0;
        double value=0.;
        LONGLONG device_time_last=0;

    } fps;

#endif // LIB_MWCAPTURE
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

#ifdef LIB_MWCAPTURE

    if(current_channel)
        MWCloseChannel((HCHANNEL)current_channel);

#endif

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
#ifdef LIB_MWCAPTURE

    return d->event_capture!=0;

#endif

    return false;
}

bool MagewellDeviceWorker::gotSignal()
{
    return state==State::running;
}

AVRational MagewellDeviceWorker::currentFramerate()
{
#ifdef LIB_MWCAPTURE

    return d->framerate;

#endif

    return { 1, 1 };
}

PixelFormat MagewellDeviceWorker::currentPixelFormat()
{
#ifdef LIB_MWCAPTURE

    return d->pixel_format;

#endif

    return PixelFormat::undefined;
}

QSize MagewellDeviceWorker::currentFramesize()
{
#ifdef LIB_MWCAPTURE

    return d->framesize;

#endif

    return QSize();
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
#ifdef LIB_MWCAPTURE

    if(d->event_capture==0) {
        if(state==State::running)
            setState(State::no_signal);

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


    // if(d->status_bits&MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE || d->status_bits&MWCAP_NOTIFY_INPUT_SPECIFIC_CHANGE) {
    //     updateVideoSignalInfo();
    // }


    if(d->status_bits&d->notify_flag) {
        if(d->half_fps) {
            if((d->skip_frame=!d->skip_frame))
                return true;
        }

        /*
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
        */

        //

        if(!updateVideoSignalInfo())
            return false;

        //

        ret=MWGetVideoBufferInfo((HCHANNEL)current_channel, &d->video_buffer_info);

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWGetVideoBufferInfo err";
            return true;
        }

        //

        MWGetVideoFrameInfo((HCHANNEL)current_channel, d->low_latency ? d->video_buffer_info.iNewestBuffering : d->video_buffer_info.iNewestBufferedFullFrame, &d->video_frame_info);

        //

        // HDMI_INFOFRAME_PACKET hdmi_infoframe_packet;

        // int ret=MWGetHDMIInfoFramePacket((HCHANNEL)current_channel, MWCAP_HDMI_INFOFRAME_ID_HDR, &hdmi_infoframe_packet);

        // qDebug() << (ret==MW_SUCCEEDED) << (hdmi_infoframe_packet.hdrInfoFramePayload.maximum_content_light_level_lsb | (hdmi_infoframe_packet.hdrInfoFramePayload.maximum_content_light_level_lsb << 8));

        //


        Frame::ptr frame=Frame::make();

        frame->video.dummy.resize(d->frame_buffer_size);
        frame->video.data_ptr=(uint8_t*)frame->video.dummy.constData();
        frame->video.data_size=d->frame_buffer_size;
        frame->video.size=d->framesize;


        ret=MWCaptureVideoFrameToVirtualAddressEx(
                    (HCHANNEL)current_channel,
                    d->low_latency ? d->video_buffer_info.iNewestBuffering : d->video_buffer_info.iNewestBufferedFullFrame,
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
                    (MWCAP_VIDEO_COLOR_FORMAT)d->color_format_out,
                    (MWCAP_VIDEO_QUANTIZATION_RANGE)d->quantization_range_out,
                    MWCAP_VIDEO_SATURATION_UNKNOWN
                    );

        if(ret!=MW_SUCCEEDED) {
            qCritical() << "MWCaptureVideoFrameToVirtualAddressEx err";
            return true;
        }


        //

        MWWaitEvent(d->event_capture, mw_timeout);

        //

        MWCAP_VIDEO_CAPTURE_STATUS video_capture_status;

        do {
            ret=MWGetVideoCaptureStatus((HCHANNEL)current_channel, &video_capture_status);
            qApp->processEvents();

        } while(ret==MW_SUCCEEDED && video_capture_status.bFrameCompleted==false && d->event_capture);

        if(ret!=MW_SUCCEEDED || video_capture_status.bFrameCompleted!=true) {
            d->framesize=QSize();
            return false;
        }

        //

        frame->video.pixel_format=d->pixel_format;

        //

        frame->video.time_base=d->framerate;

        if(d->video_frame_info.allFieldStartTimes[0]<1) {
            qWarning() << "wrong all field times";
            // return true;
        }


        switch(d->pts_mode) {
        case MagewellDevice::Device::PtsMode::device:
            a->getData(&d->ba_audio, &d->audio_timestamp);

            frame->video.pts=av_rescale_q(d->video_frame_info.allFieldStartTimes[0], { 1, 10000000 }, d->framerate);

            if(!d->ba_audio.isEmpty()) {
                frame->setDataAudio(d->ba_audio, a->channels(), a->sampleSize());

                frame->audio.time_base={ 1, 10000000 };

                frame->audio.pts=d->audio_timestamp;

            } else {
                // qDebug() << "no audio";
                // qWarning() << "no audio";
            }

            break;

        case MagewellDevice::Device::PtsMode::audio:
            a->getData2(&d->ba_audio, &frame->video.pts);

            if(!d->ba_audio.isEmpty()) {
                frame->setDataAudio(d->ba_audio, a->channels(), a->sampleSize());
                frame->video.time_base=d->framerate;
            }

            break;

        case MagewellDevice::Device::PtsMode::disabled:
        default:
            d->ba_audio=a->getData();
            frame->setDataAudio(d->ba_audio, a->channels(), a->sampleSize());
            break;
        }

        //

        foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
            buf->append(frame);


        setState(State::running);

    } else {
        updateVideoSignalInfo();
        qDebug() << "!MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED(ING)";
    }

    return true;

#endif

    return false;
}

void MagewellDeviceWorker::setDevice(MagewellDevice::Device dev)
{
#ifdef LIB_MWCAPTURE

    if(current_channel)
        MWCloseChannel((HCHANNEL)current_channel);

    current_channel=MWOpenChannel(dev.index_board, dev.index_channel);

    if(current_channel==0) {
        qCritical() << "MWOpenChannelByPath err";
        return;
    }

    MWSetDeviceTime((HCHANNEL)current_channel, 0ll);

    emit channelChanged(current_channel);

    //

    d->pixel_format=dev.pixel_format;
    d->fourcc=toMagewellPixelFormat(dev.pixel_format);

    updateVideoSignalInfo();

    //

    d->half_fps=dev.half_fps;

    d->low_latency=dev.low_latency;

    d->color_format_in=dev.color_format_in;
    d->color_format_out=dev.color_format_out;

    d->quantization_range_in=dev.quantization_range_in;
    d->quantization_range_out=dev.quantization_range_out;

    d->pts_mode=dev.pts_mode;

    a->setAudioRemapMode(dev.audio_remap_mode);

    d->frameresize=dev.framesize;

#endif
}

void MagewellDeviceWorker::deviceStart()
{
#ifdef LIB_MWCAPTURE

    deviceStop();

    qDebug() << "current_channel" << current_channel;

    //

    if(MWSetVideoInputQuantizationRange((HCHANNEL)current_channel, (MWCAP_VIDEO_QUANTIZATION_RANGE)d->quantization_range_in)!=MW_SUCCEEDED) {
        qCritical() << "MWSetVideoInputQuantizationRange err";
    }

    if(MWSetVideoInputColorFormat((HCHANNEL)current_channel, (MWCAP_VIDEO_COLOR_FORMAT)d->color_format_in)!=MW_SUCCEEDED) {
        qCritical() << "MWSetVideoInputColorFormat err";
    }

    //

    d->event_capture=MWCreateEvent();

    if(d->event_capture==0) {
        qCritical() << "MWCreateEvent err";
        goto stop;
    }

    //

    d->event_buf=MWCreateEvent();

    if(d->event_buf==0) {
        qCritical() << "MWCreateEvent err";
        goto stop;
    }

    //

    if(d->low_latency)
        d->notify_flag=MWCAP_NOTIFY_VIDEO_FRAME_BUFFERING;

    else
        d->notify_flag=MWCAP_NOTIFY_VIDEO_FRAME_BUFFERED;

    d->notify_event_buf=MWRegisterNotify((HCHANNEL)current_channel, d->event_buf,
                                         d->notify_flag | MWCAP_NOTIFY_VIDEO_SIGNAL_CHANGE | MWCAP_NOTIFY_INPUT_SPECIFIC_CHANGE);

    if(d->notify_event_buf==0) {
        qCritical() << "MWRegisterNotify err";
        goto stop;
    }

    //

    if(MWStartVideoCapture((HCHANNEL)current_channel, d->event_capture)!=MW_SUCCEEDED) {
        qCritical() << "MWStartVideoCapture err";
        goto stop;
    }

    //

    QMetaObject::invokeMethod(a, "deviceStart", Qt::QueuedConnection);

    //

    d->fps.device_time_last=0;
    d->fps.frame_counter=0;
    d->framesize=QSize();

    //

    qInfo() << "ok";

    return;

stop:
    qCritical() << "start err";

    deviceStop();

#endif
}

void MagewellDeviceWorker::deviceStop()
{
#ifdef LIB_MWCAPTURE

    if(current_channel) {
        MWStopVideoCapture((HCHANNEL)current_channel);
    }

    if(d->notify_event_buf) {
        MWUnregisterNotify((HCHANNEL)current_channel, d->notify_event_buf);
        d->notify_event_buf=0;
    }

    if(d->event_buf) {
        MWCloseEvent(d->event_buf);
        d->event_buf=0;
    }

    if(d->event_capture) {
        MWCloseEvent(d->event_capture);
        d->event_capture=0;
    }

    setState(State::no_signal);

    QMetaObject::invokeMethod(a, "deviceStop", Qt::QueuedConnection);

#endif
}

bool MagewellDeviceWorker::updateVideoSignalInfo()
{
#ifdef LIB_MWCAPTURE

    MWCAP_VIDEO_SIGNAL_STATUS signal_status;

    if(MWGetVideoSignalStatus((HCHANNEL)current_channel, &signal_status)!=MW_SUCCEEDED) {
        qWarning() << "MWGetVideoSignalStatus err";
        setState(State::no_signal);
        d->framesize=QSize();
        return false;
    }

    if(signal_status.state==MWCAP_VIDEO_SIGNAL_UNSUPPORTED) {
        qWarning() << "MWCAP_VIDEO_SIGNAL_UNSUPPORTED";
        setState(State::unsupported);
        d->framesize=QSize();
        return false;
    }

    if(signal_status.state==MWCAP_VIDEO_SIGNAL_NONE) {
        setState(State::no_signal);
        d->framesize=QSize();
        return false;
    }

    MWCAP_INPUT_SPECIFIC_STATUS specific_status;

    MWGetInputSpecificStatus((HCHANNEL)current_channel, &specific_status);


    QSize framesize=QSize(signal_status.cx, signal_status.cy);

    if(d->frameresize.width()>0)
        framesize=d->frameresize;

    d->min_stride=FOURCC_CalcMinStride(d->fourcc, framesize.width(), 4);
    d->frame_buffer_size=FOURCC_CalcImageSize(d->fourcc, framesize.width(), framesize.height(), d->min_stride);

    AVRational framerate=Framerate::toRational(10000000./signal_status.dwFrameDuration);


    if(d->half_fps)
        framerate={ framerate.num, int(framerate.den*.5) };


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

        QMetaObject::invokeMethod(a, "setVideoFramerate", Qt::QueuedConnection, Q_ARG(AVRational, d->framerate));

        emit framerateChanged(d->framerate);
        emit framesizeChanged(d->framesize);


        if(d->frameresize.width()>0 && d->framesize.height()!=signal_status.cy)
            emit formatChanged(QString("%1(%2)%3@%4 %5").arg(d->framesize.height()).arg(signal_status.cy).arg(signal_status.bInterlaced ? 'i' : 'p')
                               .arg(Framerate::rnd2(Framerate::fromRational(d->framerate))).arg(str_pixel_format));

        else
            emit formatChanged(QString("%1%2@%3 %4").arg(d->framesize.height()).arg(signal_status.bInterlaced ? 'i' : 'p')
                               .arg(Framerate::rnd2(Framerate::fromRational(d->framerate))).arg(str_pixel_format));

        qInfo() << d->framesize << Framerate::fromRational(d->framerate) << str_pixel_format;
    }

    return signal_status.state==MWCAP_VIDEO_SIGNAL_LOCKED;

#endif

    return false;
}

void MagewellDeviceWorker::setState(int value)
{
    if(state==value)
        return;

    state=value;

    if(state==State::running) {
        emit signalLost(false);

    } else if(state==State::no_signal) {
        emit signalLost(true);
        emit formatChanged("no signal");

    } else {
        emit signalLost(true);
        emit formatChanged("UNSUPPORTED");
    }
}
