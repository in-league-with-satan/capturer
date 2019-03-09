/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QApplication>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QDateTime>

#include <signal.h>

#include "magewell_device.h"
#include "decklink_tools.h"
#include "data_types.h"
#include "mainwindow.h"
#include "store_location.h"
#include "settings.h"

#ifdef __linux__

#include <unistd.h>

void checkRoot()
{
    return;

    if(getuid()!=0) {
        if(!QProcess::startDetached(QString("gksu %1").arg(QApplication::applicationFilePath())))
            QProcess::startDetached(QApplication::applicationFilePath(), QStringList() << "--dont-check-root");

        exit(0);
    }
}

#endif

MainWindow *root_obj=nullptr;

void signal_handler(int signum)
{
    root_obj->deleteLater();

    qApp->exit(signum);
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);

    qSetMessagePattern("%{time hh:mm:ss.zzz}:%{qthreadptr}: %{file}(%{line}) %{function}: %{message}");

    qputenv("QML_DISABLE_DISK_CACHE", "true");

    QCoreApplication *application=nullptr;

    bool headless=false;

    for(int i=1; i<argc; ++i) {
        if(QString::compare(QString(argv[i]), QString("--headless"), Qt::CaseInsensitive)==0)
            headless=true;
    }

    if(headless)
        application=new QCoreApplication(argc, argv);

    else
        application=new QApplication(argc, argv);


    application->setApplicationName(QString("capturer (%1)").arg(QString(VERSION_STRING).split("-").first()));
    application->setApplicationVersion(QString(VERSION_STRING));

    if(application->arguments().contains("--log-file", Qt::CaseInsensitive)) {
        qInstallMessageHandler([](QtMsgType, const QMessageLogContext &context, const QString &msg) {
            static QMutex mutex;

            QMutexLocker ml(&mutex);

            static QFile file;

            if(!file.isOpen()) {
                file.setFileName(QApplication::applicationDirPath() + QString("/capturer_%1.log").arg(QDateTime::currentDateTime().toString(Qt::ISODate).replace("T", "_").replace(":", "-")));
                file.open(QFile::ReadWrite);
            }

            if(!file.isOpen())
                return;

            QTextStream stream(&file);

#ifdef QT_NO_MESSAGELOGCONTEXT

            stream << QString("%1: %2")
                      .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
                      .arg(msg);

#else

            stream << QString("%1 %2(%3) %4: %5")
                      .arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs))
                      .arg(context.file)
                      .arg(context.line)
                      .arg(context.function)
                      .arg(msg);

#endif

            endl(stream);
        });
    }


    MagewellDevice::init();


#ifndef __linux__

    if(!comInit())
        return 1;

#else

    if(!application->arguments().contains("--dont-check-root", Qt::CaseInsensitive))
        checkRoot();

#endif

    //

    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<qintptr>("qintptr");

    qRegisterMetaType<AVRational>("AVRational");

    qRegisterMetaType<NRecStats>("NRecStats");

    qRegisterMetaType<PixelFormat>("PixelFormat");

    qRegisterMetaType<SourceInterface::AudioChannels::T>("SourceInterface::AudioChannels::T");
    qRegisterMetaType<SourceInterface::AudioSampleSize::T>("SourceInterface::AudioSampleSize::T");

    qRegisterMetaType<MagewellDevice::Device>("MagewellDevice::Device");

    qRegisterMetaType<MGHCHANNEL>("MGHCHANNEL");

    qRegisterMetaType<FFEncoder::Config>("FFEncoder::Config");
    qRegisterMetaType<FFEncoder::Stats>("FFEncoder::Stats");

    qRegisterMetaType<NvState>("NvState");


    initLibAV();

    StoreLocation::createInstance();

    Settings::createInstance();

    qputenv("QML_DISABLE_DISK_CACHE", "true");

    root_obj=new MainWindow();

    return application->exec();
}
