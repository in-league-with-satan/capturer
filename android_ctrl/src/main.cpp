/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
