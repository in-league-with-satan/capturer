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

#include <QDebug>
#include <QElapsedTimer>

#ifdef __WIN32__
#define INITGUID
#include <wtypes.h>
#include <functiondiscoverykeys_devpkey.h>
#include <audioclient.h>
#include <mmdeviceapi.h>
#endif

#include "ff_audio_converter.h"

#include "audio_wasapi.h"


#ifdef __WIN32__

struct DeviceData
{
    DeviceData() {}

    ~DeviceData() {
        if(device) {
            device->Release();
            CoTaskMemFree(id);
        }
    }

    IMMDevice *device=nullptr;
    LPWSTR id;
};

struct AudioWasapiPrivate
{
    IAudioClient *pAudioClient=nullptr;
    IAudioCaptureClient *pCaptureClient=nullptr;
    WAVEFORMATEX *pwfx=nullptr;

    void release() {
        if(pwfx) {
            CoTaskMemFree(pwfx);
            pwfx=nullptr;
        }

        if(pAudioClient) {
            pAudioClient->Release();
            pAudioClient=nullptr;
        }

        if(pCaptureClient) {
            pCaptureClient->Release();
            pCaptureClient=nullptr;
        }
    }
};

#endif

AudioWasapi::AudioWasapi(QObject *parent)
    : QObject(parent)
#ifdef __WIN32__
    , d(new AudioWasapiPrivate())
    , audio_converter(new AudioConverter())
#endif
{
#ifdef __WIN32__
    CoInitialize(0);
#endif

    updateDevList();
}

AudioWasapi::~AudioWasapi()
{
    deviceStop();

#ifdef __WIN32__

    delete d;
    delete audio_converter;

    CoUninitialize();

#endif
}

QList <AudioWasapi::Device> AudioWasapi::availableAudioInput() const
{
    return list_devices;
}

QStringList AudioWasapi::availableAudioInputStr()
{
    return QStringList(list_devices_str);
}

bool AudioWasapi::deviceStart(const AudioWasapi::Device &device)
{
#ifdef __WIN32__

    DeviceData *dd=(DeviceData*)device.d;

    HRESULT hr=dd->device->Activate(IID_IAudioClient, CLSCTX_ALL, NULL, (void**)&d->pAudioClient);

    if(FAILED(hr)) {
        qCritical() << "IMMDevice::Activate err";
        goto Exit;
    }


    hr=d->pAudioClient->GetMixFormat(&d->pwfx);

    if(FAILED(hr)) {
        qCritical() << "IAudioClient::GetMixFormat err";
        goto Exit;
    }


    switch(d->pwfx->wFormatTag) {
    case WAVE_FORMAT_EXTENSIBLE:
        break;

    case WAVE_FORMAT_PCM:
    case WAVE_FORMAT_IEEE_FLOAT:
    default:
        qCritical() << "bad format";
        goto Exit;
    }


    hr=d->pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 10000000, 0, d->pwfx, NULL);

    if(FAILED(hr)) {
        qCritical() << "IAudioClient::Initialize err";
        goto Exit;
    }


    hr=d->pAudioClient->GetService(IID_IAudioCaptureClient, (void**)&d->pCaptureClient);

    if(FAILED(hr)) {
        qCritical() << "IAudioClient::GetService err";
        goto Exit;
    }


    hr=d->pAudioClient->Start();

    if(FAILED(hr)) {
        qCritical() << "IAudioClient::Start err";
        goto Exit;
    }


    if(!audio_converter->init(av_get_default_channel_layout(d->pwfx->nChannels), d->pwfx->nSamplesPerSec, AV_SAMPLE_FMT_FLT,
                              av_get_default_channel_layout(d->pwfx->nChannels), 48000, AV_SAMPLE_FMT_S16)) {
        goto Exit;
    }

    return true;

Exit:
    deviceStop();

#endif

    return false;
}

void AudioWasapi::deviceStop()
{
#ifdef __WIN32__

    if(d->pAudioClient)
        d->pAudioClient->Stop();

    d->release();

#endif
}

