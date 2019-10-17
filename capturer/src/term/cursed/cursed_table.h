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

#ifndef CURSED_TABLE_H
#define CURSED_TABLE_H

#include <QVector>

#include "cursed_widget.h"

class CursedTable : public CursedWidget
{
    Q_OBJECT

public:
   explicit CursedTable(QObject *parent=0);

    ~CursedTable();

    void setDimensions(const int &cols, const int &rows);
    QSize dimensions() const;

    void setData(const int &col, const int &row, const QString &str);
    void setData(const int &row, const QVariant &data);
    QVariant getData(const int &row) const;
    void clearData();
    void setHeader(const QStringList &header);
    void setFooter(const QStringList &footer);
    void setColSpreading(const int &index, const double &value);
    void setColWidth(const int &index, const int &value);
    void setColAlignment(const int &index, Qt::AlignmentFlag alignment);

    int cursorUp();
    int cursorDown();

    int cursorPosition() const;
    int setCursorPosition(const int &idx);

    void setTitle(const QString &title);

    int maxRows() const;

    void update();

public:
    static QVector <double> spread(const QVector <double> &kf, const double &value);
    static QVector <double> spreadPrepare(const QVector <double> &spread, const QVector <int> &const_values, int *const_sum, int *const_count);

    template <typename T>
    static T calcSum(const QVector <T> &values, int stop_index=-1) {
        T sum=0;

        T *ptr=(T*)values.data();

        stop_index=qMin(values.size(), stop_index);

        if(stop_index<0)
            stop_index=values.size();

        for(int i=0; i<stop_index; ++i)
            sum+=ptr[i];

        return sum;
    }

private:
    QVector <int> colsWidth() const;

    int index_selection;

    QVector <QVector <QString>> table;
    QStringList header;
    QVector <QVariant> data;

    QVector <double> col_spreading;
    QVector <int> col_width;
    QVector <int> col_alignment;

    QStringList footer;

    QString last_title;
};

#endif // CURSED_TABLE_H
