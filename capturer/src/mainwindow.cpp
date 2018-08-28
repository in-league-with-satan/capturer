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
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QFile>
#include <QGenericArgument>
#include <QLineEdit>
#include <QCheckBox>
#include <QMessageBox>
#include <QJsonDocument>
#include <QFile>
#include <QDir>

#include "DeckLinkAPI.h"

#include "settings.h"
#include "device_list.h"
#include "capture.h"
#include "audio_output.h"
#include "audio_level.h"
#include "qml_messenger.h"
#include "overlay_view.h"
#include "http_server.h"
#include "data_types.h"
#include "dialog_keyboard_shortcuts.h"
#include "audio_sender.h"
#include "tools_cam.h"
#include "ff_cam.h"
#include "decklink_dummy.h"
#include "cuda_tools.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mb_rec_stopped(nullptr)
{
    QDir dir(qApp->applicationDirPath() + "/videos");

    if(!dir.exists())
        dir.mkdir(dir.absolutePath());

    //

    if(settings->device_decklink.dummy.enabled)
        decklink_thread=new DeckLinkDummy(settings->device_decklink.dummy.frame_counter, settings->device_decklink.dummy.frame_height, this);

    else
        decklink_thread=new DeckLinkCapture(this);


    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            SLOT(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);
    connect(decklink_thread, SIGNAL(frameSkipped()), SLOT(frameSkipped()), Qt::QueuedConnection);

    //

    QStringList cuda_devices=cuda::availableDevices();

    //

    cam_device=new FFCam(this);

    //

    qmlRegisterType<SettingsModel>("FuckTheSystem", 0, 0, "SettingsModel");
    qmlRegisterType<FileSystemModel>("FuckTheSystem", 0, 0, "FileSystemModel");
    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");
    qmlRegisterType<QuickVideoSource>("FuckTheSystem", 0, 0, "QuickVideoSource");

    messenger=new QmlMessenger();

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            messenger, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);

    connect(decklink_thread, SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

    overlay_view=new OverlayView(this);

    overlay_view->setMessenger(messenger);

    overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));

    overlay_view->addImageProvider("fs_image_provider", (QQmlImageProviderBase*)messenger->fileSystemModel()->imageProvider());

    decklink_thread->subscribe(messenger->videoSourceMain()->frameBuffer());


    cam_device->subscribe(messenger->videoSourceCam()->frameBuffer());

    //

    audio_output=newAudioOutput(this);

    decklink_thread->subscribe(audio_output->frameBuffer());

    connect(decklink_thread, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

    //

    ff_enc=new FFEncoderThread(FFEncoder::Mode::primary, &enc_base_filename, QString("capturer %1").arg(VERSION_STRING), this);

    decklink_thread->subscribe(ff_enc->frameBuffer());

    connect(&ff_enc->frameBuffer()->signaler, SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

    //

    ff_enc_cam=new FFEncoderThread(FFEncoder::Mode::webcam, &enc_base_filename, QString("capturer %1").arg(VERSION_STRING), this);

    cam_device->subscribe(ff_enc_cam->frameBuffer());

    // connect(ff_enc_cam->frameBuffer().get(), SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc_cam, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc_cam, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);
    connect(ff_enc_cam, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

    //

    ff_dec=new FFDecoderThread(this);

    ff_dec->subscribeVideo(messenger->videoSourceMain()->frameBuffer());
    ff_dec->subscribeAudio(audio_output->frameBuffer());


    connect(messenger->fileSystemModel(), SIGNAL(playMedia(QString)), ff_dec, SLOT(open(QString)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(durationChanged(qint64)), messenger, SIGNAL(playerDurationChanged(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(positionChanged(qint64)), messenger, SIGNAL(playerPositionChanged(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(stateChanged(int)), SLOT(playerStateChanged(int)), Qt::QueuedConnection);
    connect(messenger, SIGNAL(playerSetPosition(qint64)), ff_dec, SLOT(seek(qint64)));
    connect(messenger->settingsModel(), SIGNAL(dataChanged(int,int,bool)), SLOT(settingsModelDataChanged(int,int,bool)));

    //

    audio_level=new AudioLevel(this);
    decklink_thread->subscribe(audio_level->frameBuffer());

    connect(audio_level, SIGNAL(levels(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
            messenger, SIGNAL(audioLevels(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::QueuedConnection);

    //

    settings->load();

    if(settings->keyboard_shortcuts.need_setup || qApp->arguments().contains("--setup_shortcuts", Qt::CaseInsensitive)) {
        DialogKeyboardShortcuts dlg;

        for(int i=0; i<KeyCodeC::enm_size; ++i)
            dlg.setKey(i, (Qt::Key)settings->keyboard_shortcuts.code.key(i, DialogKeyboardShortcuts::defaultQtKey(i)));

        if(dlg.exec()==DialogKeyboardShortcuts::Accepted) {
            for(int i=0; i<KeyCodeC::enm_size; ++i)
                settings->keyboard_shortcuts.code.insert(dlg.toQtKey(i), i);
        }
    }

    //

    AudioSender *audio_sender=new AudioSender(this);
    decklink_thread->subscribe(audio_sender->frameBuffer());

    //

    http_server=new HttpServer(settings->http_server.enabled ? settings->http_server.port : 0, this);
    http_server->setSettingsModel(messenger->settingsModel());
    connect(http_server, SIGNAL(keyPressed(int)), SLOT(keyPressed(int)));
    connect(http_server, SIGNAL(deviceCamRestart()), SLOT(deviceCamRestart()), Qt::DirectConnection);
    connect(http_server, SIGNAL(deviceCamStop()), SLOT(deviceCamStop()), Qt::DirectConnection);
    connect(http_server, SIGNAL(deviceDecklinkRestart()), SLOT(deviceDecklinkRestart()), Qt::DirectConnection);
    connect(http_server, SIGNAL(checkEncoders()), SLOT(checkEncoders()), Qt::DirectConnection);
    connect(http_server, SIGNAL(playerSeek(qint64)), ff_dec, SLOT(seek(qint64)));
    connect(ff_dec, SIGNAL(durationChanged(qint64)), http_server, SLOT(setPlayerDuration(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(positionChanged(qint64)), http_server, SLOT(setPlayerPosition(qint64)), Qt::QueuedConnection);
    connect(messenger, SIGNAL(freeSpace(qint64)), http_server, SLOT(setFreeSpace(qint64)), Qt::QueuedConnection);
    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            http_server, SLOT(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);

    //

    SettingsModel::Data set_model_data;

    // { cam

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.name="camera";

    messenger->settingsModel()->add(set_model_data);

    //

    QStringList cam_devices=FFCam::availableAudioInput();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device cam";
    set_model_data.name="audio device";


    if(cam_devices.isEmpty()) {
        set_model_data.values << "null";

    } else {
        foreach(QString dev_name, cam_devices) {
            set_model_data.values << dev_name;
        }
    }

    set_model_data.value=&settings->device_cam.index_audio;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    cam_devices=FFCam::availableCameras();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="video device";
    set_model_data.priority=SettingsModel::Priority::high;

    if(cam_devices.isEmpty()) {
        set_model_data.values << "null";

    } else {
        foreach(QString dev_name, cam_devices) {
            qInfo() << cam_devices.size() << dev_name;
            set_model_data.values << dev_name;
        }
    }

    if(!cam_device->setVideoDevice(settings->device_cam.index_video))
        settings->device_cam.index_video=0;

    set_model_data.value=&settings->device_cam.index_video;

    int model_index_cam_name=messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="resolution";

    QList <QSize> supported_resolutions=cam_device->supportedResolutions();

    if(settings->device_cam.resolution>=supported_resolutions.size())
        settings->device_cam.resolution=0;

    set_model_data.value=&settings->device_cam.resolution;

    QSize cam_resolution;

    if(!supported_resolutions.isEmpty())
        cam_resolution=supported_resolutions[settings->device_cam.resolution];

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="pixel format";

    QList <int64_t> supported_pix_fmts=cam_device->supportedPixelFormats(cam_resolution);

    if(settings->device_cam.pixel_format>=supported_pix_fmts.size())
        settings->device_cam.pixel_format=0;

    int64_t cam_pix_fmt=0;

    if(!supported_pix_fmts.isEmpty())
        cam_pix_fmt=supported_pix_fmts[settings->device_cam.pixel_format];

    foreach(qint64 val, supported_pix_fmts) {
        set_model_data.values << PixelFormat::toStringView(val);
        set_model_data.values_data << val;
    }

    set_model_data.value=&settings->device_cam.pixel_format;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="framerate";
    set_model_data.priority=SettingsModel::Priority::low;

    QList <AVRational> framerate=cam_device->supportedFramerates(cam_resolution, cam_pix_fmt);

    foreach(AVRational val, framerate) {
        set_model_data.values << QString::number(ToolsCam::rationalToFramerate(val));
        set_model_data.values_data << QSize(val.num, val.den);
    }

    if(settings->device_cam.framerate>=framerate.size())
        settings->device_cam.framerate=0;

    set_model_data.value=&settings->device_cam.framerate;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    if(!cam_devices.isEmpty())
        settingsModelDataChanged(model_index_cam_name, 0, false);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="restart device";
    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->device_cam.restart;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="stop device";
    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->device_cam.stop;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    // } cam

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;
    messenger->settingsModel()->add(set_model_data);

    // { decklink

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="decklink";

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device decklink";
    set_model_data.name="name";

    DeckLinkDevices devices;

    GetDevices(&devices);

    if(devices.isEmpty()) {
        set_model_data.values << "null";

    } else {
        for(int i_device=0; i_device<devices.size(); ++i_device) {
            DeckLinkDevice dev=devices[i_device];

            QVariant var;
            var.setValue(dev);

            set_model_data.values << dev.name;
            set_model_data.values_data << var;
        }
    }

    set_model_data.value=&settings->device_decklink.index;


    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="audio sample size";
    set_model_data.values << "16 bit" << "32 bit";
    set_model_data.values_data << 16 << 32;

    set_model_data.value=&settings->device_decklink.audio_sample_size;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="video depth";
    set_model_data.values << "8 bit" << "10 bit";
    set_model_data.values_data << 0 << 1;

    set_model_data.value=&settings->device_decklink.video_depth_10bit;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="half-fps";
    set_model_data.value=&settings->device_decklink.half_fps;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="restart device";
    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->device_decklink.restart;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    // } decklink

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;
    messenger->settingsModel()->add(set_model_data);

    // { rec

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="rec";

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="check encoders";
    set_model_data.group="rec";
    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->rec.check_encoders;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="audio encoder";

    set_model_data.values << "pcm" << "flac";

    set_model_data.value=&settings->rec.encoder_audio;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="video encoder";
    set_model_data.priority=SettingsModel::Priority::high;


    set_model_data.value=&settings->rec.encoder_video;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="preset";
    set_model_data.priority=SettingsModel::Priority::low;

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->rec.preset_current;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="pixel format";
    set_model_data.value=&settings->rec.pixel_format_current;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    updateEncList();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="constant rate factor / quality";
    set_model_data.value=&settings->rec.crf;

    for(int i=0; i<=42; ++i)
        set_model_data.values.append(QString::number(i));

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="downscale";
    set_model_data.value=&settings->rec.downscale;

    for(int i=0; i<=FFEncoder::DownScale::to1800; i++)
        set_model_data.values << FFEncoder::DownScale::toString(i);

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="scale filter";
    set_model_data.value=&settings->rec.scale_filter;

    for(int i=0; i<=FFEncoder::ScaleFilter::Spline; i++)
        set_model_data.values << FFEncoder::ScaleFilter::toString(i);

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="half-fps";
    set_model_data.value=&settings->rec.half_fps;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="stop rec on frames drop";
    set_model_data.value=&settings->rec.stop_rec_on_frames_drop;

    messenger->settingsModel()->add(set_model_data);

    // } rec

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;
    messenger->settingsModel()->add(set_model_data);

    // { nvenc
    if(!cuda_devices.isEmpty()) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->main.dummy;
        set_model_data.name="nvenc";

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.group="nvenc";
        set_model_data.name="enabled";
        set_model_data.value=&settings->nvenc.enabled;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="device";
        set_model_data.value=&settings->nvenc.device;

        set_model_data.values << "any";

        foreach(QString val, cuda_devices)
            set_model_data.values << val;

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="b frames (h264 only)";
        set_model_data.value=&settings->nvenc.b_frames;

        for(int i=0; i<5; i++)
            set_model_data.values << QString::number(i);

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="ref frames";
        set_model_data.value=&settings->nvenc.ref_frames;

        for(int i=0; i<16; i++) {
            if(i>5)
                set_model_data.values << QString("%1 hevc only").arg(i);

            else
                set_model_data.values << QString::number(i);
        }

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="gop size";
        set_model_data.value=&settings->nvenc.gop_size;

        for(int i=0; i<301; i++)
            set_model_data.values << QString::number(i);

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        for(int i=-1; i<52; i++)
            set_model_data.values << QString::number(i);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="qpI";
        set_model_data.value=&settings->nvenc.qp_i;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.name="qpP";
        set_model_data.value=&settings->nvenc.qp_p;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.name="qpB (h264 only)";
        set_model_data.value=&settings->nvenc.qp_b;

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="aq mode";
        set_model_data.value=&settings->nvenc.aq_mode;
        set_model_data.values << "disabled" << "spatial" << "temporal. not working with constqp?";
        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="aq strength";
        set_model_data.value=&settings->nvenc.aq_strength;

        for(int i=0; i<16; i++)
            set_model_data.values << QString::number(i);

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="rc lookahead";
        set_model_data.value=&settings->nvenc.rc_lookahead;

        for(int i=-1; i<33; i++)
            set_model_data.values << QString::number(i);

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="surfaces";
        set_model_data.value=&settings->nvenc.surfaces;

        for(int i=-1; i<65; i++)
            set_model_data.values << QString::number(i);

        messenger->settingsModel()->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="no scenecut";
        set_model_data.value=&settings->nvenc.no_scenecut;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="forced idr";
        set_model_data.value=&settings->nvenc.forced_idr;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="b adapt (h264 only)";
        set_model_data.value=&settings->nvenc.b_adapt;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="nonref p";
        set_model_data.value=&settings->nvenc.nonref_p;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="strict gop";
        set_model_data.value=&settings->nvenc.strict_gop;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="weighted pred (disables bframes)";
        set_model_data.value=&settings->nvenc.weighted_pred;

        messenger->settingsModel()->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="bluray compatibility workarounds";
        set_model_data.value=&settings->nvenc.bluray_compat;

        messenger->settingsModel()->add(set_model_data);
    }

    // } nvenc

    setCentralWidget(overlay_view);

    QApplication::instance()->installEventFilter(this);

    setMinimumSize(640, 640/(16/9.));

    if(qApp->arguments().contains("--windowed")) {
        show();

    } else {
        showFullScreen();
    }

    emit messenger->signalLost(true);

    startStopCapture();
}

MainWindow::~MainWindow()
{
    settings->save();
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if(event->type()==QEvent::KeyPress) {
        QKeyEvent *e=static_cast<QKeyEvent*>(event);

        if(e) {
            // qInfo() << "key pressed" << e->key();

            int key=e->key();

            if(settings->keyboard_shortcuts.code.contains(key)) {
                keyPressed(settings->keyboard_shortcuts.code.value(key));
                return true;
            }
        }
    }

    if(event->type()==QEvent::HoverEnter) {
        QApplication::setOverrideCursor(Qt::BlankCursor);
    }

    if(event->type()==QEvent::HoverLeave) {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
    }

    return QObject::eventFilter(object, event);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    settings->save();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton)
        pos_mouse_press=event->globalPos() - frameGeometry().topLeft();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if(event->buttons()&Qt::LeftButton)
        move(event->globalPos() - pos_mouse_press);
}

void MainWindow::keyPressed(int code)
{
    switch(code) {
    case KeyCodeC::FileBrowser:
        if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
            messenger->showFileBrowser();

        break;

    case KeyCodeC::About:
        messenger->showHideAbout();
        break;

    case KeyCodeC::Rec:
        startStopRecording();

        break;

    case KeyCodeC::Info:
        if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
            messenger->showHideInfo();

        else
            messenger->showHidePlayerState();

        break;

    case KeyCodeC::RecState:
        if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
            messenger->showHideDetailedRecState();

        break;

    case KeyCodeC::Preview:
        previewOnOff();
        break;

    case KeyCodeC::PreviewFastYuv:
        messenger->videoSourceMain()->setFastYuv(!messenger->videoSourceMain()->fastYuv());
        break;

    case KeyCodeC::PreviewCam:
        previewCamOnOff();
        break;

    case KeyCodeC::PreviewCamChangePosition:
        messenger->camPreviewChangePosition();
        break;

    case KeyCodeC::FullScreen:
        if(isFullScreen())
            showNormal();

        else
            showFullScreen();

        break;

    case KeyCodeC::Exit:
        // QApplication::exit(0);
        close();

        break;

    case KeyCodeC::Menu:
        // qInfo() << "show_menu";
        if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
            emit messenger->showMenu();

        else {
            if(ff_dec->currentState()==FFDecoderThread::ST_PLAY)
                ff_dec->pause();

            else
                ff_dec->play();
        }

        break;

    case KeyCodeC::Back:
        if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED)
            ff_dec->stop();

        else
            emit messenger->back();

        break;

    case KeyCodeC::Enter:
        emit messenger->keyPressed(Qt::Key_Right);

        break;

    case KeyCodeC::Left:
    case KeyCodeC::Right:
        if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED) {
            int64_t pos=0;

            if(KeyCodeC::Left==code) {
                pos=ff_dec->currentPos() - 30*1000;

                if(pos<0)
                    pos=0;

                ff_dec->seek(pos);
            }

            if(KeyCodeC::Right==code) {
                pos=ff_dec->currentPos() + 30*1000;

                if(pos<ff_dec->currentDuration()) {
                    ff_dec->seek(pos);
                }
            }
        }

        emit messenger->keyPressed(KeyCodeC::Left==code ? Qt::Key_Left : Qt::Key_Right);

        break;

    case KeyCodeC::Up:
        emit messenger->keyPressed(Qt::Key_Up);
        break;

    case KeyCodeC::Down:
        emit messenger->keyPressed(Qt::Key_Down);
        break;
    }
}

void MainWindow::formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format)
{
    Q_UNUSED(progressive_frame)
    Q_UNUSED(pixel_format)

    current_frame_size=QSize(width, height);
    current_frame_duration=frame_duration;
    current_frame_scale=frame_scale;
}

void MainWindow::deviceCamRestart()
{
    SettingsModel::Data *model_data_resolution=messenger->settingsModel()->data_p(&settings->device_cam.resolution);
    SettingsModel::Data *model_data_framerate=messenger->settingsModel()->data_p(&settings->device_cam.framerate);
    SettingsModel::Data *model_data_pixel_format=messenger->settingsModel()->data_p(&settings->device_cam.pixel_format);

    QSize framerate=model_data_framerate->values_data.value(settings->device_cam.framerate).toSize();
    int64_t pixel_format=model_data_pixel_format->values_data.value(settings->device_cam.pixel_format).toLongLong();

    cam_device->setAudioDevice(settings->device_cam.index_audio);
    cam_device->setVideoDevice(settings->device_cam.index_video);

    cam_device->setConfig(model_data_resolution->values_data.value(settings->device_cam.resolution).toSize(), { framerate.width(), framerate.height() }, pixel_format);

    QMetaObject::invokeMethod(cam_device, "startCam", Qt::QueuedConnection);
}

void MainWindow::deviceCamStop()
{
    cam_device->stop();
}

void MainWindow::deviceDecklinkRestart()
{
    captureRestart();
}

void MainWindow::checkEncoders()
{
    settings->checkEncoders();

    updateEncList();
}

void MainWindow::settingsModelDataChanged(int index, int role, bool qml)
{
    Q_UNUSED(qml)

    SettingsModel *model=messenger->settingsModel();

    SettingsModel::Data *data=model->data_p(index);

    // if(data->value==&settings->device_cam.index_audio) {
    //     cam_device->setAudioDevice(settings->device_cam.index_audio);
    // }

    static bool ignore_event=false;

    if(ignore_event)
        return;

    struct Tmp {  Tmp() { ignore_event=true; }  ~Tmp() { ignore_event=false; }  } tmp;

    Q_UNUSED(tmp);


    if(data->value==&settings->device_cam.index_video) {
        cam_device->setVideoDevice(settings->device_cam.index_video);

        QStringList list_values;
        QVariantList list_values_data;

        foreach(QSize val, cam_device->supportedResolutions()) {
            list_values << QString("%1x%2").arg(val.width()).arg(val.height());
            list_values_data << val;
        }

        if(settings->device_cam.resolution>=list_values.size()) {
            settings->device_cam.resolution=0;
        }

        model->setData(&settings->device_cam.resolution, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.resolution, SettingsModel::Role::values_data, list_values_data);

        if(!list_values_data.isEmpty()) {
            const QSize size=list_values_data.value(settings->device_cam.resolution).toSize();

            list_values.clear();
            list_values_data.clear();

            if(!messenger->settingsModel()->data_p(&settings->device_cam.pixel_format)->values_data.isEmpty()) {
                foreach(AVRational val, cam_device->supportedFramerates(size, messenger->settingsModel()->data_p(&settings->device_cam.pixel_format)->values_data[settings->device_cam.pixel_format].toLongLong())) {
                    list_values << QString::number(ToolsCam::rationalToFramerate(val));
                    list_values_data << QSize(val.num, val.den);
                }

            } else {
                qWarning() << "model no data. pixel_format" << settings->device_cam.pixel_format;
            }

            model->setData(&settings->device_cam.framerate, SettingsModel::Role::values, list_values);
            model->setData(&settings->device_cam.framerate, SettingsModel::Role::values_data, list_values_data);

            //

            list_values.clear();
            list_values_data.clear();

            foreach(qint64 val, cam_device->supportedPixelFormats(size)) {
                list_values << PixelFormat::toStringView(val);
                list_values_data << val;
            }

            model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values, list_values);
            model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values_data, list_values_data);

        } else {
            qInfo() << "list_values_data is empty!!!!1";
        }
    }

    if(data->value==&settings->device_cam.resolution) {
        QSize size=data->values_data.value(settings->device_cam.resolution).toSize();

        QStringList list_values;
        QVariantList list_values_data;

        //

        foreach(qint64 val, cam_device->supportedPixelFormats(size)) {
            list_values << PixelFormat::toStringView(val);
            list_values_data << val;
        }

        if(settings->device_cam.pixel_format>=list_values.size()) {
            settings->device_cam.pixel_format=0;
        }

        model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values_data, list_values_data);

        //

        list_values.clear();
        list_values_data.clear();

        if(!messenger->settingsModel()->data_p(&settings->device_cam.pixel_format)->values_data.isEmpty())
            foreach(AVRational val, cam_device->supportedFramerates(size, messenger->settingsModel()->data_p(&settings->device_cam.pixel_format)->values_data[settings->device_cam.pixel_format].toLongLong())) {
                list_values << QString::number(ToolsCam::rationalToFramerate(val));
                list_values_data << QSize(val.num, val.den);
            }

        model->setData(&settings->device_cam.framerate, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.framerate, SettingsModel::Role::values_data, list_values_data);
    }

    if(data->value==&settings->device_cam.pixel_format) {
        const QSize size=
                messenger->settingsModel()->data_p(&settings->device_cam.resolution)->values_data[settings->device_cam.resolution].toSize();

        const int64_t pixel_format=
                messenger->settingsModel()->data_p(&settings->device_cam.pixel_format)->values_data[settings->device_cam.pixel_format].toLongLong();

        QStringList list_values;
        QVariantList list_values_data;

        foreach(AVRational val, cam_device->supportedFramerates(size, pixel_format)) {
            list_values << QString::number(ToolsCam::rationalToFramerate(val));
            list_values_data << QSize(val.num, val.den);

        }

        model->setData(&settings->device_cam.framerate, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.framerate, SettingsModel::Role::values_data, list_values_data);
    }

    //

    if(data->value==&settings->rec.encoder_video) {
        // pix_fmt
        const int index_encoder=data->values_data[settings->rec.encoder_video].toInt();

        QList <PixelFormat> fmts;
        PixelFormat pf;

        foreach(const QString &fmt_str, settings->rec.supported_enc[FFEncoder::VideoEncoder::toString(index_encoder)].toStringList()) {
            if(pf.fromString(fmt_str))
                fmts << pf;
        }

        QStringList list_values;
        QVariantList list_values_data;

        for(int i=0; i<fmts.size(); ++i) {
            list_values << fmts[i].toStringView();
            list_values_data << (int)fmts[i];
        }

        for(int i=0; i<model->rowCount(); ++i) {
            if(model->data_p(i)->value==&settings->rec.pixel_format_current) {
                model->setData(i, SettingsModel::Role::values_data, list_values_data);
                model->setData(i, SettingsModel::Role::values, list_values);
                model->setData(i, SettingsModel::Role::value, settings->rec.pixel_format.value(QString::number(settings->rec.encoder_video), 0));

                break;
            }
        }

        // preset
        list_values.clear();
        list_values_data.clear();

        QStringList presets=FFEncoder::compatiblePresets((FFEncoder::VideoEncoder::T)data->values_data.value(settings->rec.encoder_video, 0).toInt());

        foreach(const QString &preset, presets) {
            list_values << preset;
            list_values_data << FFEncoder::presetVisualNameToParamName(preset);
        }

        for(int i=0; i<model->rowCount(); ++i) {
            if(model->data_p(i)->value==&settings->rec.preset_current) {
                model->setData(i, SettingsModel::Role::values_data, list_values_data);
                model->setData(i, SettingsModel::Role::values, list_values);
                model->setData(i, SettingsModel::Role::value, settings->rec.preset.value(QString::number(settings->rec.encoder_video), 0));

                break;
            }
        }
    }


    if(data->value==&settings->rec.pixel_format_current)
        if(role==SettingsModel::Role::value)
            settings->rec.pixel_format[QString::number(settings->rec.encoder_video)]=settings->rec.pixel_format_current;


    if(data->value==&settings->rec.preset_current)
        if(role==SettingsModel::Role::value)
            settings->rec.preset[QString::number(settings->rec.encoder_video)]=settings->rec.preset_current;


    if(data->value==&settings->device_decklink.restart) {
        deviceDecklinkRestart();
    }

    if(data->value==&settings->device_cam.restart) {
        deviceCamRestart();
    }

    if(data->value==&settings->device_cam.stop) {
        deviceCamStop();
    }

    if(data->value==&settings->rec.check_encoders) {
        checkEncoders();
    }


    messenger->settingsModel()->updateQml();
}

void MainWindow::startStopCapture()
{
    if(decklink_thread->isRunning()) {
        captureStop();
        return;
    }

    captureStart();
}

void MainWindow::captureRestart()
{
    if(decklink_thread->isRunning()) {
        captureStop();

        while(decklink_thread->isRunning()) {
            qApp->processEvents();

            QThread::msleep(300);
        }
    }

    captureStart();
}

void MainWindow::captureStart()
{
    SettingsModel::Data *model_data_device=messenger->settingsModel()->data_p(&settings->device_decklink.index);
    SettingsModel::Data *model_data_audio=messenger->settingsModel()->data_p(&settings->device_decklink.audio_sample_size);

    if(!model_data_device || !model_data_audio) {
        qCritical() << "model_data null pointer";
        return;
    }


    if(!settings->device_decklink.dummy.enabled) {
        if(model_data_device->values_data.isEmpty()) {
            qCritical() << "values_data is empty";
            return;
        }

        decklink_thread->setup(model_data_device->values_data[settings->device_decklink.index].value<DeckLinkDevice>(),
                DeckLinkFormat(),
                DeckLinkPixelFormat(),
                8,
                model_data_audio->values_data[settings->device_decklink.audio_sample_size].toInt(),
                settings->device_decklink.video_depth_10bit
                );

        decklink_thread->setHalfFps(settings->device_decklink.half_fps);
    }

    QMetaObject::invokeMethod(decklink_thread, "captureStart", Qt::QueuedConnection);
}

void MainWindow::captureStop()
{
    QMetaObject::invokeMethod(decklink_thread, "captureStop", Qt::QueuedConnection);
}

void MainWindow::startStopRecording()
{
    bool enc_running=ff_enc->isWorking() || ff_enc_cam->isWorking();

    if(enc_running) {
        ff_enc->stopCoder();
        ff_enc_cam->stopCoder();

    } else {
        if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED)
            return;

        enc_base_filename=
                QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

        if(decklink_thread->gotSignal()) {
            FFEncoder::Config cfg;

            cfg.framerate=FFEncoder::calcFps(current_frame_duration, current_frame_scale, settings->rec.half_fps);
            cfg.frame_resolution_src=current_frame_size;
            cfg.pixel_format_dst=messenger->settingsModel()->data_p(&settings->rec.pixel_format_current)->values_data[settings->rec.pixel_format_current].toInt();
            cfg.preset=messenger->settingsModel()->data_p(&settings->rec.preset_current)->values_data[settings->rec.preset_current].toString();
            cfg.video_encoder=(FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder_video)->values_data[settings->rec.encoder_video].toInt();
            cfg.crf=settings->rec.crf;
            cfg.pixel_format_src.fromBMDPixelFormat(decklink_thread->pixelFormat());
            cfg.audio_sample_size=messenger->settingsModel()->data_p(&settings->device_decklink.audio_sample_size)->values_data[settings->device_decklink.audio_sample_size].toInt();
            cfg.downscale=settings->rec.downscale;
            cfg.scale_filter=settings->rec.scale_filter;
            cfg.color_primaries=settings->rec.color_primaries;
            cfg.color_space=settings->rec.color_space;
            cfg.color_transfer_characteristic=settings->rec.color_transfer_characteristic;
            cfg.nvenc=settings->nvenc;
            cfg.audio_flac=settings->rec.encoder_audio==1;

            ff_enc->setConfig(cfg);
        }

        if(cam_device->isActive()) {
            SettingsModel::Data *model_data_resolution=messenger->settingsModel()->data_p(&settings->device_cam.resolution);
            SettingsModel::Data *model_data_framerate=messenger->settingsModel()->data_p(&settings->device_cam.framerate);

            QSize resolution=model_data_resolution->values_data.value(*model_data_resolution->value).toSize();
            QSize framerate=model_data_framerate->values_data.value(*model_data_framerate->value).toSize();

            FFEncoder::Config cfg;

            cfg.framerate=FFEncoder::calcFps(framerate.width(), framerate.height(), false);

            if(cfg.framerate==FFEncoder::Framerate::unknown) {
                cfg.framerate_force=cam_device->currentFrameRate();
            }

            cfg.frame_resolution_src=resolution;

            cfg.pixel_format_dst=messenger->settingsModel()->data_p(&settings->rec.pixel_format_current)->values_data[settings->rec.pixel_format_current].toInt();
            cfg.preset=messenger->settingsModel()->data_p(&settings->rec.preset_current)->values_data[settings->rec.preset_current].toString();
            cfg.video_encoder=(FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder_video)->values_data[settings->rec.encoder_video].toInt();
            cfg.crf=settings->rec.crf;
            cfg.pixel_format_src=cam_device->pixelFormat();
            cfg.audio_sample_size=16;
            cfg.audio_channels_size=2;
            cfg.downscale=settings->rec.downscale;
            cfg.scale_filter=settings->rec.scale_filter;

            ff_enc_cam->setConfig(cfg);
        }

        messenger->updateRecStats();

        dropped_frames_counter=0;
    }
}

void MainWindow::updateEncList()
{
    if(settings->rec.supported_enc.isEmpty()) {
        qCritical() << "supported_enc.isEmpty";
        exit(1);
        return;
    }

    SettingsModel::Data *set_model_data=
            messenger->settingsModel()->data_p(&settings->rec.encoder_video);

    if(!set_model_data) {
        qCritical() << "set_model_data null pointer";
        return;
    }

    set_model_data->values.clear();
    set_model_data->values_data.clear();

    for(int i=0; i<settings->rec.supported_enc.size(); ++i) {
        set_model_data->values << settings->rec.supported_enc.keys()[i];
        set_model_data->values_data << (quint64)FFEncoder::VideoEncoder::fromString(settings->rec.supported_enc.keys()[i]);
    }

    settingsModelDataChanged(messenger->settingsModel()->data_p_index(&settings->rec.encoder_video), 0, false);
}

void MainWindow::frameSkipped()
{
    dropped_frames_counter++;

    if(!settings->rec.stop_rec_on_frames_drop)
        return;

    if(!ff_enc->isWorking())
        return;

    ff_enc->stopCoder();

    // QMetaObject::invokeMethod(decklink_thread, "captureStop", Qt::QueuedConnection);

    if(!mb_rec_stopped)
        mb_rec_stopped=new QMessageBox(QMessageBox::Critical, "", "", QMessageBox::Ok);

    mb_rec_stopped->setText("some frames was dropped, recording stopped");

    mb_rec_stopped->show();
    mb_rec_stopped->raise();
    mb_rec_stopped->exec();
}

void MainWindow::encoderBufferOverload()
{
    if(!ff_enc->isWorking())
        return;

    ff_enc->stopCoder();

    if(!mb_rec_stopped)
        mb_rec_stopped=new QMessageBox(QMessageBox::Critical, "", "", QMessageBox::Ok);

    mb_rec_stopped->setText("encoder buffer overload, recording stopped");

    mb_rec_stopped->show();
    mb_rec_stopped->raise();
    mb_rec_stopped->exec();
}

void MainWindow::previewOnOff()
{
    settings->main.preview=!settings->main.preview;

    messenger->videoSourceMain()->frameBuffer()->setEnabled(settings->main.preview);
}

void MainWindow::previewCamOnOff()
{
    messenger->videoSourceCam()->frameBuffer()->setEnabled(!messenger->videoSourceCam()->frameBuffer()->isEnabled());

    messenger->camPreview(messenger->videoSourceCam()->frameBuffer()->isEnabled());
}

void MainWindow::encoderStateChanged(bool state)
{
    messenger->setRecStarted(state);

    http_server->setRecState(state);
}

void MainWindow::playerStateChanged(int state)
{
    if(state!=FFDecoderThread::ST_STOPPED || decklink_thread->gotSignal()) {
        emit messenger->signalLost(false);

    } else {
        emit messenger->signalLost(true);
    }

    if(state==FFDecoderThread::ST_STOPPED) {
        decklink_thread->captureStart();

        emit messenger->showPlayerState(false);

    } else {
        decklink_thread->captureStop();
    }
}

void MainWindow::updateStats(FFEncoder::Stats s)
{
    static FFEncoder::Stats st_main={ };
    static FFEncoder::Stats st_cam={ };
    static qint64 last_update=0;

    if(sender()==ff_enc)
        st_main=s;

    else
        st_cam=s;

    s.avg_bitrate_audio=st_main.avg_bitrate_audio + st_cam.avg_bitrate_audio;
    s.avg_bitrate_video=st_main.avg_bitrate_video + st_cam.avg_bitrate_video;
    s.streams_size=st_main.streams_size + st_cam.streams_size;
    s.time=st_main.time;

    if(s.time.isNull())
        s.time=st_cam.time;

    if(QDateTime::currentMSecsSinceEpoch() - last_update<1000)
        return;

    last_update=QDateTime::currentMSecsSinceEpoch();

    const QPair <int, int> buffer_size=ff_enc->frameBuffer()->size();

    messenger->updateRecStats(s.time.toString(QStringLiteral("HH:mm:ss")),
                              QString(QLatin1String("%1 Mbits/s (%2 MB/s)")).arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2),
                              QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString(QLatin1String("%1 bytes")).arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString(QLatin1String("buf state: %1/%2")).arg(buffer_size.first).arg(buffer_size.second),
                              QString(QLatin1String("frames dropped: %1")).arg(dropped_frames_counter));

    http_server->setRecStats(NRecStats(s.time, s.avg_bitrate_video + s.avg_bitrate_audio, s.streams_size, dropped_frames_counter, buffer_size.second, buffer_size.first));
}
