#ifndef DECKLINK_VIDEO_FRAME_H
#define DECKLINK_VIDEO_FRAME_H

#include <QSize>
#include <QByteArray>

#include "DeckLinkAPI.h"

class DeckLinkVideoFrame : public IDeckLinkVideoFrame
{
public:
    DeckLinkVideoFrame();
    virtual ~DeckLinkVideoFrame();

    void init(QSize size, BMDPixelFormat pixel_format, BMDFrameFlags flags=bmdFrameFlagDefault);

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

private:
    BMDPixelFormat pixel_format;
    BMDFrameFlags flags;

    QSize size;

    void *buffer;
    QByteArray ba_buffer;
};

#endif // DECKLINK_VIDEO_FRAME_H
