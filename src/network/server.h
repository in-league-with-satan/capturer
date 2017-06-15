#ifndef SERVER_H
#define SERVER_H

#include <QThread>
#include <QMutex>
#include <QTcpServer>

#include "data_types.h"

class TcpServer : public QTcpServer
{
    Q_OBJECT

public:
    explicit TcpServer(QObject *parent=0) : QTcpServer(parent) {}

protected:
    void incomingConnection(qintptr socket_descriptor) {
        emit newConnection(socket_descriptor);
    }

signals:
    void newConnection(qintptr socket_descriptor);
};

class Server : public QThread
{
    Q_OBJECT

public:
    Server(quint16 port=0, QObject *parent=0);
    ~Server();

private slots:
    void newConnection(qintptr socket_descriptor);

protected:
    void run();

private:
    TcpServer *tcp_server;

    quint16 port;

signals:
    void keyPressed(int code);
    void playerSeek(qint64 pos);

    void sendRecState(bool state);
    void sendRecStats(NRecStats stats);
    void sendPlayerDuration(qint64 value);
    void sendPlayerPosition(qint64 value);
};

#endif // SERVER_H
