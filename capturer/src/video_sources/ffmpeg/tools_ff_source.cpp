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

#include <QDebug>

#include <cmath>

#include "tools_video4linux2.h"
#include "tools_dshow.h"
#include "framerate.h"

#include "tools_ff_source.h"


QList <FFDevice::Dev> ToolsFFSource::devList()
{
#ifdef __linux__

    return ToolsV4L2::devList();

#else

    return ToolsDirectShow::devList();

#endif
}

QList <AVRational> ToolsFFSource::framerateBuildSequence(const qreal &fr_min, const qreal &fr_max)
{
    QList <AVRational> result;

    static AVRational framerate[]={
        { 1, 2 },           // 2
        { 1, 5 },           // 5
        { 2, 15 },          // 7.5
        { 1, 10 },          // 10
        { 1000, 15000 },    // 15
        { 1001, 24000 },    // 23.97
        { 1000, 24000 },    // 24
        { 1000, 25000 },    // 25
        { 1001, 30000 },    // 29.97
        { 1000, 30000 },    // 30
        { 1000, 50000 },    // 50
        { 1001, 60000 },    // 59.94
        { 1000, 60000 },    // 60
        { 1000, 90000 },    // 90
        { 1000, 120000 },   // 120
        { 1000, 240000 }    // 240
    };

    for(size_t i=0; i<sizeof(framerate)/sizeof(*framerate); ++i) {
        double fr_tmp=Framerate::rnd2((double)framerate[i].den/(double)framerate[i].num);

        if(Framerate::rnd2(fr_min)<=fr_tmp && Framerate::rnd2(fr_max)>=fr_tmp)
            result << framerate[i];
    }

    return result;
}

QList <QSize> ToolsFFSource::resBuildSequence(const QSize &res_min, const QSize &res_max)
{
    QList <QSize> result;

    static QList <QSize> res={
        QSize(640, 480),
        QSize(800, 600),
        QSize(1024, 768),
        QSize(1280, 720),
        QSize(1280, 800),
        QSize(1600, 900),
        QSize(1600, 1200),
        QSize(1920, 1080),
        QSize(1920, 1200),
        QSize(2560, 1440),
        QSize(2560, 1600),
        QSize(3840, 2160),
        QSize(4096, 2160)
    };

    foreach(const QSize &r, res) {
        if(res_min.width()<=r.width() && res_min.height()<=r.height()
                && res_max.width()>=r.width() && res_max.height()>=r.height())
            result << r;
    }

    return result;
}

void ToolsFFSource::testDevList(const QList <FFDevice::Dev> &list)
{
    return;

    foreach(FFDevice::Dev dev, list) {
        qInfo() << "dev name:" << dev.name << dev.dev << dev.format.size();

        foreach(FFDevice::Format fmt, dev.format) {
            qInfo() << "  pixel_format:" << fmt.pixel_format.toString();

            foreach(FFDevice::Resolution res, fmt.resolution) {
                qInfo() << "    resolution:" << res.size;

                foreach(AVRational fr, res.framerate) {
                    qInfo() << "      framerate:" << fr.den/(double)fr.num << fr.num << fr.den;
                }
            }
        }

        qInfo() << "------------";
    }
}
