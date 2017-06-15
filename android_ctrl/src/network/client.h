#ifndef CLIENT_H
#define CLIENT_H

#include <QThread>
#include <QMutex>
#include <QTcpSocket>
#include <QQueue>

#include "data_types.h"

class Client : public QThread
{
    Q_OBJECT

public:
    Client(QObject *parent=0);
    ~Client();

public slots:
    void connectToHost(const QString &host, quint16 port);
    void command(int cmd_code);
    void commandKey(int key_code);
    void commandPlayerSeek(qint64 pos);

private slots:

protected:
    void run();

private:
    void dropConnection();

    void sendRequest();
    void read();

    void checkProtocol(QVariantMap *vm);
    void recStateChanged(QVariantMap *vm);
    void recStats(QVariantMap *vm);
    void playerDuration(QVariantMap *vm);
    void playerPosition(QVariantMap *vm);

    QTcpSocket *socket;

    std::atomic <bool> running;

    QString host;
    quint16 port;

    QMutex mutex;

    QQueue < QPair <int, QVariant> > cmd;

    bool protocol_ok;

signals:
    void recordIsRunning(bool value);
    void recStats(NRecStats value);
    void playerDuration(qint64 value);
    void playerPosition(qint64 value);
};

#endif // CLIENT_H
