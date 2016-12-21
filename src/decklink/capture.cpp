#include <QDebug>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <csignal>

#include "DeckLinkAPI.h"

#include "capture.h"


class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
    DeckLinkCaptureDelegate(DeckLinkCapture *parent);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv) { return E_NOINTERFACE; }
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFormatChanged(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags);
    virtual HRESULT STDMETHODCALLTYPE VideoInputFrameArrived(IDeckLinkVideoInputFrame*, IDeckLinkAudioInputPacket*);

private:
    DeckLinkCapture *d;

    int32_t ref_count;
};

DeckLinkCaptureDelegate::DeckLinkCaptureDelegate(DeckLinkCapture *parent) :
    d(parent)
  ,ref_count(1)
{
}

ULONG DeckLinkCaptureDelegate::AddRef(void)
{
    return __sync_add_and_fetch(&ref_count, 1);
}

ULONG DeckLinkCaptureDelegate::Release(void)
{
    int32_t new_ref_value=__sync_sub_and_fetch(&ref_count, 1);

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
    deck_link_capture_delegate=nullptr;

    decklink=nullptr;
    display_mode=nullptr;
    decklink_input=nullptr;

    video_frame_converted_720p=nullptr;
    video_frame_converted_1080p=nullptr;
    video_frame_converted_2160p=nullptr;

    device.index=0;
    format.index=0;
    pixel_format.fmt=bmdFormat10BitRGB;

    setTerminationEnabled(true);

    start(QThread::NormalPriority);
}

DeckLinkCapture::~DeckLinkCapture()
{
}

void DeckLinkCapture::setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format)
{
    this->device=device;
    this->format=format;
    this->pixel_format=pixel_format;
}

void DeckLinkCapture::run()
{
    // Get the DeckLink device
    IDeckLinkIterator *decklink_iterator=CreateDeckLinkIteratorInstance();

    if(!decklink_iterator) {
        qCritical() << "This application requires the DeckLink drivers installed";
        return;
    }

    decklink_iterator->Release();

    //

    video_converter=CreateVideoConversionInstance();

    deck_link_capture_delegate=new DeckLinkCaptureDelegate(this);

    qInfo() << "DeckLinkCapture thread started";

    exec();

    deck_link_capture_delegate->Release();

    // delete deck_link_capture_delegate;

    if(decklink_iterator)
        decklink_iterator->Release();

    deleteLater();
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
}

void DeckLinkCapture::videoInputFormatChanged(uint32_t events, IDeckLinkDisplayMode *mode, uint32_t format_flags)
{
    HRESULT	result;

    char *display_mode_name=nullptr;

    BMDPixelFormat pixel_format=bmdFormat10BitYUV;

    if(format_flags & bmdDetectedVideoInputRGB444)
        pixel_format=bmdFormat10BitRGB;

    mode->GetName((const char**)&display_mode_name);

    qInfo() << "Video format changed to" << display_mode_name << (format_flags & bmdDetectedVideoInputRGB444 ? "RGB" : "YUV");

    if(display_mode_name)
        free(display_mode_name);

    if(decklink_input) {
        decklink_input->StopStreams();

        result=decklink_input->EnableVideoInput(mode->GetDisplayMode(), pixel_format, bmdVideoInputEnableFormatDetection);

        if(result!=S_OK) {
            fprintf(stderr, "Failed to switch video mode\n");
            return;
        }

        decklink_input->StartStreams();
    }
}

