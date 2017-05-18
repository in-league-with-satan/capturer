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

#include "device_list.h"
#include "capture.h"
#include "audio_output.h"
#include "video_widget.h"
#include "sdl2_video_output_thread.h"
#include "audio_level_widget.h"

#include "qml_messenger.h"
#include "overlay_view.h"


#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , mb_rec_stopped(nullptr)
{
    decklink_thread=new DeckLinkCapture(this);

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            SLOT(onFormatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);
    connect(decklink_thread, SIGNAL(frameSkipped()), SLOT(onFrameSkipped()), Qt::QueuedConnection);

    //

    qmlRegisterType<SnapshotListModel>("FuckTheSystem", 0, 0, "SnapshotListModel");

    messenger=new QmlMessenger();

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            messenger, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);

    connect(decklink_thread, SIGNAL(signalLost(bool)), messenger, SIGNAL(signalLost(bool)), Qt::QueuedConnection);

    overlay_view=new OverlayView();

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

    connect(ff_enc->frameBuffer(), SIGNAL(frameSkipped()), SLOT(onEncBufferOverload()), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stats(FFEncoder::Stats)), SLOT(updateStats(FFEncoder::Stats)), Qt::QueuedConnection);
    connect(ff_enc, SIGNAL(stateChanged(bool)), SLOT(onEncoderStateChanged(bool)), Qt::QueuedConnection);

    //

    ff_dec=new FFDecoderThread(this);
    ff_dec->subscribeVideo(out_widget->frameBuffer());
    ff_dec->subscribeAudio(audio_output->frameBuffer());

    connect(messenger->fileSystemModel(), SIGNAL(playMedia(QString)), ff_dec, SLOT(open(QString)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(durationChanged(qint64)), messenger, SIGNAL(playerDurationChanged(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(positionChanged(qint64)), messenger, SIGNAL(playerPositionChanged(qint64)), Qt::QueuedConnection);
    connect(ff_dec, SIGNAL(stateChanged(int)), SLOT(onPlayerStateChanged(int)), Qt::QueuedConnection);
    connect(messenger, SIGNAL(playerSetPosition(qint64)), ff_dec, SLOT(seek(qint64)));

    //

    audio_level=new AudioLevelWidget();
    decklink_thread->subscribe(audio_level->frameBuffer());

    connect(audio_level, SIGNAL(levels(qint16,qint16,qint16,qint16,qint16,qint16,qint16,qint16)),
            messenger, SIGNAL(audioLevels(qint16,qint16,qint16,qint16,qint16,qint16,qint16,qint16)));

    //

    cb_device=new QComboBox();
    connect(cb_device, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceChanged(int)));

    cb_device_screen_format=new QComboBox();
    connect(cb_device_screen_format, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceScreenFormatChanged(int)));

    cb_device_pixel_format=new QComboBox();

    //

    le_video_mode=new QLineEdit();
    le_video_mode->setReadOnly(true);


    cb_half_fps=new QCheckBox("half fps");

    connect(cb_half_fps, SIGNAL(toggled(bool)), messenger, SIGNAL(halfFpsSet(bool)));

    connect(messenger, SIGNAL(halfFpsChanged(bool)), cb_half_fps, SLOT(setChecked(bool)));


    cb_rec_pixel_format=new QComboBox();

    connect(cb_rec_pixel_format, SIGNAL(currentIndexChanged(int)), SLOT(onPixelFormatChanged(int)));
    connect(cb_rec_pixel_format, SIGNAL(currentIndexChanged(int)), messenger, SIGNAL(pixelFormatIndexSet(int)));

    connect(messenger, SIGNAL(pixelFormatIndexChanged(int)), cb_rec_pixel_format, SLOT(setCurrentIndex(int)));


    le_crf=new QLineEdit("10");
    le_crf->setInputMask("99");

    connect(le_crf, SIGNAL(textChanged(QString)), SLOT(onCrfChanged(QString)));
    connect(messenger, SIGNAL(crfChanged(int)), SLOT(onCrfChanged(int)));

    cb_video_encoder=new QComboBox();
    connect(cb_video_encoder, SIGNAL(currentIndexChanged(int)), SLOT(onEncoderChanged(int)));
    connect(cb_video_encoder, SIGNAL(currentIndexChanged(int)), messenger, SIGNAL(videoEncoderIndexSet(int)));

    connect(messenger, SIGNAL(videoCodecIndexChanged(int)), cb_video_encoder, SLOT(setCurrentIndex(int)));

    if(FFEncoder::isLib_x264_10bit()) {
        cb_video_encoder->addItem("libx264_10bit", FFEncoder::VideoEncoder::libx264_10bit);
        cb_video_encoder->addItem("nvenc_h264", FFEncoder::VideoEncoder::nvenc_h264);
        cb_video_encoder->addItem("nvenc_hevc", FFEncoder::VideoEncoder::nvenc_hevc);

        messenger->setModelVideoEncoder(QStringList() << "libx264_10bit" << "nvenc_h264" << "nvenc_hevc");

    } else {
        cb_video_encoder->addItem("libx264", FFEncoder::VideoEncoder::libx264);
        cb_video_encoder->addItem("libx264rgb", FFEncoder::VideoEncoder::libx264rgb);
        cb_video_encoder->addItem("nvenc_h264", FFEncoder::VideoEncoder::nvenc_h264);
        cb_video_encoder->addItem("nvenc_hevc", FFEncoder::VideoEncoder::nvenc_hevc);

        messenger->setModelVideoEncoder(QStringList() << "libx264" << "libx264rgb" << "nvenc_h264" << "nvenc_hevc");
    }

    cb_preview=new QCheckBox("preview");
    cb_preview->setChecked(true);

    connect(cb_preview, SIGNAL(stateChanged(int)), SLOT(onPreviewChanged(int)));


    cb_stop_rec_on_frames_drop=new QCheckBox("stop rec on frames drop");

    connect(cb_stop_rec_on_frames_drop, SIGNAL(toggled(bool)), messenger, SIGNAL(stopOnDropSet(bool)));

    connect(messenger, SIGNAL(stopOnDropChanged(bool)), cb_stop_rec_on_frames_drop, SLOT(setChecked(bool)));

    cb_stop_rec_on_frames_drop->setChecked(true);


    QLabel *l_device=new QLabel("device:");
    QLabel *l_device_screen_format=new QLabel("dev screen format:");
    QLabel *l_device_pixel_format=new QLabel("dev pixel format:");

    QLabel *l_video_mode=new QLabel("input video mode:");

    QLabel *l_rec_pixel_format=new QLabel("rec pixel format:");

    QLabel *l_crf=new QLabel("crf:");

    QLabel *l_video_encoder=new QLabel("video encoder:");

    QPushButton *b_start_stop_dev=new QPushButton("start/stop dev");
    connect(b_start_stop_dev, SIGNAL(clicked(bool)), SLOT(startStopCapture()));

    QPushButton *b_start_stop_rec=new QPushButton("start/stop recording");
    connect(b_start_stop_rec, SIGNAL(clicked(bool)), SLOT(onStartStopRecording()));

    //

    QLabel *l_stat_size=new QLabel("size:");
    QLabel *l_stat_br=new QLabel("avg bitrate:");
    QLabel *l_stat_time=new QLabel("time:");

    le_stat_size=new QLineEdit();
    le_stat_br=new QLineEdit();
    le_stat_time=new QLineEdit();

    le_stat_size->setReadOnly(true);
    le_stat_br->setReadOnly(true);
    le_stat_time->setReadOnly(true);

    QGridLayout *la_stats=new QGridLayout();

    la_stats->addWidget(l_stat_size, 0, 0);
    la_stats->addWidget(le_stat_size, 0, 1);

    la_stats->addWidget(l_stat_br, 1, 0);
    la_stats->addWidget(le_stat_br, 1, 1);

    la_stats->addWidget(l_stat_time, 2, 0);
    la_stats->addWidget(le_stat_time, 2, 1);

    //

    QGridLayout *la_dev=new QGridLayout();

    int row=0;

    la_dev->addWidget(l_device, row, 0);
    la_dev->addWidget(cb_device, row, 1);

    row++;

    la_dev->addWidget(l_device_screen_format, row, 0);
    la_dev->addWidget(cb_device_screen_format, row, 1);

    row++;

    la_dev->addWidget(l_device_pixel_format, row, 0);
    la_dev->addWidget(cb_device_pixel_format, row, 1);

    row++;

    la_dev->addWidget(l_video_mode, row, 0);
    la_dev->addWidget(le_video_mode, row, 1);

    row++;

    la_dev->addWidget(l_rec_pixel_format, row, 0);
    la_dev->addWidget(cb_rec_pixel_format, row, 1);

    row++;

    la_dev->addWidget(l_crf, row, 0);
    la_dev->addWidget(le_crf, row, 1);

    row++;

    la_dev->addWidget(l_video_encoder, row, 0);
    la_dev->addWidget(cb_video_encoder, row, 1);


    QVBoxLayout *la_h=new QVBoxLayout();

    la_h->addLayout(la_dev);
    la_h->addWidget(cb_half_fps);
    la_h->addWidget(cb_preview);
    la_h->addWidget(cb_stop_rec_on_frames_drop);
    la_h->addWidget(b_start_stop_dev);
    la_h->addWidget(b_start_stop_rec);
    la_h->addLayout(la_stats);
    la_h->addWidget(audio_level);


    QWidget *w_central=new QWidget();
    w_central->setLayout(la_h);

    setCentralWidget(w_central);

    //

    DeckLinkDevices devices;
    GetDevices(&devices);

    for(int i_device=0; i_device<devices.size(); ++i_device) {
        DeckLinkDevice dev=devices[i_device];
        QVariant var;
        var.setValue(dev);

        cb_device->addItem(dev.name, var);
    }

    load();

    //

    QDir dir(qApp->applicationDirPath() + "/videos");

    if(!dir.exists())
        dir.mkdir(dir.absolutePath());

    //

    QVBoxLayout *la_container=new QVBoxLayout();
    la_container->addWidget(overlay_view);
    la_container->setMargin(0);

    out_widget->setLayout(la_container);

    QApplication::instance()->installEventFilter(this);

    if(qApp->arguments().contains("--windowed")) {
        out_widget->setMinimumSize(640, 640/(16/9.));
        out_widget->resize(640, 640/(16/9.));
        out_widget->show();

    } else {
        out_widget->showFullScreen();
    }

    emit messenger->signalLost(true);

    startStopCapture();
}

