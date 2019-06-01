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
#include "keyboard_shortcuts.h"
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
    QVariantList lst_source_device=map_root.value(QStringLiteral("source_device")).toList();
    QVariantMap map_http_server=map_root.value(QStringLiteral("http_server")).toMap();
    QVariantMap map_keyboard_shortcuts=map_root.value(QStringLiteral("keyboard_shortcuts")).toMap();

    //

    foreach(const QVariant &var, lst_source_device) {
        SourceDevice device;
        setSourceDeviceSettings(&device, var.toMap());
        source_device.append(device);
    }

    if(source_device.isEmpty())
        source_device.append(SourceDevice());

    renumSourceDevices();

    //

    main.simplify_audio_for_send=map_main.value(QStringLiteral("simplify_audio_for_send"), 0).toInt();
    main.location_videos=map_main.value(QStringLiteral("location_videos"), store_location->videos()).toString();
    main.supported_enc=map_main.value(QStringLiteral("supported_enc")).toMap();

#ifdef LIB_QHTTP
    http_server.enabled=map_http_server.value(QStringLiteral("enabled"), true).toBool();
#else
    http_server.enabled=map_http_server.value(QStringLiteral("enabled"), false).toBool();
#endif
    http_server.port=map_http_server.value(QStringLiteral("port"), 8080).toUInt();


    keyboard_shortcuts.need_setup=map_keyboard_shortcuts.isEmpty();

    for(int i=0; i<KeyCodeC::enm_size; ++i) {
        keyboard_shortcuts.code.insert(KeyboardShortcuts::defaultQtKey(i), i);
    }

    for(int i=0; i<map_keyboard_shortcuts.size(); ++i) {
        QKeySequence seq(map_keyboard_shortcuts.values()[i].toString());

        keyboard_shortcuts.code.insert(seq.count()==1 ? seq[0] : Qt::Key_F1,
                                       KeyCodeC::fromString(map_keyboard_shortcuts.keys()[i]));
    }


    if(main.supported_enc.isEmpty())
        checkEncoders();


    return true;
}

