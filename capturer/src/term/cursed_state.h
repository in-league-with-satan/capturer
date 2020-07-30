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

#ifndef CURSED_STATE_H
#define CURSED_STATE_H

#include <QMap>
#include <QTime>

#include "cursed_widget.h"
#include "data_types.h"

class CursedState : public CursedWidget
{
    Q_OBJECT

public:
   explicit CursedState(QObject *parent=0);
    ~CursedState();

    struct Dev {
        QString device;
        QString format;
        QString streams_size;
        QPair <int, int> buffer_size;
        QString bitrate;
        int dropped_frames_counter=0;
    };

    void updateState(int index, const Dev &dev);
    void clear();

    void setFreeSpace(qint64 size);
    void setRecDuration(QTime time);

    void setNvState(const NvState &state);

    void update();

private slots:

private:
    QVector <int> colsWidth(const QVector<double> &col_spreading, const QVector<int> &col_width) const;

    int index_row=0;

    QMap <int, Dev> state;

    qint64 free_space=0;
    QTime rec_duration;

    NvState nv_state;
};

#endif // CURSED_STATE_H
