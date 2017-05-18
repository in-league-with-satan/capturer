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
