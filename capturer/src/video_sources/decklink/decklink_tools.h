/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef DECKLINK_TOOLS_H
#define DECKLINK_TOOLS_H

#include "decklink_global.h"

#ifdef LIB_DECKLINK

#ifndef __linux__

bool comInit();

IDeckLinkIterator *CreateDeckLinkIteratorInstance();

IDeckLinkVideoConversion *CreateVideoConversionInstance();

#endif

#endif // LIB_DECKLINK

#endif // DECKLINK_TOOLS_H
