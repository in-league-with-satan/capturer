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
#include "video_widget.h"
#include "sdl2_video_output_thread.h"
#include "audio_level.h"
#include "qml_messenger.h"
#include "overlay_view.h"

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

    qmlRegisterType<SettingsModel>("FuckTheSystem", 0, 0, "SettingsModel");
    qmlRegisterType<FileSystemModel>("FuckTheSystem", 0, 0, "FileSystemModel");
    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");

    messenger=new QmlMessenger();

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            messenger, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);

    connect(decklink_thread, SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

    overlay_view=new OverlayView(this);

    overlay_view->setMessenger(messenger);

    overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));

    overlay_view->addImageProvider("fs_image_provider", (QQmlImageProviderBase*)messenger->fileSystemModel()->imageProvider());

    //

    audio_output=newAudioOutput(this);

    decklink_thread->subscribe(audio_output->frameBuffer());

    //

    out_widget=new VideoWidget();

    decklink_thread->subscribe(out_widget->frameBuffer());

    //

    ff_enc=new FFEncoderThread(this);

    decklink_thread->subscribe(ff_enc->frameBuffer());

    connect(ff_enc->frameBuffer().get(), SIGNAL(frameSkipped()), SLOT(encoderBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stateChanged(bool)), SLOT(encoderStateChanged(bool)), Qt::QueuedConnection);

    //

    ff_dec=new FFDecoderThread(this);
    ff_dec->subscribeVideo(out_widget->frameBuffer());
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

    connect(audio_level, SIGNAL(levels(qint16,qint16,qint16,qint16,qint16,qint16,qint16,qint16)),
            messenger, SIGNAL(audioLevels(qint16,qint16,qint16,qint16,qint16,qint16,qint16,qint16)), Qt::QueuedConnection);

    //

    settings->load();

    //

    SettingsModel::Data set_model_data;


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

    set_model_data.value=&settings->device.index;


    messenger->settingsModel()->add(set_model_data);

    set_model_data.values.clear();
    set_model_data.values_data.clear();

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.group="rec";
    set_model_data.name="video encoder";

    if(FFEncoder::isLib_x264_10bit()) {
        set_model_data.values << QStringList() << "libx264_10bit" << "nvenc_h264" << "nvenc_hevc";
        set_model_data.values_data << FFEncoder::VideoEncoder::libx264_10bit << FFEncoder::VideoEncoder::nvenc_h264 << FFEncoder::VideoEncoder::nvenc_hevc;

    } else {
        set_model_data.values << QStringList() << "libx264" << "libx264rgb" << "nvenc_h264" << "nvenc_hevc";
        set_model_data.values_data << FFEncoder::VideoEncoder::libx264 << FFEncoder::VideoEncoder::libx264rgb << FFEncoder::VideoEncoder::nvenc_h264 << FFEncoder::VideoEncoder::nvenc_hevc;

    }

    set_model_data.value=&settings->rec.encoder;

    messenger->settingsModel()->add(set_model_data);

    //

    set_model_data.type=SettingsModel::Type::combobox;
    set_model_data.name="pixel format";
    set_model_data.value=&settings->rec.pixel_format_current;

    QList <FFEncoder::PixelFormat::T> fmts=
            FFEncoder::PixelFormat::compatiblePixelFormats((FFEncoder::VideoEncoder::T)set_model_data.values_data.value(settings->rec.encoder, 0).toInt());

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

    for(int i=0; i<=24; ++i)
        set_model_data.values.append(QString::number(i));

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

    QVBoxLayout *la_container=new QVBoxLayout();
    la_container->addWidget(overlay_view);
    la_container->setMargin(0);

    out_widget->setLayout(la_container);

    setCentralWidget(out_widget);

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

            switch(key) {
            case Qt::Key_F2:
                if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
                    messenger->showFileBrowser();

                return true;

            case Qt::Key_F1:
                messenger->showHideAbout();
                return true;

            case Qt::Key_F4:
                if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED && (decklink_thread->gotSignal() || ff_enc->isWorking()))
                    startStopRecording();

                return true;

            case Qt::Key_F5:
                if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
                    messenger->showHideInfo();

                else
                    messenger->showHidePlayerState();

                return true;

            case Qt::Key_F6:
                if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
                    messenger->showHideDetailedRecState();

                return true;

            case Qt::Key_F7:
                previewOnOff();
                return true;

            case Qt::Key_F11:
                if(isFullScreen())
                    showNormal();

                else
                    showFullScreen();

                return true;

            case Qt::Key_F12:
                QApplication::exit(0);
                return true;

            case Qt::Key_Menu:
            case Qt::Key_Space:
                // qInfo() << "show_menu";
                if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
                    emit messenger->showMenu();

                else {
                    if(ff_dec->currentState()==FFDecoderThread::ST_PLAY)
                        ff_dec->pause();

                    else
                        ff_dec->play();
                }

                return true;

            case Qt::Key_HomePage:
            case Qt::Key_Back:
            case Qt::Key_Backspace:
            case Qt::Key_Delete:
                if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED)
                    ff_dec->stop();

                else
                    emit messenger->back();

                return true;

            case Qt::Key_Return:
                emit messenger->keyPressed(Qt::Key_Right);

                return true;

            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left:
            case Qt::Key_Right:
                if(ff_dec->currentState()!=FFDecoderThread::ST_STOPPED) {
                    int64_t pos=0;

                    if(Qt::Key_Left==key) {
                        pos=ff_dec->currentPos() - 30*1000;

                        if(pos<0)
                            pos=0;

                        ff_dec->seek(pos);

                        return true;
                    }

                    if(Qt::Key_Right==key) {
                        pos=ff_dec->currentPos() + 30*1000;

                        if(pos<ff_dec->currentDuration()) {
                            ff_dec->seek(pos);

                            return true;
                        }
                    }
                }

                emit messenger->keyPressed((Qt::Key)key);

                return true;
            }

            return true;
        }
    }

    return QObject::eventFilter(object, event);
}

