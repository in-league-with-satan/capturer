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
#include "dialog_setup.h"
#include "audio_sender.h"
#include "tools_ff_source.h"
#include "ff_source.h"
#include "framerate.h"
#include "dummy_device.h"
#include "nv_tools.h"
#include "magewell_device.h"
#include "term_gui.h"

#include "mainwindow.h"

void recAddModel(QList <SettingsModel::Data> *list_set_model_data, Settings::Rec *rec, const QString &group, const QStringList &cuda_devices);

MainWindow::MainWindow(QObject *parent)
    : QObject(parent)
{
    qmlRegisterType<SettingsModel>("FuckTheSystem", 0, 0, "SettingsModel");
    qmlRegisterType<FileSystemModel>("FuckTheSystem", 0, 0, "FileSystemModel");
    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");
    qmlRegisterType<QuickVideoSource>("FuckTheSystem", 0, 0, "QuickVideoSource");

    //

    settings->load();

    //

    if(!settings->main.headless && (settings->keyboard_shortcuts.need_setup || qApp->arguments().contains("--setup", Qt::CaseInsensitive))) {
        DialogSetup ds;
        ds.exec();
    }

    //

    nv_tools=new NvTools(this);

    cuda_devices=nv_tools->availableDevices();

    if(!cuda_devices.isEmpty())
        nv_tools->monitoringStart(0);

    //

    settings_model=new SettingsModel();
    connect(settings_model, SIGNAL(dataChanged(int,int,bool)), SLOT(settingsModelDataChanged(int,int,bool)));

    //

    if(!QDir().mkpath(settings->main.location_videos)) {
        qCritical() << "err mkpath:" << settings->main.location_videos;
        exit(4032);
    }

    //

    http_server=new HttpServer(settings->http_server.enabled ? settings->http_server.port : 0, this);
    http_server->setSettingsModel(settings_model);
    connect(http_server, SIGNAL(keyPressed(int)), SLOT(keyPressed(int)));
    connect(http_server, SIGNAL(checkEncoders()), SLOT(checkEncoders()));
    connect(this, SIGNAL(freeSpace(qint64)), http_server, SLOT(setFreeSpace(qint64)), Qt::QueuedConnection);
    connect(this, SIGNAL(recStats(NRecStats)), http_server, SLOT(setRecStats(NRecStats)));
    connect(nv_tools, SIGNAL(stateChanged(NvState)), http_server, SLOT(setNvState(NvState)), Qt::QueuedConnection);


    encoder_streaming=new FFEncoderThread(FFEncoder::StreamingMode, &enc_streaming_url, &enc_start_sync, QString(), QString("capturer %1").arg(VERSION_STRING), this);


    if(!settings->main.headless) {
        messenger=new QmlMessenger(settings_model);

        connect(this, SIGNAL(freeSpace(qint64)), messenger, SIGNAL(freeSpace(qint64)));
        connect(this, SIGNAL(freeSpaceStr(QString)), messenger, SIGNAL(freeSpaceStr(QString)));
        connect(this, SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)));

        //

        overlay_view=new OverlayView();
        overlay_view->setMessenger(messenger);
        overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));
        overlay_view->addImageProvider("fs_image_provider", (QQmlImageProviderBase*)messenger->fileSystemModel()->imageProvider());

        //

        audio_output=newAudioOutput(this);

        //

        connect(encoder_streaming, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

        if(term)
            connect(encoder_streaming, SIGNAL(stats(FFEncoder::Stats)), term, SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);

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
    }


    QTimer *timer=new QTimer();

    connect(timer, SIGNAL(timeout()), SLOT(checkFreeSpace()));

    timer->start(1000);

    //

    SettingsModel::Data set_model_data;

    //

    if(!settings->streaming.url.isEmpty()) {
        set_model_data.group="streaming_hdr";

        set_model_data.type=SettingsModel::Type::title;
        set_model_data.name="streaming";
        set_model_data.value=&settings->main.dummy;

        settings_model->add(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="service";

        set_model_data.values << "disabled";
        set_model_data.values_data << "";

        for(int i=0; i<settings->streaming.url.size(); ++i) {
            QString title=settings->streaming.url[i].toMap().keys().value(0).simplified();
            QString url=settings->streaming.url[i].toMap().values().value(0).toString().simplified();

            set_model_data.values << title;
            set_model_data.values_data << url;
        }

        set_model_data.value=&settings->streaming.url_index;

        settings_model->add(set_model_data);

        //

        if(settings->streaming.url_index>0) {
            QList <SettingsModel::Data> list_set_model_data;

            recAddModel(&list_set_model_data, &settings->streaming.rec, "streaming", cuda_devices);

            settings_model->add(list_set_model_data);
        }

        //

        set_model_data.type=SettingsModel::Type::divider;
        set_model_data.name="";
        set_model_data.value=&settings->main.dummy;

        settings_model->add(set_model_data);
    }

    //

    set_model_data.group="global";

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="device add";
    set_model_data.value=&settings->main.source_device_add;

    settings_model->add(set_model_data);

    set_model_data.name="device remove";
    set_model_data.value=&settings->main.source_device_remove;

    settings_model->add(set_model_data);

    //

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

    //

    if(settings->main.headless_curse) {
        term=new TermGui(settings_model, this);
        connect(this, SIGNAL(freeSpace(qint64)), term, SLOT(setFreeSpace(qint64)), Qt::QueuedConnection);
        connect(nv_tools, SIGNAL(stateChanged(NvState)), term, SLOT(setNvState(NvState)), Qt::QueuedConnection);

    } else {
        term=new TermGui(nullptr, this);
    }

    for(int i=0; i<settings->source_device.size(); ++i) {
        sourceDeviceAdd();
        settingsModelDataChanged(settings_model->data_p_index(&settings->source_device[i].index), 0, 0);
    }

    if(settings->main.headless_curse) {
        term->run();
    }
}

MainWindow::~MainWindow()
{
    for(int i=0; i<stream.size(); ++i) {
        deviceStop(i);
        qApp->processEvents();

        while(stream[i].source_device && stream[i].source_device->isActive())
            QThread::msleep(10);
    }

    settings->save();

    MagewellDevice::release();
}

