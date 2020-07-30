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

#include <QDebug>

#include "http_client.h"
#include "vibro.h"
#include "settings.h"
#include "keep_screen_on.h"

#include "qml_messenger.h"


QmlMessenger::QmlMessenger(QObject *parent)
    : QObject(parent)
{
    KeyCodeC::declareQML();

    qRegisterMetaType<NRecStats>("NRecStats");

    settings->createInstance()->load();

    vibro=new Vibro(this);


    keep_screen_on=new KeepScreenOn(this);

    keep_screen_on->setEnabled(true);


    client=new HttpClient(this);

    connect(client, SIGNAL(recStats(NRecStats,qint64)), SLOT(recStats(NRecStats,qint64)));
    connect(client, SIGNAL(recordIsRunning(bool)), SIGNAL(recStateChanged(bool)));
    connect(client, SIGNAL(playerDuration(qint64)), SIGNAL(playerDurationChanged(qint64)));
    connect(client, SIGNAL(playerPosition(qint64)), SIGNAL(playerPositionChanged(qint64)));

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

void QmlMessenger::recStats(NRecStats stats, qint64 free_space)
{
    emit updateRecStats(stats.time.toString(QStringLiteral("HH:mm:ss")),
                        QString(QLatin1String("%1 MB")).arg(QLocale().toString(qulonglong(stats.size/1024/1024))),
                        QString(QLatin1String("%1 MB")).arg(QLocale().toString(qulonglong(free_space/1024/1024))),
                        QString(QLatin1String("%1 Mbits/s (%2 MB/s)")).arg(QLocale().toString((stats.avg_bitrate)/1000./1000., 'f', 2))
                        .arg(QLocale().toString((stats.avg_bitrate)/8/1024./1024., 'f', 2)),
                        QString::number(stats.dropped_frames_counter), QString(QLatin1String("%1/%2")).arg(stats.frame_buffer_used).arg(stats.frame_buffer_size));
}
