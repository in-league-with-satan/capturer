#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
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
#ifdef USE_X264_10B

    return qApp->applicationDirPath() + "/capturer_10bit.json";

#endif

    return qApp->applicationDirPath() + "/capturer.json";
}

//

Settings::Settings(QObject *parent) :
    QObject(parent)
{
}

bool Settings::load()
{
    QFile f(filename());

    if(!f.open(QFile::ReadOnly))
        return false;

    QByteArray ba=f.readAll();

    f.close();

    QCryptographicHash hash(QCryptographicHash::Md5);

    hash.addData(ba);

    ba_hash_file=hash.result();


    QVariantMap map_root=QJsonDocument::fromJson(ba).toVariant().toMap();

    //

    QVariantMap map_main=map_root.value("main").toMap();
    QVariantMap map_device=map_root.value("device").toMap();
    QVariantMap map_rec=map_root.value("rec").toMap();


    main.preview=map_main.value("preview", 1).toInt();

    device.index=map_device.value("index", 0).toInt();
    device.audio_sample_size=map_device.value("audio_sample_size", 0).toInt();

    rec.encoder=map_rec.value("encoder", 0).toInt();
    rec.pixel_format=map_rec.value("pixel_format").toMap();
    rec.crf=map_rec.value("crf", 0).toInt();
    rec.half_fps=map_rec.value("half_fps", 0).toInt();
    rec.stop_rec_on_frames_drop=map_rec.value("stop_rec_on_frames_drop", 0).toInt();


    rec.pixel_format_current=rec.pixel_format.value(QString::number(rec.encoder), 0).toInt();

    return true;
}

bool Settings::save()
{
    QVariantMap map_root;
    QVariantMap map_main;
    QVariantMap map_device;
    QVariantMap map_rec;


    map_main.insert("preview", (bool)main.preview);

    map_device.insert("index", device.index);
    map_device.insert("audio_sample_size", device.audio_sample_size);

    map_rec.insert("encoder", rec.encoder);
    map_rec.insert("pixel_format", rec.pixel_format);
    map_rec.insert("crf", rec.crf);
    map_rec.insert("half_fps", (bool)rec.half_fps);
    map_rec.insert("stop_rec_on_frames_drop", (bool)rec.stop_rec_on_frames_drop);

    map_root.insert("main", map_main);
    map_root.insert("device", map_device);
    map_root.insert("rec", map_rec);

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
