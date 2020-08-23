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


#include "cursed_widget.h"


struct CursedWidgetContext
{
#ifdef LIB_CURSES

    WINDOW *w;
    PANEL *p;

#endif
};

CursedWidget::CursedWidget(QObject *parent)
    : QObject(parent)
    , d(new CursedWidgetContext())
    , is_visible(true)
    , focus(false)
{
#ifdef LIB_CURSES

    d->w=newwin(LINES, COLS, 0, 0);

    if(d->w) {
        d->p=new_panel(d->w);

        if(!d->p)
            delwin(d->w);
    }

#endif
}

CursedWidget::~CursedWidget()
{
#ifdef LIB_CURSES

    if(d->p)
        del_panel(d->p);

    if(d->w)
        delwin(d->w);

#endif

    delete d;
}

void CursedWidget::setSize(const int &cols, const int &rows)
{
#ifdef LIB_CURSES
#  ifdef __linux__

    wresize(d->w, rows, cols);

#  else

    resize_window(d->w, rows, cols);

#  endif
#endif
}

void CursedWidget::setPos(const int &x, const int &y)
{
#ifdef LIB_CURSES

    mvwin(d->w, y, x);

#endif
}

QSize CursedWidget::size() const
{
    int max_y=0, max_x=0;

#ifdef LIB_CURSES

    getmaxyx(d->w, max_y, max_x);

#endif

    return QSize(max_x, max_y);
}

QPoint CursedWidget::pos() const
{
    int y=0, x=0;

#ifdef LIB_CURSES

    getyx(d->w, y, x);

#endif

    return QPoint(x, y);
}

QSize CursedWidget::minSize() const
{
    return QSize(0, 0);
}

bool CursedWidget::isVisible() const
{
    return is_visible;
}

void *CursedWidget::win() const
{
#ifdef LIB_CURSES

    return d->w;

#else

    return nullptr;

#endif
}

void *CursedWidget::pan() const
{
#ifdef LIB_CURSES

    return d->p;

#else

    return nullptr;

#endif
}

void CursedWidget::show()
{
#ifdef LIB_CURSES

    show_panel(d->p);

    is_visible=true;

#endif
}

void CursedWidget::hide()
{
#ifdef LIB_CURSES

    hide_panel(d->p);

    is_visible=false;

#endif
}

void CursedWidget::setFocus(const bool &state)
{
    focus=state;
}
