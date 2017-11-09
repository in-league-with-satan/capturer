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
#include "qcam.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mb_rec_stopped(nullptr)
{
    QDir dir(qApp->applicationDirPath() + "/videos");

    if(!dir.exists())
        dir.mkdir(dir.absolutePath());

    //

    decklink_thread=new DeckLinkCapture(this);

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            SLOT(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);
    connect(decklink_thread, SIGNAL(frameSkipped()), SLOT(frameSkipped()), Qt::QueuedConnection);

    //

    cam_device=new QCam(this);

    cam_device->setDevice(10);

    //

    qmlRegisterType<SettingsModel>("FuckTheSystem", 0, 0, "SettingsModel");
    qmlRegisterType<FileSystemModel>("FuckTheSystem", 0, 0, "FileSystemModel");
    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");
    qmlRegisterType<QuickVideoSource>("FuckTheSystem", 0, 0, "QuickVideoSource");

    messenger=new QmlMessenger();

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            messenger, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);

    // fix me! connect(decklink_thread, SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

    overlay_view=new OverlayView(this);

    overlay_view->setMessenger(messenger);

    overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));

    overlay_view->addImageProvider("fs_image_provider", (QQmlImageProviderBase*)messenger->fileSystemModel()->imageProvider());

    decklink_thread->subscribe(messenger->videoSourceMain()->frameBuffer());

    cam_device->subscribe(messenger->videoSourceMain()->frameBuffer());



    //

    audio_output=newAudioOutput(this);

    decklink_thread->subscribe(audio_output->frameBuffer());

    //

    ff_enc=new FFEncoderThread(this);

    decklink_thread->subscribe(ff_enc->frameBuffer());

    cam_device->subscribe(ff_enc->frameBuffer());

    connect(ff_enc->frameBuffer().get(), SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

    connect(decklink_thread, SIGNAL(errorString(QString)), messenger, SIGNAL(errorString(QString)), Qt::QueuedConnection);

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

    http_server=new HttpServer(settings->http_server.enabled ? settings->http_server.port : 0, this);
    connect(http_server, SIGNAL(keyPressed(int)), SLOT(keyPressed(int)));
    connect(http_server, SIGNAL(playerSeek(qint64)), ff_dec, SLOT(seek(qint64)));
    connect(ff_dec, SIGNAL(durationChanged(qint64)), http_server, SLOT(setPlayerDuration(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(positionChanged(qint64)), http_server, SLOT(setPlayerPosition(qint64)), Qt::QueuedConnection);
    connect(messenger, SIGNAL(freeSpace(qint64)), http_server, SLOT(setFreeSpace(qint64)), Qt::QueuedConnection);

    //

    SettingsModel::Data set_model_data;


    //

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="qcamera";
    messenger->settingsModel()->add(set_model_data);

    //

    QStringList cams=QCam::availableCameras();

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device";
    set_model_data.name="name";


    if(cams.isEmpty()) {
        set_model_data.values << "null";

    } else {
        foreach(QString cam_name, cams) {
            qInfo() << cams.size() << cam_name;
            set_model_data.values << cam_name;
        }
    }

    set_model_data.value=&settings->device_cam.index;

    int model_index_cam_name=messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device";
    set_model_data.name="resolution";

    // set_model_data.values << cam_name;

    set_model_data.value=&settings->device_cam.resolution;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();


    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device";
    set_model_data.name="framerate";

    // set_model_data.values << cam_name;

    set_model_data.value=&settings->device_cam.framerate;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();


    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device";
    set_model_data.name="pixel format";

    // set_model_data.values << cam_name;

    set_model_data.value=&settings->device_cam.pixel_format;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();

    if(!cams.isEmpty())
        settingsModelDataChanged(model_index_cam_name, 0, false);

    //

    set_model_data.values.clear();
    set_model_data.values_data.clear();


    set_model_data.type=SettingsModel::Type::button;
    set_model_data.name.clear();
    set_model_data.values << "restart device";
    set_model_data.values_data << 0;

    set_model_data.value=&settings->device_cam.restart;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.value=&settings->main.dummy;
    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="decklink";

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="device";
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
    set_model_data.name="rgb depth";
    set_model_data.values << "8 bit" << "10 bit";
    set_model_data.values_data << 0 << 1;

    set_model_data.value=&settings->device_decklink.rgb_10bit;

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
    set_model_data.name.clear();
    set_model_data.values << "restart device";
    set_model_data.values_data << 0;

    set_model_data.value=&settings->device_decklink.restart;

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::divider;
    set_model_data.value=&settings->main.dummy;
    messenger->settingsModel()->add(set_model_data);


    //

    set_model_data.type=SettingsModel::Type::title;
    set_model_data.value=&settings->main.dummy;
    set_model_data.name="rec";

    messenger->settingsModel()->add(set_model_data);

    //


    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="video encoder";

    if(FFEncoder::isLib_x264_10bit())
        set_model_data.values_data << FFEncoder::VideoEncoder::libx264_10bit << FFEncoder::VideoEncoder::nvenc_h264 << FFEncoder::VideoEncoder::nvenc_hevc;

    else
        set_model_data.values_data << FFEncoder::VideoEncoder::libx264 << FFEncoder::VideoEncoder::libx264rgb
                                   << FFEncoder::VideoEncoder::nvenc_h264 << FFEncoder::VideoEncoder::nvenc_hevc
                                   << FFEncoder::VideoEncoder::qsv_h264 << FFEncoder::VideoEncoder::ffvhuff;


    foreach(QVariant v, set_model_data.values_data)
        set_model_data.values << FFEncoder::VideoEncoder::toString(v.toInt());


    set_model_data.value=&settings->rec.encoder;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="preset";

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    QStringList presets=FFEncoder::compatiblePresets((FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder)->values_data[settings->rec.encoder].toInt());

    foreach(const QString &preset, presets) {
        set_model_data.values << preset;
        set_model_data.values_data << FFEncoder::presetVisualNameToParamName(preset);
    }

    set_model_data.value=&settings->rec.preset_current;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="pixel format";
    set_model_data.value=&settings->rec.pixel_format_current;

    QList <FFEncoder::PixelFormat::T> fmts=
            FFEncoder::PixelFormat::compatiblePixelFormats((FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder)->values_data[settings->rec.encoder].toInt());

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    for(int i=0; i<fmts.size(); ++i) {
        set_model_data.values << FFEncoder::PixelFormat::toString(fmts[i]);
        set_model_data.values_data << fmts[i];
    }

    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

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

    //

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


    messenger->signalLost(false); // fix me
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

    return QObject::eventFilter(object, event);
}

void MainWindow::closeEvent(QCloseEvent *)
{
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

    case KeyCodeC::SmoothTransform:
        settings->main.smooth_transform=!settings->main.smooth_transform;
        break;

    case KeyCodeC::FullScreen:
        if(isFullScreen())
            showNormal();

        else
            showFullScreen();

        break;

    case KeyCodeC::Exit:
        QApplication::exit(0);
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

void MainWindow::settingsModelDataChanged(int index, int role, bool qml)
{
    Q_UNUSED(qml)

    SettingsModel *model=messenger->settingsModel();

    SettingsModel::Data *data=model->data_p(index);


    if(data->value==&settings->device_cam.index) {
        cam_device->setDevice(settings->device_cam.index);

        QStringList list_values;
        QVariantList list_values_data;

        foreach(QSize val, cam_device->supportedResolutions()) {
            qInfo() << "res" << val;
            list_values << QString("%1x%2").arg(val.width()).arg(val.height());
            list_values_data << val;
        }

        model->setData(&settings->device_cam.resolution, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.resolution, SettingsModel::Role::values_data, list_values_data);

        if(!list_values_data.isEmpty()) {
            QSize size=list_values_data.value(settings->device_cam.pixel_format).toSize();

            list_values.clear();
            list_values_data.clear();

            foreach(AVRational val, cam_device->frameRateRanges(size)) {
                list_values << QString::number(QCam::rationalToFramerate(val));
                list_values_data << QSize(val.num, val.den);
            }

            model->setData(&settings->device_cam.framerate, SettingsModel::Role::values, list_values);
            model->setData(&settings->device_cam.framerate, SettingsModel::Role::values_data, list_values_data);


            //

            list_values.clear();
            list_values_data.clear();

            foreach(QVideoFrame::PixelFormat val, cam_device->pixelFormats(size)) {
                list_values << QCam::pixelFormatToString(val);
                list_values_data << val;
            }

            model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values, list_values);
            model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values_data, list_values_data);
        }
    }

    if(data->value==&settings->device_cam.resolution) {
        QSize size=data->values_data.value(settings->device_cam.resolution).toSize();

        QStringList list_values;
        QVariantList list_values_data;

        foreach(AVRational val, cam_device->frameRateRanges(size)) {
            list_values << QString::number(QCam::rationalToFramerate(val));
            list_values_data << QSize(val.num, val.den);
        }

        model->setData(&settings->device_cam.framerate, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.framerate, SettingsModel::Role::values_data, list_values_data);


        //

        list_values.clear();
        list_values_data.clear();

        foreach(QVideoFrame::PixelFormat val, cam_device->pixelFormats(size)) {
            list_values << QCam::pixelFormatToString(val);
            list_values_data << val;
        }

        model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values, list_values);
        model->setData(&settings->device_cam.pixel_format, SettingsModel::Role::values_data, list_values_data);
    }

    if(data->value==&settings->rec.encoder) {
        QList <FFEncoder::PixelFormat::T> fmts=
                FFEncoder::PixelFormat::compatiblePixelFormats((FFEncoder::VideoEncoder::T)data->values_data.value(settings->rec.encoder, 0).toInt());

        QStringList list_values;
        QVariantList list_values_data;

        for(int i=0; i<fmts.size(); ++i) {
            list_values << FFEncoder::PixelFormat::toString(fmts[i]);
            list_values_data << fmts[i];
        }

        for(int i=0; i<model->rowCount(); ++i) {
            if(model->data_p(i)->value==&settings->rec.pixel_format_current) {
                model->setData(i, SettingsModel::Role::values_data, list_values_data);
                model->setData(i, SettingsModel::Role::values, list_values);
                model->setData(i, SettingsModel::Role::value, settings->rec.pixel_format.value(QString::number(settings->rec.encoder), 0));

                break;
            }
        }

        // preset
        list_values.clear();
        list_values_data.clear();

        QStringList presets=FFEncoder::compatiblePresets((FFEncoder::VideoEncoder::T)data->values_data.value(settings->rec.encoder, 0).toInt());

        foreach(const QString &preset, presets) {
            list_values << preset;
            list_values_data << FFEncoder::presetVisualNameToParamName(preset);
        }

        for(int i=0; i<model->rowCount(); ++i) {
            if(model->data_p(i)->value==&settings->rec.preset_current) {
                model->setData(i, SettingsModel::Role::values_data, list_values_data);
                model->setData(i, SettingsModel::Role::values, list_values);
                model->setData(i, SettingsModel::Role::value, settings->rec.preset.value(QString::number(settings->rec.encoder), 0));

                break;
            }
        }
    }


    if(data->value==&settings->rec.pixel_format_current)
        if(role==SettingsModel::Role::value)
            settings->rec.pixel_format[QString::number(settings->rec.encoder)]=settings->rec.pixel_format_current;


    if(data->value==&settings->rec.preset_current)
        if(role==SettingsModel::Role::value)
            settings->rec.preset[QString::number(settings->rec.encoder)]=settings->rec.preset_current;


    if(data->value==&settings->device_decklink.restart) {
        captureRestart();
    }

    if(data->value==&settings->device_cam.restart) {
        SettingsModel::Data *model_data_resolution=messenger->settingsModel()->data_p(&settings->device_cam.resolution);
        SettingsModel::Data *model_data_framerate=messenger->settingsModel()->data_p(&settings->device_cam.framerate);
        SettingsModel::Data *model_data_pixel_format=messenger->settingsModel()->data_p(&settings->device_cam.pixel_format);

        QSize fr=model_data_framerate->values_data.value(settings->device_cam.framerate).toSize();

        cam_device->start(model_data_resolution->values_data.value(settings->device_cam.resolution).toSize()
                          , (QVideoFrame::PixelFormat)model_data_pixel_format->values_data.value(settings->device_cam.pixel_format).toULongLong()
                          , { fr.height(), fr.width() });
    }
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

    if(model_data_device->values_data.isEmpty()) {
        qCritical() << "values_data is empty";
        return;
    }


    decklink_thread->setup(model_data_device->values_data[settings->device_decklink.index].value<DeckLinkDevice>(),
            DeckLinkFormat(),
            DeckLinkPixelFormat(),
            8,
            model_data_audio->values_data[settings->device_decklink.audio_sample_size].toInt(),
            settings->device_decklink.rgb_10bit
            );

    decklink_thread->setHalfFps(settings->device_decklink.half_fps);

    QMetaObject::invokeMethod(decklink_thread, "captureStart", Qt::QueuedConnection);
}

