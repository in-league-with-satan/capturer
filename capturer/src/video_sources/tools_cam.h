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

#ifndef TOOLS_CAM_H
#define TOOLS_CAM_H

#include <QSize>
#include <QString>
#include <QVector>

#include "ff_tools.h"
#include "pixel_format.h"

#ifdef __linux__

#include <linux/videodev2.h>

#endif

struct Cam {
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
/*
    struct PixelFormat {
        enum {
#ifdef __linux__

            YUYV=V4L2_PIX_FMT_YUYV,
            UYVY=V4L2_PIX_FMT_UYVY,
            NV12=V4L2_PIX_FMT_NV12,
            YVU420=V4L2_PIX_FMT_YVU420,
            YUV420=V4L2_PIX_FMT_YUV420,
            RGB24=V4L2_PIX_FMT_RGB24,
            RGB32=V4L2_PIX_FMT_RGB32,
            BGR24=V4L2_PIX_FMT_BGR24,
            BGR32=V4L2_PIX_FMT_BGR32,
            YUV32=V4L2_PIX_FMT_YUV32,
            MJPEG=V4L2_PIX_FMT_MJPEG,

#else

            YUYV,
            MJPEG,

#endif

            size
        };

        static QString toString(const uint64_t &pix_fmt) {
            switch(pix_fmt) {
            case YUYV:
                return QStringLiteral("YUYV");

            case UYVY:
                return QStringLiteral("UYVY");

            case NV12:
                return QStringLiteral("NV12");

            case YVU420:
                return QStringLiteral("YVU420");

            case YUV420:
                return QStringLiteral("YUV420");

            case RGB24:
                return QStringLiteral("RGB24");

            case RGB32:
                return QStringLiteral("RGB32");

            case BGR24:
                return QStringLiteral("BGR24");

            case BGR32:
                return QStringLiteral("BGR32");

            case YUV32:
                return QStringLiteral("YUV32");

            case MJPEG:
                return QStringLiteral("MJPEG");
            }

            return QStringLiteral("unknown");
        }
    };
*/
};

struct ToolsCam
{
    static QList <Cam::Dev> devList();

    static QList <AVRational> framerateBuildSequence(const qreal &fr_min, const qreal &fr_max);
    static QList <QSize> resBuildSequence(const QSize &res_min, const QSize &res_max);
    static AVRational framerateToRational(const qreal &fr);
    static double rationalToFramerate(const AVRational &value);

    static void testDevList(const QList <Cam::Dev> &list);
};

#endif // TOOLS_CAM_H
