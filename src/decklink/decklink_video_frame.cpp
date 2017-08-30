#include "decklink_video_frame.h"

DeckLinkVideoFrame::DeckLinkVideoFrame()
{
    buffer=nullptr;
    buffer_size=0;
    pixel_format=bmdFormat8BitBGRA;
    flags=bmdFrameFlagDefault;
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
    return 1;
}

ULONG DeckLinkVideoFrame::Release()
{
    return 1;
}
