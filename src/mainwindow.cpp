#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QFile>
#include <QGenericArgument>
#include <QLineEdit>

#include "DeckLinkAPI.h"

#include "device_list.h"
#include "capture.h"
#include "audio_output_thread.h"
#include "out_widget.h"
#include "ffmpeg_thread.h"

#include "mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    decklink_thread=new DeckLinkCapture(this);
    connect(decklink_thread, SIGNAL(frameVideo(QByteArray,QSize)), SLOT(onFrameVideo(QByteArray,QSize)), Qt::QueuedConnection);


    audio_output=new AudioOutputThread(this);

    connect(decklink_thread, SIGNAL(frameAudio(QByteArray)), audio_output, SLOT(onInputFrameArrived(QByteArray)), Qt::QueuedConnection);


    //

    ffmpeg=new FFMpegThread(decklink_thread, this);

    //

    out_widget=new OutWidget();
    out_widget->showFullScreen();

    //

    cb_device=new QComboBox();
    cb_format=new QComboBox();
    cb_pixel_format=new QComboBox();

    cb_audio_channels=new QComboBox();

    connect(cb_device, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceChanged(int)));
    connect(cb_format, SIGNAL(currentIndexChanged(int)), SLOT(onFormatChanged(int)));
    connect(cb_pixel_format, SIGNAL(currentIndexChanged(int)), SLOT(onPixelFormatChanged(int)));


    le_crf=new QLineEdit("10");
    le_crf->setInputMask("99");

    QLabel *l_device=new QLabel("device:");
    QLabel *l_format=new QLabel("format:");
    QLabel *l_pixel_format=new QLabel("pixel format:");

    QLabel *l_audio_channels=new QLabel("audio channels:");

    QLabel *l_crf=new QLabel("crf:");

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

    la_dev->addWidget(l_crf, row, 0);
    la_dev->addWidget(le_crf, row, 1);


    QVBoxLayout *la_h=new QVBoxLayout();

    la_h->addLayout(la_dev);
    la_h->addWidget(b_start_cap);
    la_h->addWidget(b_stop_cap);
    la_h->addWidget(b_start_rec);
    la_h->addWidget(b_stop_rec);

    QWidget *w_central=new QWidget();
    w_central->setLayout(la_h);

    setCentralWidget(w_central);

    //

    cb_audio_channels->addItem(QString("2"));
    cb_audio_channels->addItem(QString("6"));
    cb_audio_channels->addItem(QString("8"));

    //

    DeckLinkDevices devices;
    GetDevices(&devices);

    for(int i_device=0; i_device<devices.size(); ++i_device) {
        DeckLinkDevice dev=devices[i_device];
        QVariant var;
        var.setValue(dev);

        cb_device->addItem(dev.name, var);
    }
}

MainWindow::~MainWindow()
{
    decklink_thread->exit();
}

void MainWindow::onFrameVideo(QByteArray ba_data, QSize size)
{
    QImage img;

    img=QImage((uchar*)ba_data.data(), size.width(), size.height(), QImage::Format_ARGB32);

    out_widget->frame(img.copy());
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
    cfg.framerate=FFMpeg::Framerate::full_59;
    cfg.frame_resolution=QSize(1920, 1080);
    cfg.crf=le_crf->text().toUInt();

    ffmpeg->setConfig(cfg);
}

void MainWindow::onStopRecording()
{
    ffmpeg->stopCoder();
}
