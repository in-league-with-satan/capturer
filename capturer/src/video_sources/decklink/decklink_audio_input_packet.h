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

#ifndef DECKLINK_AUDIO_INPUT_PACKET_H
#define DECKLINK_AUDIO_INPUT_PACKET_H

#include <QSize>
#include <QByteArray>

#include "decklink_global.h"


#ifdef LIB_DECKLINK
class DeckLinkAudioInputPacket : public IDeckLinkAudioInputPacket {
#else
class DeckLinkAudioInputPacket {
#endif
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
