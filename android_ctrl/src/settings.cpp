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
    QString store_path;

#if defined(Q_OS_ANDROID)

    store_path=QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);

    qInfo() << "store_path:" << store_path;

#endif

    return store_path.isEmpty() ? "capturer.json" : store_path + "/capturer.json";
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
