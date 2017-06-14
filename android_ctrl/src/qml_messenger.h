#ifndef QML_MESSENGER_H
#define QML_MESSENGER_H

#include <QObject>

#include "data_types.h"

class Client;
class Settings;
class Vibro;

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

public slots:

private slots:
    void recStats(NRecStats stats);

private:
    Client *client;
    Vibro *vibro;

signals:
    void updateRecStats(QString duration, QString size, QString bitrate);
    void recStateChanged(bool state);
};

#endif // QML_MESSENGER_H