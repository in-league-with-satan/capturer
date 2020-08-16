/******************************************************************************

Copyright © 2019-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <qcoreapplication.h>

#include "cursed_table.h"
#include "cursed_label.h"
#include "cursed_settings.h"
#include "cursed_state.h"
#include "utf8_draw.h"
#include "string_tools.h"

#include "settings.h"
#include "frame_buffer.h"
#include "settings_model.h"
#include "mainwindow.h"

#include "term_gui.h"

#ifdef LIB_CURSES
#  include <curses.h>
#  include <panel.h>
#  ifdef __linux__
#    include <termios.h>
#    include <unistd.h>
     termios term_attr;
#  endif

WINDOW *window_glob=nullptr;

#endif

TermGui::TermGui(SettingsModel *settings_model, MainWindow *mw)
    : QObject(mw)
    , mw(mw)
    , mode(Mode::title)
    , running(false)
{
    if(!settings_model)
        return;

#ifdef LIB_CURSES

#  ifdef __linux__

    tcgetattr(STDOUT_FILENO, &term_attr);

#  endif

    window_glob=initscr();
    cbreak();
    raw();
    nonl();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    if(has_colors()==false) {
        endwin();
        puts("\nYour terminal does not support color");
        exit(1);
    }

    start_color();
    //use_default_colors();

    init_pair(COLOR_WHITE_BLACK, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_WHITE_CYAN, COLOR_WHITE, COLOR_CYAN);
    init_pair(COLOR_RED_BLACK, COLOR_RED, COLOR_BLACK);

    //

    c_state=new CursedState();
    c_state->show();
    connect(c_state, SIGNAL(updateRequest()), SLOT(update()));


    //

    c_label=new CursedLabel();
    c_label->setText("space: start/stop rec    s: settings menu    q: quit");
    c_label->setAlignment(Qt::AlignCenter);
    c_label->show();
    connect(c_label, SIGNAL(updateRequest()), SLOT(update()));

    //

    c_settings=new CursedSettings();
    c_settings->setSettingsModel(settings_model);
    c_settings->hide();
    connect(c_settings, SIGNAL(updateRequest()), SLOT(update()));

    //

    buildForm();

#endif
}

TermGui::~TermGui()
{
#ifdef LIB_CURSES

    delete c_label;
    delete c_settings;
    stopCurses();

#endif
}

void TermGui::update()
{
#ifdef LIB_CURSES

    if(mw->recInProgress())
        wbkgdset(window_glob, COLOR_PAIR(COLOR_RED_BLACK));

    else
        wbkgdset(window_glob, COLOR_PAIR(COLOR_WHITE_BLACK));

    Utf8Draw::_box(window_glob, Utf8Draw::lt_heavy);

    update_panels();
    doupdate();

#endif
}

void TermGui::updateSettingsForm()
{
#ifdef LIB_CURSES

    if(!running)
        return;

    c_settings->update();

#endif
}

void TermGui::buildForm()
{
#ifdef LIB_CURSES

    if(max_width==COLS && max_height==LINES)
        return;

    max_width=COLS;
    max_height=LINES;

    //

    c_label->setSize(max_width - 2, 1);
    c_label->setPos(1, max_height - 2);

    c_state->setSize(max_width - 2, max_height - 4);
    c_state->setPos(1, 1);

    c_settings->setSize(max_width - 2, max_height - 4);
    c_settings->setPos(1, 1);

    wclear(window_glob);

    c_state->update();
    c_settings->update();
    c_label->update();

#endif
}

void TermGui::switchMode()
{
    if(mode==Mode::title) {
        mode=Mode::setup;
        c_state->hide();
        c_settings->show();

    } else {
        mode=Mode::title;
        c_state->show();
        c_settings->hide();
    }
}

void TermGui::onUpArrow()
{
    switch(mode) {
    case Mode::title:
        break;

    case Mode::setup:
        c_settings->cursorUp();
        break;

    default:
        break;
    }
}

void TermGui::onDownArrow()
{
    switch(mode) {
    case Mode::title:
        break;

    case Mode::setup:
        c_settings->cursorDown();
        break;

    default:
        break;
    }
}

void TermGui::onPageUpArrow()
{
    switch(mode) {
    case Mode::title:
        break;

    case Mode::setup:
        c_settings->cursorPageUp();
        break;

    default:
        break;
    }
}

void TermGui::onPageDownArrow()
{
    switch(mode) {
    case Mode::title:
        break;

    case Mode::setup:
        c_settings->cursorPageDown();
        break;

    default:
        break;
    }
}

void TermGui::onLeftArrow()
{
    if(mw->recInProgress())
        return;

    switch(mode) {
    case Mode::title:
        break;

    case Mode::setup:
        c_settings->cursorLeft();
        break;

    default:
        break;
    }
}

void TermGui::onRightArrow()
{
    if(mw->recInProgress())
        return;

    switch(mode) {
    case Mode::title:
        break;

    case Mode::setup:
        c_settings->cursorRight();
        break;

    default:
        break;
    }
}

void TermGui::run()
{
#ifdef LIB_CURSES

    wint_t wch;
    int ret=0;

    // halfdelay(1);

    noecho();
    raw();

    nonl();


    running=true;


    timeout(200);

    while(running) {
        ret=get_wch(&wch);

        if(ret==ERR) {
#  ifndef __linux__

            if(is_termresized()) {
                resize_term(0, 0);
                buildForm();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(2));
            qApp->processEvents();
            continue;

#  endif

        } else {
            switch(wch) {
            case KEY_RESIZE:
                buildForm();
                break;

            case KEY_UP:
                onUpArrow();
                break;

            case KEY_DOWN:
                onDownArrow();
                break;

            case KEY_PPAGE:
                onPageUpArrow();
                break;

            case KEY_NPAGE:
                onPageDownArrow();
                break;

            case KEY_LEFT:
                onLeftArrow();
                break;

            case KEY_RIGHT:
                onRightArrow();
                break;

            case KEY_BACKSPACE:
                break;

            case ' ':
                mw->startStopRecording();
                break;

            case 'B':
            case 'b':
                resize_term(0, 0);
                buildForm();
                break;

            case 'S':
            case 's':
                switchMode();
                break;

            case 'Q':
            case 'q':
                running=false;
                break;

            default:
                // qDebug() << "default" << (int)wch;
                break;
            }

            c_state->update();
            c_settings->update();
            c_label->update();

        }

        qApp->processEvents();
    }

    delete mw;

    echo();
    endwin();

#ifdef __linux__

    tcsetattr(STDOUT_FILENO, TCSANOW, &term_attr);

#endif

    exit(0);

#endif
}

void TermGui::stopCurses()
{
    running=false;
}

void TermGui::updateStats(FFEncoder::Stats s)
{
#ifdef LIB_CURSES

    if(!running)
        return;

    CursedState::Dev d;

    if(s.enc_num==0) {
        c_state->setRecDuration(s.time);
    }

    if(s.enc_num>=0 && s.enc_num<mw->stream.size()) {
        d.device=mw->stream[s.enc_num].source_device->currentDeviceName();
        d.format=mw->stream[s.enc_num].source_device->currentFormat();
        d.buffer_size=mw->stream[s.enc_num].encoder->frameBuffer()->size();

    } else if(s.enc_num==-1) {
        d.device=mw->settings_model->data_p(&settings->streaming.url_index)->values[settings->streaming.url_index];
        d.buffer_size=mw->encoder_streaming->frameBuffer()->size();
    }

    d.streams_size=QLocale().toString(qulonglong(s.streams_size/1024./1024.));
    d.bitrate=QString(QLatin1String("%1/%2"))
            .arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2))
            .arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2));

    c_state->updateState(s.enc_num, d);

    c_state->update();
    c_label->update();

#endif
}

void TermGui::setFreeSpace(qint64 size)
{
#ifdef LIB_CURSES

    if(!running)
        return;

    c_state->setFreeSpace(size);
    c_state->update();
    c_label->update();

#endif
}

void TermGui::setNvState(const NvState &state)
{
#ifdef LIB_CURSES

    if(!running)
        return;

    c_state->setNvState(state);
    c_state->update();
    c_label->update();

#endif
}

void TermGui::reloadDevices()
{
#ifdef LIB_CURSES

    if(!running)
        return;

    c_state->clear();

    CursedState::Dev d;

    for(int i=0; i<mw->stream.size(); ++i) {
        if(mw->stream[i].source_device) {
            d.device=mw->stream[i].source_device->currentDeviceName();

            if(!mw->stream[i].source_device->gotSignal())
                d.format="no signal";

            else
                d.format=mw->stream[i].source_device->currentFormat();

            // qInfo() << "mw->stream[i].source_device->currentFormat" << d.format << mw->stream[i].source_device->currentFormat();

        } else {
            d.device="disabled";
            d.format="no signal";
        }

        c_state->updateState(i, d);
    }

    if(settings->streaming.url_index>0) {
        d.device=mw->settings_model->data_p(&settings->streaming.url_index)->values[settings->streaming.url_index];
        d.format.clear();

        c_state->updateState(-1, d);
    }

    c_state->update();
    c_label->update();

#endif
}
