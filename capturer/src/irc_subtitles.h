/******************************************************************************

Copyright © 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef IRC_SUBTITLES_H
#define IRC_SUBTITLES_H

#include <QObject>

class QTcpSocket;
class QFile;
class FFEncoderBaseFilename;

class IrcSubtitles : public QObject
{
    Q_OBJECT

public:
    IrcSubtitles(QObject *parent=nullptr);
    ~IrcSubtitles();

    bool connectToHost(QString host, int port, QString nickname, QString token, QString channel);
    void setBaseFilename(FFEncoderBaseFilename *base_filename);
    void setStoreDir(const QString &dir);

public:
    void start();
    void stop();

private slots:
    void onConnected();
    void onDisconnected();
    void onRead();

private:
    void write(const QString &author, const QString &text);

    bool ready_read=false;
    bool ready_write=false;

    FFEncoderBaseFilename *base_filename=nullptr;

    QString store_dir;

    QFile *file;

    QTcpSocket *socket;
    QString nickname;
    QString token;
    QString channel;

    qint64 start_time=0;
    qint64 row_num=1;

signals:
    void connected(bool state);
};

#endif // IRC_SUBTITLES_H
