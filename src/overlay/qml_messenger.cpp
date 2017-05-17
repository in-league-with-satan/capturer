#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QStorageInfo>

#include "ff_tools.h"

#include "qml_messenger.h"

QmlMessenger::QmlMessenger(QObject *parent)
    : QObject(parent)
{
    file_system_model=new FileSystemModel();

    connect(file_system_model, SIGNAL(changed(FileSystemModel*)), SIGNAL(fileSystemModelChanged(FileSystemModel*)));

    file_system_model->setRootPath(getRootPath());

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


FileSystemModel *QmlMessenger::fileSystemModel()
{
    return file_system_model;
}

QString QmlMessenger::getRootPath()
{
    return qApp->applicationDirPath() + "/videos";
}

void QmlMessenger::keyEvent(const Qt::Key &key)
{
    Q_UNUSED(key)
}

void QmlMessenger::checkFreeSpace()
{
    QStorageInfo info=QStorageInfo(QApplication::applicationDirPath() + "/videos");

    emit freeSpace(QString("%1 MB").arg(QLocale().toString(info.bytesAvailable()/1024/1024)));
}
