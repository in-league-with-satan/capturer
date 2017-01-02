#include <QDebug>
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

#include "DeckLinkAPI.h"

#include "device_list.h"
#include "capture.h"
#include "audio_output.h"
#include "out_widget.h"
#include "ffmpeg_thread.h"

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
  , mb_rec_stopped(nullptr)
{
    decklink_thread=new DeckLinkCapture(this);
    connect(decklink_thread, SIGNAL(formatChanged(QSize,int64_t,int64_t)), SLOT(onFormatChanged(QSize,int64_t,int64_t)), Qt::QueuedConnection);

    //

    audio_output=newAudioOutput(this);

    decklink_thread->subscribeForAudio(audio_output->frameBuffer());

    //

    ffmpeg=new FFMpegThread(this);

    decklink_thread->subscribeForAll(ffmpeg->frameBuffer());

    connect(ffmpeg->frameBuffer(), SIGNAL(frameSkipped(size_t)), SLOT(onFrameSkipped(size_t)), Qt::QueuedConnection);

    //

    out_widget=new OutWidget();

    out_widget->showFullScreen();
    decklink_thread->subscribeForAll(out_widget->frameBuffer());

    //

    cb_device=new QComboBox();
    cb_format=new QComboBox();
    cb_pixel_format=new QComboBox();

    cb_audio_channels=new QComboBox();

    cb_rec_fps=new QComboBox();

    cb_rec_fps->addItem("25");
    cb_rec_fps->addItem("29.97");
    cb_rec_fps->addItem("30");
    cb_rec_fps->addItem("25=50/2");
    cb_rec_fps->addItem("29.97=59/2");
    cb_rec_fps->addItem("30=60/2");
    cb_rec_fps->addItem("50");
    cb_rec_fps->addItem("59.97");
    cb_rec_fps->addItem("60");

    cb_rec_fps->setCurrentText("59.97");

    cb_rec_pixel_format=new QComboBox();
    cb_rec_pixel_format->addItem("YUV420P", AV_PIX_FMT_YUV420P);
    cb_rec_pixel_format->addItem("YUV444P", AV_PIX_FMT_YUV444P);


    connect(cb_device, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceChanged(int)));
    connect(cb_format, SIGNAL(currentIndexChanged(int)), SLOT(onFormatChanged(int)));
    connect(cb_pixel_format, SIGNAL(currentIndexChanged(int)), SLOT(onPixelFormatChanged(int)));


    le_crf=new QLineEdit("10");
    le_crf->setInputMask("99");


    cb_video_encoder=new QComboBox();
    cb_video_encoder->addItem("libx264");
    cb_video_encoder->addItem("nvenc_h264");
    cb_video_encoder->addItem("nvenc_hhevc");


    cb_preview=new QCheckBox("preview");
    cb_preview->setChecked(true);
    connect(cb_preview, SIGNAL(stateChanged(int)), SLOT(onPreviewChanged(int)));

    cb_stop_rec_on_frames_drop=new QCheckBox("stop rec on frames drop");
    cb_stop_rec_on_frames_drop->setChecked(true);


    QLabel *l_device=new QLabel("device:");
    QLabel *l_format=new QLabel("format:");
    QLabel *l_pixel_format=new QLabel("pixel format:");

    QLabel *l_audio_channels=new QLabel("audio channels:");

    QLabel *l_rec_fps=new QLabel("rec fps:");

    QLabel *l_rec_pixel_format=new QLabel("rec pixel format::");

    QLabel *l_crf=new QLabel("crf:");

    QLabel *l_video_encoder=new QLabel("video encoder:");

    QPushButton *b_start_cap=new QPushButton("start capture");
    QPushButton *b_stop_cap=new QPushButton("stop capture");

    QPushButton *b_start_rec=new QPushButton("start recording");
    QPushButton *b_stop_rec=new QPushButton("stop recording");

    connect(b_start_cap, SIGNAL(clicked(bool)), SLOT(onStartCapture()));
    connect(b_stop_cap, SIGNAL(clicked(bool)), decklink_thread, SLOT(captureStop()), Qt::QueuedConnection);

    connect(b_start_rec, SIGNAL(clicked(bool)), SLOT(onStartRecording()));
    connect(b_stop_rec, SIGNAL(clicked(bool)), SLOT(onStopRecording()));



    QGridLayout *la_dev=new QGridLayout();

    int row=0;

    la_dev->addWidget(l_device, row, 0);
    la_dev->addWidget(cb_device, row, 1);

    row++;

    la_dev->addWidget(l_format, row, 0);
    la_dev->addWidget(cb_format, row, 1);

    row++;

    la_dev->addWidget(l_pixel_format, row, 0);
    la_dev->addWidget(cb_pixel_format, row, 1);

    row++;

    la_dev->addWidget(l_audio_channels, row, 0);
    la_dev->addWidget(cb_audio_channels, row, 1);

    row++;

    la_dev->addWidget(l_rec_fps, row, 0);
    la_dev->addWidget(cb_rec_fps, row, 1);

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
    la_h->addWidget(cb_preview);
    la_h->addWidget(cb_stop_rec_on_frames_drop);
    la_h->addWidget(b_start_cap);
    la_h->addWidget(b_stop_cap);
    la_h->addWidget(b_start_rec);
    la_h->addWidget(b_stop_rec);

    QWidget *w_central=new QWidget();
    w_central->setLayout(la_h);

    setCentralWidget(w_central);

    //

    cb_audio_channels->addItem(QString("2"));
    cb_audio_channels->addItem(QString("8"));

    cb_audio_channels->setCurrentText("8");

    //

    DeckLinkDevices devices;
    GetDevices(&devices);

    for(int i_device=0; i_device<devices.size(); ++i_device) {
        DeckLinkDevice dev=devices[i_device];
        QVariant var;
        var.setValue(dev);

        cb_device->addItem(dev.name, var);
    }

    onStartCapture();
}