void MainWindow::closeEvent(QCloseEvent *)
{
    out_widget->close();

    overlay_view->close();
}

void MainWindow::formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format)
{
    current_frame_size=QSize(width, height);

    current_frame_duration=frame_duration;
    current_frame_scale=frame_scale;
}

void MainWindow::settingsModelDataChanged(int index, int role, bool qml)
{
    Q_UNUSED(qml)

    SettingsModel *model=messenger->settingsModel();

    SettingsModel::Data *data=model->data_p(index);

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
    }

    if(data->value==&settings->rec.pixel_format_current)
        if(role==SettingsModel::Role::value)
            settings->rec.pixel_format[QString::number(settings->rec.encoder)]=settings->rec.pixel_format_current;
}

void MainWindow::startStopCapture()
{
    if(decklink_thread->isRunning()) {
        captureStop();
        return;
    }

    captureStart();
}

void MainWindow::captureStart()
{
    SettingsModel::Data *model_data=messenger->settingsModel()->data_p(&settings->device.index);

    if(!model_data) {
        qCritical() << "model_data null pointer";
        return;
    }

    if(model_data->values_data.isEmpty()) {
        qCritical() << "values_data is empty";
        return;
    }

    decklink_thread->setup(model_data->values_data[settings->device.index].value<DeckLinkDevice>(),
            DeckLinkFormat(),
            DeckLinkPixelFormat(),
            8);

    QMetaObject::invokeMethod(audio_output, "changeChannels", Qt::QueuedConnection, Q_ARG(int, 8));

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
        FFEncoder::Config cfg;

        cfg.framerate=FFEncoder::calcFps(current_frame_duration, current_frame_scale, settings->rec.half_fps);
        cfg.frame_resolution=current_frame_size;
        cfg.pixel_format=(AVPixelFormat)messenger->settingsModel()->data_p(&settings->rec.pixel_format_current)->values_data[settings->rec.pixel_format_current].toInt();
        cfg.video_encoder=(FFEncoder::VideoEncoder::T)messenger->settingsModel()->data_p(&settings->rec.encoder)->values_data[settings->rec.encoder].toInt();
        cfg.crf=settings->rec.crf;

        ff_enc->setConfig(cfg);

        messenger->updateRecStats();
        messenger->setRecStarted(true);

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

    out_widget->frameBuffer()->setEnabled(settings->main.preview);
}

void MainWindow::encoderStateChanged(bool state)
{
    if(!state)
        messenger->setRecStarted(false);
}

void MainWindow::playerStateChanged(int state)
{
    if(state!=FFDecoderThread::ST_STOPPED || decklink_thread->gotSignal()) {
        emit messenger->signalLost(false);

    } else {
        emit messenger->signalLost(true);

        out_widget->fillBlack();
    }

    if(state==FFDecoderThread::ST_STOPPED) {
        decklink_thread->captureStart();

        emit messenger->showPlayerState(false);

    } else {
        decklink_thread->captureStop();

        QMetaObject::invokeMethod(audio_output, "changeChannels", Qt::QueuedConnection, Q_ARG(int, 2));
    }
}

void MainWindow::updateStats(FFEncoder::Stats s)
{
    const QPair <int, int> buffer_size=ff_enc->frameBuffer()->size();

    messenger->updateRecStats(s.time.toString("HH:mm:ss"),
                              QString("%1 Mbits/s (%2 MB/s)").arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2))
                              .arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString("%1 bytes").arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString("buf state: %1/%2").arg(buffer_size.first).arg(buffer_size.second),
                              QString("frames dropped: %1").arg(dropped_frames_counter));
}
