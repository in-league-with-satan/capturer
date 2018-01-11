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

#ifndef AUDIO_NORMALIZATION_H
#define AUDIO_NORMALIZATION_H

#include <QWidget>

class AudioNormalization : public QObject
{
    Q_OBJECT

public:
    explicit AudioNormalization(QObject *parent=0);

    void proc(QByteArray *data, int channels);

    void setUpdateTime(uint16_t ms);
    void setGainChangeStep(double value);
    void setMaximumLevelPercentage(double value);

private:
    qint64 last_update_timestamp;
    qint64 update_time;

    double gain_factor;
    double gain_change_step;
    double maximum_level_percentage;

    int32_t max_level;

signals:
    void gainFactorChanged(double value);
};

#endif // AUDIO_NORMALIZATION_H
