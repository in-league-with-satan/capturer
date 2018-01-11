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

QList <Cam::Dev> ToolsCam::devList()
{
#ifdef __linux__

    return ToolsV4L2::devList();

#else

    return ToolsDirectShow::devList();

#endif
}

AVRational ToolsCam::framrateToRational(const qreal &fr)
{
    static const double eps=.0003;

    static auto rnd2=[](const double &value)->double {
       return round(value*100)/100.;
    };

    static auto rnd3=[](const double &value)->double {
       return round(value*1000)/1000.;
    };

    static auto rnd4=[](const double &value)->double {
       return round(value*10000)/10000.;
    };

    if(std::abs(fr - 2.)<eps)
        return { 1, 2 };

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

    qWarning() << "framrateToRational unknown fr:" << fr;

    return { 1000, 30000 };
}

double ToolsCam::rationalToFramerate(const AVRational &value)
{
    if(value.num==1 && value.den==2)
        return 2.;

    if(value.num==1 && value.den==10)
        return 10.;

    if(value.num==1000 && value.den==15000)
        return 15.;

    if(value.num==1001 && value.den==24000)
        return 23.976;

    if(value.num==1000 && value.den==24000)
        return 24.;

    if(value.num==1000 && value.den==25000)
        return 25.;

    if(value.num==100 && value.den==2997)
        return 29.97;

    if(value.num==1001 && value.den==30000)
        return 29.97;

    if(value.num==1 && value.den==30)
        return 30.;

    if(value.num==1000 && value.den==30000)
        return 30.;

    if(value.num==1 && value.den==50)
        return 50.;

    if(value.num==1000 && value.den==50000)
        return 50.;

    if(value.num==83 && value.den==4975)
        return 59.9398;

    if(value.num==1001 && value.den==60000)
        return 59.9398;

    if(value.num==1 && value.den==60)
        return 60.;

    if(value.num==1000 && value.den==60000)
        return 60.;

    qWarning() << "rationalToFramerate unknown:" << value.num << value.den;

    return 30.;
}

void ToolsCam::testDevList(const QList <Cam::Dev> &list)
{
    foreach(Cam::Dev dev, list) {
        qInfo() << "dev name:" << dev.name << dev.dev;

        foreach(Cam::Format fmt, dev.format) {
            qInfo() << "pixel_format:" << Cam::PixelFormat::toString(fmt.pixel_format);

            foreach(Cam::Resolution res, fmt.resolution) {
                qInfo() << "resolution:" << res.size;

                foreach(AVRational fr, res.framerate) {
                    qInfo() << "framerate:" << fr.den/(double)fr.num << fr.num << fr.den;
                }
            }
        }

        qInfo() << "------------";
    }
}
