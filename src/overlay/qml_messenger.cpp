#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>
#include <QStorageInfo>
#include <QNetworkInterface>

#include "ff_tools.h"

#include "qml_messenger.h"

QmlMessenger::QmlMessenger(QObject *parent)
    : QObject(parent)
{
    video_source_main=new QuickVideoSource(false, this);

    settings_model=new SettingsModel();

    connect(settings_model, SIGNAL(changed(SettingsModel*)), SIGNAL(settingsModelChanged(SettingsModel*)));


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

SettingsModel *QmlMessenger::settingsModel()
{
    return settings_model;
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

QString QmlMessenger::networkAddresses() const
{
    QString res;

    foreach(const QHostAddress &address, QNetworkInterface::allAddresses())
        if(address.protocol()==QAbstractSocket::IPv4Protocol && !address.isLoopback())
            res+="\t" + address.toString() + "\n";

    return res;
}

QuickVideoSource *QmlMessenger::videoSourceMain()
{
    return video_source_main;
}

FileSystemModel *QmlMessenger::fileSystemModel()
{
    return file_system_model;
}

void QmlMessenger::fileBrowserVisibleState(bool visible)
{
    file_system_model->fileBrowserVisibleState(visible);
}

QString QmlMessenger::getRootPath()
{
    return qApp->applicationDirPath() + "/videos";
}

void QmlMessenger::keyEvent(const Qt::Key &key)
{
    Q_UNUSED(key)
}

void QmlMessenger::setRecStarted(bool value)
{
    file_system_model->disableSnapshots(value);

    if(value) {
        emit recStarted();

    } else {
        emit recStopped();

    }
}

void QmlMessenger::checkFreeSpace()
{
    QStorageInfo info=QStorageInfo(QApplication::applicationDirPath() + "/videos");

    emit freeSpace(info.bytesAvailable());
    emit freeSpaceStr(QString("%1 MB").arg(QLocale().toString(info.bytesAvailable()/1024/1024)));
}
