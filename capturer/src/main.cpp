#include <QApplication>

#include "ffmpeg.h"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");


    FFMpeg::init();

    MainWindow w;
    w.show();

    return a.exec();
}
