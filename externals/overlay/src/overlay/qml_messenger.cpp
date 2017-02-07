#include <QDebug>
#include <QDateTime>

#include "qml_messenger.h"

QDateTime dt_start;
int size;

QmlMessenger::QmlMessenger(QObject *parent) :
    QObject(parent)
{
    dt_start=QDateTime::currentDateTimeUtc();
    size=0;
}

QmlMessenger::~QmlMessenger()
{
}

void QmlMessenger::hello()
{
    // qInfo() << "hello";


    emit updateRecStats(QDateTime::fromMSecsSinceEpoch(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - dt_start.toMSecsSinceEpoch()).toUTC().toString("HH:mm:ss.zzz"),
                        QString::number(48*1024 + qrand()%128*1024),
                        QString::number(size));

    size+=qrand()%128*1024;
}
