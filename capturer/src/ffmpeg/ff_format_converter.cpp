/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QDebug>

#include "ff_tools.h"

#include "ff_format_converter.h"

FFFormatConverter::FFFormatConverter()
    : convert_context(nullptr)
{
}

FFFormatConverter::~FFFormatConverter()
{
    free();
}

bool FFFormatConverter::setup(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                              int color_space_src, int color_space_dst, int color_range_src, int color_range_dst,
                              FFFormatConverter::Filter::T filter)
{
    if(compareParams(format_src, resolution_src, format_dst, resolution_dst, color_space_src, color_space_dst, color_range_src, color_range_dst, filter))
        return true;

    free();

    this->format_src=format_src;
    this->format_dst=format_dst;

    this->resolution_src=resolution_src;
    this->resolution_dst=resolution_dst;

    this->color_space_src=color_space_src;
    this->color_space_dst=color_space_dst;

    this->color_range_src=color_range_src;
    this->color_range_dst=color_range_dst;

    this->filter=filter;

    convert_context=sws_getContext(resolution_src.width(), resolution_src.height(),
                                   format_src,
                                   resolution_dst.width(), resolution_dst.height(),
                                   format_dst,
                                   filter | SWS_FULL_CHR_H_INT | SWS_FULL_CHR_H_INP,
                                   nullptr, nullptr, nullptr);
    sws_setColorspaceDetails(convert_context, sws_getCoefficients(color_space_src), color_range_src, sws_getCoefficients(color_space_dst), color_range_dst, 0, 1 << 16, 1 << 16);

    qDebug().noquote() << "convert_context ptr" << QString::number((quintptr)convert_context, 16);

    return convert_context!=nullptr;
}

bool FFFormatConverter::compareParams(AVPixelFormat format_src, QSize resolution_src, AVPixelFormat format_dst, QSize resolution_dst,
                                      int color_space_src, int color_space_dst, int color_range_src, int color_range_dst,
                                      FFFormatConverter::Filter::T filter)
{
    if(this->format_src==format_src && this->format_dst==format_dst
            && this->resolution_src==resolution_src && this->resolution_dst==resolution_dst
            && this->color_space_src==color_space_src && this->color_space_dst==color_space_dst && this->color_range_src==color_range_src && this->color_range_dst==color_range_dst
            && this->filter==filter)
        return true;

    return false;
}

AVPixelFormat FFFormatConverter::formatSrc() const
{
    return format_src;
}

AVPixelFormat FFFormatConverter::formatDst() const
{
    return format_dst;
}

bool FFFormatConverter::convert(AVFrame *src, AVFrame *dst)
{
    if(!convert_context) {
        qCritical() << "SwsContext nullptr";
        return 1;
    }

    return sws_scale(convert_context, src->data, src->linesize, 0, src->height, dst->data, dst->linesize)>0;
}

AVFrameSP::ptr FFFormatConverter::convert(AVFrame *src)
{
    if(!convert_context) {
        qCritical() << "SwsContext nullptr";
        return nullptr;
    }

    AVFrameSP::ptr result_frame=AVFrameSP::make(format_dst, resolution_dst.width(), resolution_dst.height());

    if(sws_scale(convert_context, src->data, src->linesize, 0, src->height,
                 result_frame->d->data, result_frame->d->linesize)<=0)
        return nullptr;

    return result_frame;
}

void FFFormatConverter::free()
{
    if(convert_context) {
        sws_freeContext(convert_context);
        convert_context=nullptr;
    }
}
