#include <QDebug>
#include <QDateTime>
#include <QJsonDocument>
#include <QUrlQuery>

#ifdef LIB_QHTTP

#include "qhttpserver.hpp"
#include "qhttpserverresponse.hpp"
#include "qhttpserverrequest.hpp"

using namespace qhttp::server;

#endif

#include "http_server.h"


HttpServer::HttpServer(quint16 port, QObject *parent)
    : QObject(parent)
{
    if(port<1)
        return;

#ifdef LIB_QHTTP

    QHttpServer *server=new QHttpServer(this);

    last_buf_update=0;

    ((QHttpServer*)server)->listen(QHostAddress::Any, port,
                                   [&](QHttpRequest *req, QHttpResponse *res) {
        if(req->method()==qhttp::THttpMethod::EHTTP_GET) {
            res->setStatusCode(qhttp::ESTATUS_OK);

            QUrlQuery url=QUrlQuery(req->url());

            // qInfo() << "query" << req->url() << url.queryItems();

            if(url.hasQueryItem("key_code")) {
                emit keyPressed(url.queryItemValue("key_code").toInt());
            }

            if(url.hasQueryItem("player_seek")) {
                emit playerSeek(url.queryItemValue("player_seek").toLongLong());
            }

            if(QDateTime::currentMSecsSinceEpoch() - last_buf_update>=100) {
                ba_buffer=QJsonDocument::fromVariant(status.toExt()).toJson(QJsonDocument::Compact);

                last_buf_update=QDateTime::currentMSecsSinceEpoch();
            }

            res->end(ba_buffer);
        }
    });

#endif
}

HttpServer::~HttpServer()
{
}

void HttpServer::setRecState(const bool &value)
{
    if(!value) {
        status.rec_stats=NRecStats();
    }
}

void HttpServer::setRecStats(const NRecStats &value)
{
    status.rec_stats=value;
}

void HttpServer::setPlayerDuration(const qint64 &value)
{
    status.player_state.duration=value;
}

void HttpServer::setPlayerPosition(const qint64 &value)
{
    status.player_state.position=value;
}
