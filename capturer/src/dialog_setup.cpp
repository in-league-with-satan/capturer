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

#include <QDebug>
#include <QApplication>
#include <QLayout>
#include <QFileDialog>
#include <QGroupBox>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QStyle>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QInputDialog>

#include "settings.h"
#include "data_types.h"
#include "keyboard_shortcuts.h"

#include "dialog_setup.h"


DialogSetup::DialogSetup(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlag(Qt::WindowMinMaxButtonsHint);

    setMinimumSize(640, 480);

    setWindowTitle("Setup");

    connect(this, SIGNAL(accepted()), SLOT(onAccepted()));

    //

    QPushButton *b_ok=new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogOkButton), "Ok");
    QPushButton *b_cancel=new QPushButton(qApp->style()->standardIcon(QStyle::SP_DialogCancelButton), "Cancel");

    connect(b_ok, SIGNAL(clicked(bool)), SLOT(accept()));
    connect(b_cancel, SIGNAL(clicked(bool)), SLOT(reject()));


    QLabel *l_location_videos=new QLabel("Captures location:");

    le_location_videos=new QLineEdit();
    le_location_videos->setReadOnly(true);
    le_location_videos->setAlignment(Qt::AlignRight);
    le_location_videos->setText(settings->main.location_videos);

    cb_simplify_audio_for_send=new QCheckBox("Simplify audio for send to remote player");
    cb_simplify_audio_for_send->setToolTip("convert to 16bit stereo");
    cb_simplify_audio_for_send->setChecked(settings->main.simplify_audio_for_send);

    QPushButton *b_location_videos_select_dir=new QPushButton("Select");
    connect(b_location_videos_select_dir, SIGNAL(clicked(bool)), SLOT(selectVideosDir()));

    QHBoxLayout *la_main_settings_videos_dir=new QHBoxLayout();
    la_main_settings_videos_dir->addWidget(l_location_videos);
    la_main_settings_videos_dir->addWidget(le_location_videos);
    la_main_settings_videos_dir->addWidget(b_location_videos_select_dir);


    QVBoxLayout *la_main_settings=new QVBoxLayout();
    la_main_settings->addLayout(la_main_settings_videos_dir);
    la_main_settings->addWidget(cb_simplify_audio_for_send);

    QGroupBox *gb_main_settings=new QGroupBox("Main");
    gb_main_settings->setLayout(la_main_settings);

    //

#ifdef LIB_QHTTP

    cb_http_server_state=new QCheckBox("Enabled");
    cb_http_server_state->setChecked(settings->http_server.enabled);

    le_http_server_port=new QLineEdit();
    le_http_server_port->setInputMask("99999");
    le_http_server_port->setText(QString::number(settings->http_server.port));

    QLabel *l_http_server_port=new QLabel("Listen port:");

    QGridLayout *la_http_server=new QGridLayout();

    int row=0;

    la_http_server->addWidget(cb_http_server_state, row, 0);

    row++;

    la_http_server->addWidget(l_http_server_port, row, 0);
    la_http_server->addWidget(le_http_server_port, row, 1);

    QGroupBox *gb_http_server=new QGroupBox("Http server");

    gb_http_server->setLayout(la_http_server);

#endif

    //

    keyboard_shortcuts=new KeyboardShortcuts();

    for(int i=0; i<KeyCodeC::enm_size; ++i)
        keyboard_shortcuts->setKey(i, (Qt::Key)settings->keyboard_shortcuts.code.key(i, KeyboardShortcuts::defaultQtKey(i)));

    QGroupBox *gb_keyboard_shortcuts=new QGroupBox("Keyboard shortcuts");

    QVBoxLayout *la_gb_keyboard_shortcuts=new QVBoxLayout();
    la_gb_keyboard_shortcuts->addWidget(keyboard_shortcuts);
    la_gb_keyboard_shortcuts->setMargin(0);

    gb_keyboard_shortcuts->setLayout(la_gb_keyboard_shortcuts);

    //

    QVBoxLayout *la_whole=new QVBoxLayout();

    la_whole->addWidget(gb_main_settings);

#ifdef LIB_QHTTP

    la_whole->addWidget(gb_http_server);

#endif


    la_whole->addWidget(gb_keyboard_shortcuts);

    la_whole->addWidget(streams=new Streams());

    QWidget *w_whole=new QWidget();
    w_whole->setLayout(la_whole);

    QScrollArea *scroll_area=new QScrollArea();
    scroll_area->setWidgetResizable(false);
    scroll_area->setAlignment(Qt::AlignCenter);
    scroll_area->setWidget(w_whole);


    //


    QHBoxLayout *la_ok_cancel=new QHBoxLayout();
    la_ok_cancel->addWidget(b_ok);
    la_ok_cancel->addWidget(b_cancel);


    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(scroll_area);
    la_main->addLayout(la_ok_cancel);

    setLayout(la_main);
}

