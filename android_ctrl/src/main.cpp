#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include "qml_messenger.h"


int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    QmlMessenger qml_messenger;

    engine.rootContext()->setContextProperty("messenger", &qml_messenger);

#if defined(Q_OS_ANDROID)

    engine.load(QUrl(QStringLiteral("qrc:/qml/main_android.qml")));

#else

    engine.load(QUrl(QStringLiteral("qrc:/qml/main_win.qml")));

#endif

    if(engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