MainWindow::~MainWindow()
{
    save();
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
                if(ff_dec->currentState()==FFDecoderThread::ST_STOPPED)
                    onStartStopRecording();

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
                cb_preview->setChecked(!cb_preview->isChecked());
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

void MainWindow::load()
{
    QFile f;

#ifdef USE_X264_10B

    f.setFileName(QApplication::applicationDirPath() + "/capturer_10bit.json");

#else

    f.setFileName(QApplication::applicationDirPath() + "/capturer.json");

#endif

    if(!f.open(QFile::ReadOnly))
        return;

    QVariantMap map_cfg=QJsonDocument::fromJson(f.readAll()).toVariant().toMap();

    cb_device->setCurrentIndex(map_cfg.value("device").toInt());
    map_pixel_format=map_cfg.value("rec_pixel_format").toMap();
    le_crf->setText(map_cfg.value("crf").toString());
    cb_video_encoder->setCurrentIndex(map_cfg.value("video_encoder").toInt());

    cb_half_fps->setChecked(map_cfg.value("half_fps").toBool());
    cb_preview->setChecked(map_cfg.value("preview").toBool());
    cb_stop_rec_on_frames_drop->setChecked(map_cfg.value("stop_rec_on_frames_drop").toBool());

    restoreGeometry(QByteArray::fromBase64(map_cfg.value("geometry").toByteArray()));

    onEncoderChanged(map_cfg.value("video_encoder").toInt());
}

void MainWindow::save()
{
    QFile f;

#ifdef USE_X264_10B

    f.setFileName(QApplication::applicationDirPath() + "/capturer_10bit.json");

#else

    f.setFileName(QApplication::applicationDirPath() + "/capturer.json");

#endif

    if(!f.open(QFile::ReadWrite | QFile::Truncate))
        return;

    QVariantMap map_cfg;

    map_cfg.insert("device", cb_device->currentIndex());
    map_cfg.insert("rec_pixel_format", map_pixel_format);
    map_cfg.insert("crf", le_crf->text().toInt());
    map_cfg.insert("video_encoder", cb_video_encoder->currentIndex());
    map_cfg.insert("half_fps", cb_half_fps->isChecked());
    map_cfg.insert("preview", cb_preview->isChecked());
    map_cfg.insert("stop_rec_on_frames_drop", cb_stop_rec_on_frames_drop->isChecked());
    map_cfg.insert("geometry", QString(saveGeometry().toBase64()));

    f.write(QJsonDocument::fromVariant(map_cfg).toJson());
}

void MainWindow::onEncoderChanged(const int &index)
{
    Q_UNUSED(index);

    QList <FFEncoder::PixelFormat::T> fmts=
            FFEncoder::PixelFormat::compatiblePixelFormats((FFEncoder::VideoEncoder::T)cb_video_encoder->currentData().toInt());

    cb_rec_pixel_format->blockSignals(true);

    cb_rec_pixel_format->clear();

    QStringList sl;

    for(int i=0; i<fmts.size(); ++i) {
        cb_rec_pixel_format->addItem(FFEncoder::PixelFormat::toString(fmts[i]), fmts[i]);
        sl << FFEncoder::PixelFormat::toString(fmts[i]);
    }

    messenger->setModelPixelFormat(sl);

    cb_rec_pixel_format->blockSignals(false);


    int index_pf=map_pixel_format.value(QString::number(cb_video_encoder->currentData().toInt()), 0).toInt();

    cb_rec_pixel_format->setCurrentIndex(index_pf);

    messenger->pixelFormatIndexSet(index_pf);

}

void MainWindow::onPixelFormatChanged(const int &index)
{
    if(index<0)
        return;

    map_pixel_format.insert(QString::number(cb_video_encoder->currentData().toInt()), index);
}

void MainWindow::onFormatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format)
{
    current_frame_size=QSize(width, height);

    current_frame_duration=frame_duration;
    current_frame_scale=frame_scale;

    le_video_mode->setText(QString("%1%2@%3 %4")
                           .arg(height)
                           .arg(progressive_frame ? "p" : "i")
                           .arg(QString::number((double)frame_scale/(double)frame_duration, 'f', 2))
                           .arg(pixel_format)
                           );
}

