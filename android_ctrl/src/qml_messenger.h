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

#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>

#include "data_types.h"

class HttpClient;
class Settings;
class Vibro;
class KeepScreenOn;

class QmlMessenger : public QObject
{
    Q_OBJECT

public:
    explicit QmlMessenger(QObject *parent=0);
    ~QmlMessenger();

    Q_INVOKABLE void setConnectParams(const QString &host, const quint16 &port, const QString &routing_key="");
    Q_INVOKABLE QString connectAddrHost() const;
    Q_INVOKABLE quint16 connectAddrPort() const;
    Q_INVOKABLE QString connectRoutingKey() const;

    Q_INVOKABLE void keyPressed(int code);
    Q_INVOKABLE void playerSeek(qint64 pos);

public slots:

private slots:
    void recStats(NRecStats stats, qint64 free_space);

private:
    HttpClient *client;
    Vibro *vibro;
    KeepScreenOn *keep_screen_on;

signals:
    void updateRecStats(QString duration, QString size, QString free_space, QString bitrate, QString frames_dropped, QString frame_buffer_state);
    void recStateChanged(bool state);
    void playerDurationChanged(qint64 value);
    void playerPositionChanged(qint64 value);
};

#endif // QML_MESSENGER_H
