/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifdef LIB_CURSES
#include <curses.h>
#include <panel.h>
#endif


#include "utf8_draw.h"


#ifdef LIB_CURSES

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_HORIZONTAL=
        QString::fromUtf8("─").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_HORIZONTAL=
        QString::fromUtf8("━").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL=
        QString::fromUtf8("│").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_VERTICAL=
        QString::fromUtf8("┃").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_TRIPLE_DASH_HORIZONTAL=
        QString::fromUtf8("┄").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_TRIPLE_DASH_HORIZONTAL=
        QString::fromUtf8("┅").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_TRIPLE_DASH_VERTICAL=
        QString::fromUtf8("┆").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_TRIPLE_DASH_VERTICAL=
        QString::fromUtf8("┇").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_QUADRUPLE_DASH_HORIZONTAL=
        QString::fromUtf8("┈").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_QUADRUPLE_DASH_HORIZONTAL=
        QString::fromUtf8("┉").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_QUADRUPLE_DASH_VERTICAL=
        QString::fromUtf8("┊").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_QUADRUPLE_DASH_VERTICAL=
        QString::fromUtf8("┋").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DOWN_AND_RIGHT=
        QString::fromUtf8("┌").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_LIGHT_AND_RIGHT_HEAVY=
        QString::fromUtf8("┍").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_HEAVY_AND_RIGHT_LIGHT=
        QString::fromUtf8("┎").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_DOWN_AND_RIGHT=
        QString::fromUtf8("┏").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DOWN_AND_LEFT=
        QString::fromUtf8("┐").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_LIGHT_AND_LEFT_HEAVY=
        QString::fromUtf8("┑").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_HEAVY_AND_LEFT_LIGHT=
        QString::fromUtf8("┒").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_DOWN_AND_LEFT=
        QString::fromUtf8("┓").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_UP_AND_RIGHT=
        QString::fromUtf8("└").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_LIGHT_AND_RIGHT_HEAVY=
        QString::fromUtf8("┕").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_HEAVY_AND_RIGHT_LIGHT=
        QString::fromUtf8("┖").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_UP_AND_RIGHT=
        QString::fromUtf8("┗").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_UP_AND_LEFT=
        QString::fromUtf8("┘").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_LIGHT_AND_LEFT_HEAVY=
        QString::fromUtf8("┙").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_HEAVY_AND_LEFT_LIGHT=
        QString::fromUtf8("┚").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_UP_AND_LEFT=
        QString::fromUtf8("┛").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_RIGHT=
        QString::fromUtf8("├").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_LIGHT_AND_RIGHT_HEAVY=
        QString::fromUtf8("┝").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_HEAVY_AND_RIGHT_DOWN_LIGHT=
        QString::fromUtf8("┞").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_HEAVY_AND_RIGHT_UP_LIGHT=
        QString::fromUtf8("┟").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_HEAVY_AND_RIGHT_LIGHT=
        QString::fromUtf8("┠").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_LIGHT_AND_RIGHT_UP_HEAVY=
        QString::fromUtf8("┡").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_LIGHT_AND_RIGHT_DOWN_HEAVY=
        QString::fromUtf8("┢").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_VERTICAL_AND_RIGHT=
        QString::fromUtf8("┣").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT=
        QString::fromUtf8("┤").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_LIGHT_AND_LEFT_HEAVY=
        QString::fromUtf8("┥").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_HEAVY_AND_LEFT_DOWN_LIGHT=
        QString::fromUtf8("┦").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_HEAVY_AND_LEFT_UP_LIGHT=
        QString::fromUtf8("┧").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_HEAVY_AND_LEFT_LIGHT=
        QString::fromUtf8("┨").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_LIGHT_AND_LEFT_UP_HEAVY=
        QString::fromUtf8("┩").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_LIGHT_AND_LEFT_DOWN_HEAVY=
        QString::fromUtf8("┪").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_VERTICAL_AND_LEFT=
        QString::fromUtf8("┫").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DOWN_AND_HORIZONTAL=
        QString::fromUtf8("┬").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_HEAVY_AND_RIGHT_DOWN_LIGHT=
        QString::fromUtf8("┭").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_HEAVY_AND_LEFT_DOWN_LIGHT=
        QString::fromUtf8("┮").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_LIGHT_AND_HORIZONTAL_HEAVY=
        QString::fromUtf8("┯").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_HEAVY_AND_HORIZONTAL_LIGHT=
        QString::fromUtf8("┰").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_LIGHT_AND_LEFT_DOWN_HEAVY=
        QString::fromUtf8("┱").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_LIGHT_AND_RIGHT_DOWN_HEAVY=
        QString::fromUtf8("┲").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_DOWN_AND_HORIZONTAL=
        QString::fromUtf8("┳").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_UP_AND_HORIZONTAL=
        QString::fromUtf8("┴").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_HEAVY_AND_RIGHT_UP_LIGHT=
        QString::fromUtf8("┵").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_HEAVY_AND_LEFT_UP_LIGHT=
        QString::fromUtf8("┶").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_LIGHT_AND_HORIZONTAL_HEAVY=
        QString::fromUtf8("┷").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_HEAVY_AND_HORIZONTAL_LIGHT=
        QString::fromUtf8("┸").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_LIGHT_AND_LEFT_UP_HEAVY=
        QString::fromUtf8("┹").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_LIGHT_AND_RIGHT_UP_HEAVY=
        QString::fromUtf8("┺").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_UP_AND_HORIZONTAL=
        QString::fromUtf8("┻").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_VERTICAL_AND_HORIZONTAL=
        QString::fromUtf8("┼").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_HEAVY_AND_RIGHT_VERTICAL_LIGHT=
        QString::fromUtf8("┽").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_HEAVY_AND_LEFT_VERTICAL_LIGHT=
        QString::fromUtf8("┾").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_LIGHT_AND_HORIZONTAL_HEAVY=
        QString::fromUtf8("┿").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_HEAVY_AND_DOWN_HORIZONTAL_LIGHT=
        QString::fromUtf8("╀").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_HEAVY_AND_UP_HORIZONTAL_LIGHT=
        QString::fromUtf8("╁").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_HEAVY_AND_HORIZONTAL_LIGHT=
        QString::fromUtf8("╂").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_UP_HEAVY_AND_RIGHT_DOWN_LIGHT=
        QString::fromUtf8("╃").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_UP_HEAVY_AND_LEFT_DOWN_LIGHT=
        QString::fromUtf8("╄").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_DOWN_HEAVY_AND_RIGHT_UP_LIGHT=
        QString::fromUtf8("╅").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_DOWN_HEAVY_AND_LEFT_UP_LIGHT=
        QString::fromUtf8("╆").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_LIGHT_AND_UP_HORIZONTAL_HEAVY=
        QString::fromUtf8("╇").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_LIGHT_AND_DOWN_HORIZONTAL_HEAVY=
        QString::fromUtf8("╈").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_RIGHT_LIGHT_AND_LEFT_VERTICAL_HEAVY=
        QString::fromUtf8("╉").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LEFT_LIGHT_AND_RIGHT_VERTICAL_HEAVY=
        QString::fromUtf8("╊").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_VERTICAL_AND_HORIZONTAL=
        QString::fromUtf8("╋").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DOUBLE_DASH_HORIZONTAL=
        QString::fromUtf8("╌").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_DOUBLE_DASH_HORIZONTAL=
        QString::fromUtf8("╍").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DOUBLE_DASH_VERTICAL=
        QString::fromUtf8("╎").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_DOUBLE_DASH_VERTICAL=
        QString::fromUtf8("╏").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_HORIZONTAL=
        QString::fromUtf8("═").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_VERTICAL=
        QString::fromUtf8("║").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_SINGLE_AND_RIGHT_DOUBLE=
        QString::fromUtf8("╒").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_DOUBLE_AND_RIGHT_SINGLE=
        QString::fromUtf8("╓").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_DOWN_AND_RIGHT=
        QString::fromUtf8("╔").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_SINGLE_AND_LEFT_DOUBLE=
        QString::fromUtf8("╕").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_DOUBLE_AND_LEFT_SINGLE=
        QString::fromUtf8("╖").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_DOWN_AND_LEFT=
        QString::fromUtf8("╗").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_SINGLE_AND_RIGHT_DOUBLE=
        QString::fromUtf8("╘").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_DOUBLE_AND_RIGHT_SINGLE=
        QString::fromUtf8("╙").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_UP_AND_RIGHT=
        QString::fromUtf8("╚").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_SINGLE_AND_LEFT_DOUBLE=
        QString::fromUtf8("╛").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_DOUBLE_AND_LEFT_SINGLE=
        QString::fromUtf8("╜").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_UP_AND_LEFT=
        QString::fromUtf8("╝").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_SINGLE_AND_RIGHT_DOUBLE=
        QString::fromUtf8("╞").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_RIGHT_SINGLE=
        QString::fromUtf8("╟").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_VERTICAL_AND_RIGHT=
        QString::fromUtf8("╠").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_SINGLE_AND_LEFT_DOUBLE=
        QString::fromUtf8("╡").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_LEFT_SINGLE=
        QString::fromUtf8("╢").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_VERTICAL_AND_LEFT=
        QString::fromUtf8("╣").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_SINGLE_AND_HORIZONTAL_DOUBLE=
        QString::fromUtf8("╤").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOWN_DOUBLE_AND_HORIZONTAL_SINGLE=
        QString::fromUtf8("╥").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_DOWN_AND_HORIZONTAL=
        QString::fromUtf8("╦").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_SINGLE_AND_HORIZONTAL_DOUBLE=
        QString::fromUtf8("╧").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_UP_DOUBLE_AND_HORIZONTAL_SINGLE=
        QString::fromUtf8("╨").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_UP_AND_HORIZONTAL=
        QString::fromUtf8("╩").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_SINGLE_AND_HORIZONTAL_DOUBLE=
        QString::fromUtf8("╪").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_VERTICAL_DOUBLE_AND_HORIZONTAL_SINGLE=
        QString::fromUtf8("╫").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_DOUBLE_VERTICAL_AND_HORIZONTAL=
        QString::fromUtf8("╬").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_ARC_DOWN_AND_RIGHT=
        QString::fromUtf8("╭").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_ARC_DOWN_AND_LEFT=
        QString::fromUtf8("╮").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_ARC_UP_AND_LEFT=
        QString::fromUtf8("╯").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_ARC_UP_AND_RIGHT=
        QString::fromUtf8("╰").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DIAGONAL_UPPER_RIGHT_TO_LOWER_LEFT=
        QString::fromUtf8("╱").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DIAGONAL_UPPER_LEFT_TO_LOWER_RIGHT=
        QString::fromUtf8("╲").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DIAGONAL_CROSS=
        QString::fromUtf8("╳").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_LEFT=
        QString::fromUtf8("╴").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_UP=
        QString::fromUtf8("╵").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_RIGHT=
        QString::fromUtf8("╶").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_DOWN=
        QString::fromUtf8("╷").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_LEFT=
        QString::fromUtf8("╸").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_UP=
        QString::fromUtf8("╹").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_RIGHT=
        QString::fromUtf8("╺").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_DOWN=
        QString::fromUtf8("╻").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_LEFT_AND_HEAVY_RIGHT=
        QString::fromUtf8("╼").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_LIGHT_UP_AND_HEAVY_DOWN=
        QString::fromUtf8("╽").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_LEFT_AND_LIGHT_RIGHT=
        QString::fromUtf8("╾").toStdWString();

