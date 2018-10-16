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

#ifndef NV_TOOLS_H
#define NV_TOOLS_H

#include <QObject>
#include <QStringList>

#include "data_types.h"

struct NvToolsPrivate;

class NvTools : public QObject
{
    Q_OBJECT

public:
    NvTools(QObject *parent=0);
    ~NvTools();

    QStringList availableDevices();

    void monitoringStart(int index);
    void monitoringStop(int index);

private slots:
    void onTimer();

private:
    NvToolsPrivate *d;

signals:
    void stateChanged(const NvState &state);
};

#endif // NV_TOOLS_H
