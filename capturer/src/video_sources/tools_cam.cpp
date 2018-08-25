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

#include <QDebug>

#include <cmath>

#include "tools_video4linux2.h"
#include "tools_dshow.h"

#include "tools_cam.h"

double rnd2(const double &value)
{
    return round(value*100)/100.;
}

double rnd3(const double &value)
{
    return round(value*1000)/1000.;
}

double rnd4(const double &value)
{
    return round(value*10000)/10000.;
}


QList <Cam::Dev> ToolsCam::devList()
{
#ifdef __linux__

    return ToolsV4L2::devList();

#else

    return ToolsDirectShow::devList();

#endif
}

QList <AVRational> ToolsCam::framerateBuildSequence(const qreal &fr_min, const qreal &fr_max)
{
    QList <AVRational> result;

    AVRational framerate[]={
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
        double fr_tmp=rnd2((double)framerate[i].den/(double)framerate[i].num);

        if(rnd2(fr_min)<=fr_tmp && rnd2(fr_max)>=fr_tmp)
            result << framerate[i];
    }

    return result;
}

AVRational ToolsCam::framerateToRational(const qreal &fr)
{
    static const double eps=.003;

    if(std::abs(fr - 2.)<eps)
        return { 1, 2 };

    if(std::abs(fr - 5.)<eps)
        return { 1, 5 };

    if(std::abs(fr - 7.5)<eps)
        return { 2, 15 };

    if(std::abs(fr - 10.)<eps)
        return { 1, 10 };

    if(std::abs(fr - 15.)<eps)
        return { 1000, 15000 };

    if(std::abs(rnd3(fr) - 23.976)<eps)
        return { 1001, 24000 };

    if(std::abs(fr - 24.)<eps)
        return { 1000, 24000 };

    if(std::abs(fr - 25.)<eps)
        return { 1000, 25000 };

    if(std::abs(rnd2(fr) - 29.97)<eps)
        return { 1001, 30000 };

    if(std::abs(fr - 30.)<eps)
        return { 1000, 30000 };

    if(std::abs(fr - 50.)<eps)
        return { 1000, 50000 };

    if(std::abs(rnd4(fr) - 59.9398)<eps)
        return { 1001, 60000 };

    if(std::abs(rnd2(fr) - 59.94)<eps)
        return { 1001, 60000 };

    if(std::abs(fr - 60.)<eps)
        return { 1000, 60000 };

    if(std::abs(fr - 90.)<eps)
        return { 1000, 90000 };

    if(std::abs(fr - 120.)<eps)
        return { 1000, 120000 };

    if(std::abs(fr - 240.)<eps)
        return { 1000, 240000 };

    qWarning() << "framerateToRational unknown fr:" << fr;

    return { 1000, int(fr*1000) };
}

double ToolsCam::rationalToFramerate(const AVRational &value)
{
    return rnd2(double(value.den)/double(value.num));
}

void ToolsCam::testDevList(const QList <Cam::Dev> &list)
{
    foreach(Cam::Dev dev, list) {
        qInfo() << "dev name:" << dev.name << dev.dev << dev.format.size();

        foreach(Cam::Format fmt, dev.format) {
            qInfo() << "  pixel_format:" << Cam::PixelFormat::toString(fmt.pixel_format);

            foreach(Cam::Resolution res, fmt.resolution) {
                qInfo() << "    resolution:" << res.size;

                foreach(AVRational fr, res.framerate) {
                    qInfo() << "      framerate:" << fr.den/(double)fr.num << fr.num << fr.den;
                }
            }
        }

        qInfo() << "------------";
    }
}
