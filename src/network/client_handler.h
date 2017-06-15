#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <QThread>
#include <QMutex>
#include <QTcpSocket>
#include <QTimer>
#include <QQueue>

#include "data_types.h"

class ClientHandler : public QThread
{
    Q_OBJECT

public:
    ClientHandler(qintptr socket_descriptor, QObject *parent=0);

public slots:

private slots:
    void disconnected();

    void read();
    void write();

    void sendRecState(bool value);
    void sendRecStats(NRecStats value);
    void sendPlayerDuration(qint64 value);
    void sendPlayerPosition(qint64 value);

protected:
    void run();

    void sendData(const QVariantMap &map);

private:
    void cmdProtocolVersion(QVariantMap *vm);
    void cmdKeyPressed(QVariantMap *vm);
    void cmdPlayerSeek(QVariantMap *vm);

    QTimer *timer;

    QTcpSocket *socket;

    qintptr socket_descriptor;

    QMutex mutex_queue;

    QQueue <QVariantMap> queue_send;

signals:
    void recStartStop();
    void keyPressed(int code);
    void playerSeek(qint64 pos);
};

#endif // CLIENT_HANDLER_H
