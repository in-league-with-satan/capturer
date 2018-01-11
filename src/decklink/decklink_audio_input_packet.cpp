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

