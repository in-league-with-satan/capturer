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

#ifdef LIB_CURSES
#include <curses.h>
#include <panel.h>
#endif

#include "utf8_draw.h"
#include "string_tools.h"

#include "cursed_table.h"

CursedTable::CursedTable(QObject *parent)
    : CursedWidget(parent)
    , index_selection(0)
{
}

CursedTable::~CursedTable()
{
}

void CursedTable::setDimensions(const int &cols, const int &rows)
{
    QVector <double> prev_col_spreading=col_spreading;

    col_spreading.resize(cols);

    for(int i=0; i<col_spreading.size(); ++i) {
        if(prev_col_spreading.size()>i)
            col_spreading[i]=prev_col_spreading[i];

        else
            col_spreading[i]=1.;
    }

    //

    QVector <int> prev_col_width=col_width;
    QVector <int> prev_col_alignment=col_alignment;

    col_width.resize(cols);
    col_alignment.resize(cols);

    for(int i=0; i<col_width.size(); ++i) {
        if(prev_col_width.size()>i)
            col_width[i]=prev_col_width[i];

        else
            col_width[i]=-1;


        if(prev_col_alignment.size()>i)
            col_alignment[i]=prev_col_alignment[i];

        else
            col_alignment[i]=Qt::AlignRight;
    }

    //

    data.resize(rows);

    //

    table.resize(rows);

    for(int i_row=0; i_row<rows; ++i_row) {
        QVector <QString> v=table[i_row];
        v.resize(cols);

        table[i_row]=v;
    }
}

QSize CursedTable::dimensions() const
{
    return QSize(col_spreading.size(), data.size());
}

void CursedTable::setData(const int &col, const int &row, const QString &str)
{
    if(row<0 || table.size()<=row)
        return;

    QVector <QString> v=table[row];

    if(col<0 || v.size()<=col)
        return;

    v[col]=str;

    table[row]=v;
}

void CursedTable::setData(const int &row, const QVariant &data)
{
    if(row<0 || this->data.size()<=row)
        return;

    this->data[row]=data;
}

QVariant CursedTable::getData(const int &row) const
{
    if(row<0 || data.size()<=row)
        return QVariant();

    return data[row];
}

void CursedTable::clearData()
{
    for(int i_row=0; i_row<table.size(); ++i_row) {
        data[i_row]=QVariant();

        QVector <QString> v=table[i_row];

        for(int i_col=0; i_col<v.size(); ++i_col)
            v[i_col].clear();

        table[i_row]=v;
    }
}

void CursedTable::setHeader(const QStringList &header)
{
    this->header=header;
}

void CursedTable::setFooter(const QStringList &footer)
{
    this->footer=footer;
}

void CursedTable::setColSpreading(const int &index, const double &value)
{
    if(index<0 || index>=col_spreading.size())
        return;

    col_spreading[index]=value;
}

void CursedTable::setColWidth(const int &index, const int &value)
{
    if(index<0 || index>=col_width.size())
        return;

    col_width[index]=value + 1;
}

void CursedTable::setColAlignment(const int &index, Qt::AlignmentFlag alignment)
{
    if(index<0 || index>=col_alignment.size())
        return;

    if(alignment&Qt::AlignLeft)
        alignment=Qt::AlignLeft;

    else if(alignment&Qt::AlignRight)
        alignment=Qt::AlignRight;

    else if(alignment&Qt::AlignCenter || alignment&Qt::AlignVCenter || alignment&Qt::AlignHCenter)
        alignment=Qt::AlignCenter;

    else
        alignment=Qt::AlignRight;

    col_alignment[index]=alignment;
}

int CursedTable::cursorUp()
{
    index_selection--;

    if(index_selection<0)
        index_selection=0;

    update();

    return index_selection;
}

int CursedTable::cursorDown()
{
    index_selection++;

    if(index_selection>=table.size() - 1)
        index_selection=table.size() - 1;

    update();

    return index_selection;
}

int CursedTable::cursorPosition() const
{
    return index_selection;
}

int CursedTable::setCursorPosition(const int &idx)
{
    index_selection=idx;

    if(index_selection<0)
        index_selection=0;

    if(index_selection>=table.size() - 1)
        index_selection=table.size() - 1;

    update();

    return index_selection;
}

