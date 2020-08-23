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

#ifndef DECKLINK_DEVICE_LIST_H
#define DECKLINK_DEVICE_LIST_H

#include <QtGlobal>
#include <QMetaType>
#include <QString>
#include <QList>

namespace Decklink {
QString BMDPixelFormatToString(uint32_t format);

struct PixelFormat {
    PixelFormat();

    PixelFormat(int format) {
        fmt=format;
    }

    QString name();
    int fmt;
};

typedef QList <PixelFormat> PixelFormats;

struct Format {
    QString display_mode_name;

    int index;

    int width;
    int height;

    double framerate;
    int64_t framerate_scale;
    int64_t framerate_duration;

    PixelFormats pixel_formats;
};

typedef QList <Format> Formats;

struct Device {
    int index;
    QString name;
    Formats formats;
};

typedef QList <Device> Devices;

Devices getDevices();

}

Q_DECLARE_METATYPE(Decklink::Device)
Q_DECLARE_METATYPE(Decklink::Format)
Q_DECLARE_METATYPE(Decklink::Formats)
Q_DECLARE_METATYPE(Decklink::PixelFormat)
Q_DECLARE_METATYPE(Decklink::PixelFormats)

#endif // DECKLINK_DEVICE_LIST_H
