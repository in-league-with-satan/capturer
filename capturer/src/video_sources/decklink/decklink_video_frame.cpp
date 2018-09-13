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

#ifdef _MSC_VER
#include <windows.h>
#endif

#include "decklink_video_frame.h"

DeckLinkVideoFrame::DeckLinkVideoFrame()
    : buffer(nullptr)
    , buffer_size(0)
    , pixel_format(bmdFormat8BitBGRA)
    , flags(bmdFrameFlagDefault)
    , ref_count(0)
{
}

DeckLinkVideoFrame::~DeckLinkVideoFrame()
{
}

size_t DeckLinkVideoFrame::frameSize(QSize size, BMDPixelFormat pixel_format)
{
    return rowSize(size.width(), pixel_format)*size.height();
}

size_t DeckLinkVideoFrame::rowSize(int width, BMDPixelFormat pixel_format)
{
#ifdef LIB_DECKLINK

    switch(pixel_format) {
    case bmdFormat8BitYUV:
        return width*16/8;

    case bmdFormat10BitYUV:
        return ((width + 47)/48)*128;

    case bmdFormat8BitARGB:
    case bmdFormat8BitBGRA:
        return width*32/8;

    case bmdFormat10BitRGB:
        return ((width + 63)/64)*256;

    case bmdFormat12BitRGB:
    case bmdFormat12BitRGBLE:
        return (width*36)/8;

    case bmdFormat10BitRGBXLE:
    case bmdFormat10BitRGBX:
        return ((width + 63)/64)*256;

    default:
        break;
    }

#endif

    return 0;
}

QSize DeckLinkVideoFrame::getSize()
{
    return size;
}

QByteArray *DeckLinkVideoFrame::getBuffer()
{
    return &ba_buffer;
}

void DeckLinkVideoFrame::init(QSize size, BMDPixelFormat pixel_format, BMDFrameFlags flags, bool alloc_buffer)
{
    this->size=size;
    this->pixel_format=pixel_format;
    this->flags=flags;

    buffer_size=frameSize(size, pixel_format);
    buffer=nullptr;

    if(alloc_buffer) {
        ba_buffer.resize(buffer_size);

        buffer=(void*)ba_buffer.constData();
    }
}

long DeckLinkVideoFrame::GetWidth()
{
    return size.width();
}

long DeckLinkVideoFrame::GetHeight()
{
    return size.height();
}

long DeckLinkVideoFrame::GetRowBytes()
{
    return rowSize(size.width(), pixel_format);
}

BMDPixelFormat DeckLinkVideoFrame::GetPixelFormat()
{
    return pixel_format;
}

BMDFrameFlags DeckLinkVideoFrame::GetFlags()
{
    return flags;
}

HRESULT DeckLinkVideoFrame::GetBytes(void **buffer)
{
    if(!this->buffer)
        return E_FAIL;

    *buffer=this->buffer;

    return S_OK;
}

HRESULT DeckLinkVideoFrame::GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode)
{
    Q_UNUSED(format)
    Q_UNUSED(timecode)

    return E_NOINTERFACE;
}

HRESULT DeckLinkVideoFrame::GetAncillaryData(IDeckLinkVideoFrameAncillary **ancillary)
{
    Q_UNUSED(ancillary)

    return E_NOINTERFACE;
}

HRESULT DeckLinkVideoFrame::QueryInterface(REFIID iid, LPVOID *ppv)
{
    Q_UNUSED(iid)
    Q_UNUSED(ppv)

    return E_NOINTERFACE;
}

ULONG DeckLinkVideoFrame::AddRef()
{
#ifdef _MSC_VER
    return InterlockedExchangeAdd(&ref_count, 1);
#else
    return __sync_add_and_fetch(&ref_count, 1);
#endif
}

ULONG DeckLinkVideoFrame::Release()
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