void CursedTable::setTitle(const QString &title)
{
#ifdef LIB_CURSES

    last_title=title;

    WINDOW *w=(WINDOW*)win();

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    Utf8Draw::_box(w, focus ? Utf8Draw::lt_double : Utf8Draw::lt_light);

    int max_y, max_x;

    getmaxyx(w, max_y, max_x);

    int visible_size=max_x - 2;

    if(last_title.size()>visible_size)
        last_title.resize(visible_size);

    int left=visible_size*.5 - last_title.size()*.5;

    QString str=last_title;

    str.append(QString().fill(' ', visible_size - left - str.size()));
    str.prepend(QString().fill(' ', left));

    mvwaddwstr(w, 1, 1, (wchar_t*)str.toStdWString().data());

    // горизонтальная черта, отделяет заголовок таблицы
    Utf8Draw::_mvwhline(w, 2, 1, Utf8Draw::BOX_DRAWINGS_LIGHT_HORIZONTAL, visible_size);

#endif
}

int CursedTable::maxRows() const
{
    int max_y=0, max_x=0;

#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    getmaxyx(w, max_y, max_x);

#endif

    return qMax(max_y - 6, 0);
}

void CursedTable::update()
{
#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    wclear(w);

    int max_y, max_x;

    getmaxyx(w, max_y, max_x);

    insdelln(max_y*max_y);

    setTitle(last_title);

    if(table.isEmpty())
        return;

    int col_size=table[0].size();

    if(col_size<1)
        col_size=header.size();


    int col_width=0;

    QVector <int> cols_width=colsWidth();


    // header
    if(!header.isEmpty()) {
        for(int i_col=0; i_col<qMin(header.size(), col_size); ++i_col) {
            col_width=cols_width[i_col];

            QString str=header[i_col];

            if(str.size()>col_width - 1)
                str.resize(col_width - 1);

            int cols_wdt_sum=calcSum(cols_width, i_col);

            int left=
                    col_width*.5 - str.size()*.5;

            str.append(QString().fill(' ', col_width - left - str.size()));
            str.prepend(QString().fill(' ', left));

            mvwaddwstr(w, 3, 1 + cols_wdt_sum, (wchar_t*)str.toStdWString().data());
        }
    }

    Utf8Draw::_mvwaddch(w, 2, 0, focus ? Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_RIGHT_SINGLE : Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_RIGHT);
    Utf8Draw::_mvwaddch(w, 4, 0, focus ? Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_RIGHT_SINGLE : Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_RIGHT);
    Utf8Draw::_mvwaddch(w, 2, max_x - 1, focus ? Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_LEFT_SINGLE : Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);
    Utf8Draw::_mvwaddch(w, 4, max_x - 1, focus ? Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_LEFT_SINGLE : Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);


    // таблица
    for(int i_row=0; i_row<table.size(); ++i_row) {
        QVector <QString> v_cols=table[i_row];

        for(int i_col=0; i_col<v_cols.size(); ++i_col) {
            col_width=cols_width[i_col];

            QString str=v_cols[i_col];


            const int alignment=col_alignment[i_col];

            int left=0;

            switch(alignment) {
            case Qt::AlignLeft:
                if(str.size()>col_width)
                    str=squeezeStr(str, col_width - 1);

                else
                    str.append(QString().fill(' ', col_width - str.size()));

                break;

            case Qt::AlignRight:
                if(str.size()>col_width)
                    str=squeezeStr(str, col_width - 1);

                else {
                    str.prepend(QString().fill(' ', col_width - str.size() - 1));
                    str.append(' ');
                }

                break;

            case Qt::AlignCenter:
                if(str.size()>col_width)
                    str=squeezeStr(str, col_width - 1);

                else {
                    left=
                            col_width*.5 - str.size()*.5;

                    str.append(QString().fill(' ', col_width - left - str.size()));
                    str.prepend(QString().fill(' ', left));
                }

                break;

            default:
                break;
            }


            left=1 + calcSum(cols_width, i_col);


            if(i_row==index_selection)
                wbkgdset(w, COLOR_PAIR(COLOR_WHITE_CYAN));

            else
                wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

            mvwaddwstr(w, i_row + 5, left, (wchar_t*)str.toStdWString().data());
        }
    }

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));


    // разлиновка таблицы

    Utf8Draw::_mvwhline(w, 4, 1, Utf8Draw::BOX_DRAWINGS_LIGHT_HORIZONTAL, max_x - 2); // горизонтальная черта, отделяет названия столбцов снизу

    for(int i_col=1; i_col<qMin(header.size(), col_size); ++i_col) {
        int cols_wdt_sum=calcSum(cols_width, i_col);

        if(i_col>0) {
            Utf8Draw::_mvwvline(w, 3, cols_wdt_sum, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL, max_y - 2); // вертикальные линии разделяющие колонки

            Utf8Draw::_mvwaddch(w, 2, cols_wdt_sum, Utf8Draw::BOX_DRAWINGS_LIGHT_DOWN_AND_HORIZONTAL); // пересечение вниз

            Utf8Draw::_mvwaddch(w, 4, cols_wdt_sum, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_HORIZONTAL); // пересечение +

            Utf8Draw::_mvwaddch(w, max_y - 1, cols_wdt_sum, focus ? Utf8Draw::BOX_DRAWINGS_UP_SINGLE_AND_HORIZONTAL_DOUBLE : Utf8Draw::BOX_DRAWINGS_LIGHT_UP_AND_HORIZONTAL); // пересечение вверх

            if(index_selection>=0) {
                wbkgdset(w, COLOR_PAIR(COLOR_WHITE_CYAN));

                Utf8Draw::_mvwvline(w, index_selection + 5, cols_wdt_sum, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL, 1);

                wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));
            }
        }
    }

    // footer
    if(!footer.empty()) {
        Utf8Draw::_mvwhline(w, max_y - 3, 1, Utf8Draw::BOX_DRAWINGS_LIGHT_HORIZONTAL, max_x - 2);
        Utf8Draw::_mvwaddch(w, max_y - 3, 0, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_RIGHT);
        Utf8Draw::_mvwaddch(w, max_y - 3, max_x - 1, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT);

        for(int i_col=0; i_col<footer.size(); ++i_col) {
            col_width=cols_width[i_col];

            QString str=footer[i_col];

            const int alignment=col_alignment[i_col];

            int left=0;

            switch(alignment) {
            case Qt::AlignLeft:
                if(str.size()>col_width)
                    str=squeezeStr(str, col_width - 1);

                else
                    str.append(QString().fill(' ', col_width - str.size()));

                break;

            case Qt::AlignRight:
                if(str.size()>col_width)
                    str=squeezeStr(str, col_width - 1);

                else {
                    str.prepend(QString().fill(' ', col_width - str.size() - 1));
                    str.append(' ');
                }

                break;

            case Qt::AlignCenter:
                if(str.size()>col_width)
                    str=squeezeStr(str, col_width - 1);

                else {
                    left=
                            col_width*.5 - str.size()*.5;

                    str.append(QString().fill(' ', col_width - left - str.size()));
                    str.prepend(QString().fill(' ', left));
                }

                break;

            default:
                break;
            }

            left=1 + calcSum(cols_width, i_col);

            mvwaddwstr(w, max_y - 2, left, (wchar_t*)str.toStdWString().data());

            if(i_col>0) {
                Utf8Draw::_mvwaddch(w, max_y - 3, left - 1, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_HORIZONTAL); // пересечение +
                Utf8Draw::_mvwvline(w, max_y - 2, left - 1, Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL, max_y - 2); // вертикальные линии разделяющие колонки
                Utf8Draw::_mvwaddch(w, max_y - 1, left - 1, Utf8Draw::BOX_DRAWINGS_LIGHT_UP_AND_HORIZONTAL); // пересечение вверх
            }
        }
    }

    emit updateRequest();

