#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QThread>
#include <QMutex>
#include <QTcpServer>

#include "data_types.h"

class HttpServer : public QObject
{
    Q_OBJECT

public:
    HttpServer(quint16 port=0, QObject *parent=0);
    ~HttpServer();

public slots:
    void setRecState(const bool &value);
    void setRecStats(const NRecStats &value);
    void setPlayerDuration(const qint64 &value);
    void setPlayerPosition(const qint64 &value);
    void setFreeSpace(const qint64 &value);

private:
    QByteArray ba_buffer;

    qint64 last_buf_update;

    Status status;

signals:
    void keyPressed(int code);
    void playerSeek(qint64 pos);
};

#endif // HTTP_SERVER_H
