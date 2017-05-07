#ifndef FF_TOOLS_H
#define FF_TOOLS_H

#include <QString>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libavutil/samplefmt.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

const int alignment=32;

QString ffErrorString(int code);
AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height);
AVPixelFormat correctPixelFormat(AVPixelFormat fmt);

#endif // FF_TOOLS_H
