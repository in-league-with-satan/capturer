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

#ifndef DECKLINK_VIDEO_FRAME_H
#define DECKLINK_VIDEO_FRAME_H

#include <QSize>
#include <QByteArray>

#include "decklink_global.h"

#ifdef LIB_DECKLINK
class DeckLinkVideoFrame : public IDeckLinkVideoFrame {
#else
class DeckLinkVideoFrame {
#endif
public:
    DeckLinkVideoFrame();
    virtual ~DeckLinkVideoFrame();

    void init(QSize size, BMDPixelFormat pixel_format, BMDFrameFlags flags=bmdFrameFlagDefault, bool alloc_buffer=true);

    static size_t frameSize(QSize size, BMDPixelFormat pixel_format);
    static size_t rowSize(int width, BMDPixelFormat pixel_format);

    QSize getSize();

    QByteArray *getBuffer();

    virtual long STDMETHODCALLTYPE GetWidth();
    virtual long STDMETHODCALLTYPE GetHeight();
    virtual long STDMETHODCALLTYPE GetRowBytes();
    virtual BMDPixelFormat STDMETHODCALLTYPE GetPixelFormat();
    virtual BMDFrameFlags STDMETHODCALLTYPE GetFlags();
    virtual HRESULT STDMETHODCALLTYPE GetBytes(void **buffer);

    virtual HRESULT STDMETHODCALLTYPE GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode);
    virtual HRESULT STDMETHODCALLTYPE GetAncillaryData(IDeckLinkVideoFrameAncillary **ancillary);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    void *buffer;
    size_t buffer_size;
    QSize size;

private:
    BMDPixelFormat pixel_format;
    BMDFrameFlags flags;

    QByteArray ba_buffer;

#ifdef _MSC_VER
    unsigned long long ref_count;
#else
    int32_t ref_count;
#endif
};

#endif // DECKLINK_VIDEO_FRAME_H
