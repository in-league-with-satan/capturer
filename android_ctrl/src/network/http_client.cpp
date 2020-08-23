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

#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QTimer>

#include "http_client.h"

const int get_interval_online=100;
const int get_interval_offline=1000;


HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
{
    get_interval=get_interval_online;

    last_reply_time=0;

    base_url.setScheme(QStringLiteral("http"));

    network_access_manager=new QNetworkAccessManager(this);

    connect(network_access_manager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

    timer=new QTimer();
    timer->setInterval(get_interval_online);

    connect(timer, SIGNAL(timeout()), SLOT(voidGet()));
}

HttpClient::~HttpClient()
{
}

void HttpClient::connectToHost(const QString &host, quint16 port)
{
    base_url.setHost(host);
    base_url.setPort(port);

    if(!host.isEmpty() && port>0)
        timer->start();

    else
        timer->stop();
}

void HttpClient::commandKey(int key_code)
{
    QUrl url(base_url);

    QUrlQuery query;

    query.addQueryItem(QStringLiteral("key_code"), QString::number(key_code));

    url.setQuery(query);

    network_access_manager->get(QNetworkRequest(url));
}

void HttpClient::commandPlayerSeek(qint64 pos)
{
    QUrl url(base_url);

    QUrlQuery query;

    query.addQueryItem(QStringLiteral("player_seek"), QString::number(pos));

    url.setQuery(query);

    network_access_manager->get(QNetworkRequest(url));
}

void HttpClient::replyFinished(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError) {
        // qCritical() << reply->error() << reply->errorString();

        get_interval=get_interval_offline;

        emit recordIsRunning(false);

    } else {
        last_reply_time=QDateTime::currentMSecsSinceEpoch();

        if(get_interval!=get_interval_online)
            get_interval=get_interval_online;


        QVariantMap map=
                QJsonDocument::fromJson(reply->readAll()).toVariant().toMap();

        if(!map.isEmpty()) {
            Status status;

            status.fromExt(map);

            if(status!=last_status) {
                if(status.rec_stats!=last_status.rec_stats) {
                    if(status.rec_stats.isNull()) {
                        emit recordIsRunning(false);

                    } else {
                        emit recStats(status.rec_stats, status.free_space);
                    }
                }

                if(status.player_state.duration!=last_status.player_state.duration) {
                    emit playerDuration(status.player_state.duration);
                }

                if(status.player_state.position!=last_status.player_state.position) {
                    emit playerPosition(status.player_state.position);
                }

                last_status=status;
            }
        }
    }

    reply->deleteLater();
}

void HttpClient::voidGet()
{
    if(QDateTime::currentMSecsSinceEpoch() - last_reply_time<get_interval)
        return;

    QUrl url(base_url);

    url.setPath("/data");

    network_access_manager->get(QNetworkRequest(url));
}
