/******************************************************************************

Copyright © 2018-2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
    if(qApp->arguments().contains("--headless")) {
        main.headless=1;
    }

    if(qApp->arguments().contains("--headless-curse")) {
        main.headless=1;
        main.headless_curse=1;
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
    QVariantList lst_source_device=map_root.value(QStringLiteral("source_device")).toList();
    QVariantMap map_http_server=map_root.value(QStringLiteral("http_server")).toMap();
    QVariantMap map_keyboard_shortcuts=map_root.value(QStringLiteral("keyboard_shortcuts")).toMap();
    QVariantMap map_streaming=map_root.value(QStringLiteral("streaming")).toMap();

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

    //

    QVariantMap map_streaming_service_url_example;
    map_streaming_service_url_example.insert("example", "rtmp://127.0.0.1:1935");

    recLoad(&streaming.rec, map_streaming.value(QStringLiteral("rec")).toMap());
    streaming.url=map_streaming.value(QStringLiteral("url"), QVariantList() << map_streaming_service_url_example).toList();
    streaming.url_index=map_streaming.value(QStringLiteral("url_index")).toInt();

    //

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
    QVariantMap map_streaming;

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

    map_streaming.insert(QStringLiteral("rec"), recSave(streaming.rec));
    map_streaming.insert(QStringLiteral("url"), streaming.url);
    map_streaming.insert(QStringLiteral("url_index"), streaming.url_index);

    map_root.insert(QStringLiteral("streaming"), map_streaming);


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

void Settings::recLoad(Settings::Rec *rec, QVariantMap map_rec)
{
    QVariantMap map_nvenc=map_rec.value(QStringLiteral("nvenc")).toMap();

    rec->audio_encoder=map_rec.value(QStringLiteral("audio_encoder"), 0).toInt();
    rec->audio_bitrate=map_rec.value(QStringLiteral("audio_bitrate"), 48).toInt(); // 256kb
    rec->audio_downmix_to_stereo=map_rec.value(QStringLiteral("audio_downmix_to_stereo"), 0).toInt();
    rec->video_encoder=map_rec.value(QStringLiteral("video_encoder"), 0).toInt();
    rec->video_bitrate=map_rec.value(QStringLiteral("video_bitrate"), 0).toInt();
    rec->crf=map_rec.value(QStringLiteral("crf"), 0).toInt();
    rec->pixel_format=map_rec.value(QStringLiteral("pixel_format")).toMap();
    rec->preset=map_rec.value(QStringLiteral("preset")).toMap();
    rec->half_fps=map_rec.value(QStringLiteral("half_fps"), 0).toInt();
    rec->direct_stream_copy=map_rec.value(QStringLiteral("direct_stream_copy"), 0).toInt();
    rec->fill_dropped_frames=map_rec.value(QStringLiteral("fill_dropped_frames"), 0).toInt();
    rec->downscale=map_rec.value(QStringLiteral("downscale"), FFEncoder::DownScale::Disabled).toInt();
    rec->scale_filter=map_rec.value(QStringLiteral("scale_filter"), FFEncoder::ScaleFilter::FastBilinear).toInt();
    rec->color_primaries=map_rec.value(QStringLiteral("color_primaries"), 0).toInt();
    rec->color_space=map_rec.value(QStringLiteral("color_space"), 0).toInt();
    rec->color_transfer_characteristic=map_rec.value(QStringLiteral("color_transfer_characteristic"), 0).toInt();
    rec->color_range=map_rec.value(QStringLiteral("color_range"), 0).toInt();
    rec->sws_color_space_src=map_rec.value(QStringLiteral("sws_color_space_src"), 0).toInt();
    rec->sws_color_space_dst=map_rec.value(QStringLiteral("sws_color_space_dst"), 0).toInt();
    rec->sws_color_range_src=map_rec.value(QStringLiteral("sws_color_range_src"), 0).toInt();
    rec->sws_color_range_dst=map_rec.value(QStringLiteral("sws_color_range_dst"), 0).toInt();
    rec->aspect_ratio_4_3=map_rec.value(QStringLiteral("aspect_ratio_4_3"), 0).toInt();

    rec->pixel_format_current=rec->pixel_format.value(QString::number(rec->video_encoder), 0).toInt();
    rec->preset_current=rec->preset.value(QString::number(rec->video_encoder), 0).toInt();

    rec->nvenc.enabled=map_nvenc.value(QStringLiteral("enabled"), 0).toInt();
    rec->nvenc.device=map_nvenc.value(QStringLiteral("device"), 0).toInt();
    rec->nvenc.b_frames=map_nvenc.value(QStringLiteral("b_frames"), 0).toInt();
    rec->nvenc.ref_frames=map_nvenc.value(QStringLiteral("ref_frames"), 0).toInt();
    rec->nvenc.b_ref_mode=map_nvenc.value(QStringLiteral("b_ref_mode"), 0).toInt();
    rec->nvenc.gop_size=map_nvenc.value(QStringLiteral("gop_size"), 0).toInt();
    rec->nvenc.qp_i=map_nvenc.value(QStringLiteral("qp_i"), 0).toInt();
    rec->nvenc.qp_p=map_nvenc.value(QStringLiteral("qp_p"), 0).toInt();
    rec->nvenc.qp_b=map_nvenc.value(QStringLiteral("qp_b"), 0).toInt();
    rec->nvenc.aq_mode=map_nvenc.value(QStringLiteral("aq_mode"), 0).toInt();
    rec->nvenc.aq_strength=map_nvenc.value(QStringLiteral("aq_strength"), 0).toInt();
    rec->nvenc.rc_lookahead=map_nvenc.value(QStringLiteral("rc_lookahead"), 0).toInt();
    rec->nvenc.surfaces=map_nvenc.value(QStringLiteral("surfaces"), 0).toInt();
    rec->nvenc.no_scenecut=map_nvenc.value(QStringLiteral("no_scenecut"), 0).toInt();
    rec->nvenc.forced_idr=map_nvenc.value(QStringLiteral("forced_idr"), 0).toInt();
    rec->nvenc.b_adapt=map_nvenc.value(QStringLiteral("b_adapt"), 0).toInt();
    rec->nvenc.nonref_p=map_nvenc.value(QStringLiteral("nonref_p"), 0).toInt();
    rec->nvenc.strict_gop=map_nvenc.value(QStringLiteral("strict_gop"), 0).toInt();
    rec->nvenc.weighted_pred=map_nvenc.value(QStringLiteral("weighted_pred"), 0).toInt();
    rec->nvenc.bluray_compat=map_nvenc.value(QStringLiteral("bluray_compat"), 0).toInt();
}

QVariantMap Settings::recSave(const Settings::Rec &rec)
{
    QVariantMap map_nvenc, map_rec;

    map_nvenc.insert(QStringLiteral("enabled"), rec.nvenc.enabled);
    map_nvenc.insert(QStringLiteral("device"), rec.nvenc.device);
    map_nvenc.insert(QStringLiteral("b_frames"), rec.nvenc.b_frames);
    map_nvenc.insert(QStringLiteral("ref_frames"), rec.nvenc.ref_frames);
    map_nvenc.insert(QStringLiteral("b_ref_mode"), rec.nvenc.b_ref_mode);
    map_nvenc.insert(QStringLiteral("gop_size"), rec.nvenc.gop_size);
    map_nvenc.insert(QStringLiteral("qp_i"), rec.nvenc.qp_i);
    map_nvenc.insert(QStringLiteral("qp_p"), rec.nvenc.qp_p);
    map_nvenc.insert(QStringLiteral("qp_b"), rec.nvenc.qp_b);
    map_nvenc.insert(QStringLiteral("aq_mode"), rec.nvenc.aq_mode);
    map_nvenc.insert(QStringLiteral("aq_strength"), rec.nvenc.aq_strength);
    map_nvenc.insert(QStringLiteral("rc_lookahead"), rec.nvenc.rc_lookahead);
    map_nvenc.insert(QStringLiteral("surfaces"), rec.nvenc.surfaces);
    map_nvenc.insert(QStringLiteral("no_scenecut"), rec.nvenc.no_scenecut);
    map_nvenc.insert(QStringLiteral("forced_idr"), rec.nvenc.forced_idr);
    map_nvenc.insert(QStringLiteral("b_adapt"), rec.nvenc.b_adapt);
    map_nvenc.insert(QStringLiteral("nonref_p"), rec.nvenc.nonref_p);
    map_nvenc.insert(QStringLiteral("strict_gop"), rec.nvenc.strict_gop);
    map_nvenc.insert(QStringLiteral("weighted_pred"), rec.nvenc.weighted_pred);
    map_nvenc.insert(QStringLiteral("bluray_compat"), rec.nvenc.bluray_compat);

    map_rec.insert(QStringLiteral("nvenc"), map_nvenc);
    map_rec.insert(QStringLiteral("audio_encoder"), rec.audio_encoder);
    map_rec.insert(QStringLiteral("audio_bitrate"), rec.audio_bitrate);
    map_rec.insert(QStringLiteral("audio_downmix_to_stereo"), rec.audio_downmix_to_stereo);
    map_rec.insert(QStringLiteral("video_encoder"), rec.video_encoder);
    map_rec.insert(QStringLiteral("video_bitrate"), rec.video_bitrate);
    map_rec.insert(QStringLiteral("crf"), rec.crf);
    map_rec.insert(QStringLiteral("pixel_format"), rec.pixel_format);
    map_rec.insert(QStringLiteral("preset"), rec.preset);
    map_rec.insert(QStringLiteral("half_fps"), rec.half_fps);
    map_rec.insert(QStringLiteral("direct_stream_copy"), rec.direct_stream_copy);
    map_rec.insert(QStringLiteral("fill_dropped_frames"), rec.fill_dropped_frames);
    map_rec.insert(QStringLiteral("downscale"), rec.downscale);
    map_rec.insert(QStringLiteral("scale_filter"), rec.scale_filter);
    map_rec.insert(QStringLiteral("color_primaries"), rec.color_primaries);
    map_rec.insert(QStringLiteral("color_space"), rec.color_space);
    map_rec.insert(QStringLiteral("color_transfer_characteristic"), rec.color_transfer_characteristic);
    map_rec.insert(QStringLiteral("color_range"), rec.color_range);
    map_rec.insert(QStringLiteral("sws_color_space_src"), rec.sws_color_space_src);
    map_rec.insert(QStringLiteral("sws_color_space_dst"), rec.sws_color_space_dst);
    map_rec.insert(QStringLiteral("sws_color_range_src"), rec.sws_color_range_src);
    map_rec.insert(QStringLiteral("sws_color_range_dst"), rec.sws_color_range_dst);
    map_rec.insert(QStringLiteral("aspect_ratio_4_3"), rec.aspect_ratio_4_3);

    return map_rec;
}

QVariantMap Settings::getSourceDeviceSettings(const Settings::SourceDevice &device)
{
    QVariantMap map_root;
    QVariantMap map_dummy;
    QVariantMap map_ff;
    QVariantMap map_magewell;
    QVariantMap map_decklink;
    QVariantMap map_screen_capture;
    QVariantMap map_rec;

    map_dummy.insert(QStringLiteral("framesize"), device.dummy_device.framesize);
    map_dummy.insert(QStringLiteral("show_frame_counter"), device.dummy_device.show_frame_counter);

    map_ff.insert(QStringLiteral("name_audio"), device.ff_device.name_audio);
    map_ff.insert(QStringLiteral("name_video"), device.ff_device.name_video);
    map_ff.insert(QStringLiteral("high_depth_audio"), device.ff_device.high_depth_audio);
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

    map_screen_capture.insert(QStringLiteral("name_audio"), device.screen_capture.name_audio);
    map_screen_capture.insert(QStringLiteral("upper_framerate_limit"), device.screen_capture.upper_framerate_limit);

    map_rec=recSave(device.rec);

    map_root.insert(QStringLiteral("index"), device.index);
    map_root.insert(QStringLiteral("dummy"), map_dummy);
    map_root.insert(QStringLiteral("ff"), map_ff);
    map_root.insert(QStringLiteral("magewell"), map_magewell);
    map_root.insert(QStringLiteral("decklink"), map_decklink);
    map_root.insert(QStringLiteral("screen_capture"), map_screen_capture);
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
    QVariantMap map_screen_capture=map_root.value(QStringLiteral("screen_capture")).toMap();
    QVariantMap map_rec=map_root.value(QStringLiteral("rec")).toMap();

    device->dummy_device.framesize=map_dummy.value(QStringLiteral("framesize"), 0).toUInt();
    device->dummy_device.show_frame_counter=map_dummy.value(QStringLiteral("show_frame_counter"), false).toBool();

    device->ff_device.name_audio=map_ff.value(QStringLiteral("name_audio")).toString();
    device->ff_device.name_video=map_ff.value(QStringLiteral("name_video")).toString();
    device->ff_device.high_depth_audio=map_ff.value(QStringLiteral("high_depth_audio"), 0).toUInt();
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

    device->screen_capture.name_audio=map_screen_capture.value(QStringLiteral("name_audio")).toString();
    device->screen_capture.upper_framerate_limit=map_screen_capture.value(QStringLiteral("upper_framerate_limit"), 0).toUInt();

    recLoad(&device->rec, map_rec);
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
