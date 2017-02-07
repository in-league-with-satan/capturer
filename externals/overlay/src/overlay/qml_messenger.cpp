#include <QDebug>

#include "qml_messenger.h"

QmlMessenger::QmlMessenger(QObject *parent) :
    QObject(parent)
{
}

QmlMessenger::~QmlMessenger()
{
}

void QmlMessenger::recStats(QString duration, QString bitrate, QString size)
{
    emit updateRecStats(duration,  bitrate,  size);
}

