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
#include <QApplication>
#include <QFile>
#include <QGenericArgument>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStorageInfo>
#include <QTimer>

#include "settings.h"
#include "decklink_device_list.h"
#include "decklink_thread.h"
#include "audio_output.h"
#include "audio_level.h"
#include "qml_messenger.h"
#include "overlay_view.h"
#include "http_server.h"
#include "dialog_keyboard_shortcuts.h"
#include "audio_sender.h"
#include "tools_ff_source.h"
#include "ff_source.h"
#include "framerate.h"
#include "dummy_device.h"
#include "nv_tools.h"
#include "magewell_device.h"

#include "mainwindow.h"


MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
{
    QDir dir(qApp->applicationDirPath() + "/videos");

    if(!dir.exists())
        dir.mkdir(dir.absolutePath());

    //

    qmlRegisterType<SettingsModel>("FuckTheSystem", 0, 0, "SettingsModel");
    qmlRegisterType<FileSystemModel>("FuckTheSystem", 0, 0, "FileSystemModel");
    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");
    qmlRegisterType<QuickVideoSource>("FuckTheSystem", 0, 0, "QuickVideoSource");

    //

    settings->load();

    //

    nv_tools=new NvTools(this);

    const QStringList cuda_devices=nv_tools->availableDevices();

    if(!cuda_devices.isEmpty())
        nv_tools->monitoringStart(0);

    //

    settings_model=new SettingsModel();
    connect(settings_model, SIGNAL(dataChanged(int,int,bool)), SLOT(settingsModelDataChanged(int,int,bool)));

    //

    ff_enc_primary=new FFEncoderThread(FFEncoder::Mode::primary, &enc_base_filename, QString("capturer %1").arg(VERSION_STRING), this);

    connect(&ff_enc_primary->frameBuffer()->signaler, SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc_primary, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc_primary, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);

    //

    ff_enc_secondary=new FFEncoderThread(FFEncoder::Mode::secondary, &enc_base_filename, QString("capturer %1").arg(VERSION_STRING), this);

    connect(ff_enc_secondary, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc_secondary, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);

    //

    connect(ff_enc_primary, SIGNAL(restartOut()), ff_enc_secondary, SIGNAL(restartIn()), Qt::QueuedConnection);

    //

    audio_sender=new AudioSender(this);
    audio_sender->setSimplify(settings->main.simplify_audio_for_send);

    //

    http_server=new HttpServer(settings->http_server.enabled ? settings->http_server.port : 0, this);
    http_server->setSettingsModel(settings_model);
    connect(http_server, SIGNAL(keyPressed(int)), SLOT(keyPressed(int)));
    connect(http_server, SIGNAL(checkEncoders()), SLOT(checkEncoders()));
    connect(this, SIGNAL(freeSpace(qint64)), http_server, SLOT(setFreeSpace(qint64)), Qt::QueuedConnection);
    connect(this, SIGNAL(recStats(NRecStats)), http_server, SLOT(setRecStats(NRecStats)));
    connect(nv_tools, SIGNAL(stateChanged(NvState)), http_server, SLOT(setNvState(NvState)), Qt::QueuedConnection);


    if(!settings->main.headless) {
        messenger=new QmlMessenger(settings_model);

        connect(this, SIGNAL(freeSpace(qint64)), messenger, SIGNAL(freeSpace(qint64)));
        connect(this, SIGNAL(freeSpaceStr(QString)), messenger, SIGNAL(freeSpaceStr(QString)));
        connect(this, SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)));
        connect(ff_enc_secondary, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

        //

        overlay_view=new OverlayView();
        overlay_view->setMessenger(messenger);
        overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));
        overlay_view->addImageProvider("fs_image_provider", (QQmlImageProviderBase*)messenger->fileSystemModel()->imageProvider());

        //

        audio_output=newAudioOutput(this);

        //

        connect(ff_enc_primary, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

        //

        ff_dec=new FFDecoderThread(this);

        ff_dec->subscribeVideo(messenger->videoSourcePrimary()->frameBuffer());
        ff_dec->subscribeAudio(audio_output->frameBuffer());

        connect(messenger->fileSystemModel(), SIGNAL(playMedia(QString)), ff_dec, SLOT(open(QString)), Qt::QueuedConnection);
        connect(ff_dec, SIGNAL(durationChanged(qint64)), messenger, SIGNAL(playerDurationChanged(qint64)), Qt::QueuedConnection);
        connect(ff_dec, SIGNAL(positionChanged(qint64)), messenger, SIGNAL(playerPositionChanged(qint64)), Qt::QueuedConnection);
        connect(ff_dec, SIGNAL(stateChanged(int)), SLOT(playerStateChanged(int)), Qt::QueuedConnection);
        connect(messenger, SIGNAL(playerSetPosition(qint64)), ff_dec, SLOT(seek(qint64)));
        connect(http_server, SIGNAL(playerSeek(qint64)), ff_dec, SLOT(seek(qint64)), Qt::QueuedConnection);
        connect(ff_dec, SIGNAL(durationChanged(qint64)), http_server, SLOT(setPlayerDuration(qint64)), Qt::QueuedConnection);
        connect(ff_dec, SIGNAL(positionChanged(qint64)), http_server, SLOT(setPlayerPosition(qint64)), Qt::QueuedConnection);

        //

        audio_level_primary=new AudioLevel(this);

        connect(audio_level_primary, SIGNAL(levels(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
                messenger, SIGNAL(audioLevelPrimary(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::QueuedConnection);


        audio_level_secondary=new AudioLevel(this);

        connect(audio_level_secondary, SIGNAL(levels(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)),
                messenger, SIGNAL(audioLevelSecondary(qreal,qreal,qreal,qreal,qreal,qreal,qreal,qreal)), Qt::QueuedConnection);

        //

        if(settings->keyboard_shortcuts.need_setup || qApp->arguments().contains("--setup_shortcuts", Qt::CaseInsensitive)) {
            DialogKeyboardShortcuts dlg;

            for(int i=0; i<KeyCodeC::enm_size; ++i)
                dlg.setKey(i, (Qt::Key)settings->keyboard_shortcuts.code.key(i, DialogKeyboardShortcuts::defaultQtKey(i)));

            if(dlg.exec()==DialogKeyboardShortcuts::Accepted) {
                for(int i=0; i<KeyCodeC::enm_size; ++i)
                    settings->keyboard_shortcuts.code.insert(dlg.toQtKey(i), i);
            }
        }
    }


    QTimer *timer=new QTimer();

    connect(timer, SIGNAL(timeout()), SLOT(checkFreeSpace()));

    timer->start(1000);

    //

    SettingsModel::Data set_model_data;

    // { primary device

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.name="primary device";
    set_model_data.group="primary device";

    settings_model->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="source type";

    for(int i=0; i<SourceInterface::Type::size; ++i) {
        if(SourceInterface::isImplemented(i)) {
            set_model_data.values << SourceInterface::title(i);
            set_model_data.values_data << i;
        }
    }

    set_model_data.value=&settings->device_primary.index;

    int index_device_primary=
            settings_model->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="start device";
    set_model_data.value=&settings->device_primary.start;

    settings_model->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="stop device";
    set_model_data.value=&settings->device_primary.stop;

    settings_model->add(set_model_data);

    // } primary device

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;
    settings_model->add(set_model_data);

    // { secondary device

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.name="secondary device";
    set_model_data.group="secondary device";

    settings_model->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="source type";

    for(int i=0; i<SourceInterface::Type::size; ++i) {
        if(SourceInterface::isImplemented(i)) {
            set_model_data.values << SourceInterface::title(i);
            set_model_data.values_data << i;
        }
    }

    set_model_data.value=&settings->device_secondary.index;

    int index_device_secondary=
            settings_model->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="start device";
    set_model_data.value=&settings->device_secondary.start;

    settings_model->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="stop device";
    set_model_data.value=&settings->device_secondary.stop;

    settings_model->add(set_model_data);

    // } secondary device

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;
    settings_model->add(set_model_data);

    // { rec

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="rec";

    settings_model->add(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="check encoders";
    set_model_data.group="rec";
    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->rec.check_encoders;

    settings_model->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="audio encoder";

    set_model_data.values << "pcm" << "flac";

    set_model_data.value=&settings->rec.encoder_audio;

    settings_model->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="video encoder";
    set_model_data.priority=SettingsModel::Priority::high;


    set_model_data.value=&settings->rec.encoder_video;

    settings_model->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="preset";
    set_model_data.priority=SettingsModel::Priority::low;

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&settings->rec.preset_current;

    settings_model->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="pixel format";
    set_model_data.value=&settings->rec.pixel_format_current;

    settings_model->add(set_model_data);

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

    settings_model->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color primaries";
    set_model_data.value=&settings->rec.color_primaries;

    set_model_data.values << "disabled";
    set_model_data.values_data << -1;

    foreach(int value, FFEncoder::availableColorPrimaries()) {
        set_model_data.values << FFEncoder::colorPrimariesToString(value);
        set_model_data.values_data << value;
    }

    settings_model->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color space";
    set_model_data.value=&settings->rec.color_space;

    set_model_data.values << "disabled";
    set_model_data.values_data << -1;

    foreach(int value, FFEncoder::availableColorSpaces()) {
        set_model_data.values << FFEncoder::colorSpaceToString(value);
        set_model_data.values_data << value;
    }

    settings_model->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color transfer characteristic";
    set_model_data.value=&settings->rec.color_transfer_characteristic;

    set_model_data.values << "disabled";
    set_model_data.values_data << -1;

    foreach(int value, FFEncoder::availableColorTransferCharacteristics()) {
        set_model_data.values << FFEncoder::colorTransferCharacteristicToString(value);
        set_model_data.values_data << value;
    }

    settings_model->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="downscale";
    set_model_data.value=&settings->rec.downscale;

    for(int i=0; i<=FFEncoder::DownScale::to1800; i++)
        set_model_data.values << FFEncoder::DownScale::toString(i);

    settings_model->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="scale filter";
    set_model_data.value=&settings->rec.scale_filter;

    for(int i=0; i<=FFEncoder::ScaleFilter::Spline; i++)
        set_model_data.values << FFEncoder::ScaleFilter::toString(i);

    settings_model->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="half-fps";
    set_model_data.value=&settings->rec.half_fps;

    settings_model->add(set_model_data);

    // } rec

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;

    settings_model->add(set_model_data);

    // { nvenc
    if(!cuda_devices.isEmpty()) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->main.dummy;
        set_model_data.name="nvenc";

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.group="nvenc";
        set_model_data.name="enabled";
        set_model_data.value=&settings->nvenc.enabled;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="device";
        set_model_data.value=&settings->nvenc.device;

        set_model_data.values << "any";

        foreach(QString val, cuda_devices)
            set_model_data.values << val;

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="b frames";
        set_model_data.value=&settings->nvenc.b_frames;

        for(int i=0; i<6; i++)
            set_model_data.values << QString::number(i);

        settings_model->add(set_model_data);

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

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="gop size";
        set_model_data.value=&settings->nvenc.gop_size;

        for(int i=0; i<601; i++)
            set_model_data.values << QString::number(i);

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        for(int i=-1; i<52; i++)
            set_model_data.values << QString::number(i);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="qpI";
        set_model_data.value=&settings->nvenc.qp_i;

        settings_model->add(set_model_data);

        //

        set_model_data.name="qpP";
        set_model_data.value=&settings->nvenc.qp_p;

        settings_model->add(set_model_data);

        //

        set_model_data.name="qpB";
        set_model_data.value=&settings->nvenc.qp_b;

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="aq mode";
        set_model_data.value=&settings->nvenc.aq_mode;
        set_model_data.values << "disabled" << "spatial" << "temporal. not working with constqp?";

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="aq strength";
        set_model_data.value=&settings->nvenc.aq_strength;

        for(int i=0; i<16; i++)
            set_model_data.values << QString::number(i);

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="rc lookahead";
        set_model_data.value=&settings->nvenc.rc_lookahead;

        for(int i=-1; i<33; i++)
            set_model_data.values << QString::number(i);

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="surfaces";
        set_model_data.value=&settings->nvenc.surfaces;

        for(int i=-1; i<65; i++)
            set_model_data.values << QString::number(i);

        settings_model->add(set_model_data);

        set_model_data.values.clear();

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="no scenecut";
        set_model_data.value=&settings->nvenc.no_scenecut;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="forced idr";
        set_model_data.value=&settings->nvenc.forced_idr;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="b adapt";
        set_model_data.value=&settings->nvenc.b_adapt;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="nonref p";
        set_model_data.value=&settings->nvenc.nonref_p;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="strict gop";
        set_model_data.value=&settings->nvenc.strict_gop;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="weighted pred (disables bframes)";
        set_model_data.value=&settings->nvenc.weighted_pred;

        settings_model->add(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="bluray compatibility workarounds";
        set_model_data.value=&settings->nvenc.bluray_compat;

        settings_model->add(set_model_data);
    }

    // } nvenc

    if(!settings->main.headless) {
        QApplication::instance()->installEventFilter(this);

        overlay_view->setMinimumSize(640, 640/(16/9.));

        if(qApp->arguments().contains("--windowed")) {
            overlay_view->showNormal();

        } else {
            overlay_view->showFullScreen();
        }

        emit signalLost(true);
    }


    settingsModelDataChanged(index_device_secondary, 0, 0);
    settingsModelDataChanged(index_device_primary, 0, 0);
}

MainWindow::~MainWindow()
{
    settings->save();

    MagewellDevice::release();
}

void MainWindow::setDevicePrimary(SourceInterface::Type::T type)
{
    emit signalLost(true);

    if(!settings->main.headless)
        messenger->formatChanged("no signal");

    setDevice(true, type);
}

void MainWindow::setDeviceSecondary(SourceInterface::Type::T type)
{
    setDevice(false, type);
}

void MainWindow::setDevice(bool primary, SourceInterface::Type::T type)
{
    SourceInterface **device;
    Settings::SourceDevice *settings_device;

    if(primary) {
        device=&device_primary;
        settings_device=&settings->device_primary;

    } else {
        device=&device_secondary;
        settings_device=&settings->device_secondary;
    }

    if(*device) {
        (*device)->deviceStop();

        delete (*device);
    }

    (*device)=nullptr;

    switch((int)type) {
    case SourceInterface::Type::disabled:
        break;

    case SourceInterface::Type::dummy:
        (*device)=new DummyDevice();
        break;

    case SourceInterface::Type::ffmpeg:
        (*device)=new FFSource();
        break;

    case SourceInterface::Type::magewell:
        (*device)=new MagewellDevice();
        break;

    case SourceInterface::Type::decklink:
        (*device)=new DeckLinkThread();
        break;

    default:
        break;
    }


    settings_model->removeGroup(settings_device->group_settings);

    if(!(*device))
        return;

    SettingsModel::Data set_model_data;


    if(type==SourceInterface::Type::dummy) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings_device->dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings_device->group_settings;
        set_model_data.group=settings_device->group_settings;

        settings_model->insert(&settings_device->stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings_device->dummy_device.framesize;
        set_model_data.name="frame size";

        foreach(QSize s, DummyDevice::availableFramesizes()) {
            QVariant var;
            var.setValue(s);

            set_model_data.values << QString("%1x%2").arg(s.width()).arg(s.height());
            set_model_data.values_data << var;
        }

        settings_model->insert(&settings_device->dummy, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="show frame counter";
        set_model_data.value=&settings_device->dummy_device.show_frame_counter;

        settings_model->insert(&settings_device->dummy_device.framesize, set_model_data);
    }

    if(type==SourceInterface::Type::ffmpeg) {
        FFSource *ff_device=static_cast<FFSource*>(*device);

        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings_device->dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings_device->group_settings;
        set_model_data.group=settings_device->group_settings;

        settings_model->insert(&settings_device->stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="audio device";


        QStringList ff_devices=FFSource::availableAudioInput();

        set_model_data.values << "disabled";
        set_model_data.values_data << -1;

        for(int i=0; i<ff_devices.size(); ++i) {
            set_model_data.values << ff_devices[i];
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->ff_device.index_audio;

        settings_model->insert(&settings_device->dummy, set_model_data);

        ff_device->setAudioDevice(settings_model->valueData(&settings_device->ff_device.index_audio).toInt());

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="video device";
        set_model_data.priority=SettingsModel::Priority::high;


        ff_devices=FFSource::availableVideoInput();

        set_model_data.values << "disabled";
        set_model_data.values_data << -1;

        for(int i=0; i<ff_devices.size(); ++i) {
            set_model_data.values << ff_devices[i];
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->ff_device.index_video;

        int model_index_ff_video=
                settings_model->insert(&settings_device->ff_device.index_audio, set_model_data);

        ff_device->setVideoDevice(settings_model->valueData(&settings_device->ff_device.index_video).toInt());

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="resolution";

        QList <QSize> supported_resolutions=ff_device->supportedResolutions();

        foreach(QSize val, supported_resolutions) {
            set_model_data.values << QString("%1x%2").arg(val.width()).arg(val.height());
            set_model_data.values_data << val;
        }

        if(settings_device->ff_device.framesize>=supported_resolutions.size())
            settings_device->ff_device.framesize=0;

        set_model_data.value=&settings_device->ff_device.framesize;

        QSize resolution;

        if(!supported_resolutions.isEmpty())
            resolution=supported_resolutions[settings_device->ff_device.framesize];

        settings_model->insert(&settings_device->ff_device.index_video, set_model_data);

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pixel format";

        QList <int> supported_pix_fmts=ff_device->supportedPixelFormats(resolution);

        if(settings_device->ff_device.pixel_format>=supported_pix_fmts.size())
            settings_device->ff_device.pixel_format=0;

        int64_t pix_fmt=0;

        if(!supported_pix_fmts.isEmpty())
            pix_fmt=supported_pix_fmts[settings_device->ff_device.pixel_format];

        foreach(qint64 val, supported_pix_fmts) {
            set_model_data.values << PixelFormat::toStringView(val);
            set_model_data.values_data << val;
        }

        set_model_data.value=&settings_device->ff_device.pixel_format;

        settings_model->insert(&settings_device->ff_device.framesize, set_model_data);

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="framerate";
        set_model_data.priority=SettingsModel::Priority::low;

        QList <AVRational> framerate=ff_device->supportedFramerates(resolution, pix_fmt);

        foreach(AVRational val, framerate) {
            set_model_data.values << QString::number(Framerate::fromRational(val));
            set_model_data.values_data << QVariant::fromValue<AVRational>(val);
        }

        if(settings_device->ff_device.framerate>=framerate.size())
            settings_device->ff_device.framerate=0;

        set_model_data.value=&settings_device->ff_device.framerate;

        settings_model->insert(&settings_device->ff_device.pixel_format, set_model_data);

        set_model_data.values.clear();

        //

        if(!ff_devices.isEmpty())
            settingsModelDataChanged(model_index_ff_video, 0, false);
    }

    if(type==SourceInterface::Type::magewell) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings_device->dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings_device->group_settings;
        set_model_data.group=settings_device->group_settings;

        settings_model->insert(&settings_device->stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings_device->magewell.index;
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

        settings_model->insert(&settings_device->dummy, set_model_data);

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

        set_model_data.value=&settings_device->magewell.pixel_format;

        settings_model->insert(&settings_device->magewell.index, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="frame size";

        set_model_data.values << "from source";
        set_model_data.values_data << QVariant::fromValue(QSize());

        foreach(QSize size, ToolsFFSource::resBuildSequence(QSize(), QSize(4096, 2160))) {
            set_model_data.values << QString("%1x%2").arg(size.width()).arg(size.height());
            set_model_data.values_data << QVariant::fromValue(size);
        }

        set_model_data.value=&settings_device->magewell.framesize;

        settings_model->insert(&settings_device->magewell.pixel_format, set_model_data);

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

        set_model_data.value=&settings_device->magewell.color_format;

        settings_model->insert(&settings_device->magewell.framesize, set_model_data);

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

        set_model_data.value=&settings_device->magewell.quantization_range;

        settings_model->insert(&settings_device->magewell.color_format, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pts mode";

        for(int i=0; i<MagewellDevice::Device::PtsMode::size; ++i) {
            set_model_data.values << MagewellDevice::Device::PtsMode::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->magewell.pts_mode;

        settings_model->insert(&settings_device->magewell.quantization_range, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="remap audio channels";

        for(int i=0; i<MagewellDevice::Device::AudioRemapMode::size; ++i) {
            set_model_data.values << MagewellDevice::Device::AudioRemapMode::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->magewell.audio_remap_mode;

        settings_model->insert(&settings_device->magewell.pts_mode, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="low latency";

        set_model_data.value=&settings_device->magewell.low_latency;

        settings_model->insert(&settings_device->magewell.audio_remap_mode, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="half-fps";

        set_model_data.value=&settings_device->magewell.half_fps;

        settings_model->insert(&settings_device->magewell.low_latency, set_model_data);
    }


    if(type==SourceInterface::Type::decklink) {
        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings_device->dummy;
        set_model_data.priority=SettingsModel::Priority::low;
        set_model_data.name=settings_device->group_settings;
        set_model_data.group=settings_device->group_settings;

        settings_model->insert(&settings_device->stop, set_model_data);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings_device->decklink.index;
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

        settings_model->insert(&settings_device->dummy, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="audio sample size";
        set_model_data.values << "16 bit" << "32 bit";
        set_model_data.values_data << 16 << 32;

        set_model_data.value=&settings_device->decklink.audio_sample_size;

        settings_model->insert(&settings_device->decklink.index, set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="video depth";
        set_model_data.values << "8 bit" << "10 bit";
        set_model_data.values_data << 0 << 1;

        set_model_data.value=&settings_device->decklink.video_bitdepth;

        settings_model->insert(&settings_device->decklink.audio_sample_size, set_model_data);
    }

    if(!settings->main.headless) {
        if(primary) {
            connect(dynamic_cast<QObject*>(*device),
                    SIGNAL(formatChanged(QString)),
                    messenger, SIGNAL(formatChanged(QString)), Qt::QueuedConnection);

            connect(dynamic_cast<QObject*>(*device),
                    SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

            (*device)->subscribe(messenger->videoSourcePrimary()->frameBuffer());
            (*device)->subscribe(audio_output->frameBuffer());
            (*device)->subscribe(audio_level_primary->frameBuffer());

        } else {
            (*device)->subscribe(messenger->videoSourceSecondary()->frameBuffer());
            (*device)->subscribe(audio_level_secondary->frameBuffer());
        }

        connect(dynamic_cast<QObject*>(*device),
                SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);
    }

    if(primary) {
        connect(dynamic_cast<QObject*>(*device),
                SIGNAL(formatChanged(QString)),
                http_server, SLOT(formatChanged(QString)), Qt::QueuedConnection);

        (*device)->subscribe(ff_enc_primary->frameBuffer());
        (*device)->subscribe(audio_sender->frameBuffer());

    } else {
        (*device)->subscribe(ff_enc_secondary->frameBuffer());
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if(event->type()==QEvent::KeyPress) {
        QKeyEvent *e=static_cast<QKeyEvent*>(event);

        if(e) {
            int key=e->key();

            // qInfo() << "key pressed" << e->key() << settings->keyboard_shortcuts.code.contains(key) << settings->keyboard_shortcuts.code.value(key);

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

void MainWindow::keyPressed(int code)
{
    switch(code) {
    case KeyCodeC::Rec:
        startStopRecording();

        break;

    case KeyCodeC::Exit:
        deleteLater();
        QApplication::exit(0);

        break;
    }

    static double hdr_shader_brightness=1.;
    static double hdr_shader_saturation=2.;

    if(!settings->main.headless) {
        switch(code) {
        case KeyCodeC::FileBrowser:
            if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
                messenger->showFileBrowser();

            break;

        case KeyCodeC::About:
            messenger->showHideAbout();
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

        case KeyCodeC::PreviewPrimary:
            previewPrimaryOnOff();
            break;

        case KeyCodeC::PreviewSecondary:
            previewSecondaryOnOff();
            break;

        case KeyCodeC::PreviewSecondaryChangePosition:
            messenger->previewSecondaryChangePosition();
            break;

        case KeyCodeC::PreviewSwitchHalfFps:
            messenger->videoSourcePrimary()->switchHalfFps();
            break;

        case KeyCodeC::HdrToSdr:
            static bool hdr_shader_enabled=false;
            hdr_shader_enabled=!hdr_shader_enabled;
            messenger->setHdrToSdrEnabled(hdr_shader_enabled);
            break;

        case KeyCodeC::HdrBrightnesPlus:
            hdr_shader_brightness+=.01;

            if(hdr_shader_brightness>4.)
                hdr_shader_brightness=4.;

            messenger->setHdrBrightness(hdr_shader_brightness);

            break;

        case KeyCodeC::HdrBrightnesMinus:
            hdr_shader_brightness-=.01;

            if(hdr_shader_brightness<0.)
                hdr_shader_brightness=0.;

            messenger->setHdrBrightness(hdr_shader_brightness);

            break;

        case KeyCodeC::HdrSaturationPlus:
            hdr_shader_saturation+=.1;

            if(hdr_shader_saturation>4.)
                hdr_shader_saturation=4.;

            messenger->setHdrSaturation(hdr_shader_saturation);

            break;

        case KeyCodeC::HdrSaturationMinus:
            hdr_shader_saturation-=.1;

            if(hdr_shader_saturation<0.)
                hdr_shader_saturation=0.;

            messenger->setHdrSaturation(hdr_shader_saturation);

            break;

        case KeyCodeC::FullScreen:
            if(overlay_view->isFullScreen())
                overlay_view->showNormal();

            else
                overlay_view->showFullScreen();

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
}

void MainWindow::checkEncoders()
{
    settings->checkEncoders();

    updateEncList();
}

void MainWindow::checkFreeSpace()
{
    QStorageInfo info=QStorageInfo(QApplication::applicationDirPath() + "/videos");

    emit freeSpace(info.bytesAvailable());
    emit freeSpaceStr(QString("%1 MB").arg(QLocale().toString(qint64(info.bytesAvailable()/1024./1024.))));
}

void MainWindow::settingsModelDataChanged(int index, int role, bool qml)
{
    Q_UNUSED(qml)

    SettingsModel::Data *data=settings_model->data_p(index);

    static bool ignore_event=false;

    if(ignore_event) {
        // qDebug() << "ignore event";
        return;
    }

    struct Tmp {  Tmp() { ignore_event=true; }  ~Tmp() { ignore_event=false; }  } tmp;

    Q_UNUSED(tmp);


    if(data->value==&settings->device_primary.index) {
        setDevicePrimary((SourceInterface::Type::T)settings_model->data_p(
                             &settings->device_primary.index)->values_data[settings->device_primary.index].toInt());
        deviceStart(true);
    }

    if(data->value==&settings->device_secondary.index) {
        setDeviceSecondary((SourceInterface::Type::T)settings_model->data_p(
                               &settings->device_secondary.index)->values_data[settings->device_secondary.index].toInt());
    }


    for(int i_dev=0; i_dev<2; ++i_dev) {
        SourceInterface **device;
        Settings::SourceDevice *settings_device;

        if(i_dev==0) {
            device=&device_primary;
            settings_device=&settings->device_primary;

        } else {
            device=&device_secondary;
            settings_device=&settings->device_secondary;
        }

        if((*device) && (*device)->type()==SourceInterface::Type::ffmpeg) {
            FFSource *ff_device=static_cast<FFSource*>(*device);

            if(data->value==&settings_device->ff_device.index_video) {
                QStringList list_values;
                QVariantList list_values_data;

                qml=false;

                if(settings_model->valueData(&settings_device->ff_device.index_video).toInt()<0) {
                    settings_model->setData(&settings_device->ff_device.framesize, SettingsModel::Role::values, list_values, false, true);
                    settings_model->setData(&settings_device->ff_device.framesize, SettingsModel::Role::values_data, list_values_data, false, true);

                    settings_model->setData(&settings_device->ff_device.pixel_format, SettingsModel::Role::values, list_values, false, true);
                    settings_model->setData(&settings_device->ff_device.pixel_format, SettingsModel::Role::values_data, list_values_data, false, true);

                    settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
                    settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

                } else {
                    ff_device->setVideoDevice(settings_model->valueData(&settings_device->ff_device.index_video).toInt());

                    foreach(QSize val, ff_device->supportedResolutions()) {
                        list_values << QString("%1x%2").arg(val.width()).arg(val.height());
                        list_values_data << val;
                    }


                    settings_model->setData(&settings_device->ff_device.framesize, SettingsModel::Role::values, list_values, false, true);
                    settings_model->setData(&settings_device->ff_device.framesize, SettingsModel::Role::values_data, list_values_data, false, true);

                    if(settings_device->ff_device.framesize<0)
                        settings_device->ff_device.framesize=0;

                    else if(settings_device->ff_device.framesize>=list_values.size())
                        settings_device->ff_device.framesize=list_values.size() - 1;


                    if(!list_values_data.isEmpty()) {
                        const QSize size=list_values_data.value(settings_device->ff_device.framesize).toSize();

                        list_values.clear();
                        list_values_data.clear();

                        foreach(qint64 val, ff_device->supportedPixelFormats(size)) {
                            list_values << PixelFormat::toStringView(val);
                            list_values_data << val;
                        }


                        settings_model->setData(&settings_device->ff_device.pixel_format, SettingsModel::Role::values, list_values, false, true);
                        settings_model->setData(&settings_device->ff_device.pixel_format, SettingsModel::Role::values_data, list_values_data, false, true);


                        if(settings_device->ff_device.pixel_format<0)
                            settings_device->ff_device.pixel_format=0;

                        else if(settings_device->ff_device.pixel_format>=list_values.size())
                            settings_device->ff_device.pixel_format=list_values.size() - 1;

                        //

                        list_values.clear();
                        list_values_data.clear();

                        if(!settings_model->data_p(&settings_device->ff_device.pixel_format)->values_data.isEmpty()) {
                            foreach(AVRational val, ff_device->supportedFramerates(size, settings_model->valueData(&settings_device->ff_device.pixel_format).toLongLong())) {
                                list_values << QString::number(Framerate::fromRational(val));
                                list_values_data << QVariant::fromValue<AVRational>(val);
                            }

                        } else {
                            qWarning() << "model no framerate data. pixel_format" << settings_device->ff_device.pixel_format;
                        }

                        settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
                        settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

                        if(!list_values.isEmpty()) {
                            if(settings_device->ff_device.framerate>=list_values.size())
                                settings_device->ff_device.framerate=list_values.size() - 1;
                        }

                    } else {
                        qInfo() << "list_values_data is empty!!!!1";
                    }
                }
            }


            if(data->value==&settings_device->ff_device.index_audio) {
                ff_device->setAudioDevice(settings_model->valueData(&settings_device->ff_device.index_audio).toInt());
            }


            if(data->value==&settings_device->ff_device.framesize) {
                qml=false;

                QSize size=data->values_data.value(settings_device->ff_device.framesize).toSize();

                QStringList list_values;
                QVariantList list_values_data;

                //

                foreach(qint64 val, ff_device->supportedPixelFormats(size)) {
                    list_values << PixelFormat::toStringView(val);
                    list_values_data << val;
                }

                if(settings_device->ff_device.pixel_format>=list_values.size()) {
                    settings_device->ff_device.pixel_format=0;
                }

                settings_model->setData(&settings_device->ff_device.pixel_format, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(&settings_device->ff_device.pixel_format, SettingsModel::Role::values_data, list_values_data, false, true);

                //

                list_values.clear();
                list_values_data.clear();

                if(!settings_model->data_p(&settings_device->ff_device.pixel_format)->values_data.isEmpty()) {
                    foreach(AVRational val, ff_device->supportedFramerates(size, settings_model->valueData(&settings_device->ff_device.pixel_format).toLongLong())) {
                        list_values << QString::number(Framerate::fromRational(val));
                        list_values_data << QVariant::fromValue<AVRational>(val);
                    }
                }

                settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

                if(!list_values.isEmpty()) {
                    if(settings_device->ff_device.framerate>=list_values.size())
                        settings_device->ff_device.framerate=list_values.size() - 1;
                }
            }

            if(data->value==&settings_device->ff_device.pixel_format) {
                qml=false;

                QSize size;
                int64_t pixel_format=0;

                if(settings_device->ff_device.framesize<
                        settings_model->data_p(&settings_device->ff_device.framesize)->values_data.size())
                    size=settings_model->valueData(&settings_device->ff_device.framesize).toSize();

                if(settings_device->ff_device.pixel_format<
                        settings_model->data_p(&settings_device->ff_device.pixel_format)->values_data.size())
                    pixel_format=settings_model->valueData(&settings_device->ff_device.pixel_format).toLongLong();

                QStringList list_values;
                QVariantList list_values_data;

                foreach(AVRational val, ff_device->supportedFramerates(size, pixel_format)) {
                    list_values << QString::number(Framerate::fromRational(val));
                    list_values_data << QVariant::fromValue<AVRational>(val);
                }

                settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(&settings_device->ff_device.framerate, SettingsModel::Role::values_data, list_values_data, false, true);

                if(!list_values.isEmpty()) {
                    if(settings_device->ff_device.framerate>=list_values.size())
                        settings_device->ff_device.framerate=list_values.size() - 1;
                }
            }
        }
    }

    //

    if(data->value==&settings->rec.encoder_video) {
        qml=false;

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

        for(int i=0; i<settings_model->rowCount(); ++i) {
            if(settings_model->data_p(i)->value==&settings->rec.pixel_format_current) {
                settings_model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                settings_model->setData(i, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(i, SettingsModel::Role::value, settings->rec.pixel_format.value(QString::number(settings->rec.encoder_video), 0), false, true);

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

        for(int i=0; i<settings_model->rowCount(); ++i) {
            if(settings_model->data_p(i)->value==&settings->rec.preset_current) {
                settings_model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                settings_model->setData(i, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(i, SettingsModel::Role::value, settings->rec.preset.value(QString::number(settings->rec.encoder_video), 0), false, true);

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


    if(data->value==&settings->device_primary.start) {
        deviceStart(true);
    }

    if(data->value==&settings->device_primary.stop) {
        deviceStop(true);
    }

    if(data->value==&settings->device_secondary.start) {
        deviceStart(false);
    }

    if(data->value==&settings->device_secondary.stop) {
        deviceStop(false);
    }

    if(data->value==&settings->rec.check_encoders) {
        checkEncoders();
    }

    if(!qml) {
        settings_model->updateQml();
    }
}

void MainWindow::deviceStart(bool primary)
{
    SourceInterface **device;
    Settings::SourceDevice *settings_device;

    if(primary) {
        device=&device_primary;
        settings_device=&settings->device_primary;

    } else {
        device=&device_secondary;
        settings_device=&settings->device_secondary;
    }

    if(!(*device))
        return;

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(*device), "deviceStop", Qt::QueuedConnection);

    qApp->processEvents();

    if((*device)->type()==SourceInterface::Type::dummy) {
        DummyDevice::Device *dev=new DummyDevice::Device();

        dev->frame_size=settings_model->valueData(&settings_device->dummy_device.framesize, QSize(1920, 1080)).toSize();
        dev->show_frame_counter=settings_device->dummy_device.show_frame_counter;

        (*device)->setDevice(dev);
    }

    if((*device)->type()==SourceInterface::Type::dummy) {
        ;
    }

    if((*device)->type()==SourceInterface::Type::ffmpeg) {
        if(settings_model->valueData(&settings_device->ff_device.index_video).toInt()>=0) {
            FFSource::Device *dev=new FFSource::Device();

            dev->framerate=settings_model->valueData(&settings_device->ff_device.framerate).value<AVRational>();
            dev->pixel_format=settings_model->valueData(&settings_device->ff_device.pixel_format, 0).toInt();
            dev->size=settings_model->valueData(&settings_device->ff_device.framesize, QSize(640, 480)).toSize();

            (*device)->setDevice(dev);
        }
    }

    if((*device)->type()==SourceInterface::Type::magewell) {
        MagewellDevice::Devices list=MagewellDevice::availableDevices();

        if(settings_device->magewell.index>=list.size()) {
            qInfo() << "magewell.index out of range";
            return;
        }

        MagewellDevice::Device *dev=new MagewellDevice::Device();
        (*dev)=list[settings_device->magewell.index];

        dev->pixel_format=settings_model->valueData(&settings_device->magewell.pixel_format, PixelFormat::nv12).toInt();;
        dev->framesize=settings_model->valueData(&settings_device->magewell.framesize, QSize()).value<QSize>();
        dev->color_format=(MagewellDevice::Device::ColorFormat::T)settings_device->magewell.color_format;
        dev->quantization_range=(MagewellDevice::Device::QuantizationRange::T)settings_device->magewell.quantization_range;
        dev->pts_mode=settings_device->magewell.pts_mode;
        dev->low_latency=settings_device->magewell.low_latency;
        dev->half_fps=settings_device->magewell.half_fps;
        dev->audio_remap_mode=settings_device->magewell.audio_remap_mode;

        (*device)->setDevice(dev);
    }

    if((*device)->type()==SourceInterface::Type::decklink) {
        Decklink::Devices devices=
                Decklink::getDevices();

        if(devices.isEmpty())
            return;

        DeckLinkThread::Device *dev=new DeckLinkThread::Device();

        dev->device=devices.first();
        dev->source_10bit=settings_model->valueData(&settings_device->decklink.video_bitdepth, 0).toBool();
        dev->audio_sample_size=(SourceInterface::AudioSampleSize::T)
                settings_model->valueData(&settings_device->decklink.audio_sample_size, SourceInterface::AudioSampleSize::bitdepth_16).toInt();
        (*device)->setHalfFps(false);

        (*device)->setDevice(dev);
    }

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(*device), "deviceStart", Qt::QueuedConnection);
}

void MainWindow::deviceStop(bool primary)
{
    SourceInterface **device;

    if(primary) {
        device=&device_primary;

    } else {
        device=&device_secondary;
    }

    if(!(*device))
        return;

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(*device), "deviceStop", Qt::QueuedConnection);
}

void MainWindow::startStopRecording()
{
    const bool enc_running=ff_enc_primary->isWorking() || ff_enc_secondary->isWorking();

    if(enc_running) {
        ff_enc_primary->stopCoder();
        ff_enc_secondary->stopCoder();
        return;
    }

    if(!settings->main.headless && ff_dec->currentState()!=FFDecoderThread::ST_STOPPED)
        return;

    enc_base_filename.reset();

    if(device_primary) {
        if(device_primary->gotSignal()) {
            FFEncoder::Config cfg;

            AVRational framerate=device_primary->currentFramerate();

            cfg.framerate=FFEncoder::calcFps(framerate.num, framerate.den, settings->rec.half_fps);

            if(cfg.framerate==FFEncoder::Framerate::unknown)
                cfg.framerate_force=framerate;

            cfg.frame_resolution_src=device_primary->currentFramesize();
            cfg.pixel_format_dst=settings_model->valueData(&settings->rec.pixel_format_current).toInt();
            cfg.preset=settings_model->valueData(&settings->rec.preset_current).toString();
            cfg.video_encoder=(FFEncoder::VideoEncoder::T)settings_model->valueData(&settings->rec.encoder_video).toInt();
            cfg.crf=settings->rec.crf;
            cfg.pixel_format_src=device_primary->currentPixelFormat();
            cfg.audio_sample_size=device_primary->currentAudioSampleSize();
            cfg.audio_channels_size=device_primary->currentAudioChannels();
            cfg.downscale=settings->rec.downscale;
            cfg.scale_filter=settings->rec.scale_filter;
            cfg.color_primaries=settings_model->valueData(&settings->rec.color_primaries).toInt();
            cfg.color_space=settings_model->valueData(&settings->rec.color_space).toInt();
            cfg.color_transfer_characteristic=settings_model->valueData(&settings->rec.color_transfer_characteristic).toInt();
            cfg.nvenc=settings->nvenc;
            cfg.audio_flac=settings->rec.encoder_audio==1;
            cfg.input_type_flags=device_primary->typeFlags();

            ff_enc_primary->setConfig(cfg);
        }
    }

    if(device_secondary) {
        if(device_secondary->gotSignal()) {
            FFEncoder::Config cfg;

            AVRational framerate=device_secondary->currentFramerate();

            cfg.framerate=FFEncoder::calcFps(framerate.num, framerate.den, settings->rec.half_fps);
            cfg.frame_resolution_src=device_secondary->currentFramesize();
            cfg.pixel_format_dst=settings_model->valueData(&settings->rec.pixel_format_current).toInt();
            cfg.preset=settings_model->valueData(&settings->rec.preset_current).toString();
            cfg.video_encoder=(FFEncoder::VideoEncoder::T)settings_model->valueData(&settings->rec.encoder_video).toInt();
            cfg.crf=settings->rec.crf;
            cfg.pixel_format_src=device_secondary->currentPixelFormat();
            cfg.audio_sample_size=device_secondary->currentAudioSampleSize();
            cfg.audio_channels_size=device_secondary->currentAudioChannels();
            cfg.downscale=settings->rec.downscale;
            cfg.scale_filter=settings->rec.scale_filter;
            cfg.color_primaries=settings_model->valueData(&settings->rec.color_primaries).toInt();
            cfg.color_space=settings_model->valueData(&settings->rec.color_space).toInt();
            cfg.color_transfer_characteristic=settings_model->valueData(&settings->rec.color_transfer_characteristic).toInt();
            cfg.nvenc=settings->nvenc;
            cfg.audio_flac=settings->rec.encoder_audio==1;
            cfg.input_type_flags=device_secondary->typeFlags();

            ff_enc_secondary->setConfig(cfg);
        }
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
            settings_model->data_p(&settings->rec.encoder_video);

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

    settingsModelDataChanged(settings_model->data_p_index(&settings->rec.encoder_video), 0, false);
}

void MainWindow::encoderBufferOverload()
{
    ff_enc_primary->stopCoder();

    emit messenger->errorString("encoder buffer overload, recording stopped");
}

void MainWindow::previewPrimaryOnOff()
{
    settings->main.preview=!settings->main.preview;

    messenger->videoSourcePrimary()->frameBuffer()->setEnabled(settings->main.preview);
}

void MainWindow::previewSecondaryOnOff()
{
    if(settings->main.headless)
        return;

    messenger->videoSourceSecondary()->frameBuffer()->setEnabled(!messenger->videoSourceSecondary()->frameBuffer()->isEnabled());

    messenger->previewSecondary(messenger->videoSourceSecondary()->frameBuffer()->isEnabled());
}

void MainWindow::encoderStateChanged(bool state)
{
    http_server->setRecState(state);

    if(settings->main.headless)
        return;

    messenger->setRecStarted(state);
}

void MainWindow::playerStateChanged(int state)
{
    if(state!=FFDecoderThread::ST_STOPPED) {
        emit signalLost(false);

    } else {
        if(device_primary && device_primary->gotSignal())
            emit signalLost(false);

        else
            emit signalLost(true);
    }

    if(device_primary) {
        if(state==FFDecoderThread::ST_STOPPED) {
            QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceResume", Qt::QueuedConnection);

            emit messenger->showPlayerState(false);

        } else {
            QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceHold", Qt::QueuedConnection);
        }
    }
}

void MainWindow::updateStats(FFEncoder::Stats s)
{
    static FFEncoder::Stats st_primary={ };
    static FFEncoder::Stats st_secondary={ };
    static qint64 last_update=0;

    if(sender()==ff_enc_primary)
        st_primary=s;

    else
        st_secondary=s;

    s.avg_bitrate_audio=st_primary.avg_bitrate_audio + st_secondary.avg_bitrate_audio;
    s.avg_bitrate_video=st_primary.avg_bitrate_video + st_secondary.avg_bitrate_video;
    s.streams_size=st_primary.streams_size + st_secondary.streams_size;
    s.time=st_primary.time;

    if(s.time.isNull())
        s.time=st_secondary.time;

    if(QDateTime::currentMSecsSinceEpoch() - last_update<1000)
        return;

    last_update=QDateTime::currentMSecsSinceEpoch();

    const QPair <int, int> buffer_size=ff_enc_primary->frameBuffer()->size();

    QVariantMap map_bitrate_video;

    for(int i=0; i<st_primary.bitrate_video.size(); ++i)
        map_bitrate_video.insert(QString::number(st_primary.bitrate_video.keys()[i]), st_primary.bitrate_video.values()[i]);

    emit recStats(NRecStats(s.time, s.avg_bitrate_video + s.avg_bitrate_audio, map_bitrate_video, s.streams_size, s.dropped_frames_counter, buffer_size.second, buffer_size.first));

    if(settings->main.headless)
        return;

    messenger->updateRecStats(s.time.toString(QStringLiteral("HH:mm:ss")),
                              QString(QLatin1String("%1 Mbits/s (%2 MB/s)")).arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2),
                              QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString(QLatin1String("%1 bytes")).arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString(QLatin1String("buf state: %1/%2")).arg(buffer_size.first).arg(buffer_size.second),
                              QString(QLatin1String("frames dropped: %1")).arg(s.dropped_frames_counter));
}
