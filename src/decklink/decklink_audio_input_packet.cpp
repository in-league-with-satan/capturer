#ifdef _MSC_VER
#include <windows.h>
#endif

#include "decklink_audio_input_packet.h"

DeckLinkAudioInputPacket::DeckLinkAudioInputPacket(int audio_channels, int audio_sample_size)
    : buffer(nullptr)
    , ref_count(0)
    , audio_channels(audio_channels)
    , audio_sample_size(audio_sample_size)
{
}

DeckLinkAudioInputPacket::~DeckLinkAudioInputPacket()
{
}

long DeckLinkAudioInputPacket::GetSampleFrameCount()
{
    return ba_buffer.size()/(audio_channels*(audio_sample_size/8));
}

HRESULT DeckLinkAudioInputPacket::GetBytes(void **buffer)
{
    if(!this->buffer || ba_buffer.isEmpty())
        return E_FAIL;

    *buffer=this->buffer;

    return S_OK;
}

HRESULT DeckLinkAudioInputPacket::GetPacketTime(BMDTimeValue *packet_time, BMDTimeScale time_scale)
{
    Q_UNUSED(time_scale)

    *packet_time=0;

    return S_OK;
}

HRESULT DeckLinkAudioInputPacket::QueryInterface(REFIID iid, LPVOID *ppv)
{
    Q_UNUSED(iid)
    Q_UNUSED(ppv)

    return E_NOINTERFACE;
}

ULONG DeckLinkAudioInputPacket::AddRef()
{
#ifdef _MSC_VER
    return InterlockedExchangeAdd(&ref_count, 1);
#else
    return __sync_add_and_fetch(&ref_count, 1);
#endif
}

ULONG DeckLinkAudioInputPacket::Release()
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

void DeckLinkAudioInputPacket::append(const QByteArray &data)
{
    ba_buffer.append(data);

    buffer=(void*)ba_buffer.constData();
}