void MainWindow::setDevice(uint8_t index, SourceInterface::Type::T type)
{
    if(!settings->main.headless) {
        messenger->formatChanged("no signal");
        messenger->temperatureChanged(-1.);
    }

    http_server->temperatureChanged(-1.);

    while(stream.size()<index + 1) {
        sourceDeviceAdd();
    }

    SourceInterface **device=&stream[index].source_device;
    Settings::SourceDevice *settings_device=settings->sourceDevice(index);
    FFEncoderThread *encoder=stream[index].encoder;
    AudioSender *audio_sender=stream[index].audio_sender;

    if(*device) {
        (*device)->deviceStop();

        delete (*device);

        if(0==index)
            emit signalLost(true);
    }

    (*device)=nullptr;

    switch((int)type) {
    case SourceInterface::Type::disabled:
        break;

    case SourceInterface::Type::dummy:
        (*device)=new DummyDevice(index);
        break;

    case SourceInterface::Type::ffmpeg:
        (*device)=new FFSource(index);
        break;

    case SourceInterface::Type::magewell:
        (*device)=new MagewellDevice(index);
        break;

    case SourceInterface::Type::decklink:
        (*device)=new DeckLinkThread(index);
        break;

    default:
        break;
    }

    settings_model->removeGroup(settings_device->group_settings);

    if(!(*device))
        return;


    QList <SettingsModel::Data> list_set_model_data;
    SettingsModel::Data set_model_data;

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings_device->dummy;
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.name="device setup";
    set_model_data.group=settings_device->group_settings;

    list_set_model_data.append(set_model_data);

    if(type==SourceInterface::Type::dummy) {
        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings_device->dummy_device.framesize;
        set_model_data.name="frame size";

        foreach(QSize s, DummyDevice::availableFramesizes()) {
            QVariant var;
            var.setValue(s);

            set_model_data.values << QString("%1x%2").arg(s.width()).arg(s.height());
            set_model_data.values_data << var;
        }

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="show frame counter";
        set_model_data.value=&settings_device->dummy_device.show_frame_counter;

        list_set_model_data.append(set_model_data);
    }

    if(type==SourceInterface::Type::ffmpeg) {
        FFSource *ff_device=static_cast<FFSource*>(*device);

        //

        set_model_data.type=SettingsModel::Type::button;
        set_model_data.name="reload devices";
        set_model_data.value=&settings_device->ff_device.reload_devices;

        list_set_model_data.append(set_model_data);

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

        settings_device->ff_device.index_audio=FFSource::indexAudioInput(settings_device->ff_device.name_audio);

        if(settings_device->ff_device.index_audio<0)
            settings_device->ff_device.index_audio=0;

        else
            settings_device->ff_device.index_audio++;

        list_set_model_data.append(set_model_data);

        //

        ff_device->setAudioDevice(settings_device->ff_device.index_audio - 1);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

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

        settings_device->ff_device.index_video=FFSource::indexVideoInput(settings_device->ff_device.name_video);

        if(settings_device->ff_device.index_video<0)
            settings_device->ff_device.index_video=0;

        else
            settings_device->ff_device.index_video++;


        list_set_model_data.append(set_model_data);

        ff_device->setVideoDevice(settings_device->ff_device.index_video - 1);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="resolution";

        const QList <QSize> supported_resolutions=ff_device->supportedResolutions();

        foreach(const QSize &val, supported_resolutions) {
            set_model_data.values << QString("%1x%2").arg(val.width()).arg(val.height());
            set_model_data.values_data << val;
        }

        if(settings_device->ff_device.framesize>=supported_resolutions.size())
            settings_device->ff_device.framesize=0;

        set_model_data.value=&settings_device->ff_device.framesize;

        QSize resolution;

        if(!supported_resolutions.isEmpty())
            resolution=supported_resolutions[settings_device->ff_device.framesize];

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pixel format";

        const QList <int> supported_pix_fmts=ff_device->supportedPixelFormats(resolution);

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

        list_set_model_data.append(set_model_data);

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="framerate";
        set_model_data.priority=SettingsModel::Priority::low;

        const QList <AVRational> framerate=ff_device->supportedFramerates(resolution, pix_fmt);

        foreach(const AVRational &val, framerate) {
            set_model_data.values << QString::number(Framerate::fromRational(val));
            set_model_data.values_data << QVariant::fromValue<AVRational>(val);
        }


        if(settings_device->ff_device.framerate>=framerate.size())
            settings_device->ff_device.framerate=0;


        set_model_data.value=&settings_device->ff_device.framerate;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="high depth audio";
        set_model_data.value=&settings_device->ff_device.high_depth_audio;

        list_set_model_data.append(set_model_data);
    }

    if(type==SourceInterface::Type::magewell) {
        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings_device->magewell.index;
        set_model_data.name="name";

        const MagewellDevice::Devices devices=MagewellDevice::availableDevices();

        if(devices.isEmpty()) {
            set_model_data.values << "null";

        } else {
            for(int i=0; i<devices.size(); ++i) {
                const MagewellDevice::Device dev=devices[i];

                QVariant var;
                var.setValue(dev);

                set_model_data.values << dev.name;
                set_model_data.values_data << var;
            }
        }

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pixel format";

        const PixelFormats pixel_formats=MagewellDevice::supportedPixelFormats();

        foreach(PixelFormat pf, pixel_formats) {
            set_model_data.values << pf.toStringView();
            set_model_data.values_data << (int)pf;
        }

        set_model_data.value=&settings_device->magewell.pixel_format;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="frame size";

        set_model_data.values << "from source";
        set_model_data.values_data << QVariant::fromValue(QSize());

        foreach(QSize size, ToolsFFSource::resBuildSequence(QSize(), QSize(4096, 2160))) {
            set_model_data.values << QString("%1x%2").arg(size.width()).arg(size.height());
            set_model_data.values_data << QVariant::fromValue(size);
        }

        set_model_data.value=&settings_device->magewell.framesize;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="color format in";

        for(int i=0; i<MagewellDevice::Device::ColorFormat::size; ++i) {
            set_model_data.values << MagewellDevice::Device::ColorFormat::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->magewell.color_format_in;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.name="color format out";
        set_model_data.value=&settings_device->magewell.color_format_out;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="quantization range in";

        for(int i=0; i<MagewellDevice::Device::QuantizationRange::size; ++i) {
            set_model_data.values << MagewellDevice::Device::QuantizationRange::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->magewell.quantization_range_in;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.name="quantization range out";
        set_model_data.value=&settings_device->magewell.quantization_range_out;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="pts mode";

        for(int i=0; i<MagewellDevice::Device::PtsMode::size; ++i) {
            set_model_data.values << MagewellDevice::Device::PtsMode::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->magewell.pts_mode;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="remap audio channels";

        for(int i=0; i<MagewellDevice::Device::AudioRemapMode::size; ++i) {
            set_model_data.values << MagewellDevice::Device::AudioRemapMode::toString(i);
            set_model_data.values_data << i;
        }

        set_model_data.value=&settings_device->magewell.audio_remap_mode;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="low latency";

        set_model_data.value=&settings_device->magewell.low_latency;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="half-fps";

        set_model_data.value=&settings_device->magewell.half_fps;

        list_set_model_data.append(set_model_data);
    }


    if(type==SourceInterface::Type::decklink) {
        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.value=&settings_device->decklink.index;
        set_model_data.name="name";

        const Decklink::Devices devices=Decklink::getDevices();

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

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="audio sample size";
        set_model_data.values << "16 bit" << "32 bit";
        set_model_data.values_data << 16 << 32;

        set_model_data.value=&settings_device->decklink.audio_sample_size;

        list_set_model_data.append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="video depth";
        set_model_data.values << "8 bit" << "10 bit";
        set_model_data.values_data << 0 << 1;

        set_model_data.value=&settings_device->decklink.video_bitdepth;

        list_set_model_data.append(set_model_data);
    }

    settings_model->insert(&settings_device->stop, list_set_model_data);

    if(!settings->main.headless) {
        if(index==0) {
            connect(dynamic_cast<QObject*>(*device),
                    SIGNAL(formatChanged(QString)),
                    messenger, SIGNAL(formatChanged(QString)), Qt::QueuedConnection);

            connect(dynamic_cast<QObject*>(*device),
                    SIGNAL(temperatureChanged(double)),
                    messenger, SIGNAL(temperatureChanged(double)), Qt::QueuedConnection);

            connect(dynamic_cast<QObject*>(*device),
                    SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

            (*device)->subscribe(messenger->videoSourcePrimary()->frameBuffer());
            (*device)->subscribe(audio_output->frameBuffer());
            (*device)->subscribe(audio_level_primary->frameBuffer());

        } else if(index==1) {
            (*device)->subscribe(messenger->videoSourceSecondary()->frameBuffer());
            (*device)->subscribe(audio_level_secondary->frameBuffer());
        }

        connect(dynamic_cast<QObject*>(*device),
                SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);
    }

    connect(dynamic_cast<QObject*>(*device),
            SIGNAL(signalLost(bool)), term, SLOT(reloadDevices()), Qt::QueuedConnection);

    connect(dynamic_cast<QObject*>(*device),
            SIGNAL(formatChanged(QString)), term, SLOT(reloadDevices()), Qt::QueuedConnection);

    (*device)->subscribe(encoder->frameBuffer());
    (*device)->subscribe(encoder_streaming->frameBuffer());
    (*device)->subscribe(audio_sender->frameBuffer());

    if(index==0) {
        connect(dynamic_cast<QObject*>(*device),
                SIGNAL(formatChanged(QString)),
                http_server, SLOT(formatChanged(QString)), Qt::QueuedConnection);

        connect(dynamic_cast<QObject*>(*device),
                SIGNAL(temperatureChanged(double)),
                http_server, SLOT(temperatureChanged(double)), Qt::QueuedConnection);

        (*device)->subscribe(encoder_streaming->frameBuffer());
    }

    term->reloadDevices();
}

void recAddModel(QList <SettingsModel::Data> *list_set_model_data, Settings::Rec *rec, const QString &group, const QStringList &cuda_devices)
{
    SettingsModel::Data set_model_data;

    set_model_data.group=group;

    // { rec

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="check encoders";

    set_model_data.value=&rec->check_encoders;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="audio encoder";

    foreach(FFEncoder::AudioEncoder::T enc, FFEncoder::AudioEncoder::list()) {
        set_model_data.values << FFEncoder::AudioEncoder::toString(enc);
        set_model_data.values_data << enc;
    }

    set_model_data.value=&rec->audio_encoder;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="audio bitrate";
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.value=&rec->audio_bitrate;

    for(int i=64; i<=720; i+=4) {
        set_model_data.values << QString("%1k").arg(i);
        set_model_data.values_data << i;
    }

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="downmix to stereo";
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.value=&rec->audio_downmix_to_stereo;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="video encoder";
    set_model_data.priority=SettingsModel::Priority::high;
    set_model_data.value=&rec->video_encoder;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="preset";
    set_model_data.priority=SettingsModel::Priority::low;

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.value=&rec->preset_current;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="pixel format";
    set_model_data.value=&rec->pixel_format_current;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="constant rate factor / quality";
    set_model_data.value=&rec->crf;

    for(int i=0; i<=80; ++i)
        set_model_data.values.append(QString::number(i));

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="video bitrate";
    set_model_data.value=&rec->video_bitrate;

    for(int i=0; i<=240000; i+=100) {
        if(i==0)
            set_model_data.values << "disabled";

        else
            set_model_data.values << QString("%1k").arg(i);

        set_model_data.values_data << i;
    }

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color primaries";
    set_model_data.value=&rec->color_primaries;

    set_model_data.values << "disabled";
    set_model_data.values_data << -1;

    foreach(int value, FFEncoder::availableColorPrimaries()) {
        set_model_data.values << FFEncoder::colorPrimariesToString(value);
        set_model_data.values_data << value;
    }

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color space";
    set_model_data.value=&rec->color_space;

    set_model_data.values << "disabled";
    set_model_data.values_data << -1;

    foreach(int value, FFEncoder::availableColorSpaces()) {
        set_model_data.values << FFEncoder::colorSpaceToString(value);
        set_model_data.values_data << value;
    }

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color transfer characteristic";
    set_model_data.value=&rec->color_transfer_characteristic;

    set_model_data.values << "disabled";
    set_model_data.values_data << -1;

    foreach(int value, FFEncoder::availableColorTransferCharacteristics()) {
        set_model_data.values << FFEncoder::colorTransferCharacteristicToString(value);
        set_model_data.values_data << value;
    }

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="color range";
    set_model_data.value=&rec->color_range;

    for(int i=0; i<swsColorRange::size; ++i)
        set_model_data.values << swsColorRange::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="downscale";
    set_model_data.value=&rec->downscale;

    for(int i=0; i<=FFEncoder::DownScale::to1800; i++)
        set_model_data.values << FFEncoder::DownScale::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="sws scale filter";
    set_model_data.value=&rec->scale_filter;

    for(int i=0; i<=FFEncoder::ScaleFilter::Spline; i++)
        set_model_data.values << FFEncoder::ScaleFilter::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="sws color space src";
    set_model_data.value=&rec->sws_color_space_src;

    for(int i=0; i<swsColorSpace::size; ++i)
        set_model_data.values << swsColorSpace::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="sws color space dst";
    set_model_data.value=&rec->sws_color_space_dst;

    for(int i=0; i<swsColorSpace::size; ++i)
        set_model_data.values << swsColorSpace::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="sws color range src";
    set_model_data.value=&rec->sws_color_range_src;

    for(int i=0; i<swsColorRange::size; ++i)
        set_model_data.values << swsColorRange::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="sws color range dst";
    set_model_data.value=&rec->sws_color_range_dst;

    for(int i=0; i<swsColorRange::size; ++i)
        set_model_data.values << swsColorRange::toString(i);

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="4:3 display aspect ratio";
    set_model_data.value=&rec->aspect_ratio_4_3;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="direct stream copy (for mjpeg or h264)";
    set_model_data.value=&rec->direct_stream_copy;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="fill dropped frames";
    set_model_data.value=&rec->fill_dropped_frames;

    list_set_model_data->append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::checkbox;
    set_model_data.name="half-fps";
    set_model_data.value=&rec->half_fps;

    list_set_model_data->append(set_model_data);

    // } rec

    // { nvenc
    if(!cuda_devices.isEmpty()) {
        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::title;
        set_model_data.value=&settings->main.dummy;
        set_model_data.name="nvenc";

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="enabled";
        set_model_data.value=&rec->nvenc.enabled;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="device";
        set_model_data.value=&rec->nvenc.device;

        set_model_data.values << "any";

        foreach(QString val, cuda_devices)
            set_model_data.values << val;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="b frames";
        set_model_data.value=&rec->nvenc.b_frames;

        for(int i=0; i<6; i++)
            set_model_data.values << QString::number(i);

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="ref frames";
        set_model_data.value=&rec->nvenc.ref_frames;

        for(int i=0; i<16; i++) {
            if(i>5)
                set_model_data.values << QString("%1 hevc only").arg(i);

            else
                set_model_data.values << QString::number(i);
        }

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="use b frames as references";
        set_model_data.value=&rec->nvenc.b_ref_mode;

        set_model_data.values << "disabled" << "each" << "middle";

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="gop size";
        set_model_data.value=&rec->nvenc.gop_size;

        for(int i=0; i<601; i++)
            set_model_data.values << QString::number(i);

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        for(int i=-1; i<52; i++)
            set_model_data.values << QString::number(i);

        //

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="qpI";
        set_model_data.value=&rec->nvenc.qp_i;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.name="qpP";
        set_model_data.value=&rec->nvenc.qp_p;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.name="qpB";
        set_model_data.value=&rec->nvenc.qp_b;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="aq mode";
        set_model_data.value=&rec->nvenc.aq_mode;
        set_model_data.values << "disabled" << "spatial" << "temporal. not working with constqp?";

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="aq strength";
        set_model_data.value=&rec->nvenc.aq_strength;

        for(int i=0; i<16; i++)
            set_model_data.values << QString::number(i);

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="rc lookahead";
        set_model_data.value=&rec->nvenc.rc_lookahead;

        for(int i=-1; i<33; i++)
            set_model_data.values << QString::number(i);

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::combobox;
        set_model_data.name="surfaces";
        set_model_data.value=&rec->nvenc.surfaces;

        for(int i=-1; i<65; i++)
            set_model_data.values << QString::number(i);

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="no scenecut";
        set_model_data.value=&rec->nvenc.no_scenecut;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="forced idr";
        set_model_data.value=&rec->nvenc.forced_idr;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="b adapt";
        set_model_data.value=&rec->nvenc.b_adapt;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="nonref p";
        set_model_data.value=&rec->nvenc.nonref_p;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="strict gop";
        set_model_data.value=&rec->nvenc.strict_gop;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="weighted pred (disables bframes)";
        set_model_data.value=&rec->nvenc.weighted_pred;

        list_set_model_data->append(set_model_data);

        //

        set_model_data.values.clear();
        set_model_data.values_data.clear();

        set_model_data.type=SettingsModel::Type::checkbox;
        set_model_data.name="bluray compatibility workarounds";
        set_model_data.value=&rec->nvenc.bluray_compat;

        list_set_model_data->append(set_model_data);
    }

    // } nvenc
}

void MainWindow::sourceDeviceAddModel(uint8_t index)
{
    Settings::SourceDevice *settings_device=settings->sourceDevice(index);

    QList <SettingsModel::Data> list_set_model_data;
    SettingsModel::Data set_model_data;

    set_model_data.group=settings_device->group;

    //

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.name="dummy";
    set_model_data.value=&settings->main.dummy;

    list_set_model_data.append(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.priority=SettingsModel::Priority::low;
    set_model_data.name=QString("device #%1").arg(index + 1);

    list_set_model_data.append(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="source type";

    for(int i=0; i<SourceInterface::Type::size; ++i) {
        if(SourceInterface::isImplemented(i)) {
            set_model_data.values << SourceInterface::title(i);
            set_model_data.values_data << i;
        }
    }

    if(settings_device->index>=set_model_data.values.size())
        settings_device->index=0;

    set_model_data.value=&settings_device->index;

    list_set_model_data.append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="start device";
    set_model_data.value=&settings_device->start;

    list_set_model_data.append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name="stop device";
    set_model_data.value=&settings_device->stop;

    list_set_model_data.append(set_model_data);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="rec";

    list_set_model_data.append(set_model_data);


    recAddModel(&list_set_model_data, &settings_device->rec, settings_device->group, nv_tools->availableDevices());

    //

    settings_model->add(list_set_model_data);
}

void MainWindow::sourceDeviceAdd()
{
    if(stream.size()>=Settings::SourceDevice::MaxNum)
        return;

    stream.append(ObjGrp());

    while(settings->source_device.size()<stream.size())
        settings->sourceDeviceAdd();

    ObjGrp *str=&stream.last();

    str->audio_sender=new AudioSender(stream.size() - 1);

    str->encoder=new FFEncoderThread(stream.size() - 1, &enc_base_filename, &enc_start_sync, settings->main.location_videos, QString("capturer %1").arg(VERSION_STRING), this);

    if(stream.size()==1) {
        connect(&str->encoder->frameBuffer()->signaler, SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
        connect(str->encoder, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    }

    connect(str->encoder, SIGNAL(stats(FFEncoder::Stats)), term, SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);

    connect(str->encoder, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);

    if(!settings->main.headless) {
        connect(str->encoder, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);
    }

    if(stream.size()>1) {
        connect(stream.first().encoder, SIGNAL(restartOut()), str->encoder, SIGNAL(restartIn()), Qt::QueuedConnection);
    }

    sourceDeviceAddModel(stream.size() - 1);

    updateEncList();

    term->reloadDevices();
}

void MainWindow::sourceDeviceRemove()
{
    if(stream.size()<=1)
        return;

    ObjGrp str=stream.takeLast();

    if(str.source_device) {
        QMetaObject::invokeMethod(dynamic_cast<QObject*>(str.source_device), "deviceStop", Qt::QueuedConnection);
        QMetaObject::invokeMethod(dynamic_cast<QObject*>(str.source_device), "deleteLater", Qt::QueuedConnection);
    }

    str.audio_sender->deleteLater();
    str.encoder->deleteLater();

    settings_model->removeGroup(settings->source_device.last().group);
    settings_model->removeGroup(settings->source_device.last().group_settings);

    settings->sourceDeviceRemove();

    term->reloadDevices();
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

void MainWindow::reloadFFDevices()
{
    FFSource::updateDevList();

    Settings::SourceDevice *settings_device;
    SettingsModel::Data *set_model_data;

    for(int i=0; i<stream.size(); ++i) {
        settings_device=settings->sourceDevice(i);

        if(!settings_device)
            continue;

        if(settings_device->index!=SourceInterface::Type::ffmpeg)
            continue;

        //

        set_model_data=settings_model->data_p(&settings_device->ff_device.index_audio);

        set_model_data->values.clear();
        set_model_data->values_data.clear();

        QStringList ff_devices=FFSource::availableAudioInput();

        set_model_data->values << "disabled";
        set_model_data->values_data << -1;

        for(int i=0; i<ff_devices.size(); ++i) {
            set_model_data->values << ff_devices[i];
            set_model_data->values_data << i;
        }

        settings_device->ff_device.index_audio=FFSource::indexAudioInput(settings_device->ff_device.name_audio);

        if(settings_device->ff_device.index_audio<0)
            settings_device->ff_device.index_audio=0;

        else
            settings_device->ff_device.index_audio++;

        //

        set_model_data=settings_model->data_p(&settings_device->ff_device.index_video);

        set_model_data->values.clear();
        set_model_data->values_data.clear();

        ff_devices=FFSource::availableVideoInput();

        set_model_data->values << "disabled";
        set_model_data->values_data << -1;

        for(int i=0; i<ff_devices.size(); ++i) {
            set_model_data->values << ff_devices[i];
            set_model_data->values_data << i;
        }

        settings_device->ff_device.index_video=FFSource::indexVideoInput(settings_device->ff_device.name_video);

        if(settings_device->ff_device.index_video<0)
            settings_device->ff_device.index_video=0;

        else
            settings_device->ff_device.index_video++;

        //

        QMetaObject::invokeMethod(dynamic_cast<QObject*>(this), "settingsModelDataChanged", Qt::QueuedConnection,
                                  Q_ARG(int, settings_model->data_p_index(&settings_device->ff_device.index_audio)), Q_ARG(int, 0), Q_ARG(bool, false));

        QMetaObject::invokeMethod(dynamic_cast<QObject*>(this), "settingsModelDataChanged", Qt::QueuedConnection,
                                  Q_ARG(int, settings_model->data_p_index(&settings_device->ff_device.index_video)), Q_ARG(int, 0), Q_ARG(bool, false));
    }
}

void MainWindow::checkFreeSpace()
{
    QStorageInfo info(settings->main.location_videos);

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

    for(int i=0; i<settings->source_device.size(); ++i) {
        if(data->value==&settings->source_device[i].index) {
            setDevice(i, (SourceInterface::Type::T)settings_model->data_p(&settings->source_device[i].index)->values_data[settings->source_device[i].index].toInt());

            if(i==0)
                deviceStart(0);
        }
    }

    for(int i_dev=0; i_dev<stream.size(); ++i_dev) {
        SourceInterface **device;
        Settings::SourceDevice *settings_device;

        device=&stream[i_dev].source_device;
        settings_device=settings->sourceDevice(i_dev);

        if((*device) && (*device)->type()==SourceInterface::Type::ffmpeg) {
            FFSource *ff_device=static_cast<FFSource*>(*device);

            if(data->value==&settings_device->ff_device.index_video) {
                QStringList list_values;
                QVariantList list_values_data;

                qml=false;

                settings_device->ff_device.name_video=
                        data->values.value(*data->value);

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
                settings_device->ff_device.name_audio=
                        data->values.value(*data->value);

                ff_device->setAudioDevice(settings_model->valueData(&settings_device->ff_device.index_audio, -1).toInt());
            }


            if(data->value==&settings_device->ff_device.framesize) {
                qml=false;

                const QSize size=data->values_data.value(settings_device->ff_device.framesize).toSize();

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

            if(data->value==&settings_device->ff_device.reload_devices) {
                reloadFFDevices();
            }
        }

        if(data->value==&settings_device->rec.video_encoder) {
            qml=false;

            // pix_fmt

            if(settings_device->rec.video_encoder>=data->values_data.size())
                settings_device->rec.video_encoder=data->values_data.size() - 1;

            const int index_encoder=
                data->values_data[settings_device->rec.video_encoder].toInt();

            QList <PixelFormat> fmts;
            PixelFormat pf;

            foreach(const QString &fmt_str, settings->main.supported_enc[FFEncoder::VideoEncoder::toString(index_encoder)].toStringList()) {
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
                if(settings_model->data_p(i)->value==&settings_device->rec.pixel_format_current) {
                    settings_model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                    settings_model->setData(i, SettingsModel::Role::values, list_values, false, true);
                    settings_model->setData(i, SettingsModel::Role::value, settings_device->rec.pixel_format.value(QString::number(settings_device->rec.video_encoder), 0), false, true);

                    break;
                }
            }

            // preset
            list_values.clear();
            list_values_data.clear();

            const QStringList presets=FFEncoder::compatiblePresets((FFEncoder::VideoEncoder::T)data->values_data.value(settings_device->rec.video_encoder, 0).toInt());

            foreach(const QString &preset, presets) {
                list_values << preset;
                list_values_data << FFEncoder::presetVisualNameToParamName(preset);
            }

            for(int i=0; i<settings_model->rowCount(); ++i) {
                if(settings_model->data_p(i)->value==&settings_device->rec.preset_current) {
                    settings_model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                    settings_model->setData(i, SettingsModel::Role::values, list_values, false, true);
                    settings_model->setData(i, SettingsModel::Role::value, settings_device->rec.preset.value(QString::number(settings_device->rec.video_encoder), 0), false, true);

                    break;
                }
            }
        }


        if(data->value==&settings_device->rec.pixel_format_current)
            if(role==SettingsModel::Role::value)
                settings_device->rec.pixel_format[QString::number(settings_device->rec.video_encoder)]=settings_device->rec.pixel_format_current;

        if(data->value==&settings_device->rec.preset_current)
            if(role==SettingsModel::Role::value)
                settings_device->rec.preset[QString::number(settings_device->rec.video_encoder)]=settings_device->rec.preset_current;

        if(data->value==&settings_device->start)
            deviceStart(i_dev);

        if(data->value==&settings_device->stop)
            deviceStop(i_dev);

        if(data->value==&settings_device->rec.check_encoders)
            checkEncoders();
    }

    if(data->value==&settings->streaming.rec.video_encoder) {
        qml=false;

        // pix_fmt

        if(settings->streaming.rec.video_encoder>=data->values_data.size())
            settings->streaming.rec.video_encoder=data->values_data.size() - 1;

        const int index_encoder=
            data->values_data[settings->streaming.rec.video_encoder].toInt();

        QList <PixelFormat> fmts;
        PixelFormat pf;

        foreach(const QString &fmt_str, settings->main.supported_enc[FFEncoder::VideoEncoder::toString(index_encoder)].toStringList()) {
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
            if(settings_model->data_p(i)->value==&settings->streaming.rec.pixel_format_current) {
                settings_model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                settings_model->setData(i, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(i, SettingsModel::Role::value, settings->streaming.rec.pixel_format.value(QString::number(settings->streaming.rec.video_encoder), 0), false, true);

                break;
            }
        }

        // preset
        list_values.clear();
        list_values_data.clear();

        const QStringList presets=FFEncoder::compatiblePresets((FFEncoder::VideoEncoder::T)data->values_data.value(settings->streaming.rec.video_encoder, 0).toInt());

        foreach(const QString &preset, presets) {
            list_values << preset;
            list_values_data << FFEncoder::presetVisualNameToParamName(preset);
        }

        for(int i=0; i<settings_model->rowCount(); ++i) {
            if(settings_model->data_p(i)->value==&settings->streaming.rec.preset_current) {
                settings_model->setData(i, SettingsModel::Role::values_data, list_values_data, false, true);
                settings_model->setData(i, SettingsModel::Role::values, list_values, false, true);
                settings_model->setData(i, SettingsModel::Role::value, settings->streaming.rec.preset.value(QString::number(settings->streaming.rec.video_encoder), 0), false, true);

                break;
            }
        }
    }


    if(data->value==&settings->streaming.url_index) {
        if(settings->streaming.url_index<1) {
            settings_model->removeGroup("streaming");

        } else {
            if(settings_model->countGroup("streaming")<1) {
                QList <SettingsModel::Data> list_set_model_data;

                recAddModel(&list_set_model_data, &settings->streaming.rec, "streaming", cuda_devices);

                settings_model->insert(&settings->streaming.url_index, list_set_model_data);

                updateEncList();
            }
        }

        QMetaObject::invokeMethod(dynamic_cast<QObject*>(term), "reloadDevices", Qt::QueuedConnection);
    }

    if(data->value==&settings->streaming.rec.pixel_format_current)
        if(role==SettingsModel::Role::value)
            settings->streaming.rec.pixel_format[QString::number(settings->streaming.rec.video_encoder)]=settings->streaming.rec.pixel_format_current;

    if(data->value==&settings->streaming.rec.preset_current)
        if(role==SettingsModel::Role::value)
            settings->streaming.rec.preset[QString::number(settings->streaming.rec.video_encoder)]=settings->streaming.rec.preset_current;


    if(data->value==&settings->main.source_device_add)
        sourceDeviceAdd();

    if(data->value==&settings->main.source_device_remove)
        sourceDeviceRemove();

    if(!qml) {
        settings_model->updateQml();

        if(messenger)
            messenger->focusReset();
    }
}

void MainWindow::deviceStart(uint8_t index)
{
    if(stream.size()<=index)
        return;

    if(!stream[index].source_device)
        return;

    SourceInterface **device;
    Settings::SourceDevice *settings_device;

    device=&stream[index].source_device;

    settings_device=settings->sourceDevice(index);

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
        if(settings_model->valueData(&settings_device->ff_device.index_video).toInt()>=0
                || settings_model->valueData(&settings_device->ff_device.index_audio).toInt()>=0) {
            FFSource::Device *dev=new FFSource::Device();
            FFSource *ff_device=static_cast<FFSource*>(*device);

            dev->high_depth_audio=settings_device->ff_device.high_depth_audio;
            dev->framerate=settings_model->valueData(&settings_device->ff_device.framerate, QVariant::fromValue<AVRational>({ 30000, 1000 })).value<AVRational>();
            dev->pixel_format=settings_model->valueData(&settings_device->ff_device.pixel_format, 0).toInt();
            dev->size=settings_model->valueData(&settings_device->ff_device.framesize, QSize(640, 480)).toSize();

            ff_device->setAudioDevice(settings_model->valueData(&settings_device->ff_device.index_audio, -1).toInt());
            ff_device->setVideoDevice(settings_model->valueData(&settings_device->ff_device.index_video, -1).toInt());

            (*device)->setDevice(dev);
        }
    }

    if((*device)->type()==SourceInterface::Type::magewell) {
        MagewellDevice::Devices list=MagewellDevice::availableDevices();

        if(settings_device->magewell.index>=list.size()) {
            qDebug() << "magewell.index out of range";
            return;
        }

        MagewellDevice::Device *dev=new MagewellDevice::Device();
        (*dev)=list[settings_device->magewell.index];

        dev->pixel_format=settings_model->valueData(&settings_device->magewell.pixel_format, PixelFormat::nv12).toInt();;
        dev->framesize=settings_model->valueData(&settings_device->magewell.framesize, QSize()).value<QSize>();
        dev->color_format_in=(MagewellDevice::Device::ColorFormat::T)settings_device->magewell.color_format_in;
        dev->color_format_out=(MagewellDevice::Device::ColorFormat::T)settings_device->magewell.color_format_out;
        dev->quantization_range_in=(MagewellDevice::Device::QuantizationRange::T)settings_device->magewell.quantization_range_in;
        dev->quantization_range_out=(MagewellDevice::Device::QuantizationRange::T)settings_device->magewell.quantization_range_out;
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

void MainWindow::deviceStop(uint8_t index)
{
    if(stream.size()<=index)
        return;

    if(!stream[index].source_device)
        return;

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(stream[index].source_device), "deviceStop", Qt::QueuedConnection);
}

bool MainWindow::recInProgress()
{
    if(encoder_streaming->isWorking())
        return true;

    for(int i=0; i<stream.size(); ++i) {
        if(stream[i].encoder->isWorking())
            return true;
    }

    return false;
}

void MainWindow::startStopRecording()
{
    if(recInProgress()) {
        for(int i=0; i<stream.size(); ++i)
            stream[i].encoder->stopCoder();

        encoder_streaming->stopCoder();

        return;
    }

    if(!settings->main.headless && ff_dec->currentState()!=FFDecoderThread::ST_STOPPED)
        return;

    enc_base_filename.reset();
    enc_start_sync.clear();

    FFEncoderThread *enc=nullptr;
    SourceInterface *dev=nullptr;

    uint32_t active_src_devices=0;

    for(int i=0; i<stream.size(); ++i) {
        enc=stream[i].encoder;
        dev=stream[i].source_device;

        if(!enc || !dev)
            continue;

        if(!dev->gotSignal())
            continue;

        active_src_devices|=1 << i;

        FFEncoder::Config cfg;

        AVRational framerate=dev->currentFramerate();

        cfg.framerate=FFEncoder::calcFps(framerate.num, framerate.den, settings->source_device[i].rec.half_fps);

        if(cfg.framerate==FFEncoder::Framerate::unknown)
            cfg.framerate_force=framerate;

        cfg.audio_encoder=(FFEncoder::AudioEncoder::T)settings_model->valueData(&settings->source_device[i].rec.audio_encoder).toInt();
        cfg.audio_bitrate=settings_model->valueData(&settings->source_device[i].rec.audio_bitrate).toInt();
        cfg.audio_downmix_to_stereo=settings->source_device[i].rec.audio_downmix_to_stereo;
        cfg.audio_sample_size=dev->currentAudioSampleSize();
        cfg.audio_channels_size=dev->currentAudioChannels();
        cfg.frame_resolution_src=dev->currentFramesize();
        cfg.pixel_format_dst=settings_model->valueData(&settings->source_device[i].rec.pixel_format_current).toInt();
        cfg.preset=settings_model->valueData(&settings->source_device[i].rec.preset_current).toString();
        cfg.video_encoder=(FFEncoder::VideoEncoder::T)settings_model->valueData(&settings->source_device[i].rec.video_encoder).toInt();
        cfg.video_bitrate=settings_model->valueData(&settings->source_device[i].rec.video_bitrate).toInt();
        cfg.crf=settings->source_device[i].rec.crf;
        cfg.pixel_format_src=dev->currentPixelFormat();
        cfg.direct_stream_copy=settings->source_device[i].rec.direct_stream_copy;
        cfg.fill_dropped_frames=settings->source_device[i].rec.fill_dropped_frames;
        cfg.downscale=settings->source_device[i].rec.downscale;
        cfg.scale_filter=settings->source_device[i].rec.scale_filter;
        cfg.color_primaries=settings_model->valueData(&settings->source_device[i].rec.color_primaries).toInt();
        cfg.color_space=settings_model->valueData(&settings->source_device[i].rec.color_space).toInt();
        cfg.color_transfer_characteristic=settings_model->valueData(&settings->source_device[i].rec.color_transfer_characteristic).toInt();
        cfg.color_range=swsColorRange::toff(settings->source_device[i].rec.color_range);
        cfg.sws_color_space_src=swsColorSpace::toff(settings->source_device[i].rec.sws_color_space_src);
        cfg.sws_color_space_dst=swsColorSpace::toff(settings->source_device[i].rec.sws_color_space_dst);
        cfg.sws_color_range_src=swsColorRange::toff(settings->source_device[i].rec.sws_color_range_src);
        cfg.sws_color_range_dst=swsColorRange::toff(settings->source_device[i].rec.sws_color_range_dst);
        cfg.nvenc=settings->source_device[i].rec.nvenc;
        cfg.aspect_ratio_4_3=settings->source_device[i].rec.aspect_ratio_4_3;
        cfg.input_type_flags=dev->typeFlags();

        enc->setConfig(cfg);
        enc_start_sync.add(i);
    }

    if(encoder_streaming->isWorking()) {
        qInfo() << "encoder_streaming->stopCoder";
        encoder_streaming->stopCoder();
        return;
    }

    if(!settings_model->valueData(&settings->streaming.url_index).toString().isEmpty()) {
        dev=stream[0].source_device;

        if(!dev)
            return;

        if(!dev->gotSignal())
            return;

        FFEncoder::Config cfg;

        AVRational framerate=dev->currentFramerate();

        cfg.active_src_devices=active_src_devices;

        cfg.framerate=FFEncoder::calcFps(framerate.num, framerate.den, settings->streaming.rec.half_fps);

        if(cfg.framerate==FFEncoder::Framerate::unknown)
            cfg.framerate_force=framerate;

        cfg.audio_encoder=(FFEncoder::AudioEncoder::T)settings_model->valueData(&settings->streaming.rec.audio_encoder).toInt();
        cfg.audio_bitrate=settings_model->valueData(&settings->streaming.rec.audio_bitrate).toInt();
        cfg.audio_downmix_to_stereo=settings->streaming.rec.audio_downmix_to_stereo;
        cfg.audio_sample_size=dev->currentAudioSampleSize();
        cfg.audio_channels_size=dev->currentAudioChannels();
        cfg.frame_resolution_src=dev->currentFramesize();
        cfg.pixel_format_dst=settings_model->valueData(&settings->streaming.rec.pixel_format_current).toInt();
        cfg.preset=settings_model->valueData(&settings->streaming.rec.preset_current).toString();
        cfg.video_encoder=(FFEncoder::VideoEncoder::T)settings_model->valueData(&settings->streaming.rec.video_encoder).toInt();
        cfg.video_bitrate=settings_model->valueData(&settings->streaming.rec.video_bitrate).toInt();
        cfg.crf=settings->streaming.rec.crf;
        cfg.pixel_format_src=dev->currentPixelFormat();
        cfg.direct_stream_copy=settings->streaming.rec.direct_stream_copy;
        cfg.fill_dropped_frames=settings->streaming.rec.fill_dropped_frames;
        cfg.downscale=settings->streaming.rec.downscale;
        cfg.scale_filter=settings->streaming.rec.scale_filter;
        cfg.color_primaries=settings_model->valueData(&settings->streaming.rec.color_primaries).toInt();
        cfg.color_space=settings_model->valueData(&settings->streaming.rec.color_space).toInt();
        cfg.color_transfer_characteristic=settings_model->valueData(&settings->streaming.rec.color_transfer_characteristic).toInt();
        cfg.color_range=swsColorRange::toff(settings->streaming.rec.color_range);
        cfg.sws_color_space_src=swsColorSpace::toff(settings->streaming.rec.sws_color_space_src);
        cfg.sws_color_space_dst=swsColorSpace::toff(settings->streaming.rec.sws_color_space_dst);
        cfg.sws_color_range_src=swsColorRange::toff(settings->streaming.rec.sws_color_range_src);
        cfg.sws_color_range_dst=swsColorRange::toff(settings->streaming.rec.sws_color_range_dst);
        cfg.nvenc=settings->streaming.rec.nvenc;
        cfg.aspect_ratio_4_3=settings->streaming.rec.aspect_ratio_4_3;
        cfg.input_type_flags=dev->typeFlags();

        enc_streaming_url=settings_model->valueData(&settings->streaming.url_index).toString();

        encoder_streaming->setConfig(cfg);
    }
}

void MainWindow::updateEncList()
{
    if(settings->main.supported_enc.isEmpty()) {
        qCritical() << "supported_enc.isEmpty";
        exit(1);
        return;
    }

    SettingsModel::Data *set_model_data=nullptr;

    for(int i_dev=0; i_dev<settings->source_device.size(); ++i_dev) {
        set_model_data=
            settings_model->data_p(&settings->source_device[i_dev].rec.video_encoder);

        if(!set_model_data) {
            qCritical() << "set_model_data null pointer" << i_dev;
            return;
        }

        set_model_data->values.clear();
        set_model_data->values_data.clear();

        for(int i_enc=0; i_enc<settings->main.supported_enc.size(); ++i_enc) {
            set_model_data->values << settings->main.supported_enc.keys()[i_enc];
            set_model_data->values_data << (quint64)FFEncoder::VideoEncoder::fromString(settings->main.supported_enc.keys()[i_enc]);
        }

        settingsModelDataChanged(settings_model->data_p_index(&settings->source_device[i_dev].rec.video_encoder), 0, false);

        QMetaObject::invokeMethod(dynamic_cast<QObject*>(this), "settingsModelDataChanged", Qt::QueuedConnection,
                                  Q_ARG(int, settings_model->data_p_index(&settings->source_device[i_dev].rec.video_encoder)), Q_ARG(int, 0), Q_ARG(bool, false));
    }

    //

    set_model_data=
        settings_model->data_p(&settings->streaming.rec.video_encoder);

    if(!set_model_data) {
        qCritical() << "set_model_data null pointer streaming";
        return;
    }

    set_model_data->values.clear();
    set_model_data->values_data.clear();

    for(int i_enc=0; i_enc<settings->main.supported_enc.size(); ++i_enc) {
        set_model_data->values << settings->main.supported_enc.keys()[i_enc];
        set_model_data->values_data << (quint64)FFEncoder::VideoEncoder::fromString(settings->main.supported_enc.keys()[i_enc]);
    }

    settingsModelDataChanged(settings_model->data_p_index(&settings->streaming.rec.video_encoder), 0, false);

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(this), "settingsModelDataChanged", Qt::QueuedConnection,
                              Q_ARG(int, settings_model->data_p_index(&settings->streaming.rec.video_encoder)), Q_ARG(int, 0), Q_ARG(bool, false));

    QMetaObject::invokeMethod(dynamic_cast<QObject*>(term), "updateSettingsForm", Qt::QueuedConnection);
}

void MainWindow::encoderBufferOverload()
{
    // ff_enc_primary->stopCoder();

    // emit messenger->errorString("encoder buffer overload, recording stopped");
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

    if(settings->main.headless_curse) {
        term->update();
    }

    if(settings->main.headless) {
        return;
    }

    messenger->setRecStarted(state);

    if(state && stream.first().source_device && stream.first().source_device->currentPixelFormat().isCompressed() && settings->source_device.first().rec.direct_stream_copy) {
        messenger->updateRecStats("awaiting intra frame");
    }
}

void MainWindow::playerStateChanged(int state)
{
    SourceInterface *device_primary=nullptr;

    if(!stream.isEmpty())
        device_primary=stream.first().source_device;

    if(state!=FFDecoderThread::ST_STOPPED) {
        emit signalLost(false);

    } else {
        emit messenger->showPlayerState(false);

        if(device_primary && device_primary->gotSignal())
            emit signalLost(false);

        else
            emit signalLost(true);
    }

    if(device_primary) {
        if(state==FFDecoderThread::ST_STOPPED) {
            QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceResume", Qt::QueuedConnection);

        } else {
            QMetaObject::invokeMethod(dynamic_cast<QObject*>(device_primary), "deviceHold", Qt::QueuedConnection);
        }
    }
}

void MainWindow::updateStats(FFEncoder::Stats s)
{
    const QPair <int, int> buffer_size=stream.first().encoder->frameBuffer()->size();

    QVariantMap map_bitrate_video;

    for(int i=0; i<s.bitrate_video.size(); ++i)
        map_bitrate_video.insert(QString::number(s.bitrate_video.keys()[i]), s.bitrate_video.values()[i]);

    emit recStats(NRecStats(s.time, s.avg_bitrate_video + s.avg_bitrate_audio, s.avg_bitrate_video, map_bitrate_video,
                            s.streams_size, s.dropped_frames_counter, buffer_size.second, buffer_size.first));

    if(settings->main.headless)
        return;

    messenger->updateRecStats(s.time.toString(QStringLiteral("HH:mm:ss")),
                              QString(QLatin1String("%1 Mbits/s (%2 MB/s)")).arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2),
                              QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString(QLatin1String("%1 bytes")).arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString(QLatin1String("buf state: %1/%2")).arg(buffer_size.first).arg(buffer_size.second),
                              QString(QLatin1String("frames dropped: %1")).arg(s.dropped_frames_counter));
}
