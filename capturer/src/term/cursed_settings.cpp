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

#ifdef LIB_CURSES
#  ifdef __linux__
#    include <ncursesw/curses.h>
#    include <ncursesw/panel.h>
#  else
#    include "curses.h"
#    include "panel.h"
#  endif
#endif

#include "utf8_draw.h"
#include "string_tools.h"

#include "settings_model.h"

#include "cursed_settings.h"


CursedSettings::CursedSettings(QObject *parent)
    : CursedWidget(parent)
    , settings_model(nullptr)
    , index_selection(0)
{
}

CursedSettings::~CursedSettings()
{
}

void CursedSettings::setSettingsModel(SettingsModel *model)
{
    settings_model=model;

    connect(settings_model, SIGNAL(dataChanged(int,int,bool)), SLOT(modelDataChanged(int,int,bool)));

    checkIndex(true);
}

int CursedSettings::cursorUp()
{
    index_selection--;

    if(index_selection<0)
        index_selection=settings_model->rowCount() - 1;

    checkIndex(false);

    update();

    return index_selection;
}

int CursedSettings::cursorDown()
{
    index_selection++;

    checkIndex(true);

    update();

    return index_selection;
}

void CursedSettings::changeValue(bool forward)
{
    if(index_selection<0 || index_selection>=settings_model->rowCount())
        return;

    SettingsModel::Data *data=settings_model->data_p(index_selection);

    if(!data)
        return;

    if(data->type==SettingsModel::Type::combobox) {
        if(forward)
            (*data->value)++;

        else
            (*data->value)--;

        if(*data->value<0)
            *data->value=data->values.size() - 1;

        if(*data->value>=data->values.size())
            *data->value=0;

        settings_model->setData(data->value, SettingsModel::Role::value, *data->value);

    } else if(data->type==SettingsModel::Type::checkbox) {
        settings_model->setData(data->value, SettingsModel::Role::value, !bool(*data->value));

    } else if(data->type==SettingsModel::Type::button) {
        settings_model->setData(data->value, SettingsModel::Role::value, *data->value);
    }
}

int CursedSettings::cursorLeft()
{
    changeValue(false);
}

int CursedSettings::cursorRight()
{
    changeValue(true);
}

void CursedSettings::drawTitle(int row, QString title)
{
#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    int max_y=0, max_x=0;

    getmaxyx(w, max_y, max_x);

    int visible_size=max_x;

    if(title.size()>visible_size)
        title.resize(visible_size);

    int left=visible_size*.5 - title.size()*.5;

    QString str=title;

    str.append(QString().fill(' ', visible_size - left - str.size()));
    str.prepend(QString().fill(' ', left));

    mvwaddwstr(w, row, 0, (wchar_t*)str.toStdWString().data());

#endif
}

void CursedSettings::drawDivider(int row)
{
#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    int max_y=0, max_x=0;

    getmaxyx(w, max_y, max_x);

    QString str=QString().fill('-', max_x);

    mvwaddwstr(w, row, 0, (wchar_t*)str.toStdWString().data());

#endif
}

void CursedSettings::drawCombobox(int row, bool focus, QString key, QString value)
{
#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    if(focus)
        wbkgdset(w, COLOR_PAIR(COLOR_WHITE_CYAN));

    else
        wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    int max_y=0, max_x=0;

    getmaxyx(w, max_y, max_x);

    const int visible_size=max_x;
    const int wdt_left=visible_size*.5;
    const int wdt_right=visible_size - wdt_left;

    //

    QString str=key;

    str+=":";

    if(str.size()>wdt_left) {
        str.resize(wdt_left - 2);
        str+="…:";
    }

    str.append(QString().fill(' ', wdt_left - str.size()));

    mvwaddwstr(w, row, 0, (wchar_t*)str.toStdWString().data());

    //

    int left=wdt_right*.5 - value.size()*.5;

    str=value;

    str.append(QString().fill(' ', wdt_right - left - str.size() - 1));
    str.prepend(QString().fill(' ', left - 1));

    str.prepend("<");
    str.append(">");

    if(str.size()>wdt_right) {
        str.resize(wdt_right - 2);
        str+="…>";
    }

    mvwaddwstr(w, row, wdt_left, (wchar_t*)str.toStdWString().data());

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

#endif
}

void CursedSettings::drawCheckbox(int row, bool focus, QString title, bool checked)
{
#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    if(focus)
        wbkgdset(w, COLOR_PAIR(COLOR_WHITE_CYAN));

    else
        wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    int max_y=0, max_x=0;

    getmaxyx(w, max_y, max_x);

    const int visible_size=max_x;
    const int wdt_left=visible_size*.5;
    const int wdt_right=visible_size - wdt_left;

    //

    QString str=title;

    str+=":";

    if(str.size()>wdt_left) {
        str.resize(wdt_left - 2);
        str+="…:";
    }

    str.append(QString().fill(' ', wdt_left - str.size()));

    mvwaddwstr(w, row, 0, (wchar_t*)str.toStdWString().data());

    //


    if(checked) {
#  ifdef __linux__
         str="☑";
#  else
        // str="[✓]";
        str="[x]";
#  endif

    } else {
#  ifdef __linux__
        str="☐";
#  else
        str="[ ]";
#  endif
    }

    int left=wdt_right*.5 - str.size()*.5;

    str.append(QString().fill(' ', wdt_right - left - str.size()));
    str.prepend(QString().fill(' ', left));

    mvwaddwstr(w, row, wdt_left, (wchar_t*)str.toStdWString().data());

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

#endif
}

