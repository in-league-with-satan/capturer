/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <assert.h>

#ifdef __WIN32__

#include <windows.h>
#include <dshow.h>

#endif

#include "framerate.h"
#include "ff_tools.h"
#include "pixel_format_dshow_helper.h"

#include "tools_dshow.h"

#ifdef __WIN32__

QString guidToStr(const GUID &guid)
{
    char buf[42]={};

    snprintf(buf, sizeof(buf), "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             (uint32_t)guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    return QString(buf);
}

IBaseFilter *getDevFilter(const QString &device_name)
{
    IBaseFilter *base_filter=nullptr;
    IMoniker *moniker=nullptr;
    ICreateDevEnum *dev_enum=nullptr;
    IEnumMoniker *enum_moniker=nullptr;

    if(CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&dev_enum)!=S_OK)
        return base_filter;

    if(dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, (IEnumMoniker**)&enum_moniker, 0)!=S_OK)
        return base_filter;

    while(enum_moniker->Next(1, &moniker, nullptr)==S_OK) {
        LPMALLOC co_malloc=nullptr;
        IBindCtx *bind_ctx=nullptr;
        LPOLESTR olestr=nullptr;
        QString dev_id_string;

        if(CoGetMalloc(1, &co_malloc)!=S_OK)
            goto fail;

        if(CreateBindCtx(0, &bind_ctx)!=S_OK)
            goto fail;

        if(moniker->GetDisplayName(bind_ctx, nullptr, &olestr)!=S_OK)
            goto fail;

        dev_id_string=QString::fromWCharArray(olestr).replace(":", "_");

        if(device_name!=dev_id_string)
            goto fail;

        if(moniker->BindToObject(0, 0, IID_IBaseFilter, (void**)&base_filter)!=S_OK)
            goto fail;

fail:
        if(olestr && co_malloc)
            co_malloc->Free(olestr);

        if(bind_ctx)
            bind_ctx->Release();

        moniker->Release();
    }

    enum_moniker->Release();

    if(!base_filter) {
        qCritical() << "could't find the device " << device_name;
    }

    return base_filter;
}

