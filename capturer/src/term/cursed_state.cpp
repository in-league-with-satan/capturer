/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifdef LIB_CURSES
#  ifdef __linux__
#    include <ncursesw/curses.h>
#    include <ncursesw/panel.h>
#  else
#    include "curses.h"
#    include "panel.h"
#  endif
#endif


#include "utf8_draw.h"
#include "string_tools.h"

#include "cursed_table.h"

#include "cursed_state.h"


CursedState::CursedState(QObject *parent)
    : CursedWidget(parent)
    , index_row(0)
{
}

CursedState::~CursedState()
{
}

void CursedState::updateState(int index, const CursedState::Dev &dev)
{
    state[index]=dev;
}

void CursedState::clear()
{
    state.clear();
}

void CursedState::setFreeSpace(qint64 size)
{
    free_space=size;
}

void CursedState::setRecDuration(QTime time)
{
    rec_duration=time;
}

void CursedState::setNvState(const NvState &state)
{
    nv_state=state;
}

QVector <int> CursedState::colsWidth(const QVector <double> &col_spreading, const QVector <int> &col_width) const
{
    QVector <int> result;

#ifdef LIB_CURSES

    result.resize(col_spreading.size());

    {
        int width_r;
        int width_c;

        WINDOW *w=(WINDOW*)win();

        int target_width=getmaxx(w);

        QVector <double> col_spreading_p=CursedTable::spreadPrepare(col_spreading, col_width, &width_r, &width_c);

        QVector <double> tmp=CursedTable::spread(col_spreading_p, target_width - width_r - width_c);

        for(int i=0; i<std::min(result.size(), tmp.size()); ++i) {
                result[i]=tmp[i];
        }

        int sum=0;

        int element_index=0;

        bool b_resize=col_width.count(-1);

        while(b_resize) {
            sum=CursedTable::calcSum(result);

            if(sum==target_width)
                break;


            if(sum<target_width)
                result[element_index]++;

            else
                result[element_index]--;


            while(true) {
                element_index++;

                if(element_index>=result.size())
                    element_index=0;

                if(col_width[element_index]<0)
                    break;
            }
        }
    }

#endif

    return result;
}

void CursedState::update()
{
#ifdef LIB_CURSES

    if(!is_visible)
        return;

    WINDOW *w=(WINDOW*)win();

#  ifndef __linux__

    wclear(w);

#  endif

    QStringList val;
    QVector <double> col_spreading;

    val
            << "src" << "format" << "buffer state" << "bitrate" << "size" << "frames dropped";

    col_spreading
            << 1.    << 1.       << .6             << 1.        << 1.     << .7;


    QVector <int> col_width_v;
    col_width_v.fill(-1, val.size());

    QVector <int> cols_width=colsWidth(col_spreading, col_width_v);

    auto fillRow=[=](QStringList val, int row) {
        int col_width=0;

        for(int i_col=0; i_col<val.size(); ++i_col) {
            col_width=cols_width[i_col];

            QString str=val[i_col];

            if(str.size()>col_width - 1)
                str.resize(col_width - 1);

            int cols_wdt_sum=CursedTable::calcSum(cols_width, i_col);

            int left=
                    col_width*.5 - str.size()*.5;

            str.append(QString().fill(' ', col_width - left - str.size()));
            str.prepend(QString().fill(' ', left));

            mvwaddwstr(w, row, 1 + cols_wdt_sum, (wchar_t*)str.toStdWString().data());
        }
    };


    int row=0;

    //

    fillRow(QStringList() << "free space:" << QString("%1 MB").arg(QLocale().toString(free_space/1024/1024)), row++);
    fillRow(QStringList() << "rec duration:" << rec_duration.toString("hh:mm:ss"), row++);

    row++;

    //

    if(!nv_state.dev_name.isEmpty()) {
        row++;

        val=QStringList()
                << "dev" << "temp" << "gpu" << "mcu" << "vpu";

        fillRow(val, row++);

        val=QStringList()
                << nv_state.dev_name
                << QString("%1°C").arg(nv_state.temperature)
                << QString("%1%").arg(nv_state.graphic_processing_unit)
                << QString("%1%").arg(nv_state.memory_controller_unit)
                << QString("%1%").arg(nv_state.video_processing_unit);

        fillRow(val, row++);

        row++;
        row++;
    }

    //

    val=QStringList()
            << "src" << "format" << "buffer state" << "bitrate" << "size" << "frames dropped";

    fillRow(val, row++);

    val=QStringList()
            << "" << "" << "(used/total)" << "(Mbits/s / MB/s)" << "(MB)" << "";

    fillRow(val, row++);

    for(int i=0; i<state.size(); ++i) {
        int key=state.keys()[i];

        if(key==-1)
            continue;

        Dev dev=state.value(key);

        val=QStringList()
                << dev.device << dev.format << QString("%1/%2").arg(dev.buffer_size.first).arg(dev.buffer_size.second)
                << dev.bitrate
                << dev.streams_size
                << QString::number(dev.dropped_frames_counter);

        fillRow(val, row++);
    }

    if(state.contains(-1)) {
        Dev dev=state.value(-1);

        val=QStringList()
                << dev.device << dev.format << QString("%1/%2").arg(dev.buffer_size.first).arg(dev.buffer_size.second)
                << dev.bitrate
                << dev.streams_size
                << QString::number(dev.dropped_frames_counter);

         fillRow(val, row++);
    }

    emit updateRequest();

#endif
}

