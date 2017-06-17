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

    bool isNull() const {
        return time.isNull();
    }

    NRecStats(QTime time=QTime(), double avg_bitrate=0., quint64 size=0) {
        this->time=time;
        this->avg_bitrate=avg_bitrate;
        this->size=size;
    }

    QVariantMap toExt() {
        QVariantMap map_root;

        map_root.insert("time", time);
        map_root.insert("avg_bitrate", avg_bitrate);
        map_root.insert("size", size);

        return map_root;
    }

    NRecStats &fromExt(const QVariantMap &map_root) {
        time=map_root.value("time").toTime();
        avg_bitrate=map_root.value("avg_bitrate").toDouble();
        size=map_root.value("size").toULongLong();

        return *this;
    }

    inline bool operator !=(const NRecStats &other) const {
        return time!=other.time || avg_bitrate!=other.avg_bitrate || size!=other.size;
    }
};

struct PlayerState {
    PlayerState() {
        duration=0;
        position=0;
    }

    QVariantMap toExt() {
        QVariantMap map_root;

        map_root.insert("duration", duration);
        map_root.insert("position", position);

        return map_root;
    }

    PlayerState &fromExt(const QVariantMap &map_root) {
        duration=map_root.value("duration").toLongLong();
        position=map_root.value("position").toLongLong();

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

    QVariantMap toExt() {
        QVariantMap map_root;

        map_root.insert("rec_stats", rec_stats.toExt());
        map_root.insert("player_state", player_state.toExt());

        return map_root;
    }

    Status &fromExt(const QVariantMap &map_root) {
        rec_stats.fromExt(map_root.value("rec_stats").toMap());
        player_state.fromExt(map_root.value("player_state").toMap());

        return *this;
    }

    inline bool operator !=(const Status &other) const {
        return rec_stats!=other.rec_stats || player_state!=other.player_state;
    }
};

Q_DECLARE_METATYPE(NRecStats)

#endif // DATA_TYPES_H
