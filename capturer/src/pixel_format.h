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

#ifndef PIXEL_FORMAT_H
#define PIXEL_FORMAT_H

#include <QString>
#include <QList>
#include <QVideoFrame>

#ifdef __WIN32__

#include <windows.h>
#include <dshow.h>

#endif

#include <inttypes.h>

#include "ff_tools.h"

struct PixelFormat {
    enum T {
        undefined,
        rgb24,
        bgr24,
        rgb0,
        bgr0,
        bgra,
        gbrp10le,
        rgb48le,
        yuv420p,
        yuv420p10,
        yuv422p,
        yuyv422,
        uyvy422,
        yuv444p,
        yuv422p10le,
        yuv444p10,
        yuv444p16le,
        p010le,
        nv12,
        mjpeg,

        size
    };

    PixelFormat();
    PixelFormat(const int &value);
    PixelFormat(const AVPixelFormat &value);

    static QList <PixelFormat> list();

    AVPixelFormat toAVPixelFormat() const;
    bool fromAVPixelFormat(AVPixelFormat value);

    uint32_t toV4L2PixelFormat() const;
    bool fromV4L2PixelFormat(uint32_t value);

    uint32_t toBMDPixelFormat() const;
    bool fromBMDPixelFormat(uint32_t value);

    QVideoFrame::PixelFormat toQPixelFormat() const;
    bool fromQPixelFormat(QVideoFrame::PixelFormat value);

#ifdef __WIN32__

    GUID toDshowPixelFormat() const;
    bool fromDshowPixelFormat(const GUID &value);

#endif

    static QString toString(int value);
    static QString toStringView(int value);
    QString toString() const;
    QString toStringView() const;
    bool fromString(const QString &value);

    bool isValid() const;
    static bool isValid(int value);

    bool isRgb() const;
    bool is10bit() const;
    bool is210() const;

    bool isDirect() const;

    operator int() const;
    PixelFormat &operator=(int other);

private:
    T d=undefined;
};

int frameBufSize(QSize size, PixelFormat pixel_format);

#endif // PIXEL_FORMAT_H