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

#include <QDebug>

#include "decklink_tools.h"

#ifndef __linux__

bool comInit()
{
    int64_t result=CoInitialize(nullptr);

    if(FAILED(result)) {
        qCritical() << "Initialization of COM failed" << result;

        return false;
    }

    return true;
}

IDeckLinkIterator *CreateDeckLinkIteratorInstance()
{
    IDeckLinkIterator *ptr=nullptr;

    int64_t result=CoCreateInstance(CLSID_CDeckLinkIterator, nullptr, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)&ptr);

    if(FAILED(result)) {
        qCritical() << "A DeckLink iterator could not be created.  The DeckLink drivers may not be installed";
        return 0;
    }

    return ptr;
}

IDeckLinkVideoConversion *CreateVideoConversionInstance()
{
    IDeckLinkVideoConversion *ptr=nullptr;

    int64_t result=CoCreateInstance(CLSID_CDeckLinkVideoConversion, nullptr, CLSCTX_ALL, IID_IDeckLinkVideoConversion, (void**)&ptr);

    if(FAILED(result)) {
        qCritical() << "CoCreateInstance IID_IDeckLinkVideoConversion err";
        return nullptr;
    }

    return ptr;
}

#endif
