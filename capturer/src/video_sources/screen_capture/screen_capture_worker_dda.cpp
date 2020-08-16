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

#include <ctime>
#include <thread>

#include "framerate.h"
#include "audio_wasapi.h"

#include "screen_capture_worker_dda.h"

ScreenCaptureWorkerDda::ScreenCaptureWorkerDda(SourceInterface *si, QObject *parent)
    : QObject(parent)
    , si((SourceInterfacePublic*)si)
{
    audio_wasapi=new AudioWasapi(this);

    si->pixel_format=PixelFormat::bgr0;
    si->type_flags=SourceInterface::TypeFlag::video;
    si->current_dev_name="Desktop Duplication API";
}

ScreenCaptureWorkerDda::~ScreenCaptureWorkerDda()
{
}

bool ScreenCaptureWorkerDda::step()
{
step_start:

    if(!output_duplication)
        return false;

    HRESULT res;

    ID3D11Texture2D *acquired_desktop_image=nullptr;

    IDXGIResource *desktop_resource=nullptr;
    DXGI_OUTDUPL_FRAME_INFO frame_info;

    res=output_duplication->AcquireNextFrame(0, &frame_info, &desktop_resource);

    bool frame_ready=true;

    if(FAILED(res)) {
        // emit errorString(errorString(res));

        if(res!=DXGI_ERROR_WAIT_TIMEOUT) {
            qDebug() << errorString(res);
        }

        if(res==DXGI_ERROR_INVALID_CALL || res==DXGI_ERROR_ACCESS_LOST) {
            deviceStop();
            deviceStart();
            goto step_start;
        }

        frame_ready=false;
    }

    Frame::ptr frame=Frame::make();

    frame->device_index=si->device_index;
    frame->video.time_base={ 1, 1000000000 };
    frame->video.pts=std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    if(frame_ready) {
        res=desktop_resource->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&acquired_desktop_image));

        if(FAILED(res)) {
            QString err=QString("desktop_resource->QueryInterface err: %1").arg(errorString(res));
            qCritical().noquote() << err;
            deviceStop();
            emit errorString(err);
            return false;
        }

        ID3D11Texture2D *texture=nullptr;
        D3D11_TEXTURE2D_DESC description;

        acquired_desktop_image->GetDesc(&description);

        description.BindFlags=0;
        description.CPUAccessFlags=D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        description.Usage=D3D11_USAGE_STAGING;
        description.MiscFlags=0;

        res=device->CreateTexture2D(&description, NULL, &texture);

        if(FAILED(res)) {
            qCritical() << errorString(res);
            return false;
        }

        ID3D11DeviceContext *immediate_context=nullptr;

        device->GetImmediateContext(&immediate_context);

        immediate_context->CopyResource(texture, acquired_desktop_image);


        D3D11_MAPPED_SUBRESOURCE resource;
        UINT subresource=D3D11CalcSubresource(0, 0, 0);

        immediate_context->Map(texture, subresource, D3D11_MAP_READ_WRITE, 0, &resource);

        if(description.Format!=DXGI_FORMAT_B8G8R8A8_UNORM || resource.DepthPitch!=description.Width*description.Height*4) {
            immediate_context->Release();
            texture->Release();
            QString err=QString("unknown format: %1").arg(description.Format);
            qCritical().noquote() << err;
            deviceStop();
            emit errorString(err);
            return false;
        }

        //

        frame->video.size=QSize(description.Width, description.Height);
        frame->video.data_size=resource.DepthPitch;
        frame->video.dummy.resize(resource.DepthPitch);
        frame->video.data_ptr=(uint8_t*)frame->video.dummy.data();
        frame->video.pixel_format=PixelFormat::bgr0;

        memcpy(frame->video.data_ptr, resource.pData, resource.DepthPitch);

        //

        output_duplication->ReleaseFrame();

        texture->Release();
        immediate_context->Release();
        acquired_desktop_image->Release();
        desktop_resource->Release();

        //

        if(si->framesize!=frame->video.size) {
            si->framesize=frame->video.size;
            emit formatChanged(QString("%1-bgr0").arg(si->framesize.load().width()));
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

    //

    return true;
}

