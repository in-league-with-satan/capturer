/******************************************************************************

Copyright © 2018, 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef AUDIO_LEVEL_WIDGET_H
#define AUDIO_LEVEL_WIDGET_H

#include <QWidget>

class AudioLevelWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AudioLevelWidget(QWidget *parent=0);

public slots:
    void write(const QByteArray &data, int channels, int sample_size);

protected:
    void mouseDoubleClickEvent(QMouseEvent*);
    void paintEvent(QPaintEvent *event);

private slots:

private:
    int32_t level[8];

    bool sample_size_16;

    bool disabled=false;
};


#endif // AUDIO_LEVEL_WIDGET_H
