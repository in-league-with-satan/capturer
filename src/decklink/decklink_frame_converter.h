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
