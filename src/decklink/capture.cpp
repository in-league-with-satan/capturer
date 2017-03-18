#include <QDebug>
#include <QMutexLocker>
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
#include "ffmpeg_format_converter.h"
#include "convert_thread.h"
#include "audio_tools.h"
#include "decklink_tools.h"

extern "C" {
#include <libavutil/bswap.h>
}

#include "capture.h"

const bool ext_converter=false;
//const bool ext_converter=true;

class DeckLinkCaptureDelegate : public IDeckLinkInputCallback
{
public:
    DeckLinkCaptureDelegate(DeckLinkCapture *parent);

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
  ,ref_count(1)
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
    deck_link_capture_delegate=nullptr;

    decklink=nullptr;
    display_mode=nullptr;
    decklink_input=nullptr;

    device.index=0;
    format.index=0;
    pixel_format.fmt=bmdFormat10BitRGB;

    //

    video_converter=nullptr;

    video_frame_converted_720p=nullptr;
    video_frame_converted_1080p=nullptr;
    video_frame_converted_2160p=nullptr;

    if(ext_converter) {
        conv_thread=new DlConvertThreadContainer(4, this);

        connect(conv_thread, SIGNAL(frameSkipped()), this, SIGNAL(frameSkipped()));

    } else
        video_converter=CreateVideoConversionInstance();

    //

    setTerminationEnabled();

    // start(QThread::NormalPriority);
    start(QThread::TimeCriticalPriority);
}

DeckLinkCapture::~DeckLinkCapture()
{
    // captureStop();

    terminate();
}

void DeckLinkCapture::setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format, int audio_channels)
{
    this->device=device;
    this->format=format;
    this->pixel_format=pixel_format;
    this->audio_channels=audio_channels;

    if(ext_converter)
        conv_thread->setAudioChannels(audio_channels);
}

void DeckLinkCapture::subscribeForAll(FrameBuffer *obj)
{
    if(ext_converter)
        conv_thread->subscribeForAll(obj);

    else if(!l_full.contains(obj))
        l_full.append(obj);
}

void DeckLinkCapture::subscribeForAudio(FrameBuffer *obj)
{
    if(ext_converter)
        conv_thread->subscribeForAudio(obj);

    else if(!l_audio.contains(obj))
        l_audio.append(obj);
}

void DeckLinkCapture::unsubscribe(FrameBuffer *obj)
{
    if(ext_converter)
        conv_thread->unsubscribe(obj);

    else {
        l_full.removeAll(obj);
        l_audio.removeAll(obj);
    }
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
        return;
    }

    decklink_iterator->Release();

    //

    // ff_converter=new FF::FormatConverter();

    // ff_converter->setup(AV_PIX_FMT_GBRP10LE, QSize(1920, 1080), AV_PIX_FMT_BGRA, QSize(1920, 1080));

    deck_link_capture_delegate=new DeckLinkCaptureDelegate(this);

    qInfo() << "DeckLinkCapture thread started";

    exec();

    deck_link_capture_delegate->Release();

    // delete ff_converter;

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
    Q_UNUSED(events)

    HRESULT result;


    BMDPixelFormat pixel_format=bmdFormat10BitYUV;

    if(format_flags & bmdDetectedVideoInputRGB444)
        pixel_format=bmdFormat10BitRGB;

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
                       frame_duration, frame_scale,
                       (mode->GetFieldDominance()==bmdProgressiveFrame || mode->GetFieldDominance()==bmdProgressiveSegmentedFrame),
                       pixel_format==bmdFormat10BitYUV ? "10BitYUV" : "10BitRGB");

    //msleep(1000);
}

