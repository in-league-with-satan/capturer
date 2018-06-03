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
    return qApp->applicationDirPath() + "/capturer.json";
}

//

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    foreach(const QString &arg, qApp->arguments()) {
        if(arg.contains("decklink_dummy", Qt::CaseInsensitive)) {
            device_decklink.dummy.enabled=true;

            if(arg.contains("frame_counter", Qt::CaseInsensitive))
                device_decklink.dummy.frame_counter=true;

            if(arg.contains("480"))
                device_decklink.dummy.frame_height=480;

            else if(arg.contains("720"))
                device_decklink.dummy.frame_height=720;

            else if(arg.contains("1080"))
                device_decklink.dummy.frame_height=1080;

            else if(arg.contains("2160"))
                device_decklink.dummy.frame_height=2160;

            break;
        }
    }
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
    QVariantMap map_device_decklink=map_root.value(QStringLiteral("device_decklink")).toMap();
    QVariantMap map_device_cam=map_root.value(QStringLiteral("device_cam")).toMap();
    QVariantMap map_rec=map_root.value(QStringLiteral("rec")).toMap();
    QVariantMap map_http_server=map_root.value(QStringLiteral("http_server")).toMap();
    QVariantMap map_keyboard_shortcuts=map_root.value(QStringLiteral("keyboard_shortcuts")).toMap();
    QVariantMap map_nvenc=map_root.value(QStringLiteral("nvenc")).toMap();


    main.preview=map_main.value(QStringLiteral("preview"), 1).toInt();
    main.smooth_transform=map_main.value(QStringLiteral("smooth_transform"), 0).toInt();

    device_decklink.index=map_device_decklink.value(QStringLiteral("index"), 0).toInt();
    device_decklink.audio_sample_size=map_device_decklink.value(QStringLiteral("audio_sample_size"), 0).toInt();
    device_decklink.half_fps=map_device_decklink.value(QStringLiteral("half_fps"), 0).toInt();
    device_decklink.video_depth_10bit=map_device_decklink.value(QStringLiteral("video_depth_10bit"), 0).toInt();

    device_cam.index_video=map_device_cam.value(QStringLiteral("index_video"), 0).toInt();
    device_cam.index_audio=map_device_cam.value(QStringLiteral("index_audio"), 0).toInt();
    device_cam.resolution=map_device_cam.value(QStringLiteral("resolution"), 0).toInt();
    device_cam.framerate=map_device_cam.value(QStringLiteral("framerate"), 0).toInt();
    device_cam.pixel_format=map_device_cam.value(QStringLiteral("pixel_format"), 0).toInt();

    rec.supported_enc=map_rec.value(QStringLiteral("supported_enc")).toMap();
    rec.encoder=map_rec.value(QStringLiteral("encoder"), 0).toInt();
    rec.pixel_format=map_rec.value(QStringLiteral("pixel_format")).toMap();
    rec.preset=map_rec.value(QStringLiteral("preset")).toMap();
    rec.crf=map_rec.value(QStringLiteral("crf"), 0).toInt();
    rec.half_fps=map_rec.value(QStringLiteral("half_fps"), 0).toInt();
    rec.stop_rec_on_frames_drop=map_rec.value(QStringLiteral("stop_rec_on_frames_drop"), 0).toInt();
    rec.downscale=map_rec.value(QStringLiteral("downscale"), FFEncoder::DownScale::Disabled).toInt();
    rec.scale_filter=map_rec.value(QStringLiteral("scale_filter"), FFEncoder::ScaleFilter::FastBilinear).toInt();
    rec.color_primaries=map_rec.value(QStringLiteral("color_primaries"), -1).toInt();
    rec.color_space=map_rec.value(QStringLiteral("color_space"), -1).toInt();
    rec.color_transfer_characteristic=map_rec.value(QStringLiteral("color_transfer_characteristic"), -1).toInt();

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

    nvenc.enabled=map_nvenc.value(QStringLiteral("enabled"), 0).toInt();
    nvenc.device=map_nvenc.value(QStringLiteral("device"), 0).toInt();
    nvenc.b_frames=map_nvenc.value(QStringLiteral("b_frames"), 0).toInt();
    nvenc.ref_frames=map_nvenc.value(QStringLiteral("ref_frames"), 0).toInt();
    nvenc.gop_size=map_nvenc.value(QStringLiteral("gop_size"), 0).toInt();
    nvenc.qp_i=map_nvenc.value(QStringLiteral("qp_i"), 0).toInt();
    nvenc.qp_p=map_nvenc.value(QStringLiteral("qp_p"), 0).toInt();
    nvenc.qp_b=map_nvenc.value(QStringLiteral("qp_b"), 0).toInt();
    nvenc.aq_mode=map_nvenc.value(QStringLiteral("aq_mode"), 0).toInt();
    nvenc.aq_strength=map_nvenc.value(QStringLiteral("aq_strength"), 0).toInt();
    nvenc.rc_lookahead=map_nvenc.value(QStringLiteral("rc_lookahead"), 0).toInt();
    nvenc.surfaces=map_nvenc.value(QStringLiteral("surfaces"), 0).toInt();
    nvenc.no_scenecut=map_nvenc.value(QStringLiteral("no_scenecut"), 0).toInt();
    nvenc.forced_idr=map_nvenc.value(QStringLiteral("forced_idr"), 0).toInt();
    nvenc.b_adapt=map_nvenc.value(QStringLiteral("b_adapt"), 0).toInt();
    nvenc.nonref_p=map_nvenc.value(QStringLiteral("nonref_p"), 0).toInt();
    nvenc.strict_gop=map_nvenc.value(QStringLiteral("strict_gop"), 0).toInt();
    nvenc.weighted_pred=map_nvenc.value(QStringLiteral("weighted_pred"), 0).toInt();
    nvenc.bluray_compat=map_nvenc.value(QStringLiteral("bluray_compat"), 0).toInt();


    if(rec.supported_enc.isEmpty())
        checkEncoders();


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
    QVariantMap map_nvenc;


    map_main.insert(QStringLiteral("preview"), (bool)main.preview);
    map_main.insert(QStringLiteral("smooth_transform"), (bool)main.smooth_transform);

    map_device_decklink.insert(QStringLiteral("index"), device_decklink.index);
    map_device_decklink.insert(QStringLiteral("audio_sample_size"), device_decklink.audio_sample_size);
    map_device_decklink.insert(QStringLiteral("half_fps"), device_decklink.half_fps);
    map_device_decklink.insert(QStringLiteral("video_depth_10bit"), device_decklink.video_depth_10bit);

    map_device_cam.insert(QStringLiteral("index_audio"), device_cam.index_audio);
    map_device_cam.insert(QStringLiteral("index_video"), device_cam.index_video);
    map_device_cam.insert(QStringLiteral("resolution"), device_cam.resolution);
    map_device_cam.insert(QStringLiteral("framerate"), device_cam.framerate);
    map_device_cam.insert(QStringLiteral("pixel_format"), device_cam.pixel_format);

    map_rec.insert(QStringLiteral("supported_enc"), rec.supported_enc);
    map_rec.insert(QStringLiteral("encoder"), rec.encoder);
    map_rec.insert(QStringLiteral("pixel_format"), rec.pixel_format);
    map_rec.insert(QStringLiteral("preset"), rec.preset);
    map_rec.insert(QStringLiteral("crf"), rec.crf);
    map_rec.insert(QStringLiteral("half_fps"), (bool)rec.half_fps);
    map_rec.insert(QStringLiteral("stop_rec_on_frames_drop"), (bool)rec.stop_rec_on_frames_drop);
    map_rec.insert(QStringLiteral("downscale"), rec.downscale);
    map_rec.insert(QStringLiteral("scale_filter"), rec.scale_filter);

    map_rec.insert(QStringLiteral("color_primaries"), rec.color_primaries);
    map_rec.insert(QStringLiteral("color_space"), rec.color_space);
    map_rec.insert(QStringLiteral("color_transfer_characteristic"), rec.color_transfer_characteristic);

    map_rec.insert(QStringLiteral("color_primaries_help"), "RESERVED0=0, BT709=1, UNSPECIFIED=2, RESERVED=3, BT470M=4, BT470BG=5, SMPTE170M=6, "
                                                           "SMPTE240M=7, FILM=8, BT2020=9, SMPTE428=10, SMPTE431=11, SMPTE432=12, JEDEC_P22=22");
    map_rec.insert(QStringLiteral("color_space_help"), "BT709=1, UNSPECIFIED=2, GAMMA22=4, GAMMA28=5, SMPTE170M=6, SMPTE240M=7, LINEAR=8, "
                                                       "LOG=9, LOG_SQRT=10, IEC61966_2_4=11, BT1361_ECG=12, IEC61966_2_1=13, BT2020_10=14, "
                                                       "BT2020_12=15, SMPTE2084=16, SMPTE428=17, ARIB_STD_B67=18");
    map_rec.insert(QStringLiteral("color_transfer_characteristic_help"), "RGB=0, BT709=1, UNSPECIFIED=2, FCC=4, BT470BG=5, SMPTE170M=6, SMPTE240M=7, YCGCO=8, "
                                                                         "BT2020_NCL=9, BT2020_CL=10, SMPTE2085=11, CHROMA_DERIVED_NCL=12, CHROMA_DERIVED_CL=13, "
                                                                         "ICTCP=14");

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

    map_nvenc.insert(QStringLiteral("enabled"), (bool)nvenc.enabled);
    map_nvenc.insert(QStringLiteral("device"), nvenc.device);
    map_nvenc.insert(QStringLiteral("b_frames"), nvenc.b_frames);
    map_nvenc.insert(QStringLiteral("ref_frames"), nvenc.ref_frames);
    map_nvenc.insert(QStringLiteral("gop_size"), nvenc.gop_size);
    map_nvenc.insert(QStringLiteral("qp_i"), nvenc.qp_i);
    map_nvenc.insert(QStringLiteral("qp_p"), nvenc.qp_p);
    map_nvenc.insert(QStringLiteral("qp_b"), nvenc.qp_b);
    map_nvenc.insert(QStringLiteral("aq_mode"), nvenc.aq_mode);
    map_nvenc.insert(QStringLiteral("aq_strength"), nvenc.aq_strength);
    map_nvenc.insert(QStringLiteral("rc_lookahead"), nvenc.rc_lookahead);
    map_nvenc.insert(QStringLiteral("surfaces"), nvenc.surfaces);
    map_nvenc.insert(QStringLiteral("no_scenecut"), (bool)nvenc.no_scenecut);
    map_nvenc.insert(QStringLiteral("forced_idr"), (bool)nvenc.forced_idr);
    map_nvenc.insert(QStringLiteral("b_adapt"), (bool)nvenc.b_adapt);
    map_nvenc.insert(QStringLiteral("nonref_p"), (bool)nvenc.nonref_p);
    map_nvenc.insert(QStringLiteral("strict_gop"), (bool)nvenc.strict_gop);
    map_nvenc.insert(QStringLiteral("bluray_compat"), (bool)nvenc.bluray_compat);
    map_nvenc.insert(QStringLiteral("weighted_pred"), (bool)nvenc.weighted_pred);


    map_root.insert(QStringLiteral("main"), map_main);
    map_root.insert(QStringLiteral("device_decklink"), map_device_decklink);
    map_root.insert(QStringLiteral("device_cam"), map_device_cam);
    map_root.insert(QStringLiteral("rec"), map_rec);
    map_root.insert(QStringLiteral("keyboard_shortcuts"), map_keyboard_shortcuts);
    map_root.insert(QStringLiteral("nvenc"), map_nvenc);


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

void Settings::checkEncoders()
{
    rec.supported_enc.clear();

    foreach(FFEncoder::VideoEncoder::T enc, FFEncoder::VideoEncoder::list()) {
        QStringList lst_fmt;

        foreach(FFEncoder::PixelFormat::T fmt, FFEncoder::PixelFormat::list()) {
            if(checkEncoder(FFEncoder::VideoEncoder::toEncName(enc), (AVPixelFormat)fmt))
                lst_fmt << FFEncoder::PixelFormat::toString(fmt);
        }

        if(!lst_fmt.isEmpty())
            rec.supported_enc[FFEncoder::VideoEncoder::toString(enc)]=lst_fmt;
    }

    save();
}
