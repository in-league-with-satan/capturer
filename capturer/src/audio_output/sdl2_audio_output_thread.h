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

#ifndef SDL2_AUDIO_OUTPUT_THREAD_H
#define SDL2_AUDIO_OUTPUT_THREAD_H

#include "audio_output_interface.h"

class Sdl2AudioOutputThread : public AudioOutputInterface
{
    Q_OBJECT

public:
    Sdl2AudioOutputThread(QObject *parent=0);
    ~Sdl2AudioOutputThread();

    QByteArray ba_out_buffer;

public slots:

protected:
    virtual void run();

private:
    void onInputFrameArrived(QByteArray ba_data);
};

#endif // SDL2_AUDIO_OUTPUT_THREAD_H
