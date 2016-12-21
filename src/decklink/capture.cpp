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

HRESULT DeckLinkCaptureDelegate::VideoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_frame)
{
    d->videoInputFrameArrived(video_frame, audio_frame);

    return S_OK;
}

HRESULT DeckLinkCaptureDelegate::VideoInputFormatChanged(BMDVideoInputFormatChangedEvents events, IDeckLinkDisplayMode *mode, BMDDetectedVideoInputFormatFlags format_flags)
{
/*
    // This only gets called if bmdVideoInputEnableFormatDetection was set
    // when enabling video input
    HRESULT	result;
    char *display_mode_name=nullptr;
    BMDPixelFormat pixel_format=bmdFormat10BitYUV;

    if(format_flags & bmdDetectedVideoInputRGB444)
        pixel_format=bmdFormat10BitRGB;

    mode->GetName((const char**)&display_mode_name);
    //    printf("Video format changed to %s %s\n", displayModeName, formatFlags & bmdDetectedVideoInputRGB444 ? "RGB" : "YUV");

    if(display_mode_name)
        free(display_mode_name);

    if(decklink_input) {
        decklink_input->StopStreams();

        //!!!!!!!!!!!!!!!!!!!!!
        // result=decklink_input->EnableVideoInput(mode->GetDisplayMode(), pixel_format, g_config.m_inputFlags);

        if(result!=S_OK) {
            fprintf(stderr, "Failed to switch video mode\n");
            return S_OK;
        }

        decklink_input->StartStreams();
    }
*/
    return S_OK;
}

DeckLinkCapture::DeckLinkCapture(QObject *parent) :
    QThread(parent)
{
    deck_link_capture_delegate=nullptr;

    decklink=nullptr;
    display_mode_iterator=nullptr;
    display_mode=nullptr;
    decklink_input=nullptr;
    decklink_iterator=nullptr;
    decklink_attributes=nullptr;

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
    decklink_iterator=CreateDeckLinkIteratorInstance();

    if(!decklink_iterator) {
        qCritical() << "This application requires the DeckLink drivers installed";
        return;
    }

    //

    video_converter=CreateVideoConversionInstance();

    deck_link_capture_delegate=new DeckLinkCaptureDelegate(this);

    qInfo() << "DeckLinkCapture thread started";

    exec();

    deck_link_capture_delegate->Release();

//    delete deck_link_capture_delegate;

    if(decklink_iterator)
        decklink_iterator->Release();

    deleteLater();
}

void DeckLinkCapture::captureStart()
{
    if(decklink)
        return;

    HRESULT result;

    int	device_index=device.index;
    int format_index=format.index;

    qDebug() << device_index << format_index;

    bool format_detection_supported;

    BMDDisplayModeSupport display_mode_supported;

    //

    while((result=decklink_iterator->Next(&decklink))==S_OK) {
        if(device_index==0)
            break;

        --device_index;

        decklink->Release();
    }

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

    decklink_output->CreateVideoFrame(1920, 1080, 1920*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted);

//    decklink_output->CreateVideoFrame(3840, 2160, 3840*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted);


    // Get the display mode
    if(format.pixel_formats.isEmpty()) {
        // Check the card supports format detection
        result=decklink->QueryInterface(IID_IDeckLinkAttributes, (void**)&decklink_attributes);

        if(result==S_OK) {
            result=decklink_attributes->GetFlag(BMDDeckLinkSupportsInputFormatDetection, &format_detection_supported);

            if(result!=S_OK || !format_detection_supported) {
                qCritical() << "Format detection is not supported on this device";
                goto bail;
            }
        }
    }

    //

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

//    if(g_config.m_inputFlags & bmdVideoInputDualStream3D) {
//        if(!(display_mode->GetFlags() & bmdDisplayModeSupports3D)) {
//            qCritical() << "The display mode is not supported with 3D" << format.display_mode_name;
//            goto bail;
//        }
//    }


    // Configure the capture callback
    decklink_input->SetCallback(deck_link_capture_delegate);


    // Start capturing
    result=decklink_input->EnableVideoInput(display_mode->GetDisplayMode(), (BMDPixelFormat)pixel_format.fmt, bmdVideoInputFlagDefault);

    if(result!=S_OK) {
        qCritical() << "Failed to enable video input. Is another application using the card?" << result;
        goto bail;
    }

//    result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, 16, 8); //!!!!!!!!
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

    return;

bail:
    release();
}

void DeckLinkCapture::captureStop()
{
    decklink_input->StopStreams();
    decklink_input->DisableAudioInput();
    decklink_input->DisableVideoInput();

    release();
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

    } else {
        video_converter->ConvertFrame(video_frame, video_frame_converted);

        video_frame=(IDeckLinkVideoInputFrame*)video_frame_converted;

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

void DeckLinkCapture::release()
{
    if(decklink) {
        decklink->Release();
        decklink=nullptr;
    }

    if(display_mode) {
        display_mode->Release();
        display_mode=nullptr;
    }

    if(decklink_attributes) {
        decklink_attributes->Release();
        decklink_attributes=nullptr;
    }

    if(display_mode_iterator) {
        display_mode_iterator->Release();
        display_mode_iterator=nullptr;
    }

    if(decklink_input) {
        decklink_input->Release();
        decklink_input=nullptr;
    }
}
