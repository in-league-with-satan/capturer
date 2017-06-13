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
        KeyPressed
    };
};

struct Message {
    enum {
        ProtocolVersion,
        RecStateChanged,
        RecStats,
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
};

Q_DECLARE_METATYPE(NRecStats)

#endif // DATA_TYPES_H
