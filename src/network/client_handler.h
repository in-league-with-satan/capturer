#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <QThread>
#include <QMutex>
#include <QTcpSocket>
#include <QTimer>


class ClientHandler : public QThread
{
    Q_OBJECT

public:
    ClientHandler(qintptr socket_descriptor, QObject *parent=0);

public slots:

private slots:
    void disconnected();

    void read();

protected:
    void run();

    void sendData(const QVariantMap &map);

private:
    void cmdProtocolVersion(QVariantMap *vm);
    void cmdKeyPressed(QVariantMap *vm);

    QTimer *timer;

    QTcpSocket *socket;

    qintptr socket_descriptor;

signals:
    void recStartStop();
    void keyPressed(int code);
};

#endif // CLIENT_HANDLER_H
