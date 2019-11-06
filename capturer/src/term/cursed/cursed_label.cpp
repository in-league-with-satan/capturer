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
#include <curses.h>
#include <panel.h>
#endif

#include "utf8_draw.h"
#include "string_tools.h"

#include "cursed_label.h"

CursedLabel::CursedLabel(QObject *parent)
    : CursedWidget(parent)
    , f_wrap(false)
    , alignment(Qt::AlignLeft)
{
}

CursedLabel::~CursedLabel()
{
}

void CursedLabel::setText(const QString &text)
{
    this->text=text;
}

void CursedLabel::setWrapText(const bool &value)
{
    f_wrap=value;
}

void CursedLabel::setAlignment(const Qt::Alignment &alignment)
{
    this->alignment=alignment;
}

void CursedLabel::update()
{
#ifdef LIB_CURSES

    if(!is_visible)
        return;


    WINDOW *w=(WINDOW*)win();

    wclear(w);

    int max_y, max_x;

    getmaxyx(w, max_y, max_x);

    QStringList l;

    if(f_wrap)
        l=wrap(text, max_x - 2);

    else
        l << text;


    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    // Utf8Draw::_mvwhline(w, 0, 0, QString(" ").toStdWString(), max_x);
    // Utf8Draw::_mvwhline(w, max_y - 1, 0, QString(" ").toStdWString(), max_x);

    // Utf8Draw::_mvwvline(w, 0, 0, QString(" ").toStdWString(), max_y);
    // Utf8Draw::_mvwvline(w, 0, max_x - 1, QString(" ").toStdWString(), max_y);

    // max_y-=2;

    for(int i=0; i<max_y; ++i) {
        QString str;

        if(i<l.size())
            str=l[i];

        str=horizontalAlignment(str, max_x, alignment);

        mvwaddwstr(w, i, 0, (wchar_t*)str.toStdWString().data());
    }

    emit updateRequest();

#endif
}

