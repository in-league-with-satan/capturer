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

#ifndef OVERLAY_VIEW_H
#define OVERLAY_VIEW_H

#include <QQuickWidget>

class QmlMessenger;
class QQmlImageProviderBase;

class OverlayView : public QQuickWidget
{
    Q_OBJECT

public:
    OverlayView(QWidget *parent=0);
    ~OverlayView();

    void setMessenger(QmlMessenger *messenger);

    void addImageProvider(const QString &id, QQmlImageProviderBase *image_provider);

private:
    QPoint pos_mouse_press;

protected:
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);

    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
};

#endif // OVERLAY_VIEW_H
