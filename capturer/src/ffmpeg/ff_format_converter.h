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

#ifndef FF_FORMAT_CONVERTER_H
#define FF_FORMAT_CONVERTER_H

#include <QSize>

#include "av_frame_sp.h"
#include "ff_tools.h"

class AVFrame;
class SwsContext;

class FFFormatConverter
{
public:
    FFFormatConverter();
    ~FFFormatConverter();

    struct Filter {
        enum T {
            cNull=0,
            cSWS_FAST_BILINEAR=1,
            cSWS_BILINEAR=2,
            cSWS_BICUBIC=4,
            cSWS_X=8,
            cSWS_POINT=0x10,
            cSWS_AREA=0x20,
            cSWS_BICUBLIN=0x40,
            cSWS_GAUSS=0x80,
            cSWS_SINC=0x100,
            cSWS_LANCZOS=0x200,
            cSWS_SPLINE=0x400
        };
    };

    bool setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst, Filter::T filter=Filter::cNull);

    bool compareParams(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst, Filter::T filter=Filter::cNull);

    AVPixelFormat formatSrc() const;
    AVPixelFormat formatDst() const;

    bool convert(AVFrame *src, AVFrame *dst);

    AVFrameSP::ptr convert(AVFrame *src);

private:
    void free();

    AVPixelFormat format_src;
    AVPixelFormat format_dst;

    QSize resolution_src;
    QSize resolution_dst;

    Filter::T filter;

    SwsContext *convert_context;
};

#endif // FF_FORMAT_CONVERTER_H
