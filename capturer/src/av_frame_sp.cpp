/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QDebug>

#include "av_frame_sp.h"

AVFrameSP::ptr AVFrameSP::make(AVPixelFormat pixel_format, int width, int height, bool allocate_buffer)
{
    return ptr(new AVFrameSP(pixel_format, width, height, allocate_buffer));
}

AVFrameSP::AVFrameSP(AVPixelFormat pixel_format, int width, int height, bool allocate_buffer)
{
    d=alloc_frame(pixel_format, width, height, allocate_buffer);
}

AVFrameSP::~AVFrameSP()
{
    av_frame_free(&d);
}
