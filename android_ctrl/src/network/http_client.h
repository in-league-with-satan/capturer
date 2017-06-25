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
    void recStats(NRecStats value);
    void playerDuration(qint64 value);
    void playerPosition(qint64 value);
};

#endif // HTTP_CLIENT_H
