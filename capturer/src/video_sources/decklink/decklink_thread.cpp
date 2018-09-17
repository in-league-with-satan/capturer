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
#include <QDateTime>
#include <qcoreapplication.h>

#ifdef _MSC_VER
#include <windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <csignal>


#include "decklink_global.h"

#include "framerate.h"
#include "ff_format_converter.h"
#include "decklink_convert_thread.h"
#include "audio_tools.h"
#include "decklink_tools.h"
#include "frame_decklink.h"

#include "decklink_thread.h"

#ifdef LIB_DECKLINK
class DeckLinkCaptureDelegate : public IDeckLinkInputCallback {
#else
class DeckLinkCaptureDelegate {
#endif
public:
    DeckLinkCaptureDelegate(DeckLinkThread *parent);
    virtual ~DeckLinkCaptureDelegate();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { Q_UNUSED(iid); Q_UNUSED(ppv); return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

private:
    DeckLinkThread *d;

#ifdef _MSC_VER
    unsigned long long ref_count;
#else
    int32_t ref_count;
#endif
};

DeckLinkCaptureDelegate::DeckLinkCaptureDelegate(DeckLinkThread *parent) :
    d(parent)
  , ref_count(1)
{
}

DeckLinkCaptureDelegate::~DeckLinkCaptureDelegate()
{
}

ULONG DeckLinkCaptureDelegate::AddRef(void)
{
#ifdef _MSC_VER
    return InterlockedExchangeAdd(&ref_count, 1);
#else
    return __sync_add_and_fetch(&ref_count, 1);
#endif
}

ULONG DeckLinkCaptureDelegate::Release(void)
{
#ifdef _MSC_VER
    int32_t new_ref_value=InterlockedExchangeSubtract(&ref_count, 1);
#else
    int32_t new_ref_value=__sync_sub_and_fetch(&ref_count, 1);
#endif

    if(new_ref_value==0) {
        delete this;

        return 0;
    }

    return new_ref_value;
}

HRESULT DeckLinkCaptureDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags format_flags)
{
    d->videoInputFormatChanged(events, mode, format_flags);

    return S_OK;
}

HRESULT DeckLinkCaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_frame)
{
    d->videoInputFrameArrived(video_frame, audio_frame);

    return S_OK;
}

DeckLinkThread::DeckLinkThread(QObject *parent)
    : QThread(parent)
{
    signal_lost=false;

    decklink_capture_delegate=nullptr;

    decklink=nullptr;
    decklink_display_mode=nullptr;
    decklink_input=nullptr;
    decklink_configuration=nullptr;

    audio_input_packet=nullptr;

    device.index=0;


    running=false;
    running_thread=false;

    source_rgb=true;

    half_fps=false;


    audio_channels=AudioChannels::ch_8;
    audio_sample_size=AudioSampleSize::bitdepth_16;

    //

    // start(QThread::NormalPriority);
    start(QThread::TimeCriticalPriority);
}

DeckLinkThread::~DeckLinkThread()
{
    subscription_list.clear();

    deviceStop();

    quit();

    while(running_thread) {
        msleep(30);
    }
}

SourceInterface::Type::T DeckLinkThread::type() const
{
    return Type::decklink;
}

void DeckLinkThread::setDevice(void *ptr)
{
    DeckLinkThread::Device *dev=reinterpret_cast<DeckLinkThread::Device*>(ptr);

    device=dev->device;

    source_10bit=dev->source_10bit;

    audio_sample_size=dev->audio_sample_size;

    delete dev;

    bm_pixel_format=bmdFormat8BitYUV;
}

bool DeckLinkThread::isActive()
{
    return running;
}

bool DeckLinkThread::gotSignal()
{
    if(!running)
        return false;

    return !signal_lost;
}

void DeckLinkThread::run()
{
#ifndef __linux__

    if(!comInit())
        return;

#endif

#ifdef LIB_DECKLINK

    qDebug() << "priority" << priority();

    IDeckLinkIterator *decklink_iterator=CreateDeckLinkIteratorInstance();

    if(!decklink_iterator) {
        qCritical() << "this application requires the DeckLink drivers installed";
        emit errorString("this application requires the DeckLink drivers installed");
        return;
    }

    decklink_iterator->Release();

    //

    decklink_capture_delegate=new DeckLinkCaptureDelegate(this);

    running_thread=true;

    exec();

    decklink_capture_delegate->Release();

    running_thread=false;

#endif
}

