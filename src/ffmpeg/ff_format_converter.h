#ifndef FF_FORMAT_CONVERTER_H
#define FF_FORMAT_CONVERTER_H

#include <QByteArray>
#include <QImage>

class AVFrame;

#include "libavutil/pixfmt.h"

namespace FF {

class Context;

class FormatConverter
{
public:
    FormatConverter();
    ~FormatConverter();

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

    bool setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst, bool use_internal_frames=true, Filter::T filter=Filter::cNull);

    void convert(QByteArray *src, QByteArray *dst);

    void convert(AVFrame *src, AVFrame *dst);

private:
    AVPixelFormat format_src;
    AVPixelFormat format_dst;

    QSize resolution_src;
    QSize resolution_dst;

    Filter::T filter;

    bool use_internal_frames;


    Context *context;

};

}

#endif // FF_FORMAT_CONVERTER_H
