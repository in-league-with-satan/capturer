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
#include <QGroupBox>

class QLineEdit;
class QCheckBox;
class QTableWidget;

class KeyboardShortcuts;
class Streams;

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

    Streams *streams;

#ifdef LIB_QHTTP

    QCheckBox *cb_http_server_state;
    QLineEdit *le_http_server_port;

#endif
};

class Streams : public QGroupBox
{
    Q_OBJECT

public:
    explicit Streams(QWidget *parent=0);

    QVariantList getList() const;

private slots:
    void itemSelectionChanged();
    void addRow();
    void editRow();
    void removeRow();

private:
    void addRow(const QString &title, const QString &url);

    QTableWidget *table;
    QPushButton *b_add;
    QPushButton *b_remove;
    QPushButton *b_edit;
};

#endif // DIALOG_SETUP_H
