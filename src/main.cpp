#include <QApplication>
#include <QProcess>

#include "decklink_tools.h"
#include "data_types.h"
#include "mainwindow.h"
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


int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

#ifndef __linux__

    if(!comInit())
        return 1;

#else

    if(!application.arguments().contains("--dont-check-root", Qt::CaseInsensitive))
        checkRoot();

#endif

    //

    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");
    qRegisterMetaType<qintptr>("qintptr");
    qRegisterMetaType<NRecStats>("NRecStats");

    FFEncoder::init();

    Settings::createInstance();

    MainWindow main_window;

    return application.exec();
}
