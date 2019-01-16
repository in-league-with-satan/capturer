/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef DECODE_FROM_210_H
#define DECODE_FROM_210_H

#include "ff_tools.h"
#include "frame.h"

class Context;

class DecodeFrom210 {
public:
    DecodeFrom210();
    ~DecodeFrom210();

    struct Format {
        enum T {
            Disabled,
            V210,
            R210,
            V410
        };
    };

    static AVPixelFormat v210PixelFormat();
    static AVPixelFormat r210PixelFormat();
    static AVPixelFormat v410PixelFormat();

    bool convert(Format::T format, uint8_t *data, int size, int width, int height, AVFrame *frame);
    Frame::ptr convert(Format::T format, Frame::ptr frame);

    Format::T format() const;

private:
    Context *d;
};

#endif // DECODE_FROM_210_H
