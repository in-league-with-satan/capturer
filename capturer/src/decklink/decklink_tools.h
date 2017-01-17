#ifndef DECKLINK_TOOLS_H
#define DECKLINK_TOOLS_H

#include "DeckLinkAPI.h"

#ifndef __linux__

IDeckLinkIterator *CreateDeckLinkIteratorInstance();

IDeckLinkVideoConversion *CreateVideoConversionInstance();

#endif

#endif // DECKLINK_TOOLS_H
