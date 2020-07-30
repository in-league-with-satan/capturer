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

#ifndef AV_FRAME_SHARED_PTR_H
#define AV_FRAME_SHARED_PTR_H

#include <QtGlobal>

#include <memory>

#include "ff_tools.h"

class AVFrameSP
{
    AVFrameSP(AVPixelFormat pixel_format, int width, int height, bool allocate_buffer);

public:
    typedef std::shared_ptr<AVFrameSP> ptr;

    ~AVFrameSP();

    static ptr make(AVPixelFormat pixel_format, int width, int height, bool allocate_buffer=true);

    AVFrame *d=nullptr;
    AVRational time_base={};
};

#endif // AV_FRAME_SHARED_PTR_H
