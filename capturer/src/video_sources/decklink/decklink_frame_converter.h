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

#ifndef DL_FRAME_CONVERTER_H
#define DL_FRAME_CONVERTER_H

#include "decklink_video_frame.h"

class IDeckLinkVideoConversion;

class DecklinkFrameConverter
{
public:
    DecklinkFrameConverter();
    ~DecklinkFrameConverter();

    void init(BMDPixelFormat src_format, QSize src_size, BMDPixelFormat dst_format, QSize dst_size);

    int64_t convert(void *src_data, void *dst_data);

private:
    DeckLinkVideoFrame frame_src;
    DeckLinkVideoFrame frame_dst;

    IDeckLinkVideoConversion *video_converter;


};

#endif // DL_FRAME_CONVERTER_H
