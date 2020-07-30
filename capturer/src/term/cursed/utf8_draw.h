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

#ifndef UTF8_DRAW_H
#define UTF8_DRAW_H

#include <QString>


namespace Utf8Draw
{
extern const std::wstring BOX_DRAWINGS_LIGHT_HORIZONTAL;                            // ─

extern const std::wstring BOX_DRAWINGS_HEAVY_HORIZONTAL;                            // ━

extern const std::wstring BOX_DRAWINGS_LIGHT_VERTICAL;                              // │

extern const std::wstring BOX_DRAWINGS_HEAVY_VERTICAL;                              // ┃

extern const std::wstring BOX_DRAWINGS_LIGHT_TRIPLE_DASH_HORIZONTAL;                // ┄

extern const std::wstring BOX_DRAWINGS_HEAVY_TRIPLE_DASH_HORIZONTAL;                // ┅

extern const std::wstring BOX_DRAWINGS_LIGHT_TRIPLE_DASH_VERTICAL;                  // ┆

extern const std::wstring BOX_DRAWINGS_HEAVY_TRIPLE_DASH_VERTICAL;                  // ┇

extern const std::wstring BOX_DRAWINGS_LIGHT_QUADRUPLE_DASH_HORIZONTAL;             // ┈

extern const std::wstring BOX_DRAWINGS_HEAVY_QUADRUPLE_DASH_HORIZONTAL;             // ┉

extern const std::wstring BOX_DRAWINGS_LIGHT_QUADRUPLE_DASH_VERTICAL;               // ┊

extern const std::wstring BOX_DRAWINGS_HEAVY_QUADRUPLE_DASH_VERTICAL;               // ┋

extern const std::wstring BOX_DRAWINGS_LIGHT_DOWN_AND_RIGHT;                        // ┌

extern const std::wstring BOX_DRAWINGS_DOWN_LIGHT_AND_RIGHT_HEAVY;                  // ┍

extern const std::wstring BOX_DRAWINGS_DOWN_HEAVY_AND_RIGHT_LIGHT;                  // ┎

extern const std::wstring BOX_DRAWINGS_HEAVY_DOWN_AND_RIGHT;                        // ┏

extern const std::wstring BOX_DRAWINGS_LIGHT_DOWN_AND_LEFT;                         // ┐

extern const std::wstring BOX_DRAWINGS_DOWN_LIGHT_AND_LEFT_HEAVY;                   // ┑

extern const std::wstring BOX_DRAWINGS_DOWN_HEAVY_AND_LEFT_LIGHT;                   // ┒

extern const std::wstring BOX_DRAWINGS_HEAVY_DOWN_AND_LEFT;                         // ┓

extern const std::wstring BOX_DRAWINGS_LIGHT_UP_AND_RIGHT;                          // └

extern const std::wstring BOX_DRAWINGS_UP_LIGHT_AND_RIGHT_HEAVY;                    // ┕

extern const std::wstring BOX_DRAWINGS_UP_HEAVY_AND_RIGHT_LIGHT;                    // ┖

extern const std::wstring BOX_DRAWINGS_HEAVY_UP_AND_RIGHT;                          // ┗

extern const std::wstring BOX_DRAWINGS_LIGHT_UP_AND_LEFT;                           // ┘

extern const std::wstring BOX_DRAWINGS_UP_LIGHT_AND_LEFT_HEAVY;                     // ┙

extern const std::wstring BOX_DRAWINGS_UP_HEAVY_AND_LEFT_LIGHT;                     // ┚

extern const std::wstring BOX_DRAWINGS_HEAVY_UP_AND_LEFT;                           // ┛

extern const std::wstring BOX_DRAWINGS_LIGHT_VERTICAL_AND_RIGHT;                    // ├

extern const std::wstring BOX_DRAWINGS_VERTICAL_LIGHT_AND_RIGHT_HEAVY;              // ┝

extern const std::wstring BOX_DRAWINGS_UP_HEAVY_AND_RIGHT_DOWN_LIGHT;               // ┞

extern const std::wstring BOX_DRAWINGS_DOWN_HEAVY_AND_RIGHT_UP_LIGHT;               // ┟

extern const std::wstring BOX_DRAWINGS_VERTICAL_HEAVY_AND_RIGHT_LIGHT;              // ┠

extern const std::wstring BOX_DRAWINGS_DOWN_LIGHT_AND_RIGHT_UP_HEAVY;               // ┡

extern const std::wstring BOX_DRAWINGS_UP_LIGHT_AND_RIGHT_DOWN_HEAVY;               // ┢

extern const std::wstring BOX_DRAWINGS_HEAVY_VERTICAL_AND_RIGHT;                    // ┣

extern const std::wstring BOX_DRAWINGS_LIGHT_VERTICAL_AND_LEFT;                     // ┤

extern const std::wstring BOX_DRAWINGS_VERTICAL_LIGHT_AND_LEFT_HEAVY;               // ┥

extern const std::wstring BOX_DRAWINGS_UP_HEAVY_AND_LEFT_DOWN_LIGHT;                // ┦

extern const std::wstring BOX_DRAWINGS_DOWN_HEAVY_AND_LEFT_UP_LIGHT;                // ┧

extern const std::wstring BOX_DRAWINGS_VERTICAL_HEAVY_AND_LEFT_LIGHT;               // ┨

extern const std::wstring BOX_DRAWINGS_DOWN_LIGHT_AND_LEFT_UP_HEAVY;                // ┩

extern const std::wstring BOX_DRAWINGS_UP_LIGHT_AND_LEFT_DOWN_HEAVY;                // ┪

extern const std::wstring BOX_DRAWINGS_HEAVY_VERTICAL_AND_LEFT;                     // ┫

extern const std::wstring BOX_DRAWINGS_LIGHT_DOWN_AND_HORIZONTAL;                   // ┬

extern const std::wstring BOX_DRAWINGS_LEFT_HEAVY_AND_RIGHT_DOWN_LIGHT;             // ┭

extern const std::wstring BOX_DRAWINGS_RIGHT_HEAVY_AND_LEFT_DOWN_LIGHT;             // ┮

extern const std::wstring BOX_DRAWINGS_DOWN_LIGHT_AND_HORIZONTAL_HEAVY;             // ┯

extern const std::wstring BOX_DRAWINGS_DOWN_HEAVY_AND_HORIZONTAL_LIGHT;             // ┰

extern const std::wstring BOX_DRAWINGS_RIGHT_LIGHT_AND_LEFT_DOWN_HEAVY;             // ┱

extern const std::wstring BOX_DRAWINGS_LEFT_LIGHT_AND_RIGHT_DOWN_HEAVY;             // ┲

extern const std::wstring BOX_DRAWINGS_HEAVY_DOWN_AND_HORIZONTAL;                   // ┳

extern const std::wstring BOX_DRAWINGS_LIGHT_UP_AND_HORIZONTAL;                     // ┴

extern const std::wstring BOX_DRAWINGS_LEFT_HEAVY_AND_RIGHT_UP_LIGHT;               // ┵

extern const std::wstring BOX_DRAWINGS_RIGHT_HEAVY_AND_LEFT_UP_LIGHT;               // ┶

extern const std::wstring BOX_DRAWINGS_UP_LIGHT_AND_HORIZONTAL_HEAVY;               // ┷

extern const std::wstring BOX_DRAWINGS_UP_HEAVY_AND_HORIZONTAL_LIGHT;               // ┸

extern const std::wstring BOX_DRAWINGS_RIGHT_LIGHT_AND_LEFT_UP_HEAVY;               // ┹

extern const std::wstring BOX_DRAWINGS_LEFT_LIGHT_AND_RIGHT_UP_HEAVY;               // ┺

extern const std::wstring BOX_DRAWINGS_HEAVY_UP_AND_HORIZONTAL;                     // ┻

extern const std::wstring BOX_DRAWINGS_LIGHT_VERTICAL_AND_HORIZONTAL;               // ┼

extern const std::wstring BOX_DRAWINGS_LEFT_HEAVY_AND_RIGHT_VERTICAL_LIGHT;         // ┽

extern const std::wstring BOX_DRAWINGS_RIGHT_HEAVY_AND_LEFT_VERTICAL_LIGHT;         // ┾

extern const std::wstring BOX_DRAWINGS_VERTICAL_LIGHT_AND_HORIZONTAL_HEAVY;         // ┿

extern const std::wstring BOX_DRAWINGS_UP_HEAVY_AND_DOWN_HORIZONTAL_LIGHT;          // ╀

extern const std::wstring BOX_DRAWINGS_DOWN_HEAVY_AND_UP_HORIZONTAL_LIGHT;          // ╁

extern const std::wstring BOX_DRAWINGS_VERTICAL_HEAVY_AND_HORIZONTAL_LIGHT;         // ╂

extern const std::wstring BOX_DRAWINGS_LEFT_UP_HEAVY_AND_RIGHT_DOWN_LIGHT;          // ╃

extern const std::wstring BOX_DRAWINGS_RIGHT_UP_HEAVY_AND_LEFT_DOWN_LIGHT;          // ╄

extern const std::wstring BOX_DRAWINGS_LEFT_DOWN_HEAVY_AND_RIGHT_UP_LIGHT;          // ╅

extern const std::wstring BOX_DRAWINGS_RIGHT_DOWN_HEAVY_AND_LEFT_UP_LIGHT;          // ╆

extern const std::wstring BOX_DRAWINGS_DOWN_LIGHT_AND_UP_HORIZONTAL_HEAVY;          // ╇

extern const std::wstring BOX_DRAWINGS_UP_LIGHT_AND_DOWN_HORIZONTAL_HEAVY;          // ╈

extern const std::wstring BOX_DRAWINGS_RIGHT_LIGHT_AND_LEFT_VERTICAL_HEAVY;         // ╉

extern const std::wstring BOX_DRAWINGS_LEFT_LIGHT_AND_RIGHT_VERTICAL_HEAVY;         // ╊

extern const std::wstring BOX_DRAWINGS_HEAVY_VERTICAL_AND_HORIZONTAL;               // ╋

extern const std::wstring BOX_DRAWINGS_LIGHT_DOUBLE_DASH_HORIZONTAL;                // ╌

extern const std::wstring BOX_DRAWINGS_HEAVY_DOUBLE_DASH_HORIZONTAL;                // ╍

extern const std::wstring BOX_DRAWINGS_LIGHT_DOUBLE_DASH_VERTICAL;                  // ╎

extern const std::wstring BOX_DRAWINGS_HEAVY_DOUBLE_DASH_VERTICAL;                  // ╏

extern const std::wstring BOX_DRAWINGS_DOUBLE_HORIZONTAL;                           // ═

extern const std::wstring BOX_DRAWINGS_DOUBLE_VERTICAL;                             // ║

extern const std::wstring BOX_DRAWINGS_DOWN_SINGLE_AND_RIGHT_DOUBLE;                // ╒

extern const std::wstring BOX_DRAWINGS_DOWN_DOUBLE_AND_RIGHT_SINGLE;                // ╓

extern const std::wstring BOX_DRAWINGS_DOUBLE_DOWN_AND_RIGHT;                       // ╔

extern const std::wstring BOX_DRAWINGS_DOWN_SINGLE_AND_LEFT_DOUBLE;                 // ╕

extern const std::wstring BOX_DRAWINGS_DOWN_DOUBLE_AND_LEFT_SINGLE;                 // ╖

extern const std::wstring BOX_DRAWINGS_DOUBLE_DOWN_AND_LEFT;                        // ╗

extern const std::wstring BOX_DRAWINGS_UP_SINGLE_AND_RIGHT_DOUBLE;                  // ╘

extern const std::wstring BOX_DRAWINGS_UP_DOUBLE_AND_RIGHT_SINGLE;                  // ╙

extern const std::wstring BOX_DRAWINGS_DOUBLE_UP_AND_RIGHT;                         // ╚

extern const std::wstring BOX_DRAWINGS_UP_SINGLE_AND_LEFT_DOUBLE;                   // ╛

extern const std::wstring BOX_DRAWINGS_UP_DOUBLE_AND_LEFT_SINGLE;                   // ╜

extern const std::wstring BOX_DRAWINGS_DOUBLE_UP_AND_LEFT;                          // ╝

extern const std::wstring BOX_DRAWINGS_VERTICAL_SINGLE_AND_RIGHT_DOUBLE;            // ╞

extern const std::wstring BOX_DRAWINGS_VERTICAL_DOUBLE_AND_RIGHT_SINGLE;            // ╟

extern const std::wstring BOX_DRAWINGS_DOUBLE_VERTICAL_AND_RIGHT;                   // ╠

extern const std::wstring BOX_DRAWINGS_VERTICAL_SINGLE_AND_LEFT_DOUBLE;             // ╡

extern const std::wstring BOX_DRAWINGS_VERTICAL_DOUBLE_AND_LEFT_SINGLE;             // ╢

extern const std::wstring BOX_DRAWINGS_DOUBLE_VERTICAL_AND_LEFT;                    // ╣

extern const std::wstring BOX_DRAWINGS_DOWN_SINGLE_AND_HORIZONTAL_DOUBLE;           // ╤

extern const std::wstring BOX_DRAWINGS_DOWN_DOUBLE_AND_HORIZONTAL_SINGLE;           // ╥

extern const std::wstring BOX_DRAWINGS_DOUBLE_DOWN_AND_HORIZONTAL;                  // ╦

extern const std::wstring BOX_DRAWINGS_UP_SINGLE_AND_HORIZONTAL_DOUBLE;             // ╧

extern const std::wstring BOX_DRAWINGS_UP_DOUBLE_AND_HORIZONTAL_SINGLE;             // ╨

extern const std::wstring BOX_DRAWINGS_DOUBLE_UP_AND_HORIZONTAL;                    // ╩

extern const std::wstring BOX_DRAWINGS_VERTICAL_SINGLE_AND_HORIZONTAL_DOUBLE;       // ╪

extern const std::wstring BOX_DRAWINGS_VERTICAL_DOUBLE_AND_HORIZONTAL_SINGLE;       // ╫

extern const std::wstring BOX_DRAWINGS_DOUBLE_VERTICAL_AND_HORIZONTAL;              // ╬

extern const std::wstring BOX_DRAWINGS_LIGHT_ARC_DOWN_AND_RIGHT;                    // ╭

extern const std::wstring BOX_DRAWINGS_LIGHT_ARC_DOWN_AND_LEFT;                     // ╮

extern const std::wstring BOX_DRAWINGS_LIGHT_ARC_UP_AND_LEFT;                       // ╯

extern const std::wstring BOX_DRAWINGS_LIGHT_ARC_UP_AND_RIGHT;                      // ╰

extern const std::wstring BOX_DRAWINGS_LIGHT_DIAGONAL_UPPER_RIGHT_TO_LOWER_LEFT;    // ╱

extern const std::wstring BOX_DRAWINGS_LIGHT_DIAGONAL_UPPER_LEFT_TO_LOWER_RIGHT;    // ╲

extern const std::wstring BOX_DRAWINGS_LIGHT_DIAGONAL_CROSS;                        // ╳

extern const std::wstring BOX_DRAWINGS_LIGHT_LEFT;                                  // ╴

extern const std::wstring BOX_DRAWINGS_LIGHT_UP;                                    // ╵

extern const std::wstring BOX_DRAWINGS_LIGHT_RIGHT;                                 // ╶

extern const std::wstring BOX_DRAWINGS_LIGHT_DOWN;                                  // ╷

extern const std::wstring BOX_DRAWINGS_HEAVY_LEFT;                                  // ╸

extern const std::wstring BOX_DRAWINGS_HEAVY_UP;                                    // ╹

extern const std::wstring BOX_DRAWINGS_HEAVY_RIGHT;                                 // ╺

extern const std::wstring BOX_DRAWINGS_HEAVY_DOWN;                                  // ╻

extern const std::wstring BOX_DRAWINGS_LIGHT_LEFT_AND_HEAVY_RIGHT;                  // ╼

extern const std::wstring BOX_DRAWINGS_LIGHT_UP_AND_HEAVY_DOWN;                     // ╽

extern const std::wstring BOX_DRAWINGS_HEAVY_LEFT_AND_LIGHT_RIGHT;                  // ╾

extern const std::wstring BOX_DRAWINGS_HEAVY_UP_AND_LIGHT_DOWN;                     // ╿

extern const std::wstring UPPER_HALF_BLOCK;                                         // ▀

extern const std::wstring LOWER_ONE_EIGHTH_BLOCK;                                   // ▁

extern const std::wstring LOWER_ONE_QUARTER_BLOCK;                                  // ▂

extern const std::wstring LOWER_THREE_EIGHTHS_BLOCK;                                // ▃

extern const std::wstring LOWER_HALF_BLOCK;                                         // ▄

extern const std::wstring LOWER_FIVE_EIGHTHS_BLOCK;                                 // ▅

extern const std::wstring LOWER_THREE_QUARTERS_BLOCK;                               // ▆

extern const std::wstring LOWER_SEVEN_EIGHTHS_BLOCK;                                // ▇

extern const std::wstring FULL_BLOCK;                                               // █

extern const std::wstring LEFT_SEVEN_EIGHTHS_BLOCK;                                 // ▉

extern const std::wstring LEFT_THREE_QUARTERS_BLOCK;                                // ▊

extern const std::wstring LEFT_FIVE_EIGHTHS_BLOCK;                                  // ▋

extern const std::wstring LEFT_HALF_BLOCK;                                          // ▌

extern const std::wstring LEFT_THREE_EIGHTHS_BLOCK;                                 // ▍

extern const std::wstring LEFT_ONE_QUARTER_BLOCK;                                   // ▎

extern const std::wstring LEFT_ONE_EIGHTH_BLOCK;                                    // ▏

extern const std::wstring RIGHT_HALF_BLOCK;                                         // ▐

extern const std::wstring LIGHT_SHADE;                                              // ░

extern const std::wstring MEDIUM_SHADE;                                             // ▒

extern const std::wstring DARK_SHADE;                                               // ▓

extern const std::wstring UPPER_ONE_EIGHTH_BLOCK;                                   // ▔

extern const std::wstring RIGHT_ONE_EIGHTH_BLOCK;                                   // ▕


enum LineType {
    lt_light,
    lt_heavy,
    lt_double
};

void _mvwhline(void *win, int y, int x, std::wstring ch, int n);
void _mvwvline(void *win, int y, int x, std::wstring ch, int n);
void _mvwaddch(void *win, int y, int x, std::wstring ch);
void _box(void *win, LineType t=lt_light);

}


#endif // UTF8_DRAW_H
