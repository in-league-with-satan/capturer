#ifndef AUDIO_PACKET_H
#define AUDIO_PACKET_H

#include <QVariantMap>

struct AudioPacket
{
    QVariantMap toExt() {
        QVariantMap map;

        map.insert(QStringLiteral("data"), QString(data.toBase64()));
        map.insert(QStringLiteral("channels"), channels);
        map.insert(QStringLiteral("sample_size"), sample_size);

        return map;
    }

    void fromExt(const QVariantMap &map) {
        data=QByteArray::fromBase64(map.value(QStringLiteral("data")).toByteArray());
        channels=map.value(QStringLiteral("channels")).toInt();
        sample_size=map.value(QStringLiteral("sample_size")).toInt();
    }

    QByteArray data;
    int channels;
    int sample_size;
};


#endif // AUDIO_PACKET_H
