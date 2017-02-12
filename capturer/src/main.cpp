#include <QApplication>

#include "ffmpeg.h"

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);


    qRegisterMetaType<int64_t>("int64_t");
    qRegisterMetaType<size_t>("size_t");


    FFMpeg::init();

    MainWindow main_window;

    main_window.show();

    // application.installEventFilter(&main_window);

    return application.exec();
}
