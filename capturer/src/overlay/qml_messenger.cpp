/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <QApplication>
#include <QKeyEvent>
#include <QNetworkInterface>

#include "ff_tools.h"

#include "qml_messenger.h"

QmlMessenger::QmlMessenger(SettingsModel *settings_model, QObject *parent)
    : QObject(parent)
    , settings_model(settings_model)
{
    video_source_primary=new QuickVideoSource(this);
    video_source_secondary=new QuickVideoSource(this);


    connect(settings_model, SIGNAL(changed(SettingsModel*)), SIGNAL(settingsModelChanged(SettingsModel*)));


    file_system_model=new FileSystemModel();

    connect(file_system_model, SIGNAL(changed(FileSystemModel*)), SIGNAL(fileSystemModelChanged(FileSystemModel*)));

    file_system_model->setRootPath(getRootPath());
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

QString QmlMessenger::networkAddressesStr() const
{
    QString res;

    foreach(const QString address, networkAddresses())
        res+="\t" + address + "\n";

    return res;
}

QStringList QmlMessenger::networkAddresses() const
{
    QStringList sl;

    foreach(const QHostAddress &address, QNetworkInterface::allAddresses())
        if(address.protocol()==QAbstractSocket::IPv4Protocol && !address.isLoopback())
            sl << address.toString();

    return sl;
}

QuickVideoSource *QmlMessenger::videoSourcePrimary()
{
    return video_source_primary;
}

QuickVideoSource *QmlMessenger::videoSourceSecondary()
{
    return video_source_secondary;
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
