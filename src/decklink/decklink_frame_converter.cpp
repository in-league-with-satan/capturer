#include "decklink_tools.h"

#include "decklink_frame_converter.h"


DecklinkFrameConverter::DecklinkFrameConverter()
    : video_converter(nullptr)
{
}

DecklinkFrameConverter::~DecklinkFrameConverter()
{
    if(video_converter)
        video_converter->Release();
}

void DecklinkFrameConverter::init(BMDPixelFormat src_format, QSize src_size, BMDPixelFormat dst_format, QSize dst_size)
{
    if(!video_converter)
        video_converter=CreateVideoConversionInstance();

    frame_src.init(src_size, src_format, bmdFrameFlagDefault, false);
    frame_dst.init(dst_size, dst_format, bmdFrameFlagDefault, false);
}

int64_t DecklinkFrameConverter::convert(void *src_data, void *dst_data)
{
    if(!video_converter)
        return 1;

    frame_src.buffer=src_data;
    frame_dst.buffer=dst_data;

    return video_converter->ConvertFrame(&frame_src, &frame_dst);
}
