#include <QDebug>
#include <QApplication>

#include "mainwindow.h"
#include "qml_messenger.h"
#include "overlay_view.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow main_window;

    main_window.show();

    OverlayView overlay_view;

    overlay_view.setMessenger(new QmlMessenger());

/*
#ifdef QT_DEBUG

    if(QFile::exists("qml/Root.qml"))
        overlay_view.setSource(QStringLiteral("qml/Root.qml"));

    else if(QFile::exists("../qml/Root.qml"))
        overlay_view.setSource(QStringLiteral("../qml/Root.qml"));

    else {
        qCritical() << "qml path not found";
        exit(1);
    }

#else
*/
    overlay_view.setSource(QStringLiteral("qrc:/qml/Root.qml"));

// #endif


    overlay_view.show();
    // overlay_view.showFullScreen();

    main_window.raise();

    return app.exec();
}

