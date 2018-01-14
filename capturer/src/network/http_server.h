/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

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
