#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QStorageInfo>

#include "ffmpeg_tools.h"

#include "qml_messenger.h"

QmlMessenger::QmlMessenger(QObject *parent)
    : QObject(parent)
{
    QTimer *timer=new QTimer();

    connect(timer, SIGNAL(timeout()), SLOT(checkFreeSpace()));

    timer->start(1000);
}

QmlMessenger::~QmlMessenger()
{
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

QString QmlMessenger::versionThis() const
{
    return QString(VERSION_STRING);
}

QString QmlMessenger::versionLibAVUtil() const
{
    return versionlibavutil();
}

QString QmlMessenger::versionlibAVCodec() const
{
    return versionlibavcodec();
}

QString QmlMessenger::versionlibAVFormat() const
{
    return versionlibavformat();
}

QString QmlMessenger::versionlibAVFilter() const
{
    return versionlibavfilter();
}

QString QmlMessenger::versionlibSWScale() const
{
    return versionlibswscale();
}

QString QmlMessenger::versionlibSWResample() const
{
    return versionlibswresample();
}

void QmlMessenger::keyEvent(const Qt::Key &key)
{
    switch(key) {
    case Qt::Key_Menu:
    case Qt::Key_Space:
        //qInfo() << "show_menu";
        emit showMenu();
        return;

    case Qt::Key_HomePage:
    case Qt::Key_Back:
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
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

void QmlMessenger::checkFreeSpace()
{
    QStorageInfo info=QStorageInfo(QApplication::applicationDirPath() + "/videos");

    emit freeSpace(QString("%1 MB").arg(QLocale().toString(info.bytesAvailable()/1024/1024)));
}