void CursedSettings::drawButton(int row, bool focus, QString title)
{
#ifdef LIB_CURSES

    WINDOW *w=(WINDOW*)win();

    if(focus)
        wbkgdset(w, COLOR_PAIR(COLOR_WHITE_CYAN));

    else
        wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

    int max_y=0, max_x=0;

    getmaxyx(w, max_y, max_x);

    const int visible_size=max_x;
    const int wdt_left=visible_size*.5;
    const int wdt_right=visible_size - wdt_left;

    //

    int left=wdt_left*.5 - title.size()*.5;

    QString str=title;

    str.append(QString().fill(' ', wdt_right - left - str.size() - 1));
    str.prepend(QString().fill(' ', left - 1));

    str.prepend("[");
    str.append("]");

    if(str.size()>wdt_right) {
        str.resize(wdt_right - 2);
        str+="…]";
    }

    str.prepend(QString().fill(' ', wdt_left));

    mvwaddwstr(w, row, 0, (wchar_t*)str.toStdWString().data());

    wbkgdset(w, COLOR_PAIR(COLOR_WHITE_BLACK));

#endif
}

void CursedSettings::update()
{
#ifdef LIB_CURSES

    if(!is_visible)
        return;

    if(!settings_model)
        return;


    WINDOW *w=(WINDOW*)win();

#  ifndef __linux__

    wclear(w);

#  endif

    int max_y, max_x;

    getmaxyx(w, max_y, max_x);

    insdelln(max_y*max_y);


    max_y-=1;

    int middle_y=max_y*.5;

    int mdl_start=0;
    int mdl_stop=0;

    {
        mdl_start=index_selection - middle_y;

        if(mdl_start<0)
            mdl_start=0;

        mdl_stop=mdl_start + max_y;

        if(mdl_stop>=settings_model->rowCount())
            mdl_stop=settings_model->rowCount() - 1;

        if(mdl_stop - mdl_start!=max_y) {
            mdl_start=mdl_stop - max_y;

            if(mdl_start<0)
                mdl_start=0;
        }
    }

    int row=0;

    for(int row_mdl=mdl_start; row_mdl<=mdl_stop; ++row_mdl, ++row) {
        if(settings_model->data(row_mdl, SettingsModel::Role::type).toInt()==SettingsModel::Type::title) {
            drawTitle(row, settings_model->data(row_mdl, SettingsModel::Role::name).toString());

        } else if(settings_model->data(row_mdl, SettingsModel::Role::type).toInt()==SettingsModel::Type::divider) {
            drawDivider(row);

        } else if(settings_model->data(row_mdl, SettingsModel::Role::type).toInt()==SettingsModel::Type::combobox) {
            drawCombobox(row, row_mdl==index_selection,
                         settings_model->data(row_mdl, SettingsModel::Role::name).toString(),
                         settings_model->value(row_mdl).toString());

        } else if(settings_model->data(row_mdl, SettingsModel::Role::type).toInt()==SettingsModel::Type::checkbox) {
            drawCheckbox(row, row_mdl==index_selection,
                         settings_model->data(row_mdl, SettingsModel::Role::name).toString(),
                         settings_model->data(row_mdl, SettingsModel::Role::value).toBool());

        } else if(settings_model->data(row_mdl, SettingsModel::Role::type).toInt()==SettingsModel::Type::button) {
            drawButton(row, row_mdl==index_selection,
                       settings_model->data(row_mdl, SettingsModel::Role::name).toString());

        } else {
            qWarning() << "unhandled type:" << settings_model->data(row_mdl, SettingsModel::Role::type).toInt();
        }
    }

    emit updateRequest();

#endif
}

void CursedSettings::modelDataChanged(int row, int role, bool qml)
{
    Q_UNUSED(row)
    Q_UNUSED(role)
    Q_UNUSED(qml)

    update();
}

void CursedSettings::checkIndex(bool forward)
{
    if(settings_model->rowCount()<2) {
        index_selection=0;
        return;
    }

    if(index_selection>=settings_model->rowCount())
        index_selection=0;

    int type;

    while(true) {
        type=settings_model->data(index_selection, SettingsModel::Role::type).toInt();

        if(type!=SettingsModel::Type::title && type!=SettingsModel::Type::divider)
            return;

        if(forward) {
            index_selection++;

            if(index_selection>=settings_model->rowCount()) {
                index_selection=0;
                return;
            }

        } else {
            index_selection--;

            if(index_selection<0)
                index_selection=settings_model->rowCount() - 1;
        }
    }
}

