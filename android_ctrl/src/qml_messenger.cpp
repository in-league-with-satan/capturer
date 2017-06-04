#include <QDebug>

#include "client.h"
#include "vibro.h"
#include "settings.h"

#include "qml_messenger.h"


QmlMessenger::QmlMessenger(QObject *parent)
    : QObject(parent)
{
    KeyCodeC::declareQML();

    settings->createInstance()->load();

    vibro=new Vibro(this);

    client=new Client(this);

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
