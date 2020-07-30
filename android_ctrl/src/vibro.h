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

#ifndef VIBRO_H
#define VIBRO_H

#include <QObject>

#if defined(Q_OS_ANDROID)

#include <QAndroidJniEnvironment>
#include <QAndroidJniObject>

#endif

class Vibro : public QObject
{
    Q_OBJECT

public:
    explicit Vibro(QObject *parent=0);

public slots:
    void vibrate(int milliseconds);

private:
#if defined(Q_OS_ANDROID)

    QAndroidJniObject vibrator_service;

#endif

};

#endif // VIBRO_H
