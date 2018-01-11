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

#ifndef DIALOG_KEYBOARD_SHORTCUTS_H
#define DIALOG_KEYBOARD_SHORTCUTS_H

#include <QDialog>

class QLabel;
class QLineEdit;

class DialogKeyboardShortcuts : public QDialog
{
    Q_OBJECT

public:
    explicit DialogKeyboardShortcuts(QWidget *parent=0);
    ~DialogKeyboardShortcuts();

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

#endif // DIALOG_KEYBOARD_SHORTCUTS_H