void MainWindow::onDeviceChanged(int index)
{
    Q_UNUSED(index)

    cb_device_screen_format->clear();

    DeckLinkDevice dev=cb_device->currentData().value<DeckLinkDevice>();

    for(int i=0; i<dev.formats.size(); ++i) {
        DeckLinkFormat fmt=dev.formats[i];

        QVariant var;
        var.setValue(fmt);

        cb_device_screen_format->addItem(fmt.display_mode_name, var);
    }
}

void MainWindow::onDeviceScreenFormatChanged(int index)
{
    Q_UNUSED(index)

    cb_device_pixel_format->clear();

    DeckLinkFormat sf=cb_device_screen_format->currentData().value<DeckLinkFormat>();

    for(int i=0; i<sf.pixel_formats.size(); ++i) {
        DeckLinkPixelFormat pf=sf.pixel_formats[i];

        QVariant var;
        var.setValue(pf);

        cb_device_pixel_format->addItem(pf.name(), var);
    }
}

void MainWindow::onCrfChanged(const QString &text)
{
    messenger->crfSet(text.toInt());
}

void MainWindow::onCrfChanged(const int &crf)
{
    le_crf->blockSignals(true);

    le_crf->setText(QString::number(crf));

    le_crf->blockSignals(false);
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
#ifndef __OPTIMIZE__

    decklink_thread->setup(cb_device->currentData().value<DeckLinkDevice>(),
                           cb_device_screen_format->currentData().value<DeckLinkFormat>(),
                           cb_device_pixel_format->currentData().value<DeckLinkPixelFormat>(),
                           8);

#else

    decklink_thread->setup(cb_device->currentData().value<DeckLinkDevice>(),
                           DeckLinkFormat(),
                           DeckLinkPixelFormat(),
                           8);

#endif

    QMetaObject::invokeMethod(audio_output, "changeChannels", Qt::QueuedConnection, Q_ARG(int, 8));

    QMetaObject::invokeMethod(decklink_thread, "captureStart", Qt::QueuedConnection);
}

