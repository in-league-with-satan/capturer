/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QApplication>
#include <QProcess>
#include <QMutex>
#include <QMutexLocker>
#include <QFile>
#include <QDateTime>

#include <signal.h>
#include <iostream>

#include "magewell_device.h"
#include "decklink_tools.h"
#include "data_types.h"
#include "mainwindow.h"
#include "store_location.h"
#include "settings.h"
#include "magewell_lib.h"

#ifdef __linux__

#include <unistd.h>

void checkRoot()
{
    return;

    if(getuid()!=0) {
        if(!QProcess::startDetached(QString("gksu %1").arg(QApplication::applicationFilePath()), QStringList()))
            QProcess::startDetached(QApplication::applicationFilePath(), QStringList() << "--dont-check-root");

        exit(0);
    }
}

#endif

MainWindow *root_obj=nullptr;

void signal_handler(int signum)
{
    if(root_obj)
        root_obj->deleteLater();

    qApp->exit(signum);
}

void allocConsole()
{
#ifdef __WIN32__

    AttachConsole(ATTACH_PARENT_PROCESS);
    freopen("CONOUT$", "w", stdout);
    HANDLE h_stdout=CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    SetStdHandle(STD_OUTPUT_HANDLE, h_stdout);

#endif
}

void printVersion()
{
    std::cout << VERSION_STRING << std::endl;
}

void printHelp()
{
    std::cout << QString("capturer v%1").arg(VERSION_STRING).toStdString() << std::endl;
    std::cout << std::endl;
    std::cout << "options:" << std::endl;
    std::cout << "\t" << "--version" << std::endl << "\t\t" << "show application version" << std::endl << std::endl;
    std::cout << "\t" << "--headless" << std::endl << "\t\t" << "start application without gui" << std::endl << std::endl;
#ifdef LIB_CURSES
    std::cout << "\t" << "--headless-curse" << std::endl << "\t\t" << "start application with ncurses interface" << std::endl << std::endl;
#endif
    std::cout << "\t" << "--log-file" << std::endl << "\t\t" << "redirect std-err/out to file \"capturer_STARTAPPDATETIME.log\"" << std::endl << std::endl;
    std::cout << "\t" << "--portable-mode" << std::endl << "\t\t" << "store config file in application dir" << std::endl << std::endl;
    std::cout << "\t" << "--portable-mode=path_to_dir" << std::endl << "\t\t" << "set custom location for config file" << std::endl << std::endl;
    std::cout << "\t" << "--setup" << std::endl << "\t\t" << "show setup dialog. only in gui mode" << std::endl << std::endl;
    std::cout << "\t" << "--windowed" << std::endl << "\t\t" << "start application windowed. only in gui mode" << std::endl << std::endl;
}

void printHelp(int argc, char *argv[])
{
    for(int i=1; i<argc; ++i) {
        if(QString::compare(QString(argv[i]), QString("--version"), Qt::CaseInsensitive)==0) {
            allocConsole();
            printVersion();
            exit(0);
        }

        if(QString::compare(QString(argv[i]), QString("--help"), Qt::CaseInsensitive)==0) {
            allocConsole();
            printHelp();
            exit(0);
        }
    }
}

int main(int argc, char *argv[])
{
    printHelp(argc, argv);

    signal(SIGINT, signal_handler);

    qSetMessagePattern("%{time hh:mm:ss.zzz}:%{qthreadptr}: %{file}(%{line}) %{function}: %{message}");

    qputenv("QML_DISABLE_DISK_CACHE", "true");

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QCoreApplication *application=nullptr;

    bool headless=false;

    for(int i=1; i<argc; ++i) {
        if(QString::compare(QString(argv[i]), QString("--headless"), Qt::CaseInsensitive)==0
                || QString::compare(QString(argv[i]), QString("--headless-curse"), Qt::CaseInsensitive)==0)
            headless=true;
    }

    if(headless) {
#ifdef __WIN32__

        // AttachConsole(ATTACH_PARENT_PROCESS);
        AllocConsole();

        freopen("CONIN$", "r", stdin);
        freopen("CONOUT$", "w", stdout);
        freopen("CONOUT$", "w", stderr);

        HANDLE h_stdin=CreateFileA("CONIN$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        HANDLE h_stdout=CreateFileA("CONOUT$", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        SetStdHandle(STD_INPUT_HANDLE, h_stdin);
        SetStdHandle(STD_OUTPUT_HANDLE, h_stdout);

#else

        // setlocale(LC_ALL, "");

#endif

        application=new QCoreApplication(argc, argv);

    } else
        application=new QApplication(argc, argv);


    application->setApplicationName(QString("capturer (%1)").arg(QString(VERSION_STRING)));
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

            Qt::endl(stream);
        });

    } else if(application->arguments().contains("--headless-curse", Qt::CaseInsensitive)) {
#ifdef LIB_CURSES

        qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString&) {});

#endif
    }


    MagewellLib::init();

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

    root_obj=new MainWindow();

    QObject::connect(application, SIGNAL(aboutToQuit()), root_obj, SLOT(deleteLater()));

    return application->exec();
}
