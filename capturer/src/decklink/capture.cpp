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
//#include <pthread.h>
//#include <unistd.h>
#include <fcntl.h>
#include <csignal>

#include "DeckLinkAPI.h"
#include "ff_format_converter.h"
#include "convert_thread.h"
#include "audio_tools.h"
#include "decklink_tools.h"

#include "capture.h"

//const bool ext_converter=false;
//const bool ext_converter=true;

class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
    DeckLinkCaptureDelegate(DeckLinkCapture *parent);
    virtual ~DeckLinkCaptureDelegate();

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { Q_UNUSED(iid); Q_UNUSED(ppv); return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

private:
    DeckLinkCapture *d;

#ifdef _MSC_VER
    unsigned long long ref_count;
#else
    int32_t ref_count;
#endif
};

DeckLinkCaptureDelegate::DeckLinkCaptureDelegate(DeckLinkCapture *parent) :
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

DeckLinkCapture::DeckLinkCapture(QObject *parent) :
    QThread(parent)
{
    signal_lost=false;

    decklink_capture_delegate=nullptr;

    decklink=nullptr;
    decklink_display_mode=nullptr;
    decklink_input=nullptr;
    decklink_configuration=nullptr;

    audio_input_packet=nullptr;

    device.index=0;
    format.index=0;
    pixel_format.fmt=bmdFormat10BitRGB;

    running=false;
    running_thread=false;

    rgb_source=true;

    half_fps=false;

    //

    //video_converter=nullptr;

    // if(ext_converter) {
    //     conv_thread=new DlConvertThreadContainer(4, this);

    //     connect(conv_thread, SIGNAL(frameSkipped()), this, SIGNAL(frameSkipped()));

    // } else
    //     video_converter=CreateVideoConversionInstance();

    //

    // start(QThread::NormalPriority);
    start(QThread::TimeCriticalPriority);
}

DeckLinkCapture::~DeckLinkCapture()
{
    subscription_list.clear();

    captureStop();

    quit();

    while(running_thread) {
        msleep(30);
    }


    // if(ext_converter)
    //     conv_thread->stopThreads();
}

void DeckLinkCapture::setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format, int audio_channels, int audio_sample_size, bool rgb_10bit)
{
    this->device=device;
    this->format=format;
    this->pixel_format=pixel_format;
    this->audio_channels=audio_channels;
    this->audio_sample_size=audio_sample_size;
    this->rgb_10bit=rgb_10bit;

    // if(ext_converter) {
    //     conv_thread->setAudioChannels(audio_channels);
    //     conv_thread->setSampleSize(audio_sample_size);
    // }
}