const std::wstring Utf8Draw::BOX_DRAWINGS_HEAVY_UP_AND_LIGHT_DOWN=
        QString::fromUtf8("╿").toStdWString();

const std::wstring Utf8Draw::UPPER_HALF_BLOCK=
        QString::fromUtf8("▀").toStdWString();

const std::wstring Utf8Draw::LOWER_ONE_EIGHTH_BLOCK=
        QString::fromUtf8("▁").toStdWString();

const std::wstring Utf8Draw::LOWER_ONE_QUARTER_BLOCK=
        QString::fromUtf8("▂").toStdWString();

const std::wstring Utf8Draw::LOWER_THREE_EIGHTHS_BLOCK=
        QString::fromUtf8("▃").toStdWString();

const std::wstring Utf8Draw::LOWER_HALF_BLOCK=
        QString::fromUtf8("▄").toStdWString();

const std::wstring Utf8Draw::LOWER_FIVE_EIGHTHS_BLOCK=
        QString::fromUtf8("▅").toStdWString();

const std::wstring Utf8Draw::LOWER_THREE_QUARTERS_BLOCK=
        QString::fromUtf8("▆").toStdWString();

const std::wstring Utf8Draw::LOWER_SEVEN_EIGHTHS_BLOCK=
        QString::fromUtf8("▇").toStdWString();

