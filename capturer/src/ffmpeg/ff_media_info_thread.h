/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef FF_MEDIA_INFO_THREAD_H
#define FF_MEDIA_INFO_THREAD_H

#include <QThread>
#include <QMutex>
#include <QQueue>

#include <atomic>

class QTimer;

class FFMediaInfoThread : public QThread
{
    Q_OBJECT

public:
    FFMediaInfoThread(QObject *parent=0);

public slots:
    void pause(bool state);
    void enqueue(const QString &filename);

private slots:
    void checkQueue();

protected:
    void run();

private:
    QTimer *timer;

    QQueue <QString> queue;
    QMutex mutex_queue;

    std::atomic <bool> on_pause;

signals:
    void ready(QString key, QString info);
};

#endif // FF_MEDIA_INFO_THREAD_H