QList <FFDevice::Format> getDeviceCapabilities(const QString &dev_name)
{
    QList <FFDevice::Format> capabilities;

    QMap <quint64, FFDevice::Resolution> resolution[PixelFormat::size];

    IBaseFilter *dev_filter=getDevFilter(dev_name);

    if(!dev_filter)
        return capabilities;

    GUID prop_category;
    DWORD pcb_returned;
    IEnumPins *pins_enum=nullptr;
    IPin *pin;

    if(dev_filter->EnumPins(&pins_enum)!=S_OK)
        return capabilities;

    while(pins_enum->Next(1, &pin, nullptr)==S_OK) {
        IKsPropertySet *p=nullptr;
        PIN_INFO info;

        pin->QueryPinInfo(&info);

        info.pFilter->Release();

        if(info.dir!=PINDIR_OUTPUT)
            goto next_pin;

        if(pin->QueryInterface(IID_IKsPropertySet, (void**)&p)!=S_OK)
            goto next_pin;

        if(p->Get(AMPROPSETID_Pin, AMPROPERTY_PIN_CATEGORY, nullptr, 0, &prop_category, sizeof(GUID), &pcb_returned)!=S_OK)
            goto next_pin;

        if(!IsEqualGUID(prop_category, PIN_CATEGORY_CAPTURE))
            goto next_pin;

        {
            IAMStreamConfig *config=nullptr;
            VIDEO_STREAM_CONFIG_CAPS *vcaps=nullptr;
            int size, n;

            if(pin->QueryInterface(IID_IAMStreamConfig, (void**)&config)!=S_OK)
                goto next_pin;

            if(config->GetNumberOfCapabilities(&n, &size)!=S_OK)
                goto end_pin;

            // assert(size==sizeof(VIDEO_STREAM_CONFIG_CAPS));

            vcaps=new VIDEO_STREAM_CONFIG_CAPS;

            for(int i=0; i<n; ++i) {
                AM_MEDIA_TYPE *type=nullptr;

                if(config->GetStreamCaps(i, &type, (BYTE*)vcaps)!=S_OK)
                    goto next_format;

                if(!IsEqualGUID(type->formattype, FORMAT_VideoInfo) && !IsEqualGUID(type->formattype, FORMAT_VideoInfo2))
                    goto next_format;

                {
                    quint64 res_key=QString("%1%2").arg(vcaps->MaxOutputSize.cx).arg(vcaps->MaxOutputSize.cy).toULongLong();

                    PixelFormat tmp_pix_fmt=fromDshowPixelFormat(type->subtype);

                    if(tmp_pix_fmt.isValid()) {
                        // qDebug() << "tmp_pix_fmt" << tmp_pix_fmt.toString() << tmp_pix_fmt;

                        if(resolution[tmp_pix_fmt].contains(res_key))
                            continue;

                        resolution[tmp_pix_fmt][res_key].size=QSize(vcaps->MaxOutputSize.cx, vcaps->MaxOutputSize.cy);

                        if(!resolution[tmp_pix_fmt][res_key].framerate.contains(Framerate::toRational(1e7/vcaps->MinFrameInterval)))
                            resolution[tmp_pix_fmt][res_key].framerate
                                    << ToolsFFSource::framerateBuildSequence(1e7/vcaps->MaxFrameInterval, 1e7/vcaps->MinFrameInterval);

                    } else {
                        qDebug() << "unknown pix fmt" << guidToStr(type->formattype) << guidToStr(type->subtype);
                    }
                }

next_format:
                if(type->pbFormat)
                    CoTaskMemFree(type->pbFormat);

                CoTaskMemFree(type);
            }

end_pin:
            config->Release();

            delete vcaps;
        }

next_pin:
        if(p)
            p->Release();

        pin->Release();
    }


    for(int i_pf=0; i_pf<PixelFormat::size; i_pf++) {
        if(!resolution[i_pf].isEmpty()) {
            FFDevice::Format fmt;

            foreach(FFDevice::Resolution res, resolution[i_pf].values()) {
                if(!res.framerate.isEmpty()) {
                    fmt.pixel_format=i_pf;
                    fmt.resolution=resolution[i_pf].values();
                }
            }

            if(!fmt.resolution.isEmpty())
                capabilities.append(fmt);
        }
    }


    return capabilities;
}

#endif

QList <FFDevice::Dev> ToolsDirectShow::devList()
{
    QList <FFDevice::Dev> list;

#ifdef __WIN32__

    ICreateDevEnum *dev_enum;

    HRESULT hr=CoCreateInstance(CLSID_SystemDeviceEnum, 0, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void**)&dev_enum);

    if(hr!=S_OK)
        return list;

    IEnumMoniker *enum_moniker;

    hr=dev_enum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &enum_moniker, 0);

    if(hr!=S_OK)
        return list;

    IMoniker *moniker=nullptr;

    while(enum_moniker->Next(1, &moniker, nullptr)==S_OK) {
        IPropertyBag *property_bag;
        LPOLESTR str=0;

        hr=moniker->GetDisplayName(0, 0, &str);

        if(SUCCEEDED(hr)) {
            hr=moniker->BindToStorage(0, 0, IID_IPropertyBag, (void**)&property_bag);

            if(SUCCEEDED(hr)) {
                FFDevice::Dev dev;

                VARIANT var;

                VariantInit(&var);

                hr=property_bag->Read(L"FriendlyName", &var, 0);

                dev.name=QString::fromWCharArray(var.bstrVal);

                IBindCtx *bind_ctx=nullptr;

                if(CreateBindCtx(0, &bind_ctx)!=S_OK) {
                    qCritical() << "CreateBindCtx err";
                    return list;
                }

                LPOLESTR ole_str=nullptr;

                if(moniker->GetDisplayName(bind_ctx, nullptr, &ole_str)!=S_OK) {
                    qCritical() << "moniker->GetDisplayName err";
                    return list;
                }

                dev.dev=QString::fromWCharArray(ole_str);

                dev.dev=dev.dev.replace(":", "_");

                qDebug() << dev.name;

                dev.format=getDeviceCapabilities(dev.dev);

                if(!dev.format.isEmpty())
                    list.append(dev);

            } else {
                qCritical() << "could not bind to storage";
            }
        }
    }

#endif

    return list;
}