QString ScreenCaptureWorkerDda::errorString(HRESULT error_code)
{
    switch(error_code) {
    case DXGI_ERROR_INVALID_CALL:
        return QStringLiteral("DXGI_ERROR_INVALID_CALL");

    case DXGI_ERROR_NOT_FOUND:
        return QStringLiteral("DXGI_ERROR_NOT_FOUND");

    case DXGI_ERROR_MORE_DATA:
        return QStringLiteral("DXGI_ERROR_MORE_DATA");

    case DXGI_ERROR_UNSUPPORTED:
        return QStringLiteral("DXGI_ERROR_UNSUPPORTED");

    case DXGI_ERROR_DEVICE_REMOVED:
        return QStringLiteral("DXGI_ERROR_DEVICE_REMOVED");

    case DXGI_ERROR_DEVICE_HUNG:
        return QStringLiteral("DXGI_ERROR_DEVICE_HUNG");

    case DXGI_ERROR_DEVICE_RESET:
        return QStringLiteral("DXGI_ERROR_DEVICE_RESET");

    case DXGI_ERROR_WAS_STILL_DRAWING:
        return QStringLiteral("DXGI_ERROR_WAS_STILL_DRAWING");

    case DXGI_ERROR_FRAME_STATISTICS_DISJOINT:
        return QStringLiteral("DXGI_ERROR_FRAME_STATISTICS_DISJOINT");

    case DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE:
        return QStringLiteral("DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE");

    case DXGI_ERROR_DRIVER_INTERNAL_ERROR:
        return QStringLiteral("DXGI_ERROR_DRIVER_INTERNAL_ERROR");

    case DXGI_ERROR_NONEXCLUSIVE:
        return QStringLiteral("DXGI_ERROR_NONEXCLUSIVE");

    case DXGI_ERROR_NOT_CURRENTLY_AVAILABLE:
        return QStringLiteral("DXGI_ERROR_NOT_CURRENTLY_AVAILABLE");

    case DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED:
        return QStringLiteral("DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED");

    case DXGI_ERROR_REMOTE_OUTOFMEMORY:
        return QStringLiteral("DXGI_ERROR_REMOTE_OUTOFMEMORY");

    case DXGI_ERROR_ACCESS_LOST:
        return QStringLiteral("DXGI_ERROR_ACCESS_LOST");

    case DXGI_ERROR_WAIT_TIMEOUT:
        return QStringLiteral("DXGI_ERROR_WAIT_TIMEOUT");

    case DXGI_ERROR_SESSION_DISCONNECTED:
        return QStringLiteral("DXGI_ERROR_SESSION_DISCONNECTED");

    case DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE:
        return QStringLiteral("DXGI_ERROR_RESTRICT_TO_OUTPUT_STALE");

    case DXGI_ERROR_CANNOT_PROTECT_CONTENT:
        return QStringLiteral("DXGI_ERROR_CANNOT_PROTECT_CONTENT");

    case DXGI_ERROR_ACCESS_DENIED:
        return QStringLiteral("DXGI_ERROR_ACCESS_DENIED");

    case DXGI_ERROR_NAME_ALREADY_EXISTS:
        return QStringLiteral("DXGI_ERROR_NAME_ALREADY_EXISTS");

    case DXGI_ERROR_SDK_COMPONENT_MISSING:
        return QStringLiteral("DXGI_ERROR_SDK_COMPONENT_MISSING");

    case E_INVALIDARG:
        return QStringLiteral("E_INVALIDARG");
    }

    return QString("error: %1").arg(error_code, 0, 16);
}

