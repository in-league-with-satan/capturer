/******************************************************************************

Copyright © 2018-2021 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QTime>
#include <QElapsedTimer>

#include "ff_tools.h"

#include "ff_media_info.h"


FFMediaInfo::Info FFMediaInfo::getInfo(QString filename)
{
    QElapsedTimer timer;

    timer.start();

    Info info;

    int err=0;

    AVFormatContext *format_context=nullptr;

    err=avformat_open_input(&format_context, filename.toUtf8().constData(), nullptr, nullptr);

    if(err!=0) {
        qCritical() << "avformat_open_input err:" << ffErrorString(err);
        return info;
    }

    err=avformat_find_stream_info(format_context, nullptr);

    if(err<0) {
        qCritical() << "avformat_find_stream_info err:" << ffErrorString(err);

        avformat_close_input(&format_context);

        return info;
    }

    info.duration=format_context->duration/1000.;
    info.bitrate=format_context->bit_rate;

    for(unsigned int index_stream=0; index_stream<format_context->nb_streams; ++index_stream) {
        AVStream *stream=format_context->streams[index_stream];
        AVCodec *codec;
        AVCodecContext *codec_context=nullptr;

        if(stream->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
            TrackInfo inf;

            codec=(AVCodec*)avcodec_find_decoder(stream->codecpar->codec_id);
            codec_context=avcodec_alloc_context3(codec);

            err=avcodec_parameters_to_context(codec_context, stream->codecpar);

            if(err<0) {
                qCritical() << "avcodec_parameters_to_context err:" << ffErrorString(err);

                avformat_close_input(&format_context);
                avcodec_free_context(&codec_context);

                return info;
            }

            char channel_layout_str[32];
            av_get_channel_layout_string(channel_layout_str, 32, codec_context->channels, codec_context->channel_layout);

            inf.type=FFMediaInfo::Type::audio;
            inf.codec_name=QString(avcodec_get_name(stream->codecpar->codec_id));
            inf.sample_rate=codec_context->sample_rate;
            inf.channels=codec_context->channels;
            inf.channel_layout=QString(channel_layout_str);

            AVDictionaryEntry *lang=av_dict_get(stream->metadata, "language", NULL, 0);

            if(lang)
                inf.language=QString(lang->value);

            info.track << inf;
        }

        if(stream->codecpar->codec_type==AVMEDIA_TYPE_VIDEO) {
            TrackInfo inf;

            codec=(AVCodec*)avcodec_find_decoder(stream->codecpar->codec_id);
            codec_context=avcodec_alloc_context3(codec);

              err=avcodec_parameters_to_context(codec_context, stream->codecpar);

            if(err<0) {
                qCritical() << "avcodec_parameters_to_context err:" << ffErrorString(err);

                avformat_close_input(&format_context);
                avcodec_free_context(&codec_context);

                return info;
            }

            inf.type=FFMediaInfo::Type::video;
            inf.codec_name=QString(avcodec_get_name(stream->codecpar->codec_id));
            inf.resolution=QSize(codec_context->width, codec_context->height);
            inf.fps=QString("%1").arg(av_q2d(av_guess_frame_rate(format_context, stream, nullptr)), 0, 'g', 4);

            AVDictionaryEntry *lang=av_dict_get(stream->metadata, "language", NULL, 0);

            if(lang)
                inf.language=QString(lang->value);

            info.track << inf;
        }

        if(codec_context)
            avcodec_free_context(&codec_context);
    }

    avformat_close_input(&format_context);

    qDebug().noquote() << QString("time: %1 ms (%2)").arg(timer.elapsed()).arg(filename);

    return info;
}

QString FFMediaInfo::getInfoString(QString filename)
{
    const FFMediaInfo::Info info=getInfo(filename);

    if(info.track.isEmpty())
        return QString();

    QString str;

    str+=QString(QLatin1String("duration: %1\n")).arg(info.duration>24*60*60*1000 ? QString("%1 hours").arg((int)(info.duration/1000./60./60.)) : QTime(0, 0, 0).addMSecs(info.duration).toString());
    str+=QString(QLatin1String("bitrate: %1\n")).arg(info.bitrate);

    for(int track_index=0; track_index<info.track.size(); ++track_index) {
        FFMediaInfo::TrackInfo track=info.track[track_index];

        if(track.type==FFMediaInfo::Type::audio) {
            str+=QString(QLatin1String("audio track: %1 %2 %3hz"))
                    .arg(track.codec_name)
                    .arg(track.channel_layout)
                    .arg(track.sample_rate)
                    ;

        } else {
            str+=QString(QLatin1String("video track: %1 %2x%3 %4fps"))
                    .arg(track.codec_name)
                    .arg(track.resolution.width(), track.resolution.height())
                    .arg(track.fps)
                    ;
        }

        if(!track.language.isEmpty())
            str+=QString(QLatin1String(" (%1)")).arg(track.language);

        if(track_index<info.track.size() - 1)
            str+=QString(QLatin1String("\n"));
    }

    return str;
}