const std::wstring Utf8Draw::FULL_BLOCK=
        QString::fromUtf8("█").toStdWString();

const std::wstring Utf8Draw::LEFT_SEVEN_EIGHTHS_BLOCK=
        QString::fromUtf8("▉").toStdWString();

const std::wstring Utf8Draw::LEFT_THREE_QUARTERS_BLOCK=
        QString::fromUtf8("▊").toStdWString();

const std::wstring Utf8Draw::LEFT_FIVE_EIGHTHS_BLOCK=
        QString::fromUtf8("▋").toStdWString();

const std::wstring Utf8Draw::LEFT_HALF_BLOCK=
        QString::fromUtf8("▌").toStdWString();

const std::wstring Utf8Draw::LEFT_THREE_EIGHTHS_BLOCK=
        QString::fromUtf8("▍").toStdWString();

const std::wstring Utf8Draw::LEFT_ONE_QUARTER_BLOCK=
        QString::fromUtf8("▎").toStdWString();

const std::wstring Utf8Draw::LEFT_ONE_EIGHTH_BLOCK=
        QString::fromUtf8("▏").toStdWString();

const std::wstring Utf8Draw::RIGHT_HALF_BLOCK=
        QString::fromUtf8("▐").toStdWString();

const std::wstring Utf8Draw::LIGHT_SHADE=
        QString::fromUtf8("░").toStdWString();

