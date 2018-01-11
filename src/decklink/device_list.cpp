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

#include "DeckLinkAPI.h"

#include <device_list.h>

#ifdef __linux__
const BMDPixelFormat known_pixel_formats[]={
#else
const uint32_t known_pixel_formats[]={
#endif
    bmdFormat8BitYUV,
    bmdFormat10BitYUV,
    bmdFormat8BitARGB,
    bmdFormat8BitBGRA,
    bmdFormat10BitRGB,
    bmdFormat12BitRGB,
    bmdFormat12BitRGBLE,
    bmdFormat10BitRGBXLE,
    bmdFormat10BitRGBX,
    0
};

QString BMDPixelFormatToString(uint32_t format)
{
    static QMap <int, QString> format_names;

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
    }

    return format_names.value(format, QStringLiteral("unknown"));
}

int supportedInputFormats(IDeckLink *decklink, DeckLinkFormats *formats);

int GetDevices(DeckLinkDevices *devices)
{
    IDeckLinkIterator *decklink_iterator;
    IDeckLink *decklink;

    HRESULT result;

    int num_devices=0;

    // Create an IDeckLinkIterator object to enumerate all DeckLink cards in the system
    decklink_iterator=CreateDeckLinkIteratorInstance();

    if(decklink_iterator==nullptr) {
        qCritical() << "A DeckLink iterator could not be created. The DeckLink drivers may not be installed.";
        return 1;
    }

    // Enumerate all cards in this system
    while(decklink_iterator->Next(&decklink)==S_OK) {
        DeckLinkDevice dev;

        dev.index=num_devices++;

        // the model name of the DeckLink card
        {
#ifdef __linux__

            char *device_name_string=nullptr;

            result=decklink->GetModelName((const char **)&device_name_string);

            if(result!=S_OK) {
                qCritical() << "decklink->GetModelName err";
                return 2;
            }

            dev.name=QString(device_name_string);

#else

            wchar_t *device_name_string=nullptr;

            result=decklink->GetModelName(&device_name_string);

            if(result!=S_OK) {
                qCritical() << "decklink->GetModelName err";
                return 2;
            }

            dev.name=QString::fromWCharArray(device_name_string);

#endif

#ifndef _WIN64
            free(device_name_string);
#endif
        }

        //

        supportedInputFormats(decklink, &dev.formats);


        devices->append(dev);

        // List the input and output capabilities of the card
        // print_capabilities(decklink);

        // Release the IDeckLink instance when we've finished with it to prevent leaks
        decklink->Release();
    }

    decklink_iterator->Release();


    // If no DeckLink cards were found in the system, inform the user
    if(num_devices==0) {
        qWarning() << "No Blackmagic Design devices were found";
    }

    return 0;
}

int supportedInputFormats(IDeckLink *decklink, DeckLinkFormats *formats)
{
    IDeckLinkInput *decklink_input=nullptr;
    IDeckLinkDisplayModeIterator *display_mode_iterator=nullptr;
    IDeckLinkDisplayMode *display_mode=nullptr;
    HRESULT result;
    int format_index=0;

    // Query the DeckLink for its configuration interface
    result=decklink->QueryInterface(IID_IDeckLinkInput, (void**)&decklink_input);

    if(result!=S_OK) {
        qCritical() << "Could not obtain the IDeckLinkInput interface - result =" << result;
        goto bail;
    }

    // Obtain an IDeckLinkDisplayModeIterator to enumerate the display modes supported on input
    result=decklink_input->GetDisplayModeIterator(&display_mode_iterator);

    if(result!=S_OK) {
        qCritical() << "Could not obtain the video input display mode iterator - result =" << result;
        goto bail;
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
            DeckLinkFormat format;
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
                        format.pixel_formats.append(DeckLinkPixelFormat(known_pixel_formats[pixel_format_index]));

                    pixel_format_index++;
                }
            }

            formats->append(format);
        }

        // Release the IDeckLinkDisplayMode object to prevent a leak
        display_mode->Release();
    }

bail:
    // Ensure that the interfaces we obtained are released to prevent a memory leak
    if(display_mode_iterator)
        display_mode_iterator->Release();

    if(decklink_input)
        decklink_input->Release();

    return result;
}

DeckLinkPixelFormat::DeckLinkPixelFormat()
{
    fmt=bmdFormat8BitYUV;
}

QString DeckLinkPixelFormat::name()
{
    return BMDPixelFormatToString(fmt);
}