bool Settings::save()
{
    QVariantMap map_root;
    QVariantMap map_main;
    QVariantList lst_source_device;
    QVariantMap map_http_server;
    QVariantMap map_keyboard_shortcuts;

    map_main.insert(QStringLiteral("simplify_audio_for_send"), main.simplify_audio_for_send);
    map_main.insert(QStringLiteral("location_videos"), main.location_videos);
    map_main.insert(QStringLiteral("supported_enc"), main.supported_enc);


    foreach(const SourceDevice &dev, source_device) {
        lst_source_device.append(getSourceDeviceSettings(dev));
    }


#ifdef LIB_QHTTP
    map_http_server.insert(QStringLiteral("enabled"), http_server.enabled);
    map_http_server.insert(QStringLiteral("port"), http_server.port);

    map_root.insert(QStringLiteral("http_server"), map_http_server);
#endif

    for(int i=0; i<KeyCodeC::enm_size; ++i)
        map_keyboard_shortcuts.insert(
                    KeyCodeC::toString(i),
                    QKeySequence(keyboard_shortcuts.code.key(i, KeyboardShortcuts::defaultQtKey(i))).toString()
                    );

    map_root.insert(QStringLiteral("main"), map_main);
    map_root.insert(QStringLiteral("source_device"), lst_source_device);
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

Settings::SourceDevice *Settings::sourceDevice(uint8_t index)
{
    if(index>=SourceDevice::MaxNum)
        return nullptr;

    while(source_device.size() - 1<index) {
        sourceDeviceAdd();
    }

    return &source_device[index];
}

Settings::SourceDevice *Settings::sourceDeviceAdd()
{
    if(source_device.size()>=SourceDevice::MaxNum)
        return nullptr;

    source_device.append(SourceDevice());

    source_device.last().rec=source_device.first().rec;

    renumSourceDevices();

    return &source_device[source_device.size() - 1];
}

void Settings::sourceDeviceRemove()
{
    if(source_device.size()>1)
        source_device.pop_back();
}

void Settings::renumSourceDevices()
{
    for(int i=0; i<source_device.size(); ++i) {
        source_device[i].group=QString("device-%1").arg(i + 1);
        source_device[i].group_settings=QString("setup-%1").arg(i + 1);
    }
}

QVariantMap Settings::getSourceDeviceSettings(const Settings::SourceDevice &device)
{
    QVariantMap map_root;
    QVariantMap map_dummy;
    QVariantMap map_ff;
    QVariantMap map_magewell;
    QVariantMap map_decklink;
    QVariantMap map_nvenc;
    QVariantMap map_rec;

    map_dummy.insert(QStringLiteral("framesize"), device.dummy_device.framesize);
    map_dummy.insert(QStringLiteral("show_frame_counter"), device.dummy_device.show_frame_counter);

    map_ff.insert(QStringLiteral("name_audio"), device.ff_device.name_audio);
    map_ff.insert(QStringLiteral("name_video"), device.ff_device.name_video);
    map_ff.insert(QStringLiteral("framesize"), device.ff_device.framesize);
    map_ff.insert(QStringLiteral("framerate"), device.ff_device.framerate);
    map_ff.insert(QStringLiteral("pixel_format"), device.ff_device.pixel_format);

    map_magewell.insert(QStringLiteral("index"), device.magewell.index);
    map_magewell.insert(QStringLiteral("pixel_format"), device.magewell.pixel_format);
    map_magewell.insert(QStringLiteral("framesize"), device.magewell.framesize);
    map_magewell.insert(QStringLiteral("color_format_in"), device.magewell.color_format_in);
    map_magewell.insert(QStringLiteral("color_format_out"), device.magewell.color_format_out);
    map_magewell.insert(QStringLiteral("quantization_range_in"), device.magewell.quantization_range_in);
    map_magewell.insert(QStringLiteral("quantization_range_out"), device.magewell.quantization_range_out);
    map_magewell.insert(QStringLiteral("audio_remap_mode"), device.magewell.audio_remap_mode);
    map_magewell.insert(QStringLiteral("low_latency"), device.magewell.low_latency);
    map_magewell.insert(QStringLiteral("half_fps"), device.magewell.half_fps);
    map_magewell.insert(QStringLiteral("pts_mode"), device.magewell.pts_mode);

    map_decklink.insert(QStringLiteral("index"), device.decklink.index);
    map_decklink.insert(QStringLiteral("audio_sample_size"), device.decklink.audio_sample_size);
    map_decklink.insert(QStringLiteral("index"), device.decklink.video_bitdepth);

    map_nvenc.insert(QStringLiteral("enabled"), device.rec.nvenc.enabled);
    map_nvenc.insert(QStringLiteral("device"), device.rec.nvenc.device);
    map_nvenc.insert(QStringLiteral("b_frames"), device.rec.nvenc.b_frames);
    map_nvenc.insert(QStringLiteral("ref_frames"), device.rec.nvenc.ref_frames);
    map_nvenc.insert(QStringLiteral("b_ref_mode"), device.rec.nvenc.b_ref_mode);
    map_nvenc.insert(QStringLiteral("gop_size"), device.rec.nvenc.gop_size);
    map_nvenc.insert(QStringLiteral("qp_i"), device.rec.nvenc.qp_i);
    map_nvenc.insert(QStringLiteral("qp_p"), device.rec.nvenc.qp_p);
    map_nvenc.insert(QStringLiteral("qp_b"), device.rec.nvenc.qp_b);
    map_nvenc.insert(QStringLiteral("aq_mode"), device.rec.nvenc.aq_mode);
    map_nvenc.insert(QStringLiteral("aq_strength"), device.rec.nvenc.aq_strength);
    map_nvenc.insert(QStringLiteral("rc_lookahead"), device.rec.nvenc.rc_lookahead);
    map_nvenc.insert(QStringLiteral("surfaces"), device.rec.nvenc.surfaces);
    map_nvenc.insert(QStringLiteral("no_scenecut"), device.rec.nvenc.no_scenecut);
    map_nvenc.insert(QStringLiteral("forced_idr"), device.rec.nvenc.forced_idr);
    map_nvenc.insert(QStringLiteral("b_adapt"), device.rec.nvenc.b_adapt);
    map_nvenc.insert(QStringLiteral("nonref_p"), device.rec.nvenc.nonref_p);
    map_nvenc.insert(QStringLiteral("strict_gop"), device.rec.nvenc.strict_gop);
    map_nvenc.insert(QStringLiteral("weighted_pred"), device.rec.nvenc.weighted_pred);
    map_nvenc.insert(QStringLiteral("bluray_compat"), device.rec.nvenc.bluray_compat);

    map_rec.insert(QStringLiteral("nvenc"), map_nvenc);
    map_rec.insert(QStringLiteral("encoder_audio"), device.rec.encoder_audio);
    map_rec.insert(QStringLiteral("encoder_video"), device.rec.encoder_video);
    map_rec.insert(QStringLiteral("pixel_format"), device.rec.pixel_format);
    map_rec.insert(QStringLiteral("preset"), device.rec.preset);
    map_rec.insert(QStringLiteral("crf"), device.rec.crf);
    map_rec.insert(QStringLiteral("half_fps"), device.rec.half_fps);
    map_rec.insert(QStringLiteral("direct_stream_copy"), device.rec.direct_stream_copy);
    map_rec.insert(QStringLiteral("downscale"), device.rec.downscale);
    map_rec.insert(QStringLiteral("scale_filter"), device.rec.scale_filter);
    map_rec.insert(QStringLiteral("color_primaries"), device.rec.color_primaries);
    map_rec.insert(QStringLiteral("color_space"), device.rec.color_space);
    map_rec.insert(QStringLiteral("color_transfer_characteristic"), device.rec.color_transfer_characteristic);
    map_rec.insert(QStringLiteral("sws_color_space_src"), device.rec.sws_color_space_src);
    map_rec.insert(QStringLiteral("sws_color_space_dst"), device.rec.sws_color_space_dst);
    map_rec.insert(QStringLiteral("sws_color_range_src"), device.rec.sws_color_range_src);
    map_rec.insert(QStringLiteral("sws_color_range_dst"), device.rec.sws_color_range_dst);

    map_root.insert(QStringLiteral("index"), device.index);
    map_root.insert(QStringLiteral("dummy"), map_dummy);
    map_root.insert(QStringLiteral("ff"), map_ff);
    map_root.insert(QStringLiteral("magewell"), map_magewell);
    map_root.insert(QStringLiteral("decklink"), map_decklink);
    map_root.insert(QStringLiteral("rec"), map_rec);

    return map_root;
}


void Settings::setSourceDeviceSettings(Settings::SourceDevice *device, const QVariantMap &map_root)
{
    device->index=map_root.value(QStringLiteral("index"), 0).toUInt();

    if(device->index<0)
        device->index=0;

    QVariantMap map_dummy=map_root.value(QStringLiteral("dummy")).toMap();
    QVariantMap map_ff=map_root.value(QStringLiteral("ff")).toMap();
    QVariantMap map_magewell=map_root.value(QStringLiteral("magewell")).toMap();
    QVariantMap map_decklink=map_root.value(QStringLiteral("decklink")).toMap();
    QVariantMap map_rec=map_root.value(QStringLiteral("rec")).toMap();
    QVariantMap map_nvenc=map_rec.value(QStringLiteral("nvenc")).toMap();

    device->dummy_device.framesize=map_dummy.value(QStringLiteral("framesize"), 0).toUInt();
    device->dummy_device.show_frame_counter=map_dummy.value(QStringLiteral("show_frame_counter"), false).toBool();

    device->ff_device.name_audio=map_ff.value(QStringLiteral("name_audio")).toString();
    device->ff_device.name_video=map_ff.value(QStringLiteral("name_video")).toString();
    device->ff_device.framesize=map_ff.value(QStringLiteral("framesize"), 0).toUInt();
    device->ff_device.framerate=map_ff.value(QStringLiteral("framerate"), 0).toUInt();
    device->ff_device.pixel_format=map_ff.value(QStringLiteral("pixel_format"), 0).toUInt();

    device->magewell.index=map_magewell.value(QStringLiteral("index"), 0).toUInt();
    device->magewell.pixel_format=map_magewell.value(QStringLiteral("pixel_format"), 0).toUInt();
    device->magewell.framesize=map_magewell.value(QStringLiteral("framesize"), 0).toUInt();
    device->magewell.color_format_in=map_magewell.value(QStringLiteral("color_format_in"), 0).toUInt();
    device->magewell.color_format_out=map_magewell.value(QStringLiteral("color_format_out"), 0).toUInt();
    device->magewell.quantization_range_in=map_magewell.value(QStringLiteral("quantization_range_in"), 0).toUInt();
    device->magewell.quantization_range_out=map_magewell.value(QStringLiteral("quantization_range_out"), 0).toUInt();
    device->magewell.audio_remap_mode=map_magewell.value(QStringLiteral("audio_remap_mode"), 0).toUInt();
    device->magewell.low_latency=map_magewell.value(QStringLiteral("low_latency"), 0).toUInt();
    device->magewell.half_fps=map_magewell.value(QStringLiteral("half_fps"), 0).toUInt();
    device->magewell.pts_mode=map_magewell.value(QStringLiteral("pts_mode"), 0).toUInt();

    device->decklink.index=map_decklink.value(QStringLiteral("index"), 0).toUInt();
    device->decklink.audio_sample_size=map_decklink.value(QStringLiteral("audio_sample_size"), 0).toUInt();
    device->decklink.video_bitdepth=map_decklink.value(QStringLiteral("index"), 0).toUInt();

    device->rec.encoder_audio=map_rec.value(QStringLiteral("encoder_audio"), 0).toInt();
    device->rec.encoder_video=map_rec.value(QStringLiteral("encoder_video"), 0).toInt();
    device->rec.pixel_format=map_rec.value(QStringLiteral("pixel_format")).toMap();
    device->rec.preset=map_rec.value(QStringLiteral("preset")).toMap();
    device->rec.crf=map_rec.value(QStringLiteral("crf"), 0).toInt();
    device->rec.half_fps=map_rec.value(QStringLiteral("half_fps"), 0).toInt();
    device->rec.direct_stream_copy=map_rec.value(QStringLiteral("direct_stream_copy"), 0).toInt();
    device->rec.downscale=map_rec.value(QStringLiteral("downscale"), FFEncoder::DownScale::Disabled).toInt();
    device->rec.scale_filter=map_rec.value(QStringLiteral("scale_filter"), FFEncoder::ScaleFilter::FastBilinear).toInt();
    device->rec.color_primaries=map_rec.value(QStringLiteral("color_primaries"), 0).toInt();
    device->rec.color_space=map_rec.value(QStringLiteral("color_space"), 0).toInt();
    device->rec.color_transfer_characteristic=map_rec.value(QStringLiteral("color_transfer_characteristic"), 0).toInt();
    device->rec.sws_color_space_src=map_rec.value(QStringLiteral("sws_color_space_src"), 0).toInt();
    device->rec.sws_color_space_dst=map_rec.value(QStringLiteral("sws_color_space_dst"), 0).toInt();
    device->rec.sws_color_range_src=map_rec.value(QStringLiteral("sws_color_range_src"), 0).toInt();
    device->rec.sws_color_range_dst=map_rec.value(QStringLiteral("sws_color_range_dst"), 0).toInt();

    device->rec.pixel_format_current=device->rec.pixel_format.value(QString::number(device->rec.encoder_video), 0).toInt();
    device->rec.preset_current=device->rec.preset.value(QString::number(device->rec.encoder_video), 0).toInt();

    device->rec.nvenc.enabled=map_nvenc.value(QStringLiteral("enabled"), 0).toInt();
    device->rec.nvenc.device=map_nvenc.value(QStringLiteral("device"), 0).toInt();
    device->rec.nvenc.b_frames=map_nvenc.value(QStringLiteral("b_frames"), 0).toInt();
    device->rec.nvenc.ref_frames=map_nvenc.value(QStringLiteral("ref_frames"), 0).toInt();
    device->rec.nvenc.b_ref_mode=map_nvenc.value(QStringLiteral("b_ref_mode"), 0).toInt();
    device->rec.nvenc.gop_size=map_nvenc.value(QStringLiteral("gop_size"), 0).toInt();
    device->rec.nvenc.qp_i=map_nvenc.value(QStringLiteral("qp_i"), 0).toInt();
    device->rec.nvenc.qp_p=map_nvenc.value(QStringLiteral("qp_p"), 0).toInt();
    device->rec.nvenc.qp_b=map_nvenc.value(QStringLiteral("qp_b"), 0).toInt();
    device->rec.nvenc.aq_mode=map_nvenc.value(QStringLiteral("aq_mode"), 0).toInt();
    device->rec.nvenc.aq_strength=map_nvenc.value(QStringLiteral("aq_strength"), 0).toInt();
    device->rec.nvenc.rc_lookahead=map_nvenc.value(QStringLiteral("rc_lookahead"), 0).toInt();
    device->rec.nvenc.surfaces=map_nvenc.value(QStringLiteral("surfaces"), 0).toInt();
    device->rec.nvenc.no_scenecut=map_nvenc.value(QStringLiteral("no_scenecut"), 0).toInt();
    device->rec.nvenc.forced_idr=map_nvenc.value(QStringLiteral("forced_idr"), 0).toInt();
    device->rec.nvenc.b_adapt=map_nvenc.value(QStringLiteral("b_adapt"), 0).toInt();
    device->rec.nvenc.nonref_p=map_nvenc.value(QStringLiteral("nonref_p"), 0).toInt();
    device->rec.nvenc.strict_gop=map_nvenc.value(QStringLiteral("strict_gop"), 0).toInt();
    device->rec.nvenc.weighted_pred=map_nvenc.value(QStringLiteral("weighted_pred"), 0).toInt();
    device->rec.nvenc.bluray_compat=map_nvenc.value(QStringLiteral("bluray_compat"), 0).toInt();
}

void Settings::checkEncoders()
{
    main.supported_enc.clear();

    foreach(FFEncoder::VideoEncoder::T enc, FFEncoder::VideoEncoder::list()) {
        QStringList lst_fmt;

        foreach(const PixelFormat &fmt, PixelFormat::list()) {
            if(fmt.isCompressed())
                continue;

            if(checkEncoder(FFEncoder::VideoEncoder::toEncName(enc), fmt.toAVPixelFormat()))
                lst_fmt << fmt.toString();
        }

        if(!lst_fmt.isEmpty()) {
            main.supported_enc[FFEncoder::VideoEncoder::toString(enc)]=lst_fmt;
        }
    }

    save();
}
