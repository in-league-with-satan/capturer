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

#ifndef DEVICE_LIST_H
#define DEVICE_LIST_H

#include <QtGlobal>
#include <QMetaType>
#include <QString>
#include <QList>

QString BMDPixelFormatToString(uint32_t format);

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
