/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <QtGlobal>
#include <QMetaType>
#include <QTime>
#include <QVariantMap>
#include <QQmlEngine>

const quint32 MARKER=0x63617074;
const int PROTOCOL_VERSION=0x1;
const int READ_TIMEOUT=5000;

struct Command {
    enum {
        GetProtocolVersion,
        KeyPressed,
        PlayerSeek
    };
};

struct Message {
    enum {
        ProtocolVersion,
        RecStateChanged,
        RecStats,
        PlayerDurationChanged,
        PlayerPositionChanged
    };
};

class KeyCodeC : public QObject
{
    Q_OBJECT

public:
    enum KeyCode {
        Up,
        Down,
        Left,
        Right,
        Enter,
        Menu,
        Back,
        About,
        Info,
        PreviewPrimary,
        PreviewSecondary,
        PreviewSecondaryChangePosition,
        PreviewSwitchHalfFps,
        HdrToSdr,
        HdrBrightnesPlus,
        HdrBrightnesMinus,
        HdrSaturationPlus,
        HdrSaturationMinus,
        FullScreen,
        FileBrowser,
        Rec,
        RecState,
        Exit,

        enm_size
    };
    Q_ENUM(KeyCode)

    static QString toString(int code);
    static int fromString(const QString &str);
    static void declareQML();

    static QString key_title[enm_size];
};

struct InputFormat {
    int width=0;
    int height=0;
    quint64 frame_duration=0;
    quint64 frame_scale=0;
    bool progressive_frame=true;
    QString pixel_format;

    QVariantMap toExt();
    InputFormat &fromExt(const QVariantMap &map_root);
};

struct NRecStats {
    QTime time;
    double avg_bitrate;
    quint64 size;
    quint32 dropped_frames_counter;
    quint16 frame_buffer_size;
    quint16 frame_buffer_used;
    QVariantMap bitrate_video;

    NRecStats(QTime time=QTime(), double avg_bitrate=0., QVariantMap bitrate_video=QVariantMap(), quint64 size=0,
                quint32 dropped_frames_counter=0, quint16 frame_buffer_size=0, quint16 frame_buffer_used=0);

    bool isNull() const;

    QVariantMap toExt();
    NRecStats &fromExt(const QVariantMap &map_root);

    bool operator!=(const NRecStats &other) const {
        return time!=other.time || avg_bitrate!=other.avg_bitrate || size!=other.size || dropped_frames_counter!=other.dropped_frames_counter
                || frame_buffer_used!=other.frame_buffer_used || frame_buffer_size!=other.frame_buffer_size;
    }
};

struct PlayerState {
    PlayerState();

    QVariantMap toExt();
    PlayerState &fromExt(const QVariantMap &map_root);

    inline bool operator!=(const PlayerState &other) const {
        return duration!=other.duration || position!=other.position;
    }

    qint64 duration;
    qint64 position;
};

struct NvState {
    NvState();

    QVariantMap toExt() const;
    NvState &fromExt(const QVariantMap &map_root);

    QString dev_name;
    int temperature;
    int graphic_processing_unit;
    int memory_controller_unit;
    int video_processing_unit;
};

struct Status {
    QString input_format;
    NRecStats rec_stats;
    PlayerState player_state;
    NvState nv_state;
    qint64 free_space;

    Status();

    QVariantMap toExt();
    Status &fromExt(const QVariantMap &map_root);

    inline bool operator!=(const Status &other) const {
        return rec_stats!=other.rec_stats || player_state!=other.player_state || free_space!=other.free_space;
    }
};

Q_DECLARE_METATYPE(NRecStats)

#endif // DATA_TYPES_H