#endif
}

QVector <int> CursedTable::colsWidth() const
{
    QVector <int> result;

#ifdef LIB_CURSES

    result.resize(col_spreading.size());

    {
        int width_r;
        int width_c;

        WINDOW *w=(WINDOW*)win();

        int target_width=getmaxx(w) - 2;

        QVector <double> col_spreading_p=spreadPrepare(col_spreading, col_width, &width_r, &width_c);

        QVector <double> tmp=spread(col_spreading_p, target_width - width_r - width_c);

        for(int i=0; i<tmp.size(); ++i) {
            if(col_width[i]>0)
                result[i]=col_width[i];

            else
                result[i]=tmp[i];
        }

        int sum=0;

        int element_index=0;

        bool b_resize=col_width.count(-1);

        while(b_resize) {
            sum=calcSum(result);

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

QVector <double> CursedTable::spread(const QVector <double> &kf, const double &value)
{
    QVector <double> v;

    if(kf.isEmpty() || value<0.)
        return v;

    v.resize(kf.size());

    double sum=0.;

    for(int i=0; i<kf.size(); ++i)
        sum+=kf[i];

    double tmp=value/sum;

    for(int i=0; i<kf.size(); ++i)
        v[i]=kf[i]*tmp;

    return v;
}

QVector <double> CursedTable::spreadPrepare(const QVector <double> &spread, const QVector <int> &const_values, int *const_sum, int *const_count)
{
    QVector <double> v;

    (*const_sum)=0;
    (*const_count)=0;

    if(spread.size()!=const_values.size())
        return v;

    for(int i=0; i<spread.size(); ++i) {
        if(const_values[i]<1)
            v << spread[i];

        else {
            v << 0.;
            (*const_sum)+=const_values[i];
            (*const_count)++;
        }
    }

    return v;
}

