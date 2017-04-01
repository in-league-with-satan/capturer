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

    virtual long GetWidth();
    virtual long GetHeight();
    virtual long GetRowBytes();
    virtual BMDPixelFormat GetPixelFormat();
    virtual BMDFrameFlags GetFlags();
    virtual HRESULT GetBytes(void **buffer);

    virtual HRESULT GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode);
    virtual HRESULT GetAncillaryData(IDeckLinkVideoFrameAncillary **ancillary);

    virtual HRESULT QueryInterface(REFIID iid, LPVOID *ppv);
    virtual ULONG AddRef(void);
    virtual ULONG Release(void);

private:
    BMDPixelFormat pixel_format;
    BMDFrameFlags flags;

    QSize size;

    void *buffer;
    QByteArray ba_buffer;
};

#endif // DECKLINK_VIDEO_FRAME_H