MainWindow::~MainWindow()
{
    decklink_thread->exit();
}

void MainWindow::onFormatChanged(QSize size, int64_t frame_duration, int64_t frame_scale)
{
    last_frame_size=size;
}

void MainWindow::onDeviceChanged(int index)
{
    Q_UNUSED(index);

    cb_format->clear();
    cb_pixel_format->clear();

    DeckLinkDevice dev=cb_device->currentData().value<DeckLinkDevice>();

    for(int i_format=0; i_format<dev.formats.size(); ++i_format) {
        DeckLinkFormat fmt=dev.formats[i_format];

        QVariant var;
        var.setValue(fmt);

        cb_format->addItem(fmt.display_mode_name, var);
    }
}

void MainWindow::onFormatChanged(int index)
{
    Q_UNUSED(index);

    cb_pixel_format->clear();

    DeckLinkFormat fmt=cb_format->currentData().value<DeckLinkFormat>();

    for(int i_pf=0; i_pf<fmt.pixel_formats.size(); ++i_pf) {
        DeckLinkPixelFormat pf=fmt.pixel_formats[i_pf];

        QVariant var;
        var.setValue(pf);

        cb_pixel_format->addItem(pf.name(), var);
    }
}

void MainWindow::onPixelFormatChanged(int index)
{
    Q_UNUSED(index);
}

void MainWindow::onStartCapture()
{
    decklink_thread->setup(cb_device->currentData().value<DeckLinkDevice>(),
                           cb_format->currentData().value<DeckLinkFormat>(),
                           cb_pixel_format->currentData().value<DeckLinkPixelFormat>(),
                           cb_audio_channels->currentText().toInt());

    QMetaObject::invokeMethod(audio_output, "changeChannels", Qt::QueuedConnection, Q_ARG(int, cb_audio_channels->currentText().toInt()));


    QMetaObject::invokeMethod(decklink_thread, "captureStart", Qt::QueuedConnection);
}

void MainWindow::onStartRecording()
{
    FFMpeg::Config cfg;

    cfg.audio_channels_size=cb_audio_channels->currentText().toInt();
    cfg.framerate=(FFMpeg::Framerate::T)cb_rec_fps->currentIndex();
    cfg.frame_resolution=last_frame_size;
    cfg.pixel_format=(AVPixelFormat)cb_rec_pixel_format->currentData().toInt();
    cfg.video_encoder=(FFMpeg::VideoEncoder::T)cb_video_encoder->currentIndex();
    cfg.crf=le_crf->text().toUInt();

    ffmpeg->setConfig(cfg);
}

void MainWindow::onStopRecording()
{
    ffmpeg->stopCoder();
}

void MainWindow::onFrameSkipped(size_t size)
{
    qCritical() << "frames skipped:" << size;

    if(!cb_stop_rec_on_frames_drop->isChecked())
        return;

    ffmpeg->stopCoder();

    if(!mb_rec_stopped)
        mb_rec_stopped=new QMessageBox(QMessageBox::Critical, "", "some frames was dropped, recording stopped", QMessageBox::Ok);

    mb_rec_stopped->show();
    mb_rec_stopped->raise();
    mb_rec_stopped->exec();
}

void MainWindow::onPreviewChanged(int)
{
    out_widget->frameBuffer()->setEnabled(cb_preview->isChecked());
}
