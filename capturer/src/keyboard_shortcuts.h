/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef KEYBOARD_SHORTCUTS_H
#define KEYBOARD_SHORTCUTS_H

#include <QWidget>

class QLabel;
class QLineEdit;

class KeyboardShortcuts : public QWidget
{
    Q_OBJECT

public:
    explicit KeyboardShortcuts(QWidget *parent=0);
    ~KeyboardShortcuts();

    Qt::Key toQtKey(int code);
    void setKey(int code, Qt::Key key);
    static Qt::Key defaultQtKey(int code);

protected:
    virtual bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void toDefault();

private:
    struct Row {
        QLabel *label;
        QLineEdit *line_edit;
    };

    QVector <Row*> row;
};

#endif // KEYBOARD_SHORTCUTS_H
