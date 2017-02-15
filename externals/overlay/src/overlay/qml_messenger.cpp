#include <QDebug>
#include <QKeyEvent>

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

QStringList QmlMessenger::getModelVideoEncoder() const
{
    return model_video_encoder;
}

void QmlMessenger::setModelVideoEncoder(const QStringList &model)
{
    model_video_encoder=model;

    emit modelVideoEncoderChanged(model, QPrivateSignal());
}

QStringList QmlMessenger::getModelPixelFormat() const
{
    return model_pixel_format;
}

void QmlMessenger::setModelPixelFormat(const QStringList &model)
{
    model_pixel_format=model;

    emit modelPixelFormatChanged(model, QPrivateSignal());
}

void QmlMessenger::keyEvent(const Qt::Key &key)
{
    switch(key) {
    case Qt::Key_Menu:
        //qInfo() << "show_menu";
        emit showMenu();
        return;

    case Qt::Key_HomePage:
    case Qt::Key_Back:
        emit back();
        return;

    case Qt::Key_Return:

        keyPressed(Qt::Key_Right);
        return;


    default:
        break;
    }

    keyPressed(key);
}