void DeckLinkThread::deviceStart()
{
    if(decklink)
        return;

    init();
}

void DeckLinkThread::deviceStop()
{
#ifdef LIB_DECKLINK

    if(decklink_input) {
        decklink_input->StopStreams();
        decklink_input->DisableAudioInput();
        decklink_input->DisableVideoInput();
    }

    release();

    running=false;

    emit signalLost(signal_lost=true);

#endif
}

void DeckLinkThread::videoInputFormatChanged(uint32_t events, IDeckLinkDisplayMode *mode, uint32_t format_flags)
{
    Q_UNUSED(events)

#ifdef LIB_DECKLINK

    HRESULT result;


    bm_pixel_format=bmdFormat8BitYUV;

    source_rgb=false;

    if(format_flags&bmdDetectedVideoInputRGB444) {
        source_rgb=true;

        if(source_10bit)
            bm_pixel_format=bmdFormat10BitRGB;

        else
            bm_pixel_format=bmdFormat8BitBGRA;

    } else if(source_10bit)
        bm_pixel_format=bmdFormat10BitYUV;

    if(decklink_input) {
        decklink_input->StopStreams();

        result=decklink_input->EnableVideoInput(mode->GetDisplayMode(), bm_pixel_format, bmdVideoInputEnableFormatDetection);

        if(result!=S_OK) {
            qWarning() << "failed to switch video mode" << (result==E_INVALIDARG ? "- invalid mode or video flags" : "");
            return;
        }

        decklink_input->StartStreams();
    }


    int64_t frame_duration;

    mode->GetFrameRate(&frame_duration, &frame_scale);


    PixelFormat pix_fmt_tmp;
    pix_fmt_tmp.fromBMDPixelFormat(bm_pixel_format);

    pixel_format=pix_fmt_tmp;

    framerate=Framerate::toRational((double)frame_scale/(double)frame_duration);

    framesize=QSize(mode->GetWidth(), mode->GetHeight());

    QString format=QString("%1%2@%3 %4")
            .arg(mode->GetHeight())
            .arg(mode->GetFieldDominance()==bmdProgressiveFrame || mode->GetFieldDominance()==bmdProgressiveSegmentedFrame ? "p" : "i")
            .arg(frame_scale/frame_duration)
            .arg(Decklink::BMDPixelFormatToString(bm_pixel_format));

    emit formatChanged(format);

    qDebug().noquote() << format;

#endif
}

void DeckLinkThread::videoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet)
{
#ifdef LIB_DECKLINK

    if(!video_frame || !audio_packet)
        return;

    const BMDFrameFlags frame_flags=video_frame->GetFlags();

    if(frame_flags&bmdFrameHasNoInputSource) {
        if(!signal_lost) {
            qWarning() << "no input signal";
            emit signalLost(signal_lost=true);
        }

    } else {
        if(signal_lost)
            emit signalLost(signal_lost=false);

        BMDTimeValue frame_time;
        BMDTimeValue frame_duration;

        video_frame->GetStreamTime(&frame_time, &frame_duration, frame_scale);

        //

        Frame::ptr frame=FrameDecklink::make(video_frame, audio_packet, audio_channels, audio_sample_size);

        frame->video.pts=frame->audio.pts=frame_time/frame_duration;
        frame->video.time_base=frame->audio.time_base={ (int)frame_duration, (int)frame_scale };

        //

        if(audio_channels==8) {
            if(audio_sample_size==16)
                channelsRemap<int16_t>(frame->audio.data_ptr, frame->audio.data_size);

            else
                channelsRemap<int32_t>(frame->audio.data_ptr, frame->audio.data_size);
        }

        //

        foreach(FrameBuffer<Frame::ptr>::ptr buf, subscription_list)
            buf->append(frame);
    }

#endif
}

