/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include <QStyle>
#include <QApplication>

#include "data_types.h"

#include "keyboard_shortcuts.h"

KeyboardShortcuts::KeyboardShortcuts(QWidget *parent)
    : QWidget(parent)
{
    QGridLayout *la_lines=new QGridLayout();

    row.resize(KeyCodeC::enm_size);

    for(int i=0; i<KeyCodeC::enm_size; ++i) {
        row[i]=new Row();

        la_lines->addWidget(row[i]->label=new QLabel(KeyCodeC::toString(i) + QStringLiteral(":")), i, 0);
        la_lines->addWidget(row[i]->line_edit=new QLineEdit(QKeySequence(defaultQtKey(i)).toString()), i, 1);

        row[i]->line_edit->installEventFilter(this);
    }

    QPushButton *b_set_default=new QPushButton("reset to default");

    connect(b_set_default, SIGNAL(clicked(bool)), SLOT(toDefault()));

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addLayout(la_lines);
    la_main->addWidget(b_set_default);

    setLayout(la_main);
}

KeyboardShortcuts::~KeyboardShortcuts()
{
    for(int i=0; i<KeyCodeC::enm_size; ++i) {
        row[i]->label->deleteLater();
        row[i]->line_edit->deleteLater();

        delete row[i];
    }

    row.clear();
}

Qt::Key KeyboardShortcuts::toQtKey(int code)
{
    if(code<0 || code>=KeyCodeC::enm_size)
        return Qt::Key_F1;

    QKeySequence seq(row[code]->line_edit->text());

    return seq.count()==1 ? (Qt::Key)seq[0] : defaultQtKey(code);
}

void KeyboardShortcuts::setKey(int code, Qt::Key key)
{
    if(code<0 || code>=KeyCodeC::enm_size)
        return;

    row[code]->line_edit->setText(QKeySequence(key).toString());
}

Qt::Key KeyboardShortcuts::defaultQtKey(int code)
{
    switch(code) {
    case KeyCodeC::About:
        return Qt::Key_F1;

    case KeyCodeC::FileBrowser:
        return Qt::Key_F2;

    case KeyCodeC::Rec:
        return Qt::Key_F4;

    case KeyCodeC::Info:
        return Qt::Key_F5;

    case KeyCodeC::RecState:
        return Qt::Key_F6;

    case KeyCodeC::PreviewPrimary:
        return Qt::Key_F7;

    case KeyCodeC::PreviewSecondary:
        return Qt::Key_F9;

    case KeyCodeC::PreviewSecondaryChangePosition:
        return Qt::Key_C;

    case KeyCodeC::PreviewSwitchHalfFps:
        return Qt::Key_H;

    case KeyCodeC::HdrToSdr:
        return Qt::Key_F8;

    case KeyCodeC::HdrBrightnesPlus:
        return Qt::Key_J;

    case KeyCodeC::HdrBrightnesMinus:
        return Qt::Key_N;

    case KeyCodeC::HdrSaturationPlus:
        return Qt::Key_K;

    case KeyCodeC::HdrSaturationMinus:
        return Qt::Key_M;

    case KeyCodeC::FullScreen:
        return Qt::Key_F11;

    case KeyCodeC::Exit:
        return Qt::Key_F12;

    case KeyCodeC::Menu:
        return Qt::Key_Menu;

    case KeyCodeC::Back:
        return Qt::Key_Backspace;

    case KeyCodeC::Enter:
        return Qt::Key_Return;

    case KeyCodeC::Up:
        return Qt::Key_Up;

    case KeyCodeC::Down:
        return Qt::Key_Down;

    case KeyCodeC::Left:
        return Qt::Key_Left;

    case KeyCodeC::Right:
        return Qt::Key_Right;
    }

    return Qt::Key_unknown;
}

bool KeyboardShortcuts::eventFilter(QObject *obj, QEvent *event)
{
    QLineEdit *line_edit=nullptr;

    for(int i=0; i<row.size(); ++i) {
        if(row[i]->line_edit==obj) {
            line_edit=row[i]->line_edit;
            break;
        }
    }

    if(line_edit) {
        if(event->type()==QEvent::KeyPress) {
            QKeyEvent *key_event=static_cast<QKeyEvent*>(event);

            if(key_event) {
                if(!key_event->modifiers()) {
                    line_edit->setText(QKeySequence(key_event->key()).toString());
                }

                key_event->ignore();

                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}

void KeyboardShortcuts::toDefault()
{
    for(int i=0; i<row.size(); ++i)
        row[i]->line_edit->setText(QKeySequence(defaultQtKey(i)).toString());
}

