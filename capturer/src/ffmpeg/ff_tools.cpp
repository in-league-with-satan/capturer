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

#include <QDebug>

extern "C" {
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include "libswscale/swscale.h"
}

#include "ff_tools.h"


QString ffErrorString(int code)
{
    char buf[AV_ERROR_MAX_STRING_SIZE];

    av_strerror(code, buf, AV_ERROR_MAX_STRING_SIZE);

    return QString(buf);
}

AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height, bool alloc_buffer)
{
    AVFrame *av_frame;
    int ret;

    av_frame=av_frame_alloc();

    if(!av_frame)
        return nullptr;

    av_frame->format=pix_fmt;
    av_frame->width=width;
    av_frame->height=height;

    // allocate the buffers for the frame data
    if(alloc_buffer) {
        ret=av_frame_get_buffer(av_frame, alignment);

        if(ret<0) {
            qCritical() << "Could not allocate frame data";
            return nullptr;
        }
    }

    return av_frame;
}

AVPixelFormat correctPixelFormat(AVPixelFormat fmt)
{
    switch(fmt) {
    case AV_PIX_FMT_YUVJ420P: return AV_PIX_FMT_YUV420P;
    case AV_PIX_FMT_YUVJ422P: return AV_PIX_FMT_YUV422P;
    case AV_PIX_FMT_YUVJ444P: return AV_PIX_FMT_YUV444P;
    case AV_PIX_FMT_YUVJ440P: return AV_PIX_FMT_YUV440P;
    }

    return fmt;
}

bool operator==(const AVRational &l, const AVRational &r)
{
    return l.den==r.den && l.num==r.num;
}
