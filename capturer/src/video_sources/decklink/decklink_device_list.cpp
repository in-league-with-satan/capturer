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
#include <QVariantMap>

#include <inttypes.h>

#include "decklink_tools.h"

#include "decklink_global.h"

#include "decklink_device_list.h"

#ifdef __linux__
const BMDPixelFormat known_pixel_formats[]={
#else
const uint32_t known_pixel_formats[]={
#endif

#ifdef LIB_DECKLINK

    bmdFormat8BitYUV,
    bmdFormat10BitYUV,
    bmdFormat8BitARGB,
    bmdFormat8BitBGRA,
    bmdFormat10BitRGB,
    bmdFormat12BitRGB,
    bmdFormat12BitRGBLE,
    bmdFormat10BitRGBXLE,
    bmdFormat10BitRGBX,
    bmdFormatH265,
    bmdFormatDNxHR,

#endif
    0
};

QString Decklink::BMDPixelFormatToString(uint32_t format)
{
    static QMap <int, QString> format_names;

#ifdef LIB_DECKLINK

    if(format_names.isEmpty()) {
        format_names[bmdFormat8BitYUV]=
                "8BitYUV";

        format_names[bmdFormat10BitYUV]=
                "10BitYUV";

        format_names[bmdFormat8BitARGB]=
                "8BitARGB";

        format_names[bmdFormat8BitBGRA]=
                "8BitBGRA";

        format_names[bmdFormat10BitRGB]=
                "10BitRGB";

        format_names[bmdFormat12BitRGB]=
                "12BitRGB";

        format_names[bmdFormat12BitRGBLE]=
                "12BitRGBLE";

        format_names[bmdFormat10BitRGBXLE]=
                "10BitRGBXLE";

        format_names[bmdFormat10BitRGBX]=
                "10BitRGBX";

        format_names[bmdFormatH265]=
                "H265";

        format_names[bmdFormatDNxHR]=
                "DNxHR";
    }

#endif

    return format_names.value(format, QStringLiteral("unknown"));
}

int supportedInputFormats(IDeckLink *decklink, Decklink::Formats *formats);

Decklink::Devices Decklink::getDevices()
{
    static Decklink::Devices devices;
    static bool updated=false;

#ifdef LIB_DECKLINK

    if(!updated) {
        updated=true;

        IDeckLinkIterator *decklink_iterator;
        IDeckLink *decklink;

        HRESULT result;

        int num_devices=0;

        decklink_iterator=CreateDeckLinkIteratorInstance();

        if(decklink_iterator==nullptr) {
            // qWarning() << "a DeckLink iterator could not be created. the DeckLink drivers may not be installed";
            return devices;
        }

        while(decklink_iterator->Next(&decklink)==S_OK) {
            Decklink::Device dev;

            dev.index=num_devices++;

            {
#ifdef __linux__

                char *device_name_string=nullptr;

                result=decklink->GetModelName((const char **)&device_name_string);

                if(result!=S_OK) {
                    qCritical() << "IDeckLink::GetModelName err";
                    return devices;
                }

                dev.name=QString(device_name_string);

#else

                wchar_t *device_name_string=nullptr;

                result=decklink->GetModelName(&device_name_string);

                if(result!=S_OK) {
                    qCritical() << "IDeckLink::GetModelName err";
                    return devices;
                }

                dev.name=QString::fromWCharArray(device_name_string);

#endif

#ifndef _WIN64
                free(device_name_string);
#endif
            }

            supportedInputFormats(decklink, &dev.formats);

            devices.append(dev);


            decklink->Release();
        }

        decklink_iterator->Release();


        if(num_devices==0) {
            // qWarning() << "no decklink devices were found";
        }
    }

#endif

    return devices;
}

int supportedInputFormats(IDeckLink *decklink, Decklink::Formats *formats)
{
    HRESULT result;

#ifdef LIB_DECKLINK

    IDeckLinkInput *decklink_input=nullptr;
    IDeckLinkDisplayModeIterator *display_mode_iterator=nullptr;
    IDeckLinkDisplayMode *display_mode=nullptr;
    int format_index=0;

    result=decklink->QueryInterface(IID_IDeckLinkInput, (void**)&decklink_input);

    if(result!=S_OK) {
        qCritical() << "could not obtain the IDeckLinkInput interface" << result;
        goto exit;
    }

    result=decklink_input->GetDisplayModeIterator(&display_mode_iterator);

    if(result!=S_OK) {
        qCritical() << "could not obtain the video input display mode iterator" << result;
        goto exit;
    }


    while(display_mode_iterator->Next(&display_mode)==S_OK) {
#ifdef __linux__

        char *display_mode_string=nullptr;

        result=display_mode->GetName((const char **)&display_mode_string);

#else

        wchar_t *display_mode_string=nullptr;

        result=display_mode->GetName(&display_mode_string);

#endif

        if(result==S_OK) {
            Decklink::Format format;
            format.index=format_index++;

#ifdef __linux__

            format.display_mode_name=QString(display_mode_string);

#else

            format.display_mode_name=QString::fromWCharArray(display_mode_string);

#endif

#ifndef _WIN64
            free(display_mode_string);
#endif

            format.width=display_mode->GetWidth();
            format.height=display_mode->GetHeight();

            display_mode->GetFrameRate(&format.framerate_duration, &format.framerate_scale);

            format.framerate=(double)format.framerate_scale/(double)format.framerate_duration;

            {
                int pixel_format_index=0;
                BMDDisplayModeSupport display_mode_support;

                while(known_pixel_formats[pixel_format_index]!=0) {
                    if((decklink_input->DoesSupportVideoMode(display_mode->GetDisplayMode(), (BMDPixelFormat)known_pixel_formats[pixel_format_index], bmdVideoInputFlagDefault, &display_mode_support, nullptr)==S_OK) && (display_mode_support!=bmdDisplayModeNotSupported))
                        format.pixel_formats.append(Decklink::PixelFormat(known_pixel_formats[pixel_format_index]));

                    pixel_format_index++;
                }
            }

            formats->append(format);
        }

        display_mode->Release();
    }

exit:
    if(display_mode_iterator)
        display_mode_iterator->Release();

    if(decklink_input)
        decklink_input->Release();

#endif

    return result;
}

Decklink::PixelFormat::PixelFormat()
{
    fmt=bmdFormat8BitYUV;
}

QString Decklink::PixelFormat::name()
{
    return BMDPixelFormatToString(fmt);
}
