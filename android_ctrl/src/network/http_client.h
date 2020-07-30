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

#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include <QThread>
#include <QMutex>
#include <QTcpSocket>
#include <QQueue>

#include "data_types.h"

class QNetworkAccessManager;
class QNetworkReply;
class QTimer;

class HttpClient : public QObject
{
    Q_OBJECT

public:
    HttpClient(QObject *parent=0);
    ~HttpClient();

public slots:
    void connectToHost(const QString &host, quint16 port);

    void commandKey(int key_code);
    void commandPlayerSeek(qint64 pos);

private slots:
    void replyFinished(QNetworkReply *reply);
    void voidGet();

private:
    void sendRequest();
    void read();

    QUrl base_url;

    QNetworkAccessManager *network_access_manager;

    QTimer *timer;

    qint64 last_reply_time;

    Status last_status;

    int get_interval;

signals:
    void recordIsRunning(bool value);
    void recStats(NRecStats value, qint64 free_space);
    void playerDuration(qint64 value);
    void playerPosition(qint64 value);
};

#endif // HTTP_CLIENT_H
