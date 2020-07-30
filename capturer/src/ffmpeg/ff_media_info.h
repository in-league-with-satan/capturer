/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