void MainWindow::captureStop()
{
    QMetaObject::invokeMethod(decklink_thread, "captureStop", Qt::QueuedConnection);
}

void MainWindow::onStartStopRecording()
{
    if(ff_enc->isWorking()) {
        ff_enc->stopCoder();

    } else {
        FFEncoder::Config cfg;

        cfg.framerate=FFEncoder::calcFps(current_frame_duration, current_frame_scale, cb_half_fps->isChecked());
        cfg.frame_resolution=current_frame_size;
        cfg.pixel_format=(AVPixelFormat)cb_rec_pixel_format->currentData().toInt();
        cfg.video_encoder=(FFEncoder::VideoEncoder::T)cb_video_encoder->currentData().toInt();
        cfg.crf=le_crf->text().toUInt();

        ff_enc->setConfig(cfg);

        messenger->updateRecStats();
        messenger->setRecStarted(true);

        dropped_frames_counter=0;
    }
}

void MainWindow::onFrameSkipped()
{
    dropped_frames_counter++;

    if(!cb_stop_rec_on_frames_drop->isChecked())
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

void MainWindow::onEncBufferOverload()
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

void MainWindow::onPreviewChanged(int)
{
    out_widget->frameBuffer()->setEnabled(cb_preview->isChecked());
}

void MainWindow::onEncoderStateChanged(bool state)
{
    if(!state)
        messenger->setRecStarted(false);
}

void MainWindow::onPlayerStateChanged(int state)
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

        QMetaObject::invokeMethod(audio_output, "changeChannels", Qt::QueuedConnection, Q_ARG(int, 2));
    }
}

void MainWindow::updateStats(FFEncoder::Stats s)
{
    le_stat_size->setText(QString("%1 bytes").arg(QLocale().toString((qulonglong)s.streams_size)));

    le_stat_br->setText(QString("%1 kbits/s").arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000., 'f', 0)));

    le_stat_time->setText(s.time.toString("HH:mm:ss"));

    const QPair <int, int> buffer_size=ff_enc->frameBuffer()->size();

    messenger->updateRecStats(s.time.toString("HH:mm:ss"),
                              QString("%1 Mbits/s (%2 MB/s)").arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2))
                              .arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString("%1 bytes").arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString("buf state: %1/%2").arg(buffer_size.first).arg(buffer_size.second),
                              QString("frames dropped: %1").arg(dropped_frames_counter));
}

