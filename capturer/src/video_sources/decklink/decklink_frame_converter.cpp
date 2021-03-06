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

#include "decklink_tools.h"

#include "decklink_frame_converter.h"


DecklinkFrameConverter::DecklinkFrameConverter()
    : video_converter(nullptr)
{
}

DecklinkFrameConverter::~DecklinkFrameConverter()
{
#ifdef LIB_DECKLINK

    if(video_converter)
        video_converter->Release();

#endif
}

void DecklinkFrameConverter::init(BMDPixelFormat src_format, QSize src_size, BMDPixelFormat dst_format, QSize dst_size)
{
#ifdef LIB_DECKLINK

    if(!video_converter)
        video_converter=CreateVideoConversionInstance();

    if(frame_src.GetPixelFormat()!=src_format || frame_src.getSize()!=src_size)
        frame_src.init(src_size, src_format, bmdFrameFlagDefault, false);

    if(frame_dst.GetPixelFormat()!=dst_format || frame_dst.getSize()!=dst_size)
        frame_dst.init(dst_size, dst_format, bmdFrameFlagDefault, false);

#endif
}

int64_t DecklinkFrameConverter::convert(void *src_data, void *dst_data)
{
#ifdef LIB_DECKLINK

    if(!video_converter)
        return 1;

    frame_src.buffer=src_data;
    frame_dst.buffer=dst_data;

    return video_converter->ConvertFrame(&frame_src, &frame_dst);

#endif

    return 0;
}
