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

#include "framerate.h"


double Framerate::rnd2(const double &value)
{
    return round(value*100)/100.;
}

double Framerate::rnd3(const double &value)
{
    return round(value*1000)/1000.;
}

double Framerate::rnd4(const double &value)
{
    return round(value*10000)/10000.;
}

AVRational Framerate::toRational(const double &fr)
{
    static const double eps=.004;

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

    if(std::abs(fr - 20.)<eps)
        return { 1000, 20000 };

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

    qWarning() << "unknown:" << fr;

    return { 1000, int(fr*1000) };
}

double Framerate::fromRational(const AVRational &value)
{
    return rnd2(double(value.den)/double(value.num));
}