QStringList ScreenCaptureWorkerDda::availableAudioInput()
{
    return audio_wasapi->availableAudioInputStr();
}

void ScreenCaptureWorkerDda::setAudioDevice(QString device_name)
{
    audio_device_name=device_name;
}

void ScreenCaptureWorkerDda::deviceStart()
{
    deviceStop();

    HRESULT res;

    D3D_FEATURE_LEVEL feature_levels[]={
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_1
    };

    D3D_FEATURE_LEVEL d3d_feature_level;

    IDXGIDevice *dxgi_device=nullptr;
    IDXGIAdapter *dxgi_adapter=nullptr;
    IDXGIOutput *dxgi_output=nullptr;
    IDXGIOutput1 *dxgi_output_1=nullptr;
    DXGI_ADAPTER_DESC adapter_description;
    DXGI_OUTPUT_DESC output_description;

    //

    res=D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE,
                          NULL, 0, feature_levels,
                          ARRAYSIZE(feature_levels),
                          D3D11_SDK_VERSION,
                          &device,
                          &d3d_feature_level,
                          &device_context);

    if(FAILED(res)) {
        qCritical() << errorString(res);
        emit errorString(errorString(res));
        goto init_fail;
    }

    //

    res=device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgi_device));

    if(FAILED(res)) {
        qCritical() << errorString(res);
        emit errorString(errorString(res));
        goto init_fail;
    }

    //

    res=dxgi_device->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&dxgi_adapter));

    if(FAILED(res)) {
        qCritical() << errorString(res);
        emit errorString(errorString(res));
        goto init_fail;
    }

    //

    dxgi_device->Release();
    dxgi_device=nullptr;

    //

    res=dxgi_adapter->EnumOutputs(0, &dxgi_output);

    if(FAILED(res)) {
        qCritical() << errorString(res);
        emit errorString(errorString(res));
        goto init_fail;
    }


    dxgi_adapter->GetDesc(&adapter_description);

    qDebug() << "adapter_description" << QString::fromWCharArray(adapter_description.Description);

    dxgi_adapter->Release();
    dxgi_adapter=nullptr;


    dxgi_output->GetDesc(&output_description);


    res=dxgi_output->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&dxgi_output_1));

    if(FAILED(res)) {
        qCritical() << errorString(res);
        emit errorString(errorString(res));
        goto init_fail;
    }

    dxgi_output->Release();
    dxgi_output=nullptr;

    res=dxgi_output_1->DuplicateOutput(device, &output_duplication);

    if(FAILED(res)) {
        qCritical() << errorString(res);
        emit errorString(errorString(res));
        goto init_fail;
    }

    dxgi_output_1->Release();
    dxgi_output_1=nullptr;


    output_duplication->GetDesc(&output_duplication_description);

    //

    si->type_flags=SourceInterface::TypeFlag::video;

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

    frame_duration=(1./Framerate::fromRational(si->framerate))*1000*1000*1000;
    frame_time_point=std::chrono::high_resolution_clock::now();

    qInfo() << "frame_duration" << frame_duration << Framerate::fromRational(si->framerate);

    //

    emit signalLost(false);

    si->signal_lost=false;

    return;

init_fail:
    if(dxgi_output_1)
        dxgi_output_1->Release();

    if(dxgi_output)
        dxgi_output->Release();

    if(dxgi_adapter)
        dxgi_adapter->Release();

    if(dxgi_device)
        dxgi_device->Release();

    deviceStop();
}

void ScreenCaptureWorkerDda::deviceStop()
{
    if(output_duplication) {
        output_duplication->Release();
        output_duplication=nullptr;
    }

    if(device_context) {
        device_context->Release();
        device_context=nullptr;
    }

    if(device) {
        device->Release();
        device=nullptr;
    }

    audio_wasapi->deviceStop();

    si->framesize=QSize();

    si->signal_lost=true;

    emit signalLost(true);
}

