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

#ifndef CURSED_SETTINGS_H
#define CURSED_SETTINGS_H

#include <QVector>

#include "cursed_widget.h"

class SettingsModel;

class CursedSettings : public CursedWidget
{
    Q_OBJECT

public:
   explicit CursedSettings(QObject *parent=0);
    ~CursedSettings();

    void setSettingsModel(SettingsModel *model);

    int cursorUp();
    int cursorDown();

    int cursorPageUp();
    int cursorPageDown();

    void cursorLeft();
    void cursorRight();

    void update();

private slots:
    void modelDataChanged(int row, int role, bool qml);

private:
    void changeValue(bool forward);
    void checkIndex(bool forward);
    void drawTitle(int row, QString title);
    void drawDivider(int row);
    void drawCombobox(int row, bool focus, QString key, QString value);
    void drawCheckbox(int row, bool focus, QString title, bool checked);
    void drawButton(int row, bool focus, QString title);

    SettingsModel *settings_model;

    int index_selection=0;
};

#endif // CURSED_SETTINGS_H