void DeckLinkCapture::subscribe(FrameBuffer::ptr obj)
{
    // if(ext_converter)
    //     conv_thread->subscribe(obj);

    // else
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void DeckLinkCapture::unsubscribe(FrameBuffer::ptr obj)
{
    // if(ext_converter)
    //     conv_thread->unsubscribe(obj);

    // else
        subscription_list.removeAll(obj);
}

bool DeckLinkCapture::isRunning() const
{
    return running;
}

bool DeckLinkCapture::gotSignal() const
{
    if(!running)
        return false;

    return !signal_lost;
}

bool DeckLinkCapture::rgbSource() const
{
    return rgb_source;
}

bool DeckLinkCapture::rgb10Bit() const
{
    return rgb_10bit;
}

void DeckLinkCapture::setHalfFps(bool value)
{
    half_fps=value;
}

void DeckLinkCapture::run()
{
#ifndef __linux__

    if(!comInit())
        return;

#endif

    qInfo() << "DeckLinkCapture::run: priority" << priority();

    // Get the DeckLink device
    IDeckLinkIterator *decklink_iterator=CreateDeckLinkIteratorInstance();

    if(!decklink_iterator) {
        qCritical() << "This application requires the DeckLink drivers installed";
        emit errorString("This application requires the DeckLink drivers installed");
        return;
    }

    decklink_iterator->Release();

    //

    decklink_capture_delegate=new DeckLinkCaptureDelegate(this);

    qInfo() << "DeckLinkCapture thread started";

    running_thread=true;

    exec();

    decklink_capture_delegate->Release();

    running_thread=false;

    qInfo() << "DeckLinkCapture thread stopped";
}

void DeckLinkCapture::captureStart()
{
    if(decklink)
        return;

    init();
}

void DeckLinkCapture::captureStop()
{
    if(decklink_input) {
        decklink_input->StopStreams();
        decklink_input->DisableAudioInput();
        decklink_input->DisableVideoInput();
    }

    release();

    running=false;
}

void DeckLinkCapture::videoInputFormatChanged(uint32_t events, IDeckLinkDisplayMode *mode, uint32_t format_flags)
{
    Q_UNUSED(events)

    HRESULT result;


    BMDPixelFormat pixel_format=bmdFormat10BitYUV;

    pixel_format=bmdFormat8BitYUV;

    rgb_source=false;

    if(format_flags&bmdDetectedVideoInputRGB444) {
        rgb_source=true;

        if(rgb_10bit)
            pixel_format=bmdFormat10BitRGB;

        else
            pixel_format=bmdFormat8BitBGRA;
    }

    if(decklink_input) {
        decklink_input->StopStreams();

        result=decklink_input->EnableVideoInput(mode->GetDisplayMode(), pixel_format, bmdVideoInputEnableFormatDetection);

        if(result!=S_OK) {
            fprintf(stderr, "Failed to switch video mode\n");
            return;
        }

        decklink_input->StartStreams();
    }

    int64_t frame_duration;

    mode->GetFrameRate(&frame_duration, &frame_scale);

    emit formatChanged(mode->GetWidth(), mode->GetHeight(),
                       frame_duration, half_fps ? frame_scale*.5 : frame_scale,
                       (mode->GetFieldDominance()==bmdProgressiveFrame || mode->GetFieldDominance()==bmdProgressiveSegmentedFrame),
                       BMDPixelFormatToString(pixel_format) // rgb_source ? "RGB" : "YUV"
                       );

    qInfo().noquote() << "InputFormatChanged:"
                      << QString("%1x%2@%3%4 %5")
                         .arg(mode->GetWidth())
                         .arg(mode->GetHeight())
                         .arg(frame_scale/frame_duration)
                         .arg(mode->GetFieldDominance()==bmdProgressiveFrame || mode->GetFieldDominance()==bmdProgressiveSegmentedFrame ? "p" : "i")
                         .arg(rgb_source ? "RGB" : "YUV");
}

void DeckLinkCapture::videoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet)
{
    if(!video_frame || !audio_packet)
        return;

    if(video_frame->GetFlags() & bmdFrameHasNoInputSource) {
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

        if(frame_time!=0) {
            if(frame_time - frame_time_prev!=frame_duration) {
                qCritical() << "decklink: frame dropped" << QDateTime::currentDateTime();

                for(int i=0; i<(frame_time - frame_time_prev)/frame_duration - 1; ++i) {
                    emit frameSkipped();
                }
            }

        } else
            frame_counter=0;

        frame_time_prev=frame_time;

        //

        Frame::ptr frame=Frame::make();

        frame->setData(video_frame, audio_packet, audio_channels, audio_sample_size);


        if(audio_channels==8) {
            if(audio_sample_size==16)
                channelsRemap16(frame->audio.ptr_data, frame->audio.data_size);

            else
                channelsRemap32(frame->audio.ptr_data, frame->audio.data_size);
        }

        //

        foreach(FrameBuffer::ptr buf, subscription_list)
            buf->append(frame);
    }
}

