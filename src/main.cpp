#include <QApplication>

#include "ffmpeg.h"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FFMpeg::init();

    MainWindow w;
    w.show();

    return a.exec();
}
