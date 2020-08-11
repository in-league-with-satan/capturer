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

#include <QDebug>
#include <QTcpSocket>
#include <QTimer>
#include <QRegExp>
#include <QFile>
#include <QDateTime>

#include "ff_encoder_base_filename.h"

#include "irc_subtitles.h"

#define timeformat QStringLiteral("hh:mm:ss,zzz")

IrcSubtitles::IrcSubtitles(QObject *parent)
    : QObject(parent)
{
    file=new QFile(this);

    socket=new QTcpSocket(this);

    connect(socket, SIGNAL(readyRead()), SLOT(onRead()));
    connect(socket, SIGNAL(connected()), SLOT(onConnected()));
    connect(socket, SIGNAL(disconnected()), SLOT(onConnected()));

    QTimer *timer=new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(onRead()));
    timer->start(1000);
}

IrcSubtitles::~IrcSubtitles()
{
}

bool IrcSubtitles::connectToHost(QString host, int port, QString nickname, QString token, QString channel)
{
    this->nickname=nickname;
    this->token=token;
    this->channel=channel;

    if(socket->isOpen()) {
        socket->disconnectFromHost();
    }

    ready_read=false;

    socket->connectToHost(host, port);

    if(!socket->waitForConnected())
        return false;

    return true;
}

void IrcSubtitles::setBaseFilename(FFEncoderBaseFilename *bf)
{
    this->base_filename=bf;
}

void IrcSubtitles::setStoreDir(const QString &dir)
{
    store_dir=dir;
}

void IrcSubtitles::start()
{
    stop();

    if(!base_filename)
        return;

    // if(base_filename->isEmpty())
    //     return;

    ready_write=true;

    start_time=QDateTime::currentMSecsSinceEpoch();
    row_num=1;
}

void IrcSubtitles::stop()
{
    ready_write=false;

    if(file->isOpen())
        file->close();
}

void IrcSubtitles::onConnected()
{
    socket->write(QString("PASS %1\r\n").arg(token).toUtf8());
    socket->write(QString("NICK %1\r\n").arg(nickname).toUtf8());
    socket->write(QString("JOIN #%1\r\n").arg(channel).toUtf8());
    socket->flush();
}

void IrcSubtitles::onDisconnected()
{
    ready_read=false;

    emit connected(false);
}

void IrcSubtitles::onRead()
{
    if(!socket->canReadLine())
        return;

    while(socket->canReadLine()) {
        QString str=QString::fromUtf8(socket->readLine());

        // qDebug() << str;

        if(str.startsWith(QStringLiteral("ping"), Qt::CaseInsensitive)) {
            socket->write(str.replace(QStringLiteral("ping"), QStringLiteral("PONG"), Qt::CaseInsensitive).toUtf8());
            socket->flush();
            return;
        }

        QRegExp rx(QStringLiteral("\\w+"));

        rx.indexIn(str, 0);

        QString author=rx.cap();
        QString text=str.remove(QRegExp(QStringLiteral("^:\\w+!\\w+@\\w+\\.tmi\\.twitch\\.tv PRIVMSG #\\w+ :"))).remove(QStringLiteral("\r\n"));

        if(!ready_read && text.endsWith(QStringLiteral("End of /NAMES list"))) {
            ready_read=true;
            emit connected(true);
            continue;
        }

        if(!ready_read) {
            return;
        }

        if(ready_write && !author.isEmpty() && !text.isEmpty()) {
            write(author, text);
        }
    }
}

void IrcSubtitles::write(const QString &author, const QString &text)
{
    if(!file->isOpen()) {
        if(base_filename->isEmpty())
            return;

        file->setFileName(QString("%1/%2.srt")
                          .arg(store_dir).arg(*base_filename));

        if(!file->open(QFile::WriteOnly | QIODevice::NewOnly | QIODevice::NewOnly)) {
            qCritical() << file->fileName() << file->errorString();
            ready_write=false;
            return;
        }
    }

    const QTime time_begin=QTime(0, 0, 0).addMSecs(QDateTime::currentMSecsSinceEpoch() - start_time);
    const QTime time_end=time_begin.addSecs(4);

    if(row_num>1)
        file->write("\n", 1);

    file->write(QString("%1\n"
                        "%2 --> %3\n"
                        "%4: %5\n")
                .arg(row_num++)
                .arg(time_begin.toString(timeformat))
                .arg(time_end.toString(timeformat))
                .arg(author)
                .arg(text).toUtf8());

    file->flush();
}
