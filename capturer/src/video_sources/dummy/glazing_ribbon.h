/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef GLAZING_RIBBON_H
#define GLAZING_RIBBON_H

#include <QPainter>

class GlazingRibbon
{
public:
    GlazingRibbon();

    void init(const QSize &frame_size=QSize(640, 480), uint8_t points_amount=24, int pen_width=1, bool antialiasing=true, QPainter::CompositionMode composition_mode=QPainter::CompositionMode_HardLight);

    void showFrameCounter(bool value);

    QImage next();

private:
    struct Point {
        QPoint pos;
        int speed;
        double angle;
        QColor color;
    };

    int pen_width;

    bool antialiasing;

    bool show_frame_counter=false;
    uint16_t frame_counter=0;

    QPainter::CompositionMode composition_mode;

    QVector <Point> point;

    QImage frame;
};

#endif // GLAZING_RIBBON_H
