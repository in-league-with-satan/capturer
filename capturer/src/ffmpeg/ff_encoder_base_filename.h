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

#ifndef FF_ENCODER_BASE_FILENAME_H
#define FF_ENCODER_BASE_FILENAME_H

#include <QMutex>
#include <QString>

class FFEncoderBaseFilename
{
public:
    operator QString();
    QString operator=(const QString &val);

    void reset();

    void clear();
    bool isEmpty();

private:
    QMutex mutex;
    QString str;
};

#endif // FF_ENCODER_BASE_FILENAME_H
