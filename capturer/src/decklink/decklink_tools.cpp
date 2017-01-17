#include <QDebug>

#include "decklink_tools.h"

#ifndef __linux__

IDeckLinkIterator *CreateDeckLinkIteratorInstance()
{
    IDeckLinkIterator *ptr=nullptr;

    if(CoCreateInstance(CLSID_CDeckLinkIterator, nullptr, CLSCTX_ALL, IID_IDeckLinkIterator, (void**)ptr)!=S_OK) {
        qCritical() << "CoCreateInstance CLSID_CDeckLinkIterator err";
        return nullptr;
    }

    return ptr;
}

IDeckLinkVideoConversion *CreateVideoConversionInstance()
{
    IDeckLinkVideoConversion *ptr=nullptr;

    if(CoCreateInstance(CLSID_CDeckLinkVideoConversion, nullptr, CLSCTX_ALL, IID_IDeckLinkVideoConversion, (void**)&ptr)!=S_OK) {
        qCritical() << "CoCreateInstance IID_IDeckLinkVideoConversion err";
        return nullptr;
    }

    return ptr;
}

#endif
