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

#ifndef CURSED_LABEL_H
#define CURSED_LABEL_H

#include "cursed_widget.h"

class CursedLabel : public CursedWidget
{
    Q_OBJECT

public:
    explicit CursedLabel(QObject *parent=0);
    ~CursedLabel();

    void setText(const QString &text);
    void setWrapText(const bool &value);
    void setAlignment(const Qt::Alignment &alignment);

public slots:
    void update();

private:
    QString text;
    bool f_wrap;
    Qt::Alignment alignment;
};

#endif // CURSED_LABEL_H
