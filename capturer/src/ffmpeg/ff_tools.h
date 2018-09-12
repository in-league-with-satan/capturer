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

#ifndef FF_TOOLS_H
#define FF_TOOLS_H

#include <QString>
#include <QMetaType>

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
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavdevice/avdevice.h>
#include <libavutil/bswap.h>
}

Q_DECLARE_METATYPE(AVRational)

const int alignment=32;

void initLibAV();

QString ffErrorString(int code);
AVFrame *alloc_frame(AVPixelFormat pix_fmt, int width, int height, bool alloc_buffer=true);
AVPixelFormat correctPixelFormat(AVPixelFormat fmt);

QString versionlibavutil();
QString versionlibavcodec();
QString versionlibavformat();
QString versionlibavfilter();
QString versionlibswscale();
QString versionlibswresample();

bool checkEncoder(const QString &encoder_name, const uint64_t &pixel_format);
bool checkEncoder(const QString &encoder_name, const AVPixelFormat &pixel_format);
bool isHighBitDepthBuild();

bool operator==(const AVRational &l, const AVRational &r);

#endif // FF_TOOLS_H
