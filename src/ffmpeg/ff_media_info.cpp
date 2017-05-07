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

    err=avformat_open_input(&format_context, filename.toUtf8().data(), nullptr, nullptr);

    if(err!=0) {
        qCritical() << "FFMediaInfo: Unable to open input file" << err << filename;
        return info;
    }

    err=avformat_find_stream_info(format_context, nullptr);

    if(err<0) {
        qCritical() << "FFMediaInfo: Unable to find stream info";

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

            codec=avcodec_find_decoder(stream->codecpar->codec_id);
            codec_context=avcodec_alloc_context3(codec);

            err=avcodec_parameters_to_context(codec_context, stream->codecpar);

            if(err<0) {
                qCritical() << "FFMediaInfo: avcodec_parameters_to_context" << err;

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

            codec=avcodec_find_decoder(stream->codecpar->codec_id);
            codec_context=avcodec_alloc_context3(codec);

              err=avcodec_parameters_to_context(codec_context, stream->codecpar);

            if(err<0) {
                qCritical() << "FFMediaInfo: avcodec_parameters_to_context" << err;

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

    qInfo().noquote() << QString("FFMediaInfo time: %1 ms (%2)").arg(timer.elapsed()).arg(filename);

    return info;
}

QString FFMediaInfo::getInfoString(QString filename)
{
    const FFMediaInfo::Info info=getInfo(filename);

    if(info.track.isEmpty())
        return QString();

    QString str;

    str+=QString("duration: %1\n").arg(info.duration>24*60*60*1000 ? QString("%1 hours").arg((int)(info.duration/1000./60./60.)) : QTime(0, 0, 0).addMSecs(info.duration).toString());
    str+=QString("bitrate: %1\n").arg(info.bitrate);

    for(int track_index=0; track_index<info.track.size(); ++track_index) {
        FFMediaInfo::TrackInfo track=info.track[track_index];

        if(track.type==FFMediaInfo::Type::audio) {
            str+=QString("audio track: %1 %2 %3hz")
                    .arg(track.codec_name)
                    .arg(track.channel_layout)
                    .arg(track.sample_rate)
                    ;

        } else {
            str+=QString("video track: %1 %2x%3 %4fps")
                    .arg(track.codec_name)
                    .arg(track.resolution.width())
                    .arg(track.resolution.height())
                    .arg(track.fps)
                    ;
        }

        if(!track.language.isEmpty())
            str+=QString(" (%1)").arg(track.language);

        if(track_index<info.track.size() - 1)
            str+=QString("\n");
    }

    return str;
}
