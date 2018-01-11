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
#include <QKeySequence>
#include <qcoreapplication.h>

#include "ff_encoder.h"
#include "data_types.h"
#include "dialog_keyboard_shortcuts.h"

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
    QVariantMap map_device_decklink=map_root.value(QStringLiteral("device")).toMap();
    QVariantMap map_device_cam=map_root.value(QStringLiteral("device_cam")).toMap();
    QVariantMap map_rec=map_root.value(QStringLiteral("rec")).toMap();
    QVariantMap map_http_server=map_root.value(QStringLiteral("http_server")).toMap();
    QVariantMap map_keyboard_shortcuts=map_root.value(QStringLiteral("keyboard_shortcuts")).toMap();

    main.preview=map_main.value(QStringLiteral("preview"), 1).toInt();
    main.smooth_transform=map_main.value(QStringLiteral("smooth_transform"), 0).toInt();

    device_decklink.index=map_device_decklink.value(QStringLiteral("index"), 0).toInt();
    device_decklink.audio_sample_size=map_device_decklink.value(QStringLiteral("audio_sample_size"), 0).toInt();
    device_decklink.half_fps=map_device_decklink.value(QStringLiteral("half_fps"), 0).toInt();
    device_decklink.rgb_10bit=map_device_decklink.value(QStringLiteral("rgb_10_bit"), 0).toInt();

    device_cam.index_video=map_device_cam.value(QStringLiteral("index_video"), 0).toInt();
    device_cam.index_audio=map_device_cam.value(QStringLiteral("index_audio"), 0).toInt();
    device_cam.resolution=map_device_cam.value(QStringLiteral("resolution"), 0).toInt();
    device_cam.framerate=map_device_cam.value(QStringLiteral("framerate"), 0).toInt();
    device_cam.pixel_format=map_device_cam.value(QStringLiteral("pixel_format"), 0).toInt();

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


    keyboard_shortcuts.need_setup=map_keyboard_shortcuts.isEmpty();

    for(int i=0; i<KeyCodeC::enm_size; ++i) {
        keyboard_shortcuts.code.insert(DialogKeyboardShortcuts::defaultQtKey(i), i);
    }

    for(int i=0; i<map_keyboard_shortcuts.size(); ++i) {
        QKeySequence seq(map_keyboard_shortcuts.values()[i].toString());

        keyboard_shortcuts.code.insert(seq.count()==1 ? seq[0] : Qt::Key_F1,
                                       KeyCodeC::fromString(map_keyboard_shortcuts.keys()[i]));
    }

    return true;
}

bool Settings::save()
{
    QVariantMap map_root;
    QVariantMap map_main;
    QVariantMap map_device_decklink;
    QVariantMap map_device_cam;
    QVariantMap map_rec;
    QVariantMap map_http_server;
    QVariantMap map_keyboard_shortcuts;


    map_main.insert(QStringLiteral("preview"), (bool)main.preview);
    map_main.insert(QStringLiteral("smooth_transform"), (bool)main.smooth_transform);

    map_device_decklink.insert(QStringLiteral("index"), device_decklink.index);
    map_device_decklink.insert(QStringLiteral("audio_sample_size"), device_decklink.audio_sample_size);
    map_device_decklink.insert(QStringLiteral("half_fps"), device_decklink.half_fps);
    map_device_decklink.insert(QStringLiteral("rgb_10_bit"), device_decklink.rgb_10bit);

    map_device_cam.insert(QStringLiteral("index_audio"), device_cam.index_audio);
    map_device_cam.insert(QStringLiteral("index_video"), device_cam.index_video);
    map_device_cam.insert(QStringLiteral("resolution"), device_cam.resolution);
    map_device_cam.insert(QStringLiteral("framerate"), device_cam.framerate);
    map_device_cam.insert(QStringLiteral("pixel_format"), device_cam.pixel_format);

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

    for(int i=0; i<KeyCodeC::enm_size; ++i)
        map_keyboard_shortcuts.insert(
                    KeyCodeC::toString(i),
                    QKeySequence(keyboard_shortcuts.code.key(i, DialogKeyboardShortcuts::defaultQtKey(i))).toString()
                    );

    map_root.insert(QStringLiteral("main"), map_main);
    map_root.insert(QStringLiteral("device_decklink"), map_device_decklink);
    map_root.insert(QStringLiteral("device_cam"), map_device_cam);
    map_root.insert(QStringLiteral("rec"), map_rec);
    map_root.insert(QStringLiteral("keyboard_shortcuts"), map_keyboard_shortcuts);


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
