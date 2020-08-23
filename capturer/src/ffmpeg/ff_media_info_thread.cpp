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

#include <QTimer>
#include <qcoreapplication.h>

#include "ff_media_info.h"

#include "ff_media_info_thread.h"


FFMediaInfoThread::FFMediaInfoThread(QObject *parent)
    : QThread(parent)
{
    on_pause=false;

    start();
}

void FFMediaInfoThread::enqueue(const QString &filename)
{
    QMutexLocker ml(&mutex_queue);

    queue.enqueue(filename);
}

void FFMediaInfoThread::pause(bool state)
{
    on_pause=state;
}

void FFMediaInfoThread::checkQueue()
{
    if(on_pause)
        return;

    timer->stop();

    QString filename;
    QString result;

    while(true) {
        {
            QMutexLocker ml(&mutex_queue);

            if(queue.isEmpty()) {
                timer->start();
                return;
            }

            filename=queue.dequeue();
        }

        result=FFMediaInfo::getInfoString(filename);

        emit ready(filename, result);

        qApp->processEvents();
    }
}

void FFMediaInfoThread::run()
{
    timer=new QTimer();
    timer->moveToThread(this);
    timer->setInterval(100);
    connect(timer, SIGNAL(timeout()), SLOT(checkQueue()), Qt::DirectConnection);

    timer->start();

    exec();
}
