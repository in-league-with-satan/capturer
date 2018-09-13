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

#ifndef DECKLINK_GLOBAL_H
#define DECKLINK_GLOBAL_H

#include <inttypes.h>

#ifdef LIB_DECKLINK

#include "DeckLinkAPI.h"

#else

class IDeckLink;
class IDeckLinkDisplayModeIterator;
class IDeckLinkDisplayMode;
class IDeckLinkInput;
class IDeckLinkOutput;
class IDeckLinkIterator;
class IDeckLinkVideoInputFrame;
class IDeckLinkAudioInputPacket;
class IDeckLinkAttributes;
class IDeckLinkVideoConversion;
class IDeckLinkMutableVideoFrame;
class IDeckLinkConfiguration;

typedef int BMDPixelFormat;
typedef int BMDFrameFlags;
typedef int BMDDetectedVideoInputFormatFlags;
typedef int BMDTimecodeFormat;
typedef int BMDVideoInputFormatChangedEvents;
typedef int IDeckLinkTimecode;
typedef int IDeckLinkVideoFrameAncillary;
typedef int BMDTimeValue;
typedef int BMDTimeScale;
typedef int REFIID;
typedef int LPVOID;
typedef int HRESULT;
typedef unsigned long ULONG;

#define STDMETHODCALLTYPE
#define bmdFrameFlagDefault 0
#define bmdFormat10BitRGB 0
#define bmdFormat10BitYUV 0
#define bmdFormat8BitBGRA 0
#define bmdFormat8BitYUV 0

#define E_NOINTERFACE 2
#define E_FAIL 1
#define E_OK 0
#define S_OK 0

#endif

#endif // DECKLINK_GLOBAL_H
