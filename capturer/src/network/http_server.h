/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <QThread>
#include <QMutex>
#include <QTcpServer>

#include "data_types.h"

class SettingsModel;

class HttpServer : public QObject
{
    Q_OBJECT

public:
    HttpServer(quint16 port=0, QObject *parent=0);
    ~HttpServer();

    void setSettingsModel(SettingsModel *mdl);

public slots:
    void formatChanged(QString format);
    void temperatureChanged(double temperature);

    void setRecState(const bool &value);
    void setRecStats(const NRecStats &value);
    void setPlayerDuration(const qint64 &value);
    void setPlayerPosition(const qint64 &value);
    void setFreeSpace(const qint64 &value);
    void setNvState(const NvState &state);

private:
    QByteArray pageIndex();
    QByteArray pageSettings();

    QByteArray favicon();

    QByteArray getResource(const QString &name);

    void checkSettings(QMap <QString, QString> new_settings);

    QByteArray ba_buffer;

    qint64 last_buf_update;

    Status status;

    SettingsModel *settings_model;

signals:
    void keyPressed(int code);
    void playerSeek(qint64 pos);

    void checkEncoders();
};

#endif // HTTP_SERVER_H
