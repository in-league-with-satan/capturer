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
        Preview,
        PreviewFastYuv,
        FullScreen,
        FileBrowser,
        Rec,
        RecState,
        SmoothTransform,
        Exit,

        enm_size
    };
    Q_ENUM(KeyCode)

    static QString toString(int code);
    static int fromString(const QString &str);
    static void declareQML();

    static QString key_title[enm_size];
};

struct NRecStats {
    QTime time;
    double avg_bitrate;
    quint64 size;
    quint32 dropped_frames_counter;
    quint16 frame_buffer_size;
    quint16 frame_buffer_used;

    NRecStats(QTime time=QTime(), double avg_bitrate=0., quint64 size=0, quint32 dropped_frames_counter=0, quint16 frame_buffer_size=0, quint16 frame_buffer_used=0);

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

struct Status {
    NRecStats rec_stats;
    PlayerState player_state;
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
