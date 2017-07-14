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
        FullScreen,
        FileBrowser,
        Rec,
        RecState,
        SmoothTransform,
        Exit
    };
    Q_ENUM(KeyCode)

    static void declareQML() {
        qmlRegisterType<KeyCodeC>("FuckTheSystem", 0, 0, "KeyCode");
    }
};

struct NRecStats {
    QTime time;
    double avg_bitrate;
    quint64 size;
    quint32 dropped_frames_counter;
    quint16 frame_buffer_size;
    quint16 frame_buffer_used;

    bool isNull() const {
        return time.isNull();
    }

    NRecStats(QTime time=QTime(), double avg_bitrate=0., quint64 size=0, quint32 dropped_frames_counter=0, quint16 frame_buffer_size=0, quint16 frame_buffer_used=0) {
        this->time=time;
        this->avg_bitrate=avg_bitrate;
        this->size=size;
        this->dropped_frames_counter=dropped_frames_counter;
        this->frame_buffer_size=frame_buffer_size;
        this->frame_buffer_used=frame_buffer_used;
    }

    QVariantMap toExt() {
        QVariantMap map_root;

        map_root.insert(QStringLiteral("time"), time);
        map_root.insert(QStringLiteral("avg_bitrate"), avg_bitrate);
        map_root.insert(QStringLiteral("size"), size);
        map_root.insert(QStringLiteral("dropped_frames_counter"), dropped_frames_counter);
        map_root.insert(QStringLiteral("frame_buffer_size"), frame_buffer_size);
        map_root.insert(QStringLiteral("frame_buffer_used"), frame_buffer_used);

        return map_root;
    }

    NRecStats &fromExt(const QVariantMap &map_root) {
        time=map_root.value(QStringLiteral("time")).toTime();
        avg_bitrate=map_root.value(QStringLiteral("avg_bitrate")).toDouble();
        size=map_root.value(QStringLiteral("size")).toULongLong();
        dropped_frames_counter=map_root.value(QStringLiteral("dropped_frames_counter")).toUInt();
        frame_buffer_size=map_root.value(QStringLiteral("frame_buffer_size")).toUInt();
        frame_buffer_used=map_root.value(QStringLiteral("frame_buffer_used")).toUInt();

        return *this;
    }

    inline bool operator !=(const NRecStats &other) const {
        return time!=other.time || avg_bitrate!=other.avg_bitrate || size!=other.size || dropped_frames_counter!=other.dropped_frames_counter
                || frame_buffer_used!=other.frame_buffer_used || frame_buffer_size!=other.frame_buffer_size;
    }
};

struct PlayerState {
    PlayerState() {
        duration=0;
        position=0;
    }

    QVariantMap toExt() {
        QVariantMap map_root;

        map_root.insert(QStringLiteral("duration"), duration);
        map_root.insert(QStringLiteral("position"), position);

        return map_root;
    }

    PlayerState &fromExt(const QVariantMap &map_root) {
        duration=map_root.value(QStringLiteral("duration")).toLongLong();
        position=map_root.value(QStringLiteral("position")).toLongLong();

        return *this;
    }

    inline bool operator !=(const PlayerState &other) const {
        return duration!=other.duration || position!=other.position;
    }

    qint64 duration;
    qint64 position;
};

struct Status {
    NRecStats rec_stats;
    PlayerState player_state;
    qint64 free_space;

    Status() {
        free_space=0;
    }

    QVariantMap toExt() {
        QVariantMap map_root;

        map_root.insert(QStringLiteral("rec_stats"), rec_stats.toExt());
        map_root.insert(QStringLiteral("player_state"), player_state.toExt());
        map_root.insert(QStringLiteral("free_space"), free_space);

        return map_root;
    }

    Status &fromExt(const QVariantMap &map_root) {
        rec_stats.fromExt(map_root.value(QStringLiteral("rec_stats")).toMap());
        player_state.fromExt(map_root.value(QStringLiteral("player_state")).toMap());
        free_space=map_root.value(QStringLiteral("free_space"), 0).toULongLong();

        return *this;
    }

    inline bool operator !=(const Status &other) const {
        return rec_stats!=other.rec_stats || player_state!=other.player_state || free_space!=other.free_space;
    }
};

Q_DECLARE_METATYPE(NRecStats)

#endif // DATA_TYPES_H
