/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <QMutexLocker>
#include <QStandardPaths>
#include <qcoreapplication.h>

#include "settings.h"

Settings *Settings::_instance=nullptr;

Settings *Settings::createInstance(QObject *parent)
{
    if(_instance==nullptr)
        _instance=new Settings(parent);

    return _instance;
}

Settings *Settings::instance()
{
    return _instance;
}

//

QString filename()
{
    QString store_path=qApp->applicationDirPath();

#if defined(Q_OS_ANDROID)

    store_path=QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    qInfo() << "store_path:" << store_path;

#endif

    return store_path.isEmpty() ? "capturer_ctrl.json" : store_path + "/capturer_ctrl.json";
}

//

Settings::Settings(QObject *parent) :
    QObject(parent)
{
}

bool Settings::load()
{
    QMutexLocker ml(&mutex);

    QFile f(filename());

    QByteArray ba;

    if(f.open(QFile::ReadOnly)) {
        ba=f.readAll();

        f.close();
    }

    QCryptographicHash hash(QCryptographicHash::Md5);

    hash.addData(ba);

    ba_hash_file=hash.result();


    QVariantMap map_root=QJsonDocument::fromJson(ba).toVariant().toMap();

    //

    QVariantMap map_main=map_root.value(QStringLiteral("main")).toMap();

    main.host=map_main.value(QStringLiteral("host"), QStringLiteral("localhost")).toString();
    main.port=map_main.value(QStringLiteral("port"), 8080).toInt();
    // main.routing_key=map_main.value(QStringLiteral("routing_key")).toString();

    return true;
}

bool Settings::save()
{
    QMutexLocker ml(&mutex);

    QVariantMap map_root;
    QVariantMap map_main;

    map_main.insert(QStringLiteral("host"), main.host);
    map_main.insert(QStringLiteral("port"), main.port);
    // map_main.insert(QStringLiteral("routing_key"), main.routing_key);

    map_root.insert(QStringLiteral("main"), map_main);

    QByteArray ba=QJsonDocument::fromVariant(map_root).toJson();

    QCryptographicHash hash(QCryptographicHash::Md5);

    hash.addData(ba);

    if(ba_hash_file==hash.result())
        return true;

    //

    QFile f(filename());

    if(!f.open(QFile::ReadWrite | QFile::Truncate))
        return false;

    f.write(ba);

    f.close();

    return true;
}

QString Settings::host()
{
    QMutexLocker ml(&mutex);

    return main.host;
}

void Settings::setHost(const QString &value)
{
    QMutexLocker ml(&mutex);

    main.host=value;
}

quint16 Settings::port()
{
    QMutexLocker ml(&mutex);

    return main.port;
}

void Settings::setPort(const quint16 &value)
{
    QMutexLocker ml(&mutex);

    main.port=value;
}

QString Settings::routingKey()
{
    QMutexLocker ml(&mutex);

    return main.routing_key;
}

void Settings::setRoutingKey(const QString &value)
{
    QMutexLocker ml(&mutex);

    main.routing_key=value;
}
