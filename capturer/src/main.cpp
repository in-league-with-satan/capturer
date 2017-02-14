#include <QApplication>

#include "decklink_tools.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
#ifndef __linux__

    if(!comInit())
        return 1;

#endif

    //

    QApplication a(argc, argv);

    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");


    FFMpeg::init();

    MainWindow w;
    w.show();

    return a.exec();
}
