#include <QDebug>

#include "ff_tools.h"

#include "ff_format_converter.h"

FFFormatConverter::FFFormatConverter()
    : convert_context(nullptr)
    , av_frame_src(nullptr)
    , av_frame_dst(nullptr)
{
}

FFFormatConverter::~FFFormatConverter()
{
    free();
}

bool FFFormatConverter::setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst, bool use_internal_frames, FFFormatConverter::Filter::T filter)
{
    if(this->format_src==format_src && this->format_dst==format_dst
            && this->resolution_src==resolution_src && this->resolution_dst==resolution_dst
            && this->filter==filter && this->use_internal_frames==use_internal_frames)
        return true;

    free();

    this->use_internal_frames=use_internal_frames;

    this->format_src=format_src;
    this->format_dst=format_dst;

    this->resolution_src=resolution_src;
    this->resolution_dst=resolution_dst;

    this->filter=filter;

    if(use_internal_frames) {
        av_frame_src=alloc_frame(format_src, resolution_src.width(), resolution_src.height());

        if(!av_frame_src) {
            qCritical() << "Could not allocate video frame";
            return false;
        }

        av_frame_dst=alloc_frame(format_dst, resolution_dst.width(), resolution_dst.height());

        if(!av_frame_dst) {
            qCritical() << "Could not allocate video frame";
            return false;
        }
    }

    convert_context=sws_getContext(resolution_src.width(), resolution_src.height(),
                                   format_src,
                                   resolution_dst.width(), resolution_dst.height(),
                                   format_dst,
                                   filter, nullptr, nullptr, nullptr);

    return true;
}

void FFFormatConverter::convert(QByteArray *src, QByteArray *dst)
{
    if(!av_frame_src)
        return;

     av_image_fill_arrays(av_frame_src->data, av_frame_src->linesize, (const uint8_t*)src->constData(), format_src, resolution_src.width(), resolution_src.height(), alignment);

     convert(av_frame_src, av_frame_dst);

     int buf_size=av_image_get_buffer_size(format_dst, resolution_dst.width(), resolution_dst.height(), alignment);

     if(dst->size()!=buf_size)
         dst->resize(buf_size);

     av_image_copy_to_buffer((uint8_t*)dst->constData(), dst->size(), av_frame_dst->data, av_frame_dst->linesize, format_dst, resolution_dst.width(), resolution_dst.height(), alignment);
}

void FFFormatConverter::convert(AVFrame *src, AVFrame *dst)
{
    sws_scale(convert_context, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
}

void FFFormatConverter::free()
{
    if(av_frame_src) {
        av_frame_unref(av_frame_src);
        av_freep(&av_frame_src->data[0]);
        av_frame_free(&av_frame_src);
        av_frame_src=nullptr;
    }

    if(av_frame_dst) {
        av_frame_unref(av_frame_dst);
        av_freep(&av_frame_dst->data[0]);
        av_frame_free(&av_frame_dst);
        av_frame_dst=nullptr;
    }

    if(convert_context) {
        sws_freeContext(convert_context);
        convert_context=nullptr;
    }
}

