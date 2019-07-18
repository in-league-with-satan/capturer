/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef AUDIO_PACKET_H
#define AUDIO_PACKET_H

#include <QVariantMap>

struct AudioPacket
{
    QVariantMap toExt() {
        QVariantMap map;

        map.insert(QStringLiteral("data"), data);
        map.insert(QStringLiteral("channels"), channels);
        map.insert(QStringLiteral("sample_size"), sample_size);

        return map;
    }

    void fromExt(const QVariantMap &map) {
        data=map.value(QStringLiteral("data")).toByteArray();
        channels=map.value(QStringLiteral("channels")).toInt();
        sample_size=map.value(QStringLiteral("sample_size")).toInt();
    }

    QByteArray data;
    int channels;
    int sample_size;
};


#endif // AUDIO_PACKET_H
