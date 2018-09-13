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

#include "settings.h"
#include "decklink_device_list.h"
#include "decklink_thread.h"
#include "audio_output.h"
#include "audio_level.h"
#include "qml_messenger.h"
#include "overlay_view.h"
#include "http_server.h"
#include "data_types.h"
#include "dialog_keyboard_shortcuts.h"
#include "audio_sender.h"
#include "tools_ff_source.h"
#include "ff_source.h"
#include "framerate.h"
#include "dummy_device.h"
#include "cuda_tools.h"
#include "magewell_device.h"

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mb_rec_stopped(nullptr)
{

    QDir dir(qApp->applicationDirPath() + "/videos");

    if(!dir.exists())
        dir.mkdir(dir.absolutePath());

   
    //

    QStringList cuda_devices=cuda::availableDevices();



    //

    qmlRegisterType<SettingsModel>("FuckTheSystem", 0, 0, "SettingsModel");
    qmlRegisterType<FileSystemModel>("FuckTheSystem", 0, 0, "FileSystemModel");
    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");
    qmlRegisterType<QuickVideoSource>("FuckTheSystem", 0, 0, "QuickVideoSource");

    messenger=new QmlMessenger();


    overlay_view=new OverlayView(this);

    overlay_view->setMessenger(messenger);

    overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));

    overlay_view->addImageProvider("fs_image_provider", (QQmlImageProviderBase*)messenger->fileSystemModel()->imageProvider());


    //

    QStringList cam_devices=FFSource::availableAudioInput();

    //

    audio_output=newAudioOutput(this);


    //

    ff_enc=new FFEncoderThread(FFEncoder::Mode::primary, &enc_base_filename, QString("capturer %1").arg(VERSION_STRING), this);


    connect(&ff_enc->frameBuffer()->signaler, SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

    //

    ff_enc_cam=new FFEncoderThread(FFEncoder::Mode::webcam, &enc_base_filename, QString("capturer %1").arg(VERSION_STRING), this);


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

    audio_sender=new AudioSender(this);

    //

    http_server=new HttpServer(settings->http_server.enabled ? settings->http_server.port : 0, this);
    http_server->setSettingsModel(messenger->settingsModel());
    connect(http_server, SIGNAL(keyPressed(int)), SLOT(keyPressed(int)));
    connect(http_server, SIGNAL(checkEncoders()), SLOT(checkEncoders()), Qt::DirectConnection);
    connect(http_server, SIGNAL(playerSeek(qint64)), ff_dec, SLOT(seek(qint64)));
    connect(ff_dec, SIGNAL(durationChanged(qint64)), http_server, SLOT(setPlayerDuration(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(positionChanged(qint64)), http_server, SLOT(setPlayerPosition(qint64)), Qt::QueuedConnection);
    connect(messenger, SIGNAL(freeSpace(qint64)), http_server, SLOT(setFreeSpace(qint64)), Qt::QueuedConnection);

    //

    SettingsModel::Data set_model_data;

    // { primary device
    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.name="primary device";
    set_model_data.group="primary device";

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="source type";

    for(int i=0; i<SourceInterface::Type::size; ++i) {
        if(SourceInterface::isImplemented(i)) {
            set_model_data.values << SourceInterface::title(i);
            set_model_data.values_data << i;
        }
    }

    set_model_data.value=&settings->primary_device.index;

    int index_primary_device=
            messenger->settingsModel()->add(set_model_data);


    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="start device";
    set_model_data.value=&settings->primary_device.start;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="stop device";
    set_model_data.value=&settings->primary_device.stop;

    messenger->settingsModel()->add(set_model_data);


    // } primary device

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

    settingsModelDataChanged(index_primary_device, 0, 0);

}

MainWindow::~MainWindow()
{
    settings->save();
}

void MainWindow::setDevicePrimary(SourceInterface::Type::T type)
{
    emit messenger->signalLost(true);

    if(device_primary) {
        device_primary->deviceStop();

        delete device_primary;
    }

    device_primary=nullptr;

    switch((int)type) {
    case SourceInterface::Type::disabled:
        break;

    case SourceInterface::Type::dummy:
        device_primary=new DummyDevice();
        break;

    case SourceInterface::Type::ffmpeg:
        device_primary=new FFSource();
        break;

    case SourceInterface::Type::magewell:
        device_primary=new MagewellDevice();
        break;

    case SourceInterface::Type::decklink:
        device_primary=new DeckLinkThread();
        break;

    default:
        break;
    }

    messenger->settingsModel()->removeGroup(settings->primary_device.group_settings);

    if(!device_primary)
        return;

    SettingsModel::Data set_model_data;


    if(type==SourceInterface::Type::dummy) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->primary_device.dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings->primary_device.group_settings;
        set_model_data.group=settings->primary_device.group_settings;

        messenger->settingsModel()->insert(&settings->primary_device.stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings->primary_device.dummy_device.framesize;
        set_model_data.name="frame size";

        foreach(QSize s, DummyDevice::availableFramesizes()) {
            QVariant var;
            var.setValue(s);

            set_model_data.values << QString("%1x%2").arg(s.width()).arg(s.height());
            set_model_data.values_data << var;
        }

        messenger->settingsModel()->insert(&settings->primary_device.dummy, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="show frame counter";
        set_model_data.value=&settings->primary_device.dummy_device.show_frame_counter;

        messenger->settingsModel()->insert(&settings->primary_device.dummy_device.framesize, set_model_data);
    }

    if(type==SourceInterface::Type::ffmpeg) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->primary_device.dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings->primary_device.group_settings;
        set_model_data.group=settings->primary_device.group_settings;

        messenger->settingsModel()->insert(&settings->primary_device.stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="audio device";


        QStringList cam_devices=FFSource::availableAudioInput();


        if(cam_devices.isEmpty()) {
            set_model_data.values << "null";

        } else {
            foreach(QString dev_name, cam_devices) {
                set_model_data.values << dev_name;
            }
        }

        set_model_data.value=&settings->primary_device.ff_device.index_audio;

        messenger->settingsModel()->insert(&settings->primary_device.dummy, set_model_data);

        set_model_data.values.clear();

        //

        cam_devices=FFSource::availableCameras();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="video device";
        set_model_data.priority=SettingsModel::Priority::high;

        if(cam_devices.isEmpty()) {
            set_model_data.values << "null";

        } else {
            foreach(QString dev_name, cam_devices) {
                set_model_data.values << dev_name;
            }
        }

        FFSource *ff_device=static_cast<FFSource*>(device_primary);

        if(!ff_device->setVideoDevice(settings->primary_device.ff_device.index_video))
            settings->primary_device.ff_device.index_video=0;

        set_model_data.value=&settings->primary_device.ff_device.index_video;

        int model_index_cam_name=
                messenger->settingsModel()->insert(&settings->primary_device.ff_device.index_audio, set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="resolution";

        QList <QSize> supported_resolutions=ff_device->supportedResolutions();

        foreach(QSize val, supported_resolutions) {
            set_model_data.values << QString("%1x%2").arg(val.width()).arg(val.height());
            set_model_data.values_data << val;
        }

        if(settings->primary_device.ff_device.framesize>=supported_resolutions.size())
            settings->primary_device.ff_device.framesize=0;

        set_model_data.value=&settings->primary_device.ff_device.framesize;

        QSize cam_resolution;

        if(!supported_resolutions.isEmpty())
            cam_resolution=supported_resolutions[settings->primary_device.ff_device.framesize];

        messenger->settingsModel()->insert(&settings->primary_device.ff_device.index_video, set_model_data);

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pixel format";

        QList <int> supported_pix_fmts=ff_device->supportedPixelFormats(cam_resolution);

        if(settings->primary_device.ff_device.pixel_format>=supported_pix_fmts.size())
            settings->primary_device.ff_device.pixel_format=0;

        int64_t cam_pix_fmt=0;

        if(!supported_pix_fmts.isEmpty())
            cam_pix_fmt=supported_pix_fmts[settings->primary_device.ff_device.pixel_format];

        foreach(qint64 val, supported_pix_fmts) {
            set_model_data.values << PixelFormat::toStringView(val);
            set_model_data.values_data << val;
        }

        set_model_data.value=&settings->primary_device.ff_device.pixel_format;

        messenger->settingsModel()->insert(&settings->primary_device.ff_device.framesize, set_model_data);

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="framerate";
        set_model_data.priority=SettingsModel::Priority::low;

        QList <AVRational> framerate=ff_device->supportedFramerates(cam_resolution, cam_pix_fmt);

        foreach(AVRational val, framerate) {
            set_model_data.values << QString::number(Framerate::fromRational(val));
            set_model_data.values_data << QVariant::fromValue<AVRational>(val);
        }

        if(settings->primary_device.ff_device.framerate>=framerate.size())
            settings->primary_device.ff_device.framerate=0;

        set_model_data.value=&settings->primary_device.ff_device.framerate;

        messenger->settingsModel()->insert(&settings->primary_device.ff_device.pixel_format, set_model_data);

        set_model_data.values.clear();

        //

        if(!cam_devices.isEmpty())
            settingsModelDataChanged(model_index_cam_name, 0, false);
    }

    if(type==SourceInterface::Type::magewell) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->primary_device.dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings->primary_device.group_settings;
        set_model_data.group=settings->primary_device.group_settings;

        messenger->settingsModel()->insert(&settings->primary_device.stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings->primary_device.magewell.index;
        set_model_data.name="name";

        MagewellDevice::Devices devices=MagewellDevice::availableDevices();

        if(devices.isEmpty()) {
            set_model_data.values << "null";

        } else {
            for(int i=0; i<devices.size(); ++i) {
                MagewellDevice::Device dev=devices[i];

                QVariant var;
                var.setValue(dev);

                set_model_data.values << dev.name;
                set_model_data.values_data << var;
            }
        }

        messenger->settingsModel()->insert(&settings->primary_device.dummy, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pixel format";

        PixelFormats pixel_formats=MagewellDevice::supportedPixelFormats();

        foreach(PixelFormat pf, pixel_formats) {
            set_model_data.values << pf.toStringView();
            set_model_data.values_data << (int)pf;
        }

        set_model_data.value=&settings->primary_device.magewell.pixel_format;

        messenger->settingsModel()->insert(&settings->primary_device.magewell.index, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="color format";

        for(int i=0; i<MagewellDevice::Device::ColorFormat::size; ++i) {
            set_model_data.values << MagewellDevice::Device::ColorFormat::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings->primary_device.magewell.color_format;

        messenger->settingsModel()->insert(&settings->primary_device.magewell.pixel_format, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="quantization range";

        for(int i=0; i<MagewellDevice::Device::QuantizationRange::size; ++i) {
            set_model_data.values << MagewellDevice::Device::QuantizationRange::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings->primary_device.magewell.quantization_range;

        messenger->settingsModel()->insert(&settings->primary_device.magewell.color_format, set_model_data);
    }


    if(type==SourceInterface::Type::decklink) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->primary_device.dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings->primary_device.group_settings;
        set_model_data.group=settings->primary_device.group_settings;

        messenger->settingsModel()->insert(&settings->primary_device.stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings->primary_device.decklink.index;
        set_model_data.name="name";

        Decklink::Devices devices=Decklink::getDevices();

        if(devices.isEmpty()) {
            set_model_data.values << "null";

        } else {
            for(int i=0; i<devices.size(); ++i) {
                Decklink::Device dev=devices[i];

                QVariant var;
                var.setValue(dev);

                set_model_data.values << dev.name;
                set_model_data.values_data << var;
            }
        }

        messenger->settingsModel()->insert(&settings->primary_device.dummy, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="audio sample size";
        set_model_data.values << "16 bit" << "32 bit";
        set_model_data.values_data << 16 << 32;

        set_model_data.value=&settings->primary_device.decklink.audio_sample_size;

        messenger->settingsModel()->insert(&settings->primary_device.decklink.index, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="video depth";
        set_model_data.values << "8 bit" << "10 bit";
        set_model_data.values_data << 0 << 1;

        set_model_data.value=&settings->primary_device.decklink.video_bitdepth;

        messenger->settingsModel()->insert(&settings->primary_device.decklink.audio_sample_size, set_model_data);
    }


    connect(dynamic_cast<QObject*>(device_primary),
            SIGNAL(formatChanged(QString)),
            messenger, SIGNAL(formatChanged(QString)), Qt::QueuedConnection);

    connect(dynamic_cast<QObject*>(device_primary),
            SIGNAL(formatChanged(QString)),
            http_server, SLOT(formatChanged(QString)), Qt::QueuedConnection);

    connect(dynamic_cast<QObject*>(device_primary),
            SIGNAL(frameSkipped()), SLOT(frameSkipped()), Qt::QueuedConnection);

    connect(dynamic_cast<QObject*>(device_primary),
            SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

    connect(dynamic_cast<QObject*>(device_primary),
            SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);


    device_primary->subscribe(messenger->videoSourceMain()->frameBuffer());
    device_primary->subscribe(audio_output->frameBuffer());
    device_primary->subscribe(ff_enc->frameBuffer());
    device_primary->subscribe(audio_level->frameBuffer());
    device_primary->subscribe(audio_sender->frameBuffer());
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



    static bool ignore_event=false;

    if(ignore_event) {
        qInfo() << "ignore event";
        return;
    }

    struct Tmp {  Tmp() { ignore_event=true; }  ~Tmp() { ignore_event=false; }  } tmp;

    Q_UNUSED(tmp);


    if(data->value==&settings->primary_device.index) {
        setDevicePrimary((SourceInterface::Type::T)messenger->settingsModel()->data_p(
                             &settings->primary_device.index)->values_data[settings->primary_device.index].toInt());
        deviceStart();
    }

    if(device_primary && device_primary->type()==SourceInterface::Type::ffmpeg) {
        FFSource *ff_device=static_cast<FFSource*>(device_primary);

        if(data->value==&settings->primary_device.ff_device.index_video) {
            qInfo() << "enter";

            ff_device->setVideoDevice(settings->primary_device.ff_device.index_video);

            QStringList list_values;
            QVariantList list_values_data;

            foreach(QSize val, ff_device->supportedResolutions()) {
                list_values << QString("%1x%2").arg(val.width()).arg(val.height());
                list_values_data << val;
            }

            if(settings->primary_device.ff_device.framesize>=list_values.size()) {
                settings->primary_device.ff_device.framesize=0;
            }

            model->setData(&settings->primary_device.ff_device.framesize, SettingsModel::Role::values, list_values, false, true);
            model->setData(&settings->primary_device.ff_device.framesize, SettingsModel::Role::values_data, list_values_data, false, true);

            if(settings->primary_device.ff_device.framesize>=list_values.size())
                settings->primary_device.ff_device.framesize=list_values.size() - 1;


            if(!list_values_data.isEmpty()) {
                const QSize size=list_values_data.value(settings->primary_device.ff_device.framesize).toSize();

                list_values.clear();
                list_values_data.clear();

                foreach(qint64 val, ff_device->supportedPixelFormats(size)) {
                    list_values << PixelFormat::toStringView(val);
                    list_values_data << val;
                }

                model->setData(&settings->primary_device.ff_device.pixel_format, SettingsModel::Role::values, list_values, false, true);
                model->setData(&settings->primary_device.ff_device.pixel_format, SettingsModel::Role::values_data, list_values_data, false, true);

                if(settings->primary_device.ff_device.pixel_format>=list_values.size())
                    settings->primary_device.ff_device.pixel_format=list_values.size() - 1;

                //

                list_values.clear();
                list_values_data.clear();

                if(!messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data.isEmpty()) {
                    foreach(AVRational val, ff_device->supportedFramerates(size, messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data[settings->primary_device.ff_device.pixel_format].toLongLong())) {
                        list_values << QString::number(Framerate::fromRational(val));
                        list_values_data << QVariant::fromValue<AVRational>(val);
                    }

                } else {
                    qWarning() << "model no framerate data. pixel_format" << settings->primary_device.ff_device.pixel_format;
                }

                model->setData(&settings->primary_device.ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
                model->setData(&settings->primary_device.ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

                if(!list_values.isEmpty()) {
                    if(settings->primary_device.ff_device.framerate>=list_values.size())
                        settings->primary_device.ff_device.framerate=list_values.size() - 1;
                }

            } else {
                qInfo() << "list_values_data is empty!!!!1";
            }
        }

        if(data->value==&settings->primary_device.ff_device.framesize) {
            QSize size=data->values_data.value(settings->primary_device.ff_device.framesize).toSize();

            QStringList list_values;
            QVariantList list_values_data;

            //

            foreach(qint64 val, ff_device->supportedPixelFormats(size)) {
                list_values << PixelFormat::toStringView(val);
                list_values_data << val;
            }

            if(settings->primary_device.ff_device.pixel_format>=list_values.size()) {
                settings->primary_device.ff_device.pixel_format=0;
            }

            model->setData(&settings->primary_device.ff_device.pixel_format, SettingsModel::Role::values, list_values, false, true);
            model->setData(&settings->primary_device.ff_device.pixel_format, SettingsModel::Role::values_data, list_values_data, false, true);

            //

            list_values.clear();
            list_values_data.clear();

            if(!messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data.isEmpty())
                foreach(AVRational val, ff_device->supportedFramerates(size, messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data[settings->primary_device.ff_device.pixel_format].toLongLong())) {
                    list_values << QString::number(Framerate::fromRational(val));
                    list_values_data << QVariant::fromValue<AVRational>(val);
                }

            model->setData(&settings->primary_device.ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
            model->setData(&settings->primary_device.ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

            if(!list_values.isEmpty()) {
                if(settings->primary_device.ff_device.framerate>=list_values.size())
                    settings->primary_device.ff_device.framerate=list_values.size() - 1;
            }
        }

        if(data->value==&settings->primary_device.ff_device.pixel_format) {
            QSize size;
            int64_t pixel_format=0;

            if(settings->primary_device.ff_device.framesize<
                    messenger->settingsModel()->data_p(&settings->primary_device.ff_device.framesize)->values_data.size())
                size=messenger->settingsModel()->data_p(&settings->primary_device.ff_device.framesize)->values_data[settings->primary_device.ff_device.framesize].toSize();

            if(settings->primary_device.ff_device.pixel_format<
                    messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data.size())
                pixel_format=messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data[settings->primary_device.ff_device.pixel_format].toLongLong();

            QStringList list_values;
            QVariantList list_values_data;

            foreach(AVRational val, ff_device->supportedFramerates(size, pixel_format)) {
                list_values << QString::number(Framerate::fromRational(val));
                list_values_data << QVariant::fromValue<AVRational>(val);
            }

            model->setData(&settings->primary_device.ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
            model->setData(&settings->primary_device.ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

            if(!list_values.isEmpty()) {
                if(settings->primary_device.ff_device.framerate>=list_values.size())
                    settings->primary_device.ff_device.framerate=list_values.size() - 1;
            }
        }
    }

    //

    if(data->value==&settings->rec.encoder_video) {
        // pix_fmt

        if(settings->rec.encoder_video>=data->values_data.size())
            settings->rec.encoder_video=data->values_data.size() - 1;

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
                model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                model->setData(i, SettingsModel::Role::values, list_values, false, true);
                model->setData(i, SettingsModel::Role::value, settings->rec.pixel_format.value(QString::number(settings->rec.encoder_video), 0), false, true);

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
                model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                model->setData(i, SettingsModel::Role::values, list_values, false, true);
                model->setData(i, SettingsModel::Role::value, settings->rec.preset.value(QString::number(settings->rec.encoder_video), 0), false, true);

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


    if(data->value==&settings->primary_device.start) {
        deviceStart();
    }

    if(data->value==&settings->primary_device.stop) {
        deviceStop();
    }

    if(data->value==&settings->rec.check_encoders) {
        checkEncoders();
    }


    messenger->settingsModel()->updateQml();
}

void MainWindow::deviceStart()
{
    if(!device_primary)
        return;

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceStop", Qt::QueuedConnection);

    qApp->processEvents();

    if(device_primary->type()==SourceInterface::Type::dummy) {
        DummyDevice::Device *dev=new DummyDevice::Device();

        dev->frame_size=messenger->settingsModel()->data_p(&settings->primary_device.dummy_device.framesize)->values_data.value(
                    settings->primary_device.dummy_device.framesize, QSize(1920, 1080)).toSize();

        dev->show_frame_counter=settings->primary_device.dummy_device.show_frame_counter;

        device_primary->setDevice(dev);
    }

    if(device_primary->type()==SourceInterface::Type::dummy) {
        ;
    }

    if(device_primary->type()==SourceInterface::Type::ffmpeg) {
        FFSource::Device *dev=new FFSource::Device();

        dev->framerate=messenger->settingsModel()->data_p(&settings->primary_device.ff_device.framerate)->values_data.value(
                    settings->primary_device.ff_device.framerate).value<AVRational>();

        dev->pixel_format=messenger->settingsModel()->data_p(&settings->primary_device.ff_device.pixel_format)->values_data.value(
                    settings->primary_device.ff_device.pixel_format, 0).toInt();

        dev->size=messenger->settingsModel()->data_p(&settings->primary_device.ff_device.framesize)->values_data.value(
                    settings->primary_device.ff_device.framesize, QSize(640, 480)).toSize();

        device_primary->setDevice(dev);
    }

    if(device_primary->type()==SourceInterface::Type::magewell) {
        MagewellDevice::Devices list=MagewellDevice::availableDevices();

        if(settings->primary_device.magewell.index>=list.size()) {
            qInfo() << "magewell.index out of range";
            return;
        }

        PixelFormat pix_fmt=
                messenger->settingsModel()->data_p(&settings->primary_device.magewell.pixel_format)->values_data.value(
                    settings->primary_device.magewell.pixel_format, PixelFormat::nv12).toInt();


        MagewellDevice::Device *dev=new MagewellDevice::Device();
        (*dev)=list[settings->primary_device.magewell.index];

        dev->pixel_format=pix_fmt;
        dev->color_format=(MagewellDevice::Device::ColorFormat::T)settings->primary_device.magewell.color_format;
        dev->quantization_range=(MagewellDevice::Device::QuantizationRange::T)settings->primary_device.magewell.quantization_range;
        dev->pts_enabled=true;

        device_primary->setDevice(dev);
    }

    if(device_primary->type()==SourceInterface::Type::decklink) {
        Decklink::Devices devices=
                Decklink::getDevices();

        if(devices.isEmpty())
            return;

        DeckLinkThread::Device *dev=new DeckLinkThread::Device();

        dev->device=devices.first();

        dev->source_10bit=messenger->settingsModel()->data_p(&settings->primary_device.decklink.video_bitdepth)->values_data.value(
                    settings->primary_device.decklink.video_bitdepth, 0).toBool();

        dev->audio_sample_size=(SourceInterface::AudioSampleSize::T)
                messenger->settingsModel()->data_p(&settings->primary_device.decklink.audio_sample_size)->values_data.value(
                    settings->primary_device.decklink.audio_sample_size, SourceInterface::AudioSampleSize::bitdepth_16).toInt();


        device_primary->setHalfFps(false);

        device_primary->setDevice(dev);
    }

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceStart", Qt::QueuedConnection);
}

void MainWindow::deviceStop()
{
    if(!device_primary)
        return;

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceStop", Qt::QueuedConnection);
}

void MainWindow::startStopRecording()
{
    if(!device_primary)
        return;

    bool enc_running=ff_enc->isWorking() || ff_enc_cam->isWorking();

    if(enc_running) {
        ff_enc->stopCoder();
        ff_enc_cam->stopCoder();

    } else {
        if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED)
            return;

        enc_base_filename=
                QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");

        if(device_primary->gotSignal()) {
            FFEncoder::Config cfg;

            AVRational framerate=device_primary->currentFramerate();

            cfg.framerate=FFEncoder::calcFps(framerate.num, framerate.den, settings->rec.half_fps);
            cfg.frame_resolution_src=device_primary->currentFramesize();
            cfg.pixel_format_dst=messenger->settingsModel()->data_p(&settings->rec.pixel_format_current)->values_data[settings->rec.pixel_format_current].toInt();
            cfg.preset=messenger->settingsModel()->data_p(&settings->rec.preset_current)->values_data[settings->rec.preset_current].toString();
            cfg.video_encoder=(FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder_video)->values_data[settings->rec.encoder_video].toInt();
            cfg.crf=settings->rec.crf;
            cfg.pixel_format_src=device_primary->currentPixelFormat();
            cfg.audio_sample_size=device_primary->currentAudioSampleSize();
            cfg.audio_channels_size=device_primary->currentAudioChannels();
            cfg.downscale=settings->rec.downscale;
            cfg.scale_filter=settings->rec.scale_filter;
            cfg.color_primaries=settings->rec.color_primaries;
            cfg.color_space=settings->rec.color_space;
            cfg.color_transfer_characteristic=settings->rec.color_transfer_characteristic;
            cfg.nvenc=settings->nvenc;
            cfg.audio_flac=settings->rec.encoder_audio==1;

            ff_enc->setConfig(cfg);
        }
    }


    dropped_frames_counter=0;
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
    if(!device_primary)
        return;

    if(state!=FFDecoderThread::ST_STOPPED || device_primary->gotSignal()) {
        emit messenger->signalLost(false);

    } else {
        emit messenger->signalLost(true);
    }

    if(state==FFDecoderThread::ST_STOPPED) {
        QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceStart", Qt::QueuedConnection);

        emit messenger->showPlayerState(false);

    } else {
        QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceStop", Qt::QueuedConnection);
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
