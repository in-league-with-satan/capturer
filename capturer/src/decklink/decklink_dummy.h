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

#ifndef DECKLINK_DUMMY_H
#define DECKLINK_DUMMY_H


#include "capture.h"
#include "glazing_ribbon.h"

class DeckLinkDummy : public DeckLinkCapture
{
    Q_OBJECT

public:
    explicit DeckLinkDummy(bool frame_counter=false, int frame_height=1080, QObject *parent=0);
    virtual ~DeckLinkDummy();

    bool isRunning() const;
    bool gotSignal() const;
    bool sourceRGB() const;
    bool source10Bit() const;

protected:
    void run();

public slots:
    void captureStart();
    void captureStop();

private:
    GlazingRibbon glazing_ribbon;

    qint64 time_last_frame=0;

    bool frame_counter;
    QSize frame_size;
};

#endif // DECKLINK_DUMMY_H
