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

#ifndef TERM_GUI_H
#define TERM_GUI_H

#include <QObject>
#include "ff_encoder.h"

class CursedTable;
class CursedLabel;
class CursedSettings;
class CursedState;
class SettingsModel;
class MainWindow;

class TermGui : public QObject
{
    Q_OBJECT

public:
    explicit TermGui(SettingsModel *settings_model, MainWindow *mw);
    ~TermGui();

public slots:
    void run();

    void stopCurses();

    void updateStats(FFEncoder::Stats s);
    void setFreeSpace(qint64 size);

    void reloadDevices();

    void update();

private slots:
    void buildForm();

    void switchMode();

    void onUpArrow();
    void onDownArrow();
    void onLeftArrow();
    void onRightArrow();

private:
    CursedState *c_state;
    CursedSettings *c_settings;
    CursedLabel *c_label;

    int max_width=0;
    int max_height=0;

    bool running=true;

    MainWindow *mw=nullptr;

    int mode;

    struct Mode {
        enum {
            title,
            setup
        };
    };

signals:

};

#endif // TERM_GUI_H