void MainWindow::captureStop()
{
    QMetaObject::invokeMethod(decklink_thread, "captureStop", Qt::QueuedConnection);
}

void MainWindow::startStopRecording()
{
    if(ff_enc->isWorking()) {
        ff_enc->stopCoder();

    } else {
        // fix me! if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED || !decklink_thread->gotSignal())
        //     return;

        SettingsModel::Data *model_data_resolution=messenger->settingsModel()->data_p(&settings->device_cam.resolution);
        SettingsModel::Data *model_data_framerate=messenger->settingsModel()->data_p(&settings->device_cam.framerate);

        QSize resolution=model_data_resolution->values_data.value(*model_data_resolution->value).toSize();
        QSize framerate=model_data_framerate->values_data.value(*model_data_framerate->value).toSize();


        FFEncoder::Config cfg;

        cfg.framerate=FFEncoder::calcFps(current_frame_duration, current_frame_scale, settings->rec.half_fps);

        cfg.framerate=FFEncoder::calcFps(framerate.width(), framerate.height(), false);
        cfg.frame_resolution_src=current_frame_size;
        cfg.frame_resolution_src=resolution;

        cfg.pixel_format=(AVPixelFormat)messenger->settingsModel()->data_p(&settings->rec.pixel_format_current)->values_data[settings->rec.pixel_format_current].toInt();
        cfg.preset=messenger->settingsModel()->data_p(&settings->rec.preset_current)->values_data[settings->rec.preset_current].toString();
        cfg.video_encoder=(FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder)->values_data[settings->rec.encoder].toInt();
        cfg.crf=settings->rec.crf;
        cfg.rgb_source=decklink_thread->rgbSource();
        cfg.rgb_source=true;
        cfg.rgb_10bit=decklink_thread->rgb10Bit();
        cfg.rgb_10bit=false;
        cfg.audio_sample_size=messenger->settingsModel()->data_p(&settings->device_decklink.audio_sample_size)->values_data[settings->device_decklink.audio_sample_size].toInt();
        cfg.downscale=settings->rec.downscale;
        cfg.scale_filter=settings->rec.scale_filter;

        ff_enc->setConfig(cfg);

        messenger->updateRecStats();

        dropped_frames_counter=0;
    }
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
    const QPair <int, int> buffer_size=ff_enc->frameBuffer()->size();

    messenger->updateRecStats(s.time.toString(QStringLiteral("HH:mm:ss")),
                              QString(QLatin1String("%1 Mbits/s (%2 MB/s)")).arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2),
                              QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString(QLatin1String("%1 bytes")).arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString(QLatin1String("buf state: %1/%2")).arg(buffer_size.first).arg(buffer_size.second),
                              QString(QLatin1String("frames dropped: %1")).arg(dropped_frames_counter));

    http_server->setRecStats(NRecStats(s.time, s.avg_bitrate_video + s.avg_bitrate_audio, s.streams_size, dropped_frames_counter, buffer_size.second, buffer_size.first));
}
