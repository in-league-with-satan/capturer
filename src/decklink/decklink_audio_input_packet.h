#ifndef DECKLINK_AUDIO_INPUT_PACKET_H
#define DECKLINK_AUDIO_INPUT_PACKET_H

#include <QSize>
#include <QByteArray>

#include "DeckLinkAPI.h"


class DeckLinkAudioInputPacket : public IDeckLinkAudioInputPacket
{
public:
    DeckLinkAudioInputPacket(int audio_channels, int audio_sample_size);

    virtual long STDMETHODCALLTYPE GetSampleFrameCount(void);
    virtual HRESULT STDMETHODCALLTYPE GetBytes(/* out */ void **buffer);
    virtual HRESULT STDMETHODCALLTYPE GetPacketTime(/* out */ BMDTimeValue *packet_time, /* in */ BMDTimeScale time_scale);

    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppv);
    virtual ULONG STDMETHODCALLTYPE AddRef(void);
    virtual ULONG STDMETHODCALLTYPE Release(void);

    void append(const QByteArray &data);

    //

    void *buffer;
    QByteArray ba_buffer;

    int audio_channels;
    int audio_sample_size;

protected:
    virtual ~DeckLinkAudioInputPacket(); // call Release method to drop reference count

private:
#ifdef _MSC_VER
    unsigned long long ref_count;
#else
    int32_t ref_count;
#endif
};

#endif // DECKLINK_AUDIO_INPUT_PACKET_H