void DeckLinkCapture::videoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet)
{
    if(!video_frame || !audio_packet)
        return;

    if(video_frame->GetFlags() & bmdFrameHasNoInputSource) {
        qCritical() << "No input signal detected";

        emit noInputSignalDetected();

        return;

    } else {
        BMDTimeValue frame_time;
        BMDTimeValue frame_duration;

        video_frame->GetStreamTime(&frame_time, &frame_duration, frame_scale);

        if(frame_time!=0) {
            if(frame_time - frame_time_prev!=frame_duration) {
                qCritical() << "decklink: frame dropped" << QDateTime::currentDateTime();

                for(int i=0; i<(frame_time - frame_time_prev)/frame_duration - 1; ++i) {
                    emit frameSkipped();
                }

                //

                // decklink_input->StopStreams();

                // decklink_input->StartStreams();
            }

        } else
            frame_counter=0;

        frame_time_prev=frame_time;

        //

        if(ext_converter) {
            conv_thread->addFrame((IDeckLinkVideoInputFrame*)video_frame, audio_packet, frame_counter++, frame_time==0);

            // if(frame_time!=0)
            //     conv_thread->addFrame((IDeckLinkVideoInputFrame*)video_frame, audio_packet, frame_counter++, frame_time==0);

        } else {
            FrameBuffer::Frame frame;

            void *d_video;
            void *d_audio;

            IDeckLinkMutableVideoFrame *frame_out=nullptr;

            /*

            frame_out=(IDeckLinkMutableVideoFrame *)video_frame;


            if(video_frame->GetWidth()==1280) {
                video_converter->ConvertFrame(video_frame, video_frame_converted_720p);

                frame_out=video_frame_converted_720p;

            } else if(video_frame->GetWidth()==1920) {
                video_converter->ConvertFrame(video_frame, video_frame_converted_1080p);

                frame_out=video_frame_converted_1080p;

            } else {
                video_converter->ConvertFrame(video_frame, video_frame_converted_2160p);

                frame_out=video_frame_converted_2160p;
            }

            */

            //

            frame_out=(IDeckLinkMutableVideoFrame*)video_frame;


            if(frame_out->GetPixelFormat()!=bmdFormat10BitRGB) {
                qCritical() << "wrong pixel format";
                return;
            }


            frame_out->GetBytes(&d_video);


            {
                size_t pos;
                uint32_t *d_ptr=(uint32_t*)d_video;

                for(size_t i_row=0, rows=frame_out->GetHeight(); i_row<rows; ++i_row) {
                    for(size_t i_col=0, cols=frame_out->GetWidth(); i_col<cols; ++i_col) {
                        pos=i_row*cols + i_col;

                        d_ptr[pos]=av_be2ne32(d_ptr[pos]);
                    }
                }
            }



            frame.ba_video.resize(frame_out->GetRowBytes() * frame_out->GetHeight());

            memcpy(frame.ba_video.data(), d_video, frame.ba_video.size());

            frame.size_video=QSize(frame_out->GetWidth(), frame_out->GetHeight());

            frame.bmd_pixel_format=frame_out->GetPixelFormat();




            //

            audio_packet->GetBytes(&d_audio);

            frame.ba_audio.resize(audio_packet->GetSampleFrameCount()*audio_channels*(16/8));

            memcpy(frame.ba_audio.data(), d_audio, frame.ba_audio.size());

            if(audio_channels==8)
                channelsRemap(&frame.ba_audio);

            //


            QMutexLocker ml(&mutex_subscription);

            foreach(FrameBuffer *buf, l_full)
                buf->appendFrame(frame);

            if(!l_audio.isEmpty()) {
                frame.ba_video.clear();

                foreach(FrameBuffer *buf, l_audio)
                    buf->appendFrame(frame);
            }
        }
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

    if(ext_converter)
        conv_thread->init(decklink_output);

    else {
        if(!video_frame_converted_720p)
            decklink_output->CreateVideoFrame(1280, 720, 720*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted_720p);

        if(!video_frame_converted_1080p)
            decklink_output->CreateVideoFrame(1920, 1080, 1920*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted_1080p);

        if(!video_frame_converted_2160p)
            decklink_output->CreateVideoFrame(3840, 2160, 3840*4, bmdFormat8BitBGRA, bmdFrameFlagDefault, &video_frame_converted_2160p);
    }

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

        switch(audio_channels) {
        case 6:
        case 8:
            qInfo() << "decklink_input->EnableAudioInput 8";
            result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, 8);
            break;

        case 2:
        default:
            qInfo() << "decklink_input->EnableAudioInput 2";
            result=decklink_input->EnableAudioInput(bmdAudioSampleRate48kHz, bmdAudioSampleType16bitInteger, 2);
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

    if(decklink) {
        decklink->Release();
        decklink=nullptr;
    }
}
