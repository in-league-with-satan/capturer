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

#ifndef TOOLS_FF_SOURCE_H
#define TOOLS_FF_SOURCE_H

#include <QSize>
#include <QString>
#include <QVector>

#include "ff_tools.h"
#include "pixel_format.h"

#ifdef __linux__

#include <linux/videodev2.h>

#endif

struct FFDevice {
    struct Resolution {
        QSize size;
        QList <AVRational> framerate;
    };

    struct Format {
        PixelFormat pixel_format;
        QList <Resolution> resolution;
    };

    struct Dev {
        QString name;
        QString dev;
        QList <Format> format;
    };
};

struct ToolsFFSource
{
    static QList <FFDevice::Dev> devList();

    static QList <AVRational> framerateBuildSequence(const qreal &fr_min, const qreal &fr_max);
    static QList <QSize> resBuildSequence(const QSize &res_min, const QSize &res_max);
    static void testDevList(const QList <FFDevice::Dev> &list);
};

#endif // TOOLS_FF_SOURCE_H
