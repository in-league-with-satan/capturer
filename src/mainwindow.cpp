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

#include "DeckLinkAPI.h"

#include "device_list.h"
#include "capture.h"
#include "audio_output.h"
#include "out_widget_2.h"
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

    messenger=new QmlMessenger();

    connect(decklink_thread, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)),
            messenger, SIGNAL(formatChanged(int,int,quint64,quint64,bool,QString)), Qt::QueuedConnection);


    overlay_view=new OverlayView();

    overlay_view->installEventFilter(this);

    overlay_view->setMessenger(messenger);

    overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));

    //

    audio_output=newAudioOutput(this);

    decklink_thread->subscribe(audio_output->frameBuffer());

    //

    ffmpeg=new FFMpegThread(this);

    decklink_thread->subscribe(ffmpeg->frameBuffer());

    connect(ffmpeg->frameBuffer(), SIGNAL(frameSkipped()), SLOT(onEncBufferOverload()), Qt::QueuedConnection);
    connect(ffmpeg, SIGNAL(stats(FFMpeg::Stats)), SLOT(updateStats(FFMpeg::Stats)));

    //

    out_widget=new OutWidget2();

    decklink_thread->subscribe(out_widget->frameBuffer());

    connect(out_widget, SIGNAL(focusEvent()), overlay_view, SLOT(raise()));

    //

    connect(overlay_view, SIGNAL(closing(QQuickCloseEvent*)), out_widget, SLOT(close()));

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

    if(FFMpeg::isLib_x264_10bit()) {
        cb_video_encoder->addItem("libx264_10bit", FFMpeg::VideoEncoder::libx264_10bit);
        cb_video_encoder->addItem("nvenc_h264", FFMpeg::VideoEncoder::nvenc_h264);
        cb_video_encoder->addItem("nvenc_hevc", FFMpeg::VideoEncoder::nvenc_hevc);

        messenger->setModelVideoEncoder(QStringList() << "libx264_10bit" << "nvenc_h264" << "nvenc_hevc");

    } else {
        cb_video_encoder->addItem("libx264", FFMpeg::VideoEncoder::libx264);
        cb_video_encoder->addItem("libx264rgb", FFMpeg::VideoEncoder::libx264rgb);
        cb_video_encoder->addItem("nvenc_h264", FFMpeg::VideoEncoder::nvenc_h264);
        cb_video_encoder->addItem("nvenc_hevc", FFMpeg::VideoEncoder::nvenc_hevc);

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

    startStopCapture();


#ifdef _WIN32

    out_widget->showFullScreen();

    overlay_view->show();
    overlay_view->resize(overlay_view->size() - QSize(1, 1));

#else

#ifndef __OPTIMIZE__

    out_widget->setMinimumSize(640, 480);
    out_widget->show();

#else

    out_widget->showFullScreen();
    overlay_view->showFullScreen();

#endif // __OPTIMIZE__

#endif // _WIN32
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

            switch(e->key()) {
            case Qt::Key_F4:
                onStartStopRecording();
                return true;

            case Qt::Key_F5:
                messenger->showHideInfo();
                return true;

            case Qt::Key_F6:
                messenger->showHideDetailedRecState();
                return true;

            case Qt::Key_F7:
                cb_preview->setChecked(!cb_preview->isChecked());
                return true;

            case Qt::Key_F12:
                QApplication::exit(0);
                return true;
            }

            messenger->keyEvent((Qt::Key)e->key());

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

    QList <FFMpeg::PixelFormat::T> fmts=
            FFMpeg::PixelFormat::compatiblePixelFormats((FFMpeg::VideoEncoder::T)cb_video_encoder->currentData().toInt());

    cb_rec_pixel_format->blockSignals(true);

    cb_rec_pixel_format->clear();

    QStringList sl;

    for(int i=0; i<fmts.size(); ++i) {
        cb_rec_pixel_format->addItem(FFMpeg::PixelFormat::toString(fmts[i]), fmts[i]);
        sl << FFMpeg::PixelFormat::toString(fmts[i]);
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
        QMetaObject::invokeMethod(decklink_thread, "captureStop", Qt::QueuedConnection);
        return;
    }


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

void MainWindow::onStartStopRecording()
{
    if(ffmpeg->isWorking()) {
        ffmpeg->stopCoder();

        messenger->recStopped();

    } else {
        FFMpeg::Config cfg;

        cfg.framerate=FFMpeg::calcFps(current_frame_duration, current_frame_scale, cb_half_fps->isChecked());
        cfg.frame_resolution=current_frame_size;
        cfg.pixel_format=(AVPixelFormat)cb_rec_pixel_format->currentData().toInt();
        cfg.video_encoder=(FFMpeg::VideoEncoder::T)cb_video_encoder->currentData().toInt();
        cfg.crf=le_crf->text().toUInt();

        ffmpeg->setConfig(cfg);

        messenger->updateRecStats();
        messenger->recStarted();

        dropped_frames_counter=0;
    }
}

void MainWindow::onFrameSkipped()
{
    dropped_frames_counter++;

    if(!cb_stop_rec_on_frames_drop->isChecked())
        return;

    if(!ffmpeg->isWorking())
        return;

    ffmpeg->stopCoder();
    messenger->recStopped();

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
    if(!ffmpeg->isWorking())
        return;

    ffmpeg->stopCoder();
    messenger->recStopped();


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

void MainWindow::updateStats(FFMpeg::Stats s)
{
    le_stat_size->setText(QString("%1 bytes").arg(QLocale().toString((qulonglong)s.streams_size)));

    le_stat_br->setText(QString("%1 kbits/s").arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000., 'f', 0)));

    le_stat_time->setText(s.time.toString("HH:mm:ss"));

    const QPair <int, int> buffer_size=ffmpeg->frameBuffer()->size();

    messenger->updateRecStats(s.time.toString("HH:mm:ss"),
                              QString("%1 Mbits/s (%2 MB/s)").arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/1000./1000., 'f', 2))
                              .arg(QLocale().toString((s.avg_bitrate_video + s.avg_bitrate_audio)/8/1024./1024., 'f', 2)),
                              QString("%1 bytes").arg(QLocale().toString((qulonglong)s.streams_size)),
                              QString("buf state: %1/%2").arg(buffer_size.first).arg(buffer_size.second),
                              QString("frames dropped: %1").arg(dropped_frames_counter));
}

