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

#ifndef CURSED_WIDGET_H
#define CURSED_WIDGET_H

#include <QObject>
#include <QSize>
#include <QPoint>

#define COLOR_WHITE_BLACK 1
#define COLOR_WHITE_CYAN 2
#define COLOR_RED_BLACK 3

struct CursedWidgetContext;

class CursedWidget : public QObject
{
    Q_OBJECT

public:
    explicit CursedWidget(QObject *parent=0);

    ~CursedWidget();

    virtual void setSize(const int &cols, const int &rows);
    virtual void setPos(const int &x, const int &y);

    virtual QSize size() const;
    virtual QPoint pos() const;

    virtual QSize minSize() const;

    virtual bool isVisible() const;

    void *win() const;
    void *pan() const;

public slots:
    virtual void show();
    virtual void hide();

    virtual void setFocus(const bool &state);

    virtual void update()=0;

protected:
    CursedWidgetContext *d;

    bool is_visible;
    bool focus;

signals:
    void updateRequest();
};

#endif // CURSED_WIDGET_H
