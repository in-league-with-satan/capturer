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

#ifndef DIALOG_SETUP_H
#define DIALOG_SETUP_H

#include <QDialog>

class QLineEdit;
class QCheckBox;

class KeyboardShortcuts;

class DialogSetup : public QDialog
{
  Q_OBJECT

public:
    explicit DialogSetup(QWidget *parent=0);
    ~DialogSetup();

private slots:
    void selectVideosDir();

    void onAccepted();

private:
    KeyboardShortcuts *keyboard_shortcuts;

    QLineEdit *le_location_videos;
    QCheckBox *cb_simplify_audio_for_send;

#ifdef LIB_QHTTP

    QCheckBox *cb_http_server_state;
    QLineEdit *le_http_server_port;

#endif
};

#endif // DIALOG_SETUP_H
