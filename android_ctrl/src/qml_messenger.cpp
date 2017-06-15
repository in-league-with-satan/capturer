#include <QDebug>

#include "client.h"
#include "vibro.h"
#include "settings.h"

#include "qml_messenger.h"


QmlMessenger::QmlMessenger(QObject *parent)
    : QObject(parent)
{
    KeyCodeC::declareQML();

    qRegisterMetaType<NRecStats>("NRecStats");

    settings->createInstance()->load();

    vibro=new Vibro(this);

    client=new Client(this);

    connect(client, SIGNAL(recStats(NRecStats)), SLOT(recStats(NRecStats)), Qt::QueuedConnection);
    connect(client, SIGNAL(recordIsRunning(bool)), SIGNAL(recStateChanged(bool)), Qt::QueuedConnection);
    connect(client, SIGNAL(playerDuration(qint64)), SIGNAL(playerDurationChanged(qint64)), Qt::QueuedConnection);
    connect(client, SIGNAL(playerPosition(qint64)), SIGNAL(playerPositionChanged(qint64)), Qt::QueuedConnection);

    if(!settings->host().isEmpty())
        client->connectToHost(settings->host(), settings->port());
}

QmlMessenger::~QmlMessenger()
{
}

void QmlMessenger::setConnectParams(const QString &host, const quint16 &port, const QString &routing_key)
{
    settings->setHost(host);
    settings->setPort(port);
    settings->setRoutingKey(routing_key);

    settings->save();

    // qInfo() << "QmlMessenger::connectToHost" << host << port << routing_key;

    client->connectToHost(host, port);
}

QString QmlMessenger::connectAddrHost() const
{
    return settings->host();
}

quint16 QmlMessenger::connectAddrPort() const
{
    return settings->port();
}

QString QmlMessenger::connectRoutingKey() const
{
    return settings->routingKey();
}

void QmlMessenger::keyPressed(int code)
{
    client->commandKey(code);
    // qInfo() << "keyPressed" << code;

    vibro->vibrate(30);
}

void QmlMessenger::playerSeek(qint64 pos)
{
    client->commandPlayerSeek(pos);

    vibro->vibrate(30);
}

void QmlMessenger::recStats(NRecStats stats)
{
    emit updateRecStats(stats.time.toString("HH:mm:ss"),
                        QString("%1 bytes").arg(QLocale().toString((qulonglong)stats.size)),
                        QString("%1 Mbits/s (%2 MB/s)").arg(QLocale().toString((stats.avg_bitrate)/1000./1000., 'f', 2))
                        .arg(QLocale().toString((stats.avg_bitrate)/8/1024./1024., 'f', 2)));
}