void DeckLinkCapture::init()
{
    HRESULT result;

    int	device_index=device.index;
    int format_index=format.index;

    BMDDisplayModeSupport display_mode_supported;

    //

    IDeckLinkIterator *decklink_iterator=CreateDeckLinkIteratorInstance();

    if(!decklink_iterator) {
        qCritical() << "This application requires the DeckLink drivers installed";
        emit errorString("This application requires the DeckLink drivers installed");
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
        qCritical() << "Unable to get DeckLink device" << device.index;
        goto bail;
    }


    result=decklink->QueryInterface(IID_IDeckLinkConfiguration, (void**)&decklink_configuration);

    if(result!=S_OK) {
        qCritical() << "The device is not configurable";
        goto bail;
    }


    result=decklink_configuration->SetInt(bmdDeckLinkConfigVideoInputConnection, bmdVideoConnectionHDMI);

    if(result!=S_OK) {
        qCritical() << "The device does not have an hdmi input";
        emit errorString("The device does not have an hdmi input");
        goto bail;
    }

    // Get the input (capture) interface of the DeckLink device
    result=decklink->QueryInterface(IID_IDeckLinkInput, (void**)&decklink_input);

    if(result!=S_OK) {
        qCritical() << "QueryInterface DeckLinkInput err";
        goto bail;
    }

    //

    {
        IDeckLinkDisplayModeIterator *display_mode_iterator=nullptr;

        result=decklink_input->GetDisplayModeIterator(&display_mode_iterator);

        if(result!=S_OK) {
            qCritical() << "GetDisplayModeIterator err";
            goto bail;
        }

        while((result=display_mode_iterator->Next(&decklink_display_mode))==S_OK) {
            if(format_index==0)
                break;

            --format_index;

            decklink_display_mode->Release();
        }

        display_mode_iterator->Release();

        if(result!=S_OK || decklink_display_mode==nullptr) {
            qCritical() << "Unable to get display mode" << format.index;
            goto bail;
        }


        // Check display mode is supported with given options
        result=decklink_input->DoesSupportVideoMode(decklink_display_mode->GetDisplayMode(), (BMDPixelFormat)pixel_format.fmt, bmdVideoInputFlagDefault, &display_mode_supported, nullptr);

        if(result!=S_OK) {
            qCritical() << "DoesSupportVideoMode err" << result;
            goto bail;
        }

        if(display_mode_supported==bmdDisplayModeNotSupported) {
            qCritical() << "The display mode is not supported with the selected pixel format" << format.display_mode_name;
            goto bail;
        }

        // Configure the capture callback
        decklink_input->SetCallback(decklink_capture_delegate);

        // Start capturing
        result=decklink_input->EnableVideoInput(decklink_display_mode->GetDisplayMode(), (BMDPixelFormat)pixel_format.fmt, bmdVideoInputEnableFormatDetection);

        if(result!=S_OK) {
            qCritical() << "Failed to enable video input. Is another application using the card?" << result;
            emit errorString("Failed to enable video input. Is another application using the card?");
            goto bail;
        }

        switch(audio_channels) {
        case 6:
        case 8:
            qInfo() << "decklink_input->EnableAudioInput 8";
            result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, audio_sample_size==16 ? bmdAudioSampleType16bitInteger : bmdAudioSampleType32bitInteger, 8);
            break;

        case 2:
        default:
            qInfo() << "decklink_input->EnableAudioInput 2";
            result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, audio_sample_size==16 ? bmdAudioSampleType16bitInteger : bmdAudioSampleType32bitInteger, 2);
            break;
        }

        if(result!=S_OK) {
            qCritical() << "EnableAudioInput err";
            goto bail;
        }

        result=decklink_input->StartStreams();

        if(result!=S_OK) {
            qCritical() << "StartStreams err";
            goto bail;
        }
    }

    running=true;

    qInfo() << "DeckLinkCapture: started";

    return;

bail:
    release();
}

void DeckLinkCapture::release()
{
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

    // qInfo() << "DeckLinkCapture: released";
}