const std::wstring Utf8Draw::MEDIUM_SHADE=
        QString::fromUtf8("▒").toStdWString();

const std::wstring Utf8Draw::DARK_SHADE=
        QString::fromUtf8("▓").toStdWString();

const std::wstring Utf8Draw::UPPER_ONE_EIGHTH_BLOCK=
        QString::fromUtf8("▔").toStdWString();

const std::wstring Utf8Draw::RIGHT_ONE_EIGHTH_BLOCK=
        QString::fromUtf8("▕").toStdWString();

//

void Utf8Draw::_mvwhline(void *win, int y, int x, std::wstring ch, int n)
{
    for(int i=0; i<n; ++i)
        mvwaddwstr((WINDOW*)win, y, x + i, ch.data());
}

void Utf8Draw::_mvwvline(void *win, int y, int x, std::wstring ch, int n)
{
    for(int i=0; i<n; ++i)
        mvwaddwstr((WINDOW*)win, y + i, x, ch.data());
}

void Utf8Draw::_mvwaddch(void *win, int y, int x, std::wstring ch)
{
    mvwaddwstr((WINDOW*)win, y, x, ch.data());
}

void Utf8Draw::_box(void *win, LineType t)
{
    int max_y, max_x;

    getmaxyx((WINDOW*)win, max_y, max_x);

    if(t==lt_light) {
        _mvwhline((WINDOW*)win, 0, 0, BOX_DRAWINGS_LIGHT_HORIZONTAL, max_x);
        _mvwhline((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_LIGHT_HORIZONTAL, max_x);

        _mvwvline((WINDOW*)win, 0, 0, BOX_DRAWINGS_LIGHT_VERTICAL, max_y);
        _mvwvline((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_LIGHT_VERTICAL, max_y);

        _mvwaddch((WINDOW*)win, 0, 0, BOX_DRAWINGS_LIGHT_DOWN_AND_RIGHT);
        _mvwaddch((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_LIGHT_DOWN_AND_LEFT);
        _mvwaddch((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_LIGHT_UP_AND_RIGHT);
        _mvwaddch((WINDOW*)win, max_y - 1, max_x - 1, BOX_DRAWINGS_LIGHT_UP_AND_LEFT);

        // _mvwaddch((WINDOW*)win, 0, 0, BOX_DRAWINGS_LIGHT_ARC_DOWN_AND_RIGHT);
        // _mvwaddch((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_LIGHT_ARC_DOWN_AND_LEFT);
        // _mvwaddch((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_LIGHT_ARC_UP_AND_RIGHT);
        // _mvwaddch((WINDOW*)win, max_y - 1, max_x - 1, BOX_DRAWINGS_LIGHT_ARC_UP_AND_LEFT);

    } else if(t==lt_heavy) {
        _mvwhline((WINDOW*)win, 0, 0, BOX_DRAWINGS_HEAVY_HORIZONTAL, max_x);
        _mvwhline((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_HEAVY_HORIZONTAL, max_x);

        _mvwvline((WINDOW*)win, 0, 0, BOX_DRAWINGS_HEAVY_VERTICAL, max_y);
        _mvwvline((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_HEAVY_VERTICAL, max_y);

        _mvwaddch((WINDOW*)win, 0, 0, BOX_DRAWINGS_HEAVY_DOWN_AND_RIGHT);
        _mvwaddch((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_HEAVY_DOWN_AND_LEFT);
        _mvwaddch((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_HEAVY_UP_AND_RIGHT);
        _mvwaddch((WINDOW*)win, max_y - 1, max_x - 1, BOX_DRAWINGS_HEAVY_UP_AND_LEFT);

    } else if(t==lt_double) {
        _mvwhline((WINDOW*)win, 0, 0, BOX_DRAWINGS_DOUBLE_HORIZONTAL, max_x);
        _mvwhline((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_DOUBLE_HORIZONTAL, max_x);

        _mvwvline((WINDOW*)win, 0, 0, BOX_DRAWINGS_DOUBLE_VERTICAL, max_y);
        _mvwvline((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_DOUBLE_VERTICAL, max_y);

        _mvwaddch((WINDOW*)win, 0, 0, BOX_DRAWINGS_DOUBLE_DOWN_AND_RIGHT);
        _mvwaddch((WINDOW*)win, 0, max_x - 1, BOX_DRAWINGS_DOUBLE_DOWN_AND_LEFT);
        _mvwaddch((WINDOW*)win, max_y - 1, 0, BOX_DRAWINGS_DOUBLE_UP_AND_RIGHT);
        _mvwaddch((WINDOW*)win, max_y - 1, max_x - 1, BOX_DRAWINGS_DOUBLE_UP_AND_LEFT);
    }
}

#endif
