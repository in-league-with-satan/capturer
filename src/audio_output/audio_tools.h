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

void channelsRemap16(void *data, size_t size);
void channelsRemap32(void *data, size_t size);

void channelsRemap16(QByteArray *ba_data);
void channelsRemap32(QByteArray *ba_data);

void mix8channelsTo2(QByteArray *ba_src, QByteArray *ba_dst);

void mix8channelsTo6(QByteArray *ba_src, QByteArray *ba_dst);


#endif // AUDIO_TOOLS_H
