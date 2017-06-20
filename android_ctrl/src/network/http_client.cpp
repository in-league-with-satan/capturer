#include <QUrl>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QTimer>

#include "http_client.h"

const int get_interval=100;


HttpClient::HttpClient(QObject *parent)
    : QObject(parent)
{
    last_reply_time=0;

    base_url.setScheme("http");

    network_access_manager=new QNetworkAccessManager(this);

    connect(network_access_manager, SIGNAL(finished(QNetworkReply*)), SLOT(replyFinished(QNetworkReply*)));

    timer=new QTimer();
    timer->setInterval(get_interval);

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

    query.addQueryItem("key_code", QString::number(key_code));

    url.setQuery(query);

    network_access_manager->get(QNetworkRequest(url));
}

void HttpClient::commandPlayerSeek(qint64 pos)
{
    QUrl url(base_url);

    QUrlQuery query;

    query.addQueryItem("player_seek", QString::number(pos));

    url.setQuery(query);

    network_access_manager->get(QNetworkRequest(url));
}

void HttpClient::replyFinished(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError) {
        // qCritical() << reply->error() << reply->errorString();
    }

    last_reply_time=QDateTime::currentMSecsSinceEpoch();

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
                    emit recStats(status.rec_stats);
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

    reply->deleteLater();
}

void HttpClient::voidGet()
{
    if(QDateTime::currentMSecsSinceEpoch() - last_reply_time<get_interval)
        return;

    network_access_manager->get(QNetworkRequest((base_url)));
}