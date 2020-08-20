/******************************************************************************

Copyright © 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QPixmap>

#ifdef __WIN32__
#include <QtWin>
#endif

#include <ctime>
#include <thread>

#include "framerate.h"
#include "audio_wasapi.h"

#include "screen_capture_worker_bitblt.h"

ScreenCaptureWorkerBitBlt::ScreenCaptureWorkerBitBlt(SourceInterface *si, QObject *parent)
    : QObject(parent)
    , si((SourceInterfacePublic*)si)
{
    audio_wasapi=new AudioWasapi(this);

    si->pixel_format=PixelFormat::bgr0;
    si->type_flags=SourceInterface::TypeFlag::video;
    si->current_dev_name="bit blit";
}

ScreenCaptureWorkerBitBlt::~ScreenCaptureWorkerBitBlt()
{
}

bool ScreenCaptureWorkerBitBlt::isImplemented()
{
#ifdef __WIN32__
    return true;
#else
    return false;
#endif
}

bool ScreenCaptureWorkerBitBlt::step()
{
#ifdef __WIN32__

    if(!dc_desktop || !dc_capture)
        return false;

    BitBlt(dc_capture, 0, 0, screen_width, screen_height, dc_desktop, 0, 0, SRCCOPY);

    QImage img=QtWin::imageFromHBITMAP(compatible_bitmap);

    Frame::ptr frame=Frame::make();

    frame->device_index=si->device_index;
    frame->video.time_base={ 1, 1000000000 };
    frame->video.pts=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if(!img.isNull()) {
        frame->video.size=img.size();
        frame->video.data_size=img.bytesPerLine()*img.height();
        frame->video.dummy.resize(frame->video.data_size);
        frame->video.data_ptr=(uint8_t*)frame->video.dummy.data();

        if(!frame->video.pixel_format.fromQImageFormat(img.format())) {
            QString err=QString("unknown image format: %1").arg(img.format());
            qCritical().noquote() << err;
            deviceStop();
            emit errorString(err);
            return false;
        }

        if(si->pixel_format!=frame->video.pixel_format) {
            QString err=QString("wrong pixel format: %1").arg(frame->video.pixel_format.toString());
            qCritical().noquote() << err;
            deviceStop();
            emit errorString(err);
            return false;
        }

        memcpy(frame->video.data_ptr, img.bits(), frame->video.data_size);

        if(si->framesize!=frame->video.size) {
            si->framesize=frame->video.size;
            si->current_format=QString("%1/%2").arg(si->framesize.load().height()).arg(frame->video.pixel_format.toString());
            emit formatChanged(si->current_format);
        }
    }

    frame->setDataAudio(audio_wasapi->getData(), audio_wasapi->channels(), audio_wasapi->sampleSize());
    frame->audio.time_base={ 1, 48000 };
    frame->audio.pts=av_rescale_q(frame->video.pts, frame->video.time_base, frame->audio.time_base);
    frame->audio.loopback=true;

    if(frame->video.data_ptr || frame->audio.data_ptr) {
        foreach(FrameBuffer<Frame::ptr>::ptr buf, si->subscription_list)
            buf->append(frame);

    } else {
        frame.reset();
    }

    //

    const uint64_t elapsed=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - frame_time_point).count();

    if(elapsed<frame_duration)
        std::this_thread::sleep_for(std::chrono::nanoseconds(int64_t((frame_duration - elapsed)*.98)));

    frame_time_point=std::chrono::high_resolution_clock::now();

    return true;

#endif

    return false;
}

QStringList ScreenCaptureWorkerBitBlt::availableAudioInput()
{
    return audio_wasapi->availableAudioInputStr();
}

void ScreenCaptureWorkerBitBlt::setAudioDevice(QString device_name)
{
    audio_device_name=device_name;
}

void ScreenCaptureWorkerBitBlt::deviceStart()
{
#ifdef __WIN32__

    screen_width=GetSystemMetrics(SM_CXSCREEN);
    screen_height=GetSystemMetrics(SM_CYSCREEN);

    if(screen_width<1 || screen_height<1) {
        qCritical() << "GetSystemMetrics err";
        goto init_fail;
    }

    dc_desktop=GetDC(0);

    if(!dc_desktop) {
        qCritical() << "GetDC err";
        goto init_fail;
    }

    dc_capture=CreateCompatibleDC(dc_desktop);

    if(!dc_capture) {
        qCritical() << "CreateCompatibleDC err";
        goto init_fail;
    }

    compatible_bitmap=CreateCompatibleBitmap(dc_desktop, screen_width, screen_height);

    if(!compatible_bitmap) {
        qCritical() << "CreateCompatibleBitmap err";
        goto init_fail;
    }

    SelectObject(dc_capture, compatible_bitmap);


    si->type_flags=SourceInterface::TypeFlag::video;

    //

    foreach(AudioWasapi::Device dev, audio_wasapi->availableAudioInput()) {
        if(audio_device_name==dev.name) {
            if(audio_wasapi->deviceStart(dev)) {
                si->type_flags|=SourceInterface::TypeFlag::audio;
                si->audio_sample_size=audio_wasapi->sampleSize()==16 ? SourceInterface::AudioSampleSize::bitdepth_16 : SourceInterface::AudioSampleSize::bitdepth_32;

                switch(audio_wasapi->channels()) {
                case 2:
                    si->audio_channels=SourceInterface::AudioChannels::ch_2;
                    break;

                case 6:
                    si->audio_channels=SourceInterface::AudioChannels::ch_6;
                    break;

                case 8:
                    si->audio_channels=SourceInterface::AudioChannels::ch_8;
                    break;

                default:
                    si->audio_sample_size=SourceInterface::AudioSampleSize::bitdepth_null;
                    si->type_flags=SourceInterface::TypeFlag::video;
                    break;
                }
            }

            break;
        }
    }

    //

    emit signalLost(false);

    si->signal_lost=false;

    frame_duration=(1./Framerate::fromRational(si->framerate))*1000*1000*1000;
    frame_time_point=std::chrono::high_resolution_clock::now();

    qInfo() << "frame_duration" << frame_duration << Framerate::fromRational(si->framerate);

    return;

init_fail:
    deviceStop();

#endif
}

void ScreenCaptureWorkerBitBlt::deviceStop()
{
#ifdef __WIN32__

    if(dc_desktop) {
        ReleaseDC(0, dc_desktop);
        dc_desktop=0;
    }

    if(dc_capture) {
        DeleteDC(dc_capture);
        dc_capture=0;
    }

    if(compatible_bitmap) {
        DeleteObject(compatible_bitmap);
        compatible_bitmap=0;
    }

    audio_wasapi->deviceStop();

    si->framesize=QSize();
    si->current_format=QString();
    si->signal_lost=true;

    emit signalLost(true);

#endif
}
