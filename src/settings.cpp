#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QCryptographicHash>
#include <qcoreapplication.h>

#include "ff_encoder.h"

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
    QVariantMap map_device=map_root.value(QStringLiteral("device")).toMap();
    QVariantMap map_rec=map_root.value(QStringLiteral("rec")).toMap();
    QVariantMap map_http_server=map_root.value(QStringLiteral("http_server")).toMap();

    main.preview=map_main.value(QStringLiteral("preview"), 1).toInt();
    main.smooth_transform=map_main.value(QStringLiteral("smooth_transform"), 0).toInt();

    device.index=map_device.value(QStringLiteral("index"), 0).toInt();
    device.audio_sample_size=map_device.value(QStringLiteral("audio_sample_size"), 0).toInt();
    device.half_fps=map_device.value(QStringLiteral("half_fps"), 0).toInt();
    device.rgb_10bit=map_device.value(QStringLiteral("rgb_10_bit"), 0).toInt();

    rec.encoder=map_rec.value(QStringLiteral("encoder"), 0).toInt();
    rec.pixel_format=map_rec.value(QStringLiteral("pixel_format")).toMap();
    rec.preset=map_rec.value(QStringLiteral("preset")).toMap();
    rec.crf=map_rec.value(QStringLiteral("crf"), 0).toInt();
    rec.half_fps=map_rec.value(QStringLiteral("half_fps"), 0).toInt();
    rec.stop_rec_on_frames_drop=map_rec.value(QStringLiteral("stop_rec_on_frames_drop"), 0).toInt();
    rec.downscale=map_rec.value(QStringLiteral("downscale"), FFEncoder::DownScale::Disabled).toInt();
    rec.scale_filter=map_rec.value(QStringLiteral("scale_filter"), FFEncoder::ScaleFilter::FastBilinear).toInt();

#ifdef LIB_QHTTP
    http_server.enabled=map_http_server.value(QStringLiteral("enabled"), true).toBool();
#else
    http_server.enabled=map_http_server.value(QStringLiteral("enabled"), false).toBool();
#endif
    http_server.port=map_http_server.value(QStringLiteral("port"), 8080).toUInt();

    rec.pixel_format_current=rec.pixel_format.value(QString::number(rec.encoder), 0).toInt();
    rec.preset_current=rec.preset.value(QString::number(rec.encoder), 0).toInt();

    return true;
}

bool Settings::save()
{
    QVariantMap map_root;
    QVariantMap map_main;
    QVariantMap map_device;
    QVariantMap map_rec;
    QVariantMap map_http_server;


    map_main.insert(QStringLiteral("preview"), (bool)main.preview);
    map_main.insert(QStringLiteral("smooth_transform"), (bool)main.smooth_transform);

    map_device.insert(QStringLiteral("index"), device.index);
    map_device.insert(QStringLiteral("audio_sample_size"), device.audio_sample_size);
    map_device.insert(QStringLiteral("half_fps"), device.half_fps);
    map_device.insert(QStringLiteral("rgb_10_bit"), device.rgb_10bit);

    map_rec.insert(QStringLiteral("encoder"), rec.encoder);
    map_rec.insert(QStringLiteral("pixel_format"), rec.pixel_format);
    map_rec.insert(QStringLiteral("preset"), rec.preset);
    map_rec.insert(QStringLiteral("crf"), rec.crf);
    map_rec.insert(QStringLiteral("half_fps"), (bool)rec.half_fps);
    map_rec.insert(QStringLiteral("stop_rec_on_frames_drop"), (bool)rec.stop_rec_on_frames_drop);
    map_rec.insert(QStringLiteral("downscale"), rec.downscale);
    map_rec.insert(QStringLiteral("scale_filter"), rec.scale_filter);

#ifdef LIB_QHTTP
    map_http_server.insert(QStringLiteral("enabled"), http_server.enabled);
    map_http_server.insert(QStringLiteral("port"), http_server.port);

    map_root.insert(QStringLiteral("http_server"), map_http_server);
#endif

    map_root.insert(QStringLiteral("main"), map_main);
    map_root.insert(QStringLiteral("device"), map_device);
    map_root.insert(QStringLiteral("rec"), map_rec);

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
