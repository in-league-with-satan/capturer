#include "ffmpeg_tools.h"

#include "ffmpeg_format_converter.h"

namespace FF {

class Context
{
public:
    Context() {
        convert_context=nullptr;
        av_frame_src=nullptr;
        av_frame_dst=nullptr;
    }

    void free() {
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

    SwsContext *convert_context;
    AVFrame *av_frame_src;
    AVFrame *av_frame_dst;
};

}

FF::FormatConverter::FormatConverter()
{
    context=new Context();
}

FF::FormatConverter::~FormatConverter()
{
    context->free();

    delete context;
}

bool FF::FormatConverter::setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst, bool use_internal_frames, FF::FormatConverter::Filter::T filter)
{
    if(this->format_src==format_src && this->format_dst==format_dst
            && this->resolution_src==resolution_src && this->resolution_dst==resolution_dst
            && this->filter==filter && this->use_internal_frames==use_internal_frames)
        return true;

    context->free();

    this->use_internal_frames=use_internal_frames;

    this->format_src=format_src;
    this->format_dst=format_dst;

    this->resolution_src=resolution_src;
    this->resolution_dst=resolution_dst;

    this->filter=filter;

    if(use_internal_frames) {
        context->av_frame_src=alloc_frame(format_src, resolution_src.width(), resolution_src.height());

        if(!context->av_frame_src) {
            qCritical() << "Could not allocate video frame";
            return false;
        }

        context->av_frame_dst=alloc_frame(format_dst, resolution_dst.width(), resolution_dst.height());

        if(!context->av_frame_dst) {
            qCritical() << "Could not allocate video frame";
            return false;
        }
    }

    context->convert_context=sws_getContext(resolution_src.width(), resolution_src.height(),
                                            format_src,
                                            resolution_dst.width(), resolution_dst.height(),
                                            format_dst,
                                            filter, nullptr, nullptr, nullptr);
    return true;
}

void FF::FormatConverter::convert(QByteArray *src, QByteArray *dst)
{
    if(!context->av_frame_src)
        return;

//    av_image_fill_arrays(context->av_frame_src->data, context->av_frame_src->linesize, (const uint8_t*)src->data(), format_src, resolution_src.width(), resolution_src.height(), alignment);

//    convert(context->av_frame_src, context->av_frame_dst);

//    int buf_size=av_image_get_buffer_size(format_dst, resolution_dst.width(), resolution_dst.height(), alignment);

//    if(dst->size()!=buf_size)
//        dst->resize(buf_size);

//    av_image_copy_to_buffer((uint8_t*)dst->data(), dst->size(), context->av_frame_dst->data, context->av_frame_dst->linesize, format_dst, resolution_dst.width(), resolution_dst.width(), alignment);

     byteArrayToAvFrame(src, context->av_frame_src);

     convert(context->av_frame_src, context->av_frame_dst);

     avFrameToByteArray(context->av_frame_dst, dst);
}

void FF::FormatConverter::convert(AVFrame *src, AVFrame *dst)
{
    sws_scale(context->convert_context, src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
}
