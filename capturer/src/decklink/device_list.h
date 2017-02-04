#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <QtGlobal>
#include <QMetaType>

struct DeckLinkPixelFormat {
    DeckLinkPixelFormat();

    DeckLinkPixelFormat(int format) {
        fmt=format;
    }

    QString name();
    int fmt;
};

typedef QList <DeckLinkPixelFormat> DeckLinkPixelFormats;

struct DeckLinkFormat {
    QString display_mode_name;

    int index;

    int width;
    int height;

    double framerate;
    int64_t framerate_scale;
    int64_t framerate_duration;

    DeckLinkPixelFormats pixel_formats;
};

typedef QList <DeckLinkFormat> DeckLinkFormats;

struct DeckLinkDevice {
    int index;
    QString name;
    DeckLinkFormats formats;
};


typedef QList <DeckLinkDevice> DeckLinkDevices;

int GetDevices(DeckLinkDevices *devices);

Q_DECLARE_METATYPE(DeckLinkDevice)
Q_DECLARE_METATYPE(DeckLinkFormat)
Q_DECLARE_METATYPE(DeckLinkPixelFormat)
Q_DECLARE_METATYPE(DeckLinkFormats)
Q_DECLARE_METATYPE(DeckLinkPixelFormats)

#endif // DEVICE_LIST_H