DialogSetup::~DialogSetup()
{
}

void DialogSetup::selectVideosDir()
{
    QString new_location=QFileDialog::getExistingDirectory(0, "", le_location_videos->text());

    if(new_location.isEmpty())
        return;

    le_location_videos->setText(new_location);
}

void DialogSetup::onAccepted()
{
    settings->main.location_videos=le_location_videos->text();
    settings->main.simplify_audio_for_send=cb_simplify_audio_for_send->isChecked();


#ifdef LIB_QHTTP

    settings->http_server.enabled=cb_http_server_state->isChecked();
    settings->http_server.port=le_http_server_port->text().simplified().toInt();

#endif

    //

    settings->streaming.url=streams->getList();

    //

    settings->keyboard_shortcuts.code.clear();

    for(int i=0; i<KeyCodeC::enm_size; ++i)
        settings->keyboard_shortcuts.code.insert(keyboard_shortcuts->toQtKey(i), i);

    //

    settings->save();
}

//

Streams::Streams(QWidget *parent)
    : QGroupBox("Streaming services", parent)
{
    table=new QTableWidget();
    table->setSortingEnabled(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList() << "title" << "url");
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();


    connect(table, SIGNAL(itemSelectionChanged()), SLOT(itemSelectionChanged()));

    b_add=new QPushButton("add");
    b_edit=new QPushButton("edit");
    b_remove=new QPushButton("remove");

    connect(b_add, SIGNAL(clicked(bool)), SLOT(addRow()));
    connect(b_edit, SIGNAL(clicked(bool)), SLOT(editRow()));
    connect(b_remove, SIGNAL(clicked(bool)), SLOT(removeRow()));

    b_remove->setEnabled(false);
    b_edit->setEnabled(false);

    settings->streaming.url;


    for(int i=0; i<settings->streaming.url.size(); ++i) {
        QString title=settings->streaming.url[i].toMap().keys().value(0).simplified();
        QString url=settings->streaming.url[i].toMap().values().value(0).toString().simplified();

        addRow(title, url);
    }

    QVBoxLayout *la_buttons=new QVBoxLayout();
    la_buttons->addWidget(b_add);
    la_buttons->addWidget(b_edit);
    la_buttons->addWidget(b_remove);
    la_buttons->addStretch(1);

    QHBoxLayout *la_main=new QHBoxLayout();
    la_main->addWidget(table);
    la_main->addLayout(la_buttons);

    setLayout(la_main);
}

QVariantList Streams::getList() const
{
    QVariantList list;

    for(int i=0; i<table->rowCount(); ++i) {
        QVariantMap map;

        map.insert(table->item(i, 0)->text().simplified(), table->item(i, 1)->text().simplified());

        list << map;
    }

    return list;
}

void Streams::itemSelectionChanged()
{
    if(table->currentRow()<0) {
        b_remove->setEnabled(false);
        b_edit->setEnabled(false);

    } else {
        b_remove->setEnabled(true);
        b_edit->setEnabled(true);
    }
}

void Streams::addRow()
{
    const QString name=QInputDialog::getText(this, " ", "service title/name:", QLineEdit::Normal).simplified();

    if(name.isEmpty())
        return;

    const QString url=QInputDialog::getText(this, " ", "stream url:", QLineEdit::Normal).simplified();

    if(url.isEmpty())
        return;

    addRow(name, url);
}


void Streams::addRow(const QString &title, const QString &url)
{
    const int row_index=table->rowCount();

    table->setRowCount(row_index + 1);

    QTableWidgetItem *item;

    table->setItem(row_index, 0, item=new QTableWidgetItem(title));
    item->setData(Qt::ToolTipRole, title);

    table->setItem(row_index, 1, item=new QTableWidgetItem(url));
    item->setData(Qt::ToolTipRole, url);
}

void Streams::editRow()
{
    const int row_index=table->currentRow();

    if(row_index<0)
        return;

    const QString name=QInputDialog::getText(this, " ", "service title/name:", QLineEdit::Normal, table->item(row_index, 0)->text()).simplified();

    if(name.isEmpty())
        return;

    const QString url=QInputDialog::getText(this, " ", "stream url:", QLineEdit::Normal, table->item(row_index, 1)->text()).simplified();

    if(url.isEmpty())
        return;

    table->item(row_index, 0)->setText(name);
    table->item(row_index, 1)->setText(url);
}

void Streams::removeRow()
{
    const int row_index=table->currentRow();

    if(row_index<0)
        return;

    QMessageBox msg_box;
    msg_box.setWindowTitle(" ");
    msg_box.setText(QString("are you sure you want to remove \"%1\" service?").arg(table->item(row_index, 0)->text()));
    msg_box.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msg_box.setDefaultButton(QMessageBox::Cancel);

    if(msg_box.exec()!=QMessageBox::Yes)
        return;

    table->removeRow(row_index);

    itemSelectionChanged();
}

