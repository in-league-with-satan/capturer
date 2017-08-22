#ifndef DECKLINK_AUDIO_INPUT_PACKET_H
#define DECKLINK_AUDIO_INPUT_PACKET_H

#include <QSize>
#include <QByteArray>

#include "DeckLinkAPI.h"


class DeckLinkAudioInputPacket : public IDeckLinkAudioInputPacket
{
public:
    DeckLinkAudioInputPacket(int audio_channels, int audio_sample_size);

    virtual long GetSampleFrameCount(void);
    virtual HRESULT GetBytes(/* out */ void **buffer);
    virtual HRESULT GetPacketTime(/* out */ BMDTimeValue *packetTime, /* in */ BMDTimeScale timeScale);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    void append(const QByteArray &data);


protected:
    virtual ~DeckLinkAudioInputPacket(); // call Release method to drop reference count

private:
    void *buffer;
    QByteArray ba_buffer;

//    long sample_frame_count;
    int audio_channels;
    int audio_sample_size;

#ifdef _MSC_VER
    unsigned long long ref_count;
#else
    int32_t ref_count;
#endif
};



//class DeckLinkVideoFrame : public IDeckLinkVideoFrame
//{
//public:
//    DeckLinkVideoFrame();
//    virtual ~DeckLinkVideoFrame();

//    void init(QSize size, BMDPixelFormat pixel_format, BMDFrameFlags flags=bmdFrameFlagDefault);

//    static size_t frameSize(QSize size, BMDPixelFormat pixel_format);
//    static size_t rowSize(int width, BMDPixelFormat pixel_format);

//    QSize getSize();

//    QByteArray *getBuffer();

//    virtual long STDMETHODCALLTYPE GetWidth();
//    virtual long STDMETHODCALLTYPE GetHeight();
//    virtual long STDMETHODCALLTYPE GetRowBytes();
//    virtual BMDPixelFormat STDMETHODCALLTYPE GetPixelFormat();
//    virtual BMDFrameFlags STDMETHODCALLTYPE GetFlags();
//    virtual HRESULT STDMETHODCALLTYPE GetBytes(void **buffer);

//    virtual HRESULT STDMETHODCALLTYPE GetTimecode(BMDTimecodeFormat format, IDeckLinkTimecode **timecode);
//    virtual HRESULT STDMETHODCALLTYPE GetAncillaryData(IDeckLinkVideoFrameAncillary **ancillary);

//    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv);
//    virtual ULONG STDMETHODCALLTYPE AddRef(void);
//    virtual ULONG STDMETHODCALLTYPE Release(void);

//private:
//    BMDPixelFormat pixel_format;
//    BMDFrameFlags flags;

//    QSize size;

//    void *buffer;
//    QByteArray ba_buffer;
//};

#endif // DECKLINK_AUDIO_INPUT_PACKET_H
