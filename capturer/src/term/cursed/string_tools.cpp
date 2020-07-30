/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QStringList>

#include "string_tools.h"

QString squeezeStr(QString str, int size)
{
    if(str.size()>size) {
        str.resize(size - 1);
        str.append("…");
    }

    return str;
}

QString horizontalAlignment(QString str, const int &width, const Qt::Alignment &alignment)
{
    if(str.size()>width)
        return squeezeStr(str, width);

    if(alignment & Qt::AlignLeft) {
        str.append(QString().fill(' ', width - str.size()));

    } else if(alignment & Qt::AlignCenter || alignment & Qt::AlignHCenter) {
        int left=width*.5 - str.size()*.5;

        str.append(QString().fill(' ', width - left - str.size()));
        str.prepend(QString().fill(' ', left));

    } else if(alignment & Qt::AlignRight) {
        str.prepend(QString().fill(' ', width - str.size()));
    }

    return str;
}

QStringList wrap(QString str, const int &width)
{
    QStringList result;

    while(true) {
        int i=0;

        while(i<str.length()) {
            if(str.left(++i + 1).size()>width) {
                int j=str.lastIndexOf(' ', i);
                int jj=str.mid(i).lastIndexOf('\n');

                if(j>0 && jj>0)
                    j=qMin(j, jj + i);

                if(j>0)
                    i=j;

                result << str.left(i);

                str=str.mid(i + 1);
                break;
            }
        }

        if(i>=str.length())
            break;
    }

    if(str.contains('\n'))
        result << str.split('\n');

    else
        result << str;

    return result;
}
