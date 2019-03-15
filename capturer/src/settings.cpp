/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include "store_location.h"

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
    return store_location->config() + "/capturer.json";
}

//

Settings::Settings(QObject *parent) :
    QObject(parent)
{
    device_primary.group="primary device";
    device_primary.group_settings="primary device setup";

    device_secondary.group="secondary device";
    device_secondary.group_settings="secondary device setup";


    if(qApp->arguments().contains("--headless"))
        main.headless=1;
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
    QVariantMap map_device_primary=map_root.value(QStringLiteral("device_primary")).toMap();
    QVariantMap map_device_secondary=map_root.value(QStringLiteral("device_secondary")).toMap();
    QVariantMap map_rec=map_root.value(QStringLiteral("rec")).toMap();
    QVariantMap map_http_server=map_root.value(QStringLiteral("http_server")).toMap();
    QVariantMap map_keyboard_shortcuts=map_root.value(QStringLiteral("keyboard_shortcuts")).toMap();
    QVariantMap map_nvenc=map_root.value(QStringLiteral("nvenc")).toMap();

    QVariantMap map_device_primary_dummy=map_device_primary.value(QStringLiteral("dummy")).toMap();
    QVariantMap map_device_primary_ff=map_device_primary.value(QStringLiteral("ff")).toMap();
    QVariantMap map_device_primary_magewell=map_device_primary.value(QStringLiteral("magewell")).toMap();
    QVariantMap map_device_primary_decklink=map_device_primary.value(QStringLiteral("decklink")).toMap();

    QVariantMap map_device_secondary_dummy=map_device_secondary.value(QStringLiteral("dummy")).toMap();
    QVariantMap map_device_secondary_ff=map_device_secondary.value(QStringLiteral("ff")).toMap();
    QVariantMap map_device_secondary_magewell=map_device_secondary.value(QStringLiteral("magewell")).toMap();
    QVariantMap map_device_secondary_decklink=map_device_secondary.value(QStringLiteral("decklink")).toMap();


    main.simplify_audio_for_send=map_main.value(QStringLiteral("simplify_audio_for_send"), 0).toInt();
    main.location_videos=map_main.value(QStringLiteral("location_videos"), store_location->videos()).toString();

    device_primary.index=map_device_primary.value(QStringLiteral("index"), 0).toInt();

    device_primary.dummy_device.framesize=map_device_primary_dummy.value(QStringLiteral("framesize"), 0).toInt();
    device_primary.dummy_device.show_frame_counter=map_device_primary_dummy.value(QStringLiteral("show_frame_counter"), 0).toInt();

    device_primary.ff_device.index_video=map_device_primary_ff.value(QStringLiteral("index_video"), 0).toInt();
    device_primary.ff_device.index_audio=map_device_primary_ff.value(QStringLiteral("index_audio"), 0).toInt();
    device_primary.ff_device.framesize=map_device_primary_ff.value(QStringLiteral("framesize"), 0).toInt();
    device_primary.ff_device.framerate=map_device_primary_ff.value(QStringLiteral("framerate"), 0).toInt();
    device_primary.ff_device.pixel_format=map_device_primary_ff.value(QStringLiteral("pixel_format"), 0).toInt();

    device_primary.magewell.index=map_device_primary_magewell.value(QStringLiteral("index"), 0).toInt();
    device_primary.magewell.pixel_format=map_device_primary_magewell.value(QStringLiteral("pixel_format"), 0).toInt();
    device_primary.magewell.framesize=map_device_primary_magewell.value(QStringLiteral("framesize"), 0).toInt();
    device_primary.magewell.color_format_in=map_device_primary_magewell.value(QStringLiteral("color_format_in"), 0).toInt();
    device_primary.magewell.color_format_out=map_device_primary_magewell.value(QStringLiteral("color_format_out"), 0).toInt();
    device_primary.magewell.quantization_range_in=map_device_primary_magewell.value(QStringLiteral("quantization_range_in"), 0).toInt();
    device_primary.magewell.quantization_range_out=map_device_primary_magewell.value(QStringLiteral("quantization_range_out"), 0).toInt();
    device_primary.magewell.audio_remap_mode=map_device_primary_magewell.value(QStringLiteral("audio_remap_mode"), 0).toInt();
    device_primary.magewell.low_latency=map_device_primary_magewell.value(QStringLiteral("low_latency"), 0).toInt();
    device_primary.magewell.half_fps=map_device_primary_magewell.value(QStringLiteral("half_fps"), 0).toInt();
    device_primary.magewell.pts_mode=map_device_primary_magewell.value(QStringLiteral("pts_mode"), 0).toInt();

    device_primary.decklink.index=map_device_primary_decklink.value(QStringLiteral("index"), 0).toInt();
    device_primary.decklink.audio_sample_size=map_device_primary_decklink.value(QStringLiteral("audio_sample_size"), 0).toInt();
    device_primary.decklink.video_bitdepth=map_device_primary_decklink.value(QStringLiteral("video_bitdepth"), 0).toInt();


    device_secondary.index=map_device_secondary.value(QStringLiteral("index"), 0).toInt();

    device_secondary.dummy_device.framesize=map_device_secondary_dummy.value(QStringLiteral("framesize"), 0).toInt();
    device_secondary.dummy_device.show_frame_counter=map_device_secondary_dummy.value(QStringLiteral("show_frame_counter"), 0).toInt();

    device_secondary.ff_device.index_video=map_device_secondary_ff.value(QStringLiteral("index_video"), 0).toInt();
    device_secondary.ff_device.index_audio=map_device_secondary_ff.value(QStringLiteral("index_audio"), 0).toInt();
    device_secondary.ff_device.framesize=map_device_secondary_ff.value(QStringLiteral("framesize"), 0).toInt();
    device_secondary.ff_device.framerate=map_device_secondary_ff.value(QStringLiteral("framerate"), 0).toInt();
    device_secondary.ff_device.pixel_format=map_device_secondary_ff.value(QStringLiteral("pixel_format"), 0).toInt();

    device_secondary.magewell.index=map_device_secondary_magewell.value(QStringLiteral("index"), 0).toInt();
    device_secondary.magewell.pixel_format=map_device_secondary_magewell.value(QStringLiteral("pixel_format"), 0).toInt();
    device_secondary.magewell.framesize=map_device_secondary_magewell.value(QStringLiteral("framesize"), 0).toInt();
    device_secondary.magewell.color_format_out=map_device_secondary_magewell.value(QStringLiteral("color_format"), 0).toInt();
    device_secondary.magewell.quantization_range_out=map_device_secondary_magewell.value(QStringLiteral("quantization_range"), 0).toInt();
    device_secondary.magewell.audio_remap_mode=map_device_secondary_magewell.value(QStringLiteral("audio_remap_mode"), 0).toInt();
    device_secondary.magewell.low_latency=map_device_secondary_magewell.value(QStringLiteral("low_latency"), 0).toInt();
    device_secondary.magewell.half_fps=map_device_secondary_magewell.value(QStringLiteral("half_fps"), 0).toInt();
    device_secondary.magewell.pts_mode=map_device_secondary_magewell.value(QStringLiteral("pts_mode"), 0).toInt();

    device_secondary.decklink.index=map_device_secondary_decklink.value(QStringLiteral("index"), 0).toInt();
    device_secondary.decklink.audio_sample_size=map_device_secondary_decklink.value(QStringLiteral("audio_sample_size"), 0).toInt();
    device_secondary.decklink.video_bitdepth=map_device_secondary_decklink.value(QStringLiteral("video_bitdepth"), 0).toInt();


    rec.supported_enc=map_rec.value(QStringLiteral("supported_enc")).toMap();
    rec.encoder_audio=map_rec.value(QStringLiteral("encoder_audio"), 0).toInt();
    rec.encoder_video=map_rec.value(QStringLiteral("encoder_video"), 0).toInt();
    rec.pixel_format=map_rec.value(QStringLiteral("pixel_format")).toMap();
    rec.preset=map_rec.value(QStringLiteral("preset")).toMap();
    rec.crf=map_rec.value(QStringLiteral("crf"), 0).toInt();
    rec.half_fps=map_rec.value(QStringLiteral("half_fps"), 0).toInt();
    rec.downscale=map_rec.value(QStringLiteral("downscale"), FFEncoder::DownScale::Disabled).toInt();
    rec.scale_filter=map_rec.value(QStringLiteral("scale_filter"), FFEncoder::ScaleFilter::FastBilinear).toInt();
    rec.color_primaries=map_rec.value(QStringLiteral("color_primaries"), 0).toInt();
    rec.color_space=map_rec.value(QStringLiteral("color_space"), 0).toInt();
    rec.color_transfer_characteristic=map_rec.value(QStringLiteral("color_transfer_characteristic"), 0).toInt();
    rec.sws_color_space_src=map_rec.value(QStringLiteral("sws_color_space_src"), 0).toInt();
    rec.sws_color_space_dst=map_rec.value(QStringLiteral("sws_color_space_dst"), 0).toInt();
    rec.sws_color_range_src=map_rec.value(QStringLiteral("sws_color_range_src"), 0).toInt();
    rec.sws_color_range_dst=map_rec.value(QStringLiteral("sws_color_range_dst"), 0).toInt();

#ifdef LIB_QHTTP
    http_server.enabled=map_http_server.value(QStringLiteral("enabled"), true).toBool();
#else
    http_server.enabled=map_http_server.value(QStringLiteral("enabled"), false).toBool();
#endif
    http_server.port=map_http_server.value(QStringLiteral("port"), 8080).toUInt();

    rec.pixel_format_current=rec.pixel_format.value(QString::number(rec.encoder_video), 0).toInt();
    rec.preset_current=rec.preset.value(QString::number(rec.encoder_video), 0).toInt();


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
    QVariantMap map_device_primary;
    QVariantMap map_device_secondary;
    QVariantMap map_rec;
    QVariantMap map_http_server;
    QVariantMap map_keyboard_shortcuts;
    QVariantMap map_nvenc;

    QVariantMap map_device_primary_dummy;
    QVariantMap map_device_primary_ff;
    QVariantMap map_device_primary_magewell;
    QVariantMap map_device_primary_decklink;

    QVariantMap map_device_secondary_dummy;
    QVariantMap map_device_secondary_ff;
    QVariantMap map_device_secondary_magewell;
    QVariantMap map_device_secondary_decklink;


    map_main.insert(QStringLiteral("simplify_audio_for_send"), main.simplify_audio_for_send);
    map_main.insert(QStringLiteral("location_videos"), main.location_videos);

    map_device_primary_dummy.insert(QStringLiteral("framesize"), device_primary.dummy_device.framesize);
    map_device_primary_dummy.insert(QStringLiteral("show_frame_counter"), device_primary.dummy_device.show_frame_counter);

    map_device_primary_ff.insert(QStringLiteral("index_video"), device_primary.ff_device.index_video);
    map_device_primary_ff.insert(QStringLiteral("index_audio"), device_primary.ff_device.index_audio);
    map_device_primary_ff.insert(QStringLiteral("framesize"), device_primary.ff_device.framesize);
    map_device_primary_ff.insert(QStringLiteral("framerate"), device_primary.ff_device.framerate);
    map_device_primary_ff.insert(QStringLiteral("pixel_format"), device_primary.ff_device.pixel_format);

    map_device_primary_magewell.insert(QStringLiteral("index"), device_primary.magewell.index);
    map_device_primary_magewell.insert(QStringLiteral("pixel_format"), device_primary.magewell.pixel_format);
    map_device_primary_magewell.insert(QStringLiteral("framesize"), device_primary.magewell.framesize);
    map_device_primary_magewell.insert(QStringLiteral("color_format_in"), device_primary.magewell.color_format_in);
    map_device_primary_magewell.insert(QStringLiteral("color_format_out"), device_primary.magewell.color_format_out);
    map_device_primary_magewell.insert(QStringLiteral("quantization_range_in"), device_primary.magewell.quantization_range_in);
    map_device_primary_magewell.insert(QStringLiteral("quantization_range_out"), device_primary.magewell.quantization_range_out);
    map_device_primary_magewell.insert(QStringLiteral("audio_remap_mode"), device_primary.magewell.audio_remap_mode);
    map_device_primary_magewell.insert(QStringLiteral("low_latency"), device_primary.magewell.low_latency);
    map_device_primary_magewell.insert(QStringLiteral("half_fps"), device_primary.magewell.half_fps);
    map_device_primary_magewell.insert(QStringLiteral("pts_mode"), device_primary.magewell.pts_mode);

    map_device_primary_decklink.insert(QStringLiteral("index"), device_primary.decklink.index);
    map_device_primary_decklink.insert(QStringLiteral("audio_sample_size"), device_primary.decklink.audio_sample_size);
    map_device_primary_decklink.insert(QStringLiteral("index"), device_primary.decklink.video_bitdepth);

    map_device_primary.insert(QStringLiteral("index"), device_primary.index);
    map_device_primary.insert(QStringLiteral("dummy"), map_device_primary_dummy);
    map_device_primary.insert(QStringLiteral("ff"), map_device_primary_ff);
    map_device_primary.insert(QStringLiteral("magewell"), map_device_primary_magewell);
    map_device_primary.insert(QStringLiteral("decklink"), map_device_primary_decklink);



    map_device_secondary_dummy.insert(QStringLiteral("framesize"), device_secondary.dummy_device.framesize);
    map_device_secondary_dummy.insert(QStringLiteral("show_frame_counter"), device_secondary.dummy_device.show_frame_counter);

    map_device_secondary_ff.insert(QStringLiteral("index_video"), device_secondary.ff_device.index_video);
    map_device_secondary_ff.insert(QStringLiteral("index_audio"), device_secondary.ff_device.index_audio);
    map_device_secondary_ff.insert(QStringLiteral("framesize"), device_secondary.ff_device.framesize);
    map_device_secondary_ff.insert(QStringLiteral("framerate"), device_secondary.ff_device.framerate);
    map_device_secondary_ff.insert(QStringLiteral("pixel_format"), device_secondary.ff_device.pixel_format);

    map_device_secondary_magewell.insert(QStringLiteral("index"), device_secondary.magewell.index);
    map_device_secondary_magewell.insert(QStringLiteral("pixel_format"), device_secondary.magewell.pixel_format);
    map_device_secondary_magewell.insert(QStringLiteral("framesize"), device_secondary.magewell.framesize);
    map_device_secondary_magewell.insert(QStringLiteral("color_format"), device_secondary.magewell.color_format_out);
    map_device_secondary_magewell.insert(QStringLiteral("quantization_range"), device_secondary.magewell.quantization_range_out);
    map_device_secondary_magewell.insert(QStringLiteral("audio_remap_mode"), device_secondary.magewell.audio_remap_mode);
    map_device_secondary_magewell.insert(QStringLiteral("low_latency"), device_secondary.magewell.low_latency);
    map_device_secondary_magewell.insert(QStringLiteral("half_fps"), device_secondary.magewell.half_fps);
    map_device_secondary_magewell.insert(QStringLiteral("pts_mode"), device_secondary.magewell.pts_mode);

    map_device_secondary_decklink.insert(QStringLiteral("index"), device_secondary.decklink.index);
    map_device_secondary_decklink.insert(QStringLiteral("audio_sample_size"), device_secondary.decklink.audio_sample_size);
    map_device_secondary_decklink.insert(QStringLiteral("index"), device_secondary.decklink.video_bitdepth);

    map_device_secondary.insert(QStringLiteral("index"), device_secondary.index);
    map_device_secondary.insert(QStringLiteral("dummy"), map_device_secondary_dummy);
    map_device_secondary.insert(QStringLiteral("ff"), map_device_secondary_ff);
    map_device_secondary.insert(QStringLiteral("magewell"), map_device_secondary_magewell);
    map_device_secondary.insert(QStringLiteral("decklink"), map_device_secondary_decklink);


    map_rec.insert(QStringLiteral("supported_enc"), rec.supported_enc);
    map_rec.insert(QStringLiteral("encoder_audio"), rec.encoder_audio);
    map_rec.insert(QStringLiteral("encoder_video"), rec.encoder_video);
    map_rec.insert(QStringLiteral("pixel_format"), rec.pixel_format);
    map_rec.insert(QStringLiteral("preset"), rec.preset);
    map_rec.insert(QStringLiteral("crf"), rec.crf);
    map_rec.insert(QStringLiteral("half_fps"), (bool)rec.half_fps);
    map_rec.insert(QStringLiteral("downscale"), rec.downscale);
    map_rec.insert(QStringLiteral("scale_filter"), rec.scale_filter);

    map_rec.insert(QStringLiteral("color_primaries"), rec.color_primaries);
    map_rec.insert(QStringLiteral("color_space"), rec.color_space);
    map_rec.insert(QStringLiteral("color_transfer_characteristic"), rec.color_transfer_characteristic);

    map_rec.insert(QStringLiteral("sws_color_space_src"), rec.sws_color_space_src);
    map_rec.insert(QStringLiteral("sws_color_space_dst"), rec.sws_color_space_dst);
    map_rec.insert(QStringLiteral("sws_color_range_src"), rec.sws_color_range_src);
    map_rec.insert(QStringLiteral("sws_color_range_dst"), rec.sws_color_range_dst);


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
    map_root.insert(QStringLiteral("device_primary"), map_device_primary);
    map_root.insert(QStringLiteral("device_secondary"), map_device_secondary);
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

        foreach(const PixelFormat &fmt, PixelFormat::list()) {
            if(fmt.onlyForDevices())
                continue;

            if(checkEncoder(FFEncoder::VideoEncoder::toEncName(enc), fmt.toAVPixelFormat()))
                lst_fmt << fmt.toString();
        }

        if(!lst_fmt.isEmpty())
            rec.supported_enc[FFEncoder::VideoEncoder::toString(enc)]=lst_fmt;
    }

    save();
}