int AudioWasapi::channels() const
{
#ifdef __WIN32__

    if(!d->pwfx)
        return 0;

    return d->pwfx->nChannels;

#endif

    return 2;
}

int AudioWasapi::sampleSize() const
{
    return 16;
}

QByteArray AudioWasapi::getData()
{
    QByteArray data;

#ifdef __WIN32__

    if(!d->pCaptureClient)
        return data;

    UINT32 packetLength=0;
    BYTE *pData;
    DWORD flags=0;
    UINT32 numFramesAvailable=0;

    int framesize=d->pwfx->wBitsPerSample/8*d->pwfx->nChannels;

    HRESULT hr=d->pCaptureClient->GetNextPacketSize(&packetLength);

    if(FAILED(hr)) {
        qCritical() << "IAudioClient::GetNextPacketSize err";
        return data;
    }

    while(packetLength!=0) {
        hr=d->pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, NULL, NULL);

        if(FAILED(hr)) {
            qCritical() << "IAudioClient::GetBuffer err";
            break;
        }

        if(flags&AUDCLNT_BUFFERFLAGS_SILENT) {
            data+=QByteArray(numFramesAvailable*framesize, 0);

        } else {
            data+=QByteArray((char*)pData, numFramesAvailable*framesize);
        }

        hr=d->pCaptureClient->ReleaseBuffer(numFramesAvailable);

        if(FAILED(hr)) {
            qCritical() << "IAudioClient::ReleaseBuffer err";
            break;
        }

        hr=d->pCaptureClient->GetNextPacketSize(&packetLength);

        if(FAILED(hr)) {
            qCritical() << "IAudioClient::GetNextPacketSize err";
            break;
        }
    }

    if(!audio_converter->isReady())
        return QByteArray();

    audio_converter->convert(&data);

#endif

    return data;
}

void AudioWasapi::updateDevList()
{
#ifdef __WIN32__

    while(!list_devices.isEmpty())
        delete (DeviceData*)list_devices.takeLast().d;

    QStringList(list_devices_str).clear();

    QStringList list_devices_str_tmp;

    //

    IMMDeviceEnumerator *device_enumerator=NULL;
    IMMDeviceCollection *device_collection=NULL;

    HRESULT hr=CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&device_enumerator));

    if(FAILED(hr)) {
        qCritical() << "CoCreateInstance err" << hr;
        return;
    }

    hr=device_enumerator->EnumAudioEndpoints(eRender , DEVICE_STATE_ACTIVE, &device_collection);

    if(FAILED(hr)) {
        qCritical() << "IMMDeviceEnumerator::EnumAudioEndpoints err";
        return;
    }

    UINT device_count;
    hr=device_collection->GetCount(&device_count);

    if(FAILED(hr)) {
        qCritical() << "IMMDeviceCollection::GetCount err";
        return;
    }

    for(UINT i_device=0; i_device<device_count; ++i_device) {
        Device dev;
        DeviceData *dd=new DeviceData();
        dev.d=dd;

        hr=device_collection->Item(i_device, &dd->device);

        if(FAILED(hr)) {
            qCritical() << "IMMDeviceCollection::Item err";
            continue;
        }

        hr=dd->device->GetId(&dd->id);

        if(FAILED(hr)) {
            qCritical() << "GetId err";
            continue;
        }

        IPropertyStore *property_store;
        hr=dd->device->OpenPropertyStore(STGM_READ, &property_store);

        if(FAILED(hr)) {
            qCritical() << "IMMDevice::OpenPropertyStore err";
            continue;
        }


        PROPVARIANT friendly_name;
        PropVariantInit(&friendly_name);
        hr=property_store->GetValue(PKEY_Device_FriendlyName, &friendly_name);

        property_store->Release();

        if(FAILED(hr)) {
            qCritical() << "IPropertyStore::GetValue failed";
            return;
        }

        dev.name=QString::fromWCharArray(friendly_name.pwszVal);

        PropVariantClear(&friendly_name);

        list_devices.append(dev);
        list_devices_str_tmp.append(dev.name);
    }

    list_devices_str=list_devices_str_tmp;

    device_collection->Release();
    device_enumerator->Release();

#endif
}
