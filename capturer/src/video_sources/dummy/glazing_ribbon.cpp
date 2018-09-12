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

#include <math.h>

#include "glazing_ribbon.h"

const double m_pi=std::atan(1.)*4.;
const QColor background_color=QColor(10, 10, 10);

GlazingRibbon::GlazingRibbon()
{
}

void GlazingRibbon::init(const QSize &frame_size, uint8_t points_amount, int pen_width, bool antialiasing, QPainter::CompositionMode composition_mode)
{
    this->pen_width=pen_width;
    this->antialiasing=antialiasing;
    this->composition_mode=composition_mode;

    frame=QImage(frame_size, QImage::Format_RGB888);

    frame.fill(background_color);


    const int color_start=64;
    const int color_stop=255 - color_start;

    point.resize(points_amount);

    for(int i=0; i<points_amount; ++i) {
        Point p;

        p.color=QColor(rand()%color_stop + color_start, rand()%color_stop + color_start, rand()%color_stop + color_start, rand()%color_stop + color_start);
        p.speed=rand()%2 + 2;
        p.angle=rand()%360;
        p.pos=QPoint(rand()%frame_size.width(), rand()%frame_size.height());

        point[i]=p;
    }
}

void GlazingRibbon::showFrameCounter(bool value)
{
    show_frame_counter=value;
}

QImage GlazingRibbon::next()
{
    if(frame.isNull())
        return QImage();

    QPainter painter(&frame);

    painter.setRenderHint(QPainter::Antialiasing, antialiasing);

    painter.fillRect(frame.rect(), QColor(0, 0, 0, 13));
    // painter.fillRect(frame.rect(), QColor(0, 0, 0, 33));

    painter.setCompositionMode(composition_mode);

    QPen pen;
    pen.setWidth(pen_width);

    const int width=frame.width();
    const int height=frame.height();

    const double max_distance=(width + height)*.13;

    const int points_amount=point.size();

    for(int i=0; i<points_amount; ++i) {
        Point &p1=point[i];

        for(int j=0; j<points_amount; ++j) {
            if(i!=j) {
                Point &p2=point[j];

                double yd=p2.pos.y() - p1.pos.y();
                double xd=p2.pos.x() - p1.pos.x();

                double distance=sqrt(xd*xd + yd*yd);

                if(distance<max_distance) {
                    pen.setColor(p1.color);

                    painter.setPen(pen);

                    painter.drawLine(p1.pos, p2.pos);
                }
            }
        }
    }

    for(int i=0; i<points_amount; ++i) {
        Point &p1=point[i];

        painter.setPen(Qt::white);

        painter.fillRect(p1.pos.x() - 1, p1.pos.y() - 1, 3, 3, Qt::white);

        p1.pos.setX(p1.pos.x() + p1.speed*cos(p1.angle*m_pi/180.));
        p1.pos.setY(p1.pos.y() + p1.speed*sin(p1.angle*m_pi/180.));

        if(p1.pos.x()<0)
            p1.pos.setX(width);

        if(p1.pos.x()>width)
            p1.pos.setX(0);

        if(p1.pos.y()<0)
            p1.pos.setY(height);

        if(p1.pos.y()>height)
            p1.pos.setY(0);
    }

    if(show_frame_counter) {
        pen.setColor(Qt::white);

        painter.setPen(pen);


        QFont font;

        font.setPixelSize(max_distance*.1);

        painter.setFont(font);


        const QFontMetrics fm(font);

        const QString text=QString::number(frame_counter++).rightJustified(5, '0');

        const QRect rect=QRect(width*.1, height*.16, fm.width(text), fm.height());

        painter.fillRect(rect, background_color);

        painter.drawText(rect, text);
    }

    return frame.copy();
}
