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