void DeckLinkThread::init()
{
#ifdef LIB_DECKLINK

    HRESULT result;

    int	device_index=device.index;
    int format_index=0;

    BMDDisplayModeSupport display_mode_supported;

    //

    IDeckLinkIterator *decklink_iterator=CreateDeckLinkIteratorInstance();

    if(!decklink_iterator) {
        qCritical() << "this application requires the DeckLink drivers installed";
        emit errorString("this application requires the DeckLink drivers installed");
        return;
    }

    while((result=decklink_iterator->Next(&decklink))==S_OK) {
        if(device_index==0)
            break;

        --device_index;

        decklink->Release();
    }

    decklink_iterator->Release();

    if(result!=S_OK || decklink==nullptr) {
        qCritical() << "unable to get DeckLink device" << device.index;
        goto exit;
    }


    result=decklink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&decklink_configuration);

    if(result!=S_OK) {
        qCritical() << "the device is not configurable";
        goto exit;
    }


    result=decklink_configuration->SetInt(bmdDeckLinkConfigVideoInputConnection, bmdVideoConnectionHDMI);

    if(result!=S_OK) {
        qCritical() << "the device does not have an hdmi input";
        emit errorString("the device does not have an hdmi input");
        goto exit;
    }

    result=decklink->QueryInterface(IID_IDeckLinkInput, (void**)&decklink_input);

    if(result!=S_OK) {
        qCritical() << "IDeckLink::QueryInterface err";
        goto exit;
    }

    //

    {
        IDeckLinkDisplayModeIterator *display_mode_iterator=nullptr;

        result=decklink_input->GetDisplayModeIterator(&display_mode_iterator);

        if(result!=S_OK) {
            qCritical() << "IDeckLinkInput::GetDisplayModeIterator err";
            goto exit;
        }

        while((result=display_mode_iterator->Next(&decklink_display_mode))==S_OK) {
            if(format_index==0)
                break;

            --format_index;

            decklink_display_mode->Release();
        }

        display_mode_iterator->Release();

        if(result!=S_OK || decklink_display_mode==nullptr) {
            qCritical() << "unable to get display mode";
            goto exit;
        }


        result=decklink_input->DoesSupportVideoMode(decklink_display_mode->GetDisplayMode(), bm_pixel_format, bmdVideoInputFlagDefault, &display_mode_supported, nullptr);

        if(result!=S_OK) {
            qCritical() << "IDeckLinkInput::DoesSupportVideoMode err" << result;
            goto exit;
        }

        if(display_mode_supported==bmdDisplayModeNotSupported) {
            qCritical() << "the display mode is not supported with the selected pixel format";
            goto exit;
        }

        decklink_input->SetCallback(decklink_capture_delegate);

        result=decklink_input->EnableVideoInput(decklink_display_mode->GetDisplayMode(), bm_pixel_format, bmdVideoInputEnableFormatDetection);

        if(result!=S_OK) {
            qCritical() << "failed to enable video input. is another application using the card?" << result;
            emit errorString("failed to enable video input. is another application using the card?");
            goto exit;
        }

        switch(audio_channels) {
        case 6:
        case 8:
            result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, audio_sample_size==16 ? bmdAudioSampleType16bitInteger : bmdAudioSampleType32bitInteger, 8);
            break;

        case 2:
        default:
            result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, audio_sample_size==16 ? bmdAudioSampleType16bitInteger : bmdAudioSampleType32bitInteger, 2);
            break;
        }

        if(result!=S_OK) {
            qCritical() << "IDeckLinkInput::EnableAudioInput err";
            goto exit;
        }

        result=decklink_input->StartStreams();

        if(result!=S_OK) {
            qCritical() << "IDeckLinkInput::StartStreams err";
            goto exit;
        }
    }

    running=true;

    qDebug() << "started";

    return;

exit:
    release();

#endif
}

void DeckLinkThread::release()
{
#ifdef LIB_DECKLINK

    if(decklink_input) {
        decklink_input->Release();
        decklink_input=nullptr;
    }

    if(decklink_display_mode) {
        decklink_display_mode->Release();
        decklink_display_mode=nullptr;
    }

    if(decklink_configuration) {
        decklink_configuration->Release();
        decklink_configuration=nullptr;
    }

    if(decklink) {
        decklink->Release();
        decklink=nullptr;
    }

#endif
}