void DeckLinkCapture::videoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet)
{
    if(!video_frame || !audio_packet)
        return;

    void *video_frame_bytes;
    void *audio_packet_bytes;

    QByteArray ba_video;
    QByteArray ba_audio;


    if(video_frame->GetFlags() & bmdFrameHasNoInputSource) {
        qCritical() << "No input signal detected";

        emit noInputSignalDetected();

        return;

    } else {
        if(video_frame->GetWidth()==1280) {
            video_converter->ConvertFrame(video_frame, video_frame_converted_720p);

            video_frame=(IDeckLinkVideoInputFrame*)video_frame_converted_720p;

        } else if(video_frame->GetWidth()==1920) {
            video_converter->ConvertFrame(video_frame, video_frame_converted_1080p);

            video_frame=(IDeckLinkVideoInputFrame*)video_frame_converted_1080p;

        } else {
            video_converter->ConvertFrame(video_frame, video_frame_converted_2160p);

            video_frame=(IDeckLinkVideoInputFrame*)video_frame_converted_2160p;
        }


        video_frame->GetBytes(&video_frame_bytes);

        ba_video.resize(video_frame->GetRowBytes() * video_frame->GetHeight());

        memcpy(ba_video.data(), video_frame_bytes, ba_video.size());
    }


    // Handle Audio Frame

    audio_packet->GetBytes(&audio_packet_bytes);

    ba_audio.resize(audio_packet->GetSampleFrameCount()*2*(16/8));

    memcpy(ba_audio.data(), audio_packet_bytes, ba_audio.size());


    // video_frame_converted->Release();

    emit frameVideo(ba_video, QSize(video_frame->GetWidth(), video_frame->GetHeight()));

    emit frameAudio(ba_audio);
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

    // Get the input (capture) interface of the DeckLink device
    result=decklink->QueryInterface(IID_IDeckLinkInput, (void**)&decklink_input);

    if(result!=S_OK)
        goto bail;

    result=decklink->QueryInterface(IID_IDeckLinkOutput, (void**)&decklink_output);

    if(result!=S_OK)
        goto bail;


    decklink_output->CreateVideoFrame(1280, 720, 720*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted_720p);
    decklink_output->CreateVideoFrame(1920, 1080, 1920*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted_1080p);
    decklink_output->CreateVideoFrame(3840, 2160, 3840*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted_2160p);

    //

    {
        IDeckLinkDisplayModeIterator *display_mode_iterator=nullptr;

        result=decklink_input->GetDisplayModeIterator(&display_mode_iterator);

        if(result!=S_OK) {
            qCritical() << "GetDisplayModeIterator err";
            goto bail;
        }

        while((result=display_mode_iterator->Next(&display_mode))==S_OK) {
            if(format_index==0)
                break;

            --format_index;

            display_mode->Release();
        }

        display_mode_iterator->Release();

        if(result!=S_OK || display_mode==nullptr) {
            qCritical() << "Unable to get display mode" << format.index;
            goto bail;
        }


        // Check display mode is supported with given options
        result=decklink_input->DoesSupportVideoMode(display_mode->GetDisplayMode(), (BMDPixelFormat)pixel_format.fmt, bmdVideoInputFlagDefault, &display_mode_supported, nullptr);

        if(result!=S_OK) {
            qCritical() << "DoesSupportVideoMode err" << result;
            goto bail;
        }

        if(display_mode_supported==bmdDisplayModeNotSupported) {
            qCritical() << "The display mode is not supported with the selected pixel format" << format.display_mode_name;
            goto bail;
        }

        // Configure the capture callback
        decklink_input->SetCallback(deck_link_capture_delegate);


        // Start capturing
        result=decklink_input->EnableVideoInput(display_mode->GetDisplayMode(), (BMDPixelFormat)pixel_format.fmt, bmdVideoInputEnableFormatDetection);

        if(result!=S_OK) {
            qCritical() << "Failed to enable video input. Is another application using the card?" << result;
            goto bail;
        }

        // result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, 16, 8); //!!!!!!!!
        result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, 16, 2);

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

    if(decklink_output) {
        decklink_output->Release();
        decklink_output=nullptr;
    }

    if(display_mode) {
        display_mode->Release();
        display_mode=nullptr;
    }

    if(video_frame_converted_720p) {
        video_frame_converted_720p->Release();
        video_frame_converted_720p=nullptr;
    }

    if(video_frame_converted_1080p) {
        video_frame_converted_1080p->Release();
        video_frame_converted_1080p=nullptr;
    }

    if(video_frame_converted_2160p) {
        video_frame_converted_2160p->Release();
        video_frame_converted_2160p=nullptr;
    }

    if(decklink) {
        decklink->Release();
        decklink=nullptr;
    }
}
