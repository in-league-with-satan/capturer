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

#ifndef AUDIO_TOOLS_H
#define AUDIO_TOOLS_H

#include <QByteArray>

template <typename T>
void channelsRemap(void *data, size_t size)
{
    T *ptr_data=(T*)data;

    for(int pos=0, csize=size/sizeof(T); pos<csize; pos+=8) {
        // swap center and lfe
        std::swap(ptr_data[pos + 2], ptr_data[pos + 3]);

        // swap side left and rear left
        std::swap(ptr_data[pos + 4], ptr_data[pos + 6]);

        // swap side right and rear right
        std::swap(ptr_data[pos + 5], ptr_data[pos + 7]);
    }
}

template <typename T>
void channelsRemapMagewell(void *data, size_t size)
{
    T *ptr_data=(T*)data;

    for(int pos=0, csize=size/sizeof(T); pos<csize; pos+=8) {
        std::swap(ptr_data[pos + 1], ptr_data[pos + 4]);
        std::swap(ptr_data[pos + 2], ptr_data[pos + 5]);
        std::swap(ptr_data[pos + 3], ptr_data[pos + 4]);
        std::swap(ptr_data[pos + 5], ptr_data[pos + 7]);
        std::swap(ptr_data[pos + 6], ptr_data[pos + 7]);
    }
}

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst);

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst);

#endif // AUDIO_TOOLS_H
