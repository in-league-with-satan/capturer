#ifndef FF_MEDIA_INFO_H
#define FF_MEDIA_INFO_H

#include <QObject>
#include <QThread>
#include <QSize>


struct FFMediaInfo
{
    struct Type
    {
        enum T {
            audio,
            video
        };
    };

    struct TrackInfo
    {
        Type::T type;

        QString codec_name;

        QSize resolution;
        int32_t sample_rate;
        QString fps;
        int channels;
        QString channel_layout;
        QString language;
    };

    typedef QList <TrackInfo> TrackInfoList;

    struct Info {
        TrackInfoList track;

        int64_t duration;
        int64_t bitrate;
    };


    static Info getInfo(QString filename);
    static QString getInfoString(QString filename);
};

#endif // FF_MEDIA_INFO_H
