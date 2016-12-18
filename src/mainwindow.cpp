#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QPixmap>
#include <QFile>

#include "DeckLinkAPI.h"

#include "device_list.h"
#include "capture.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    capture_thread=new DeckLinkCapture();
    connect(capture_thread, SIGNAL(inputFrameArrived(QByteArray,QByteArray)), SLOT(onInputFrameArrived(QByteArray,QByteArray)), Qt::QueuedConnection);

    cb_device=new QComboBox();
    cb_format=new QComboBox();
    cb_pixel_format=new QComboBox();

    cb_audio_channels=new QComboBox();
    cb_audio_depth=new QComboBox();

    connect(cb_device, SIGNAL(currentIndexChanged(int)), SLOT(onDeviceChanged(int)));
    connect(cb_format, SIGNAL(currentIndexChanged(int)), SLOT(onFormatChanged(int)));
    connect(cb_pixel_format, SIGNAL(currentIndexChanged(int)), SLOT(onPixelFormatChanged(int)));

    QLabel *l_device=new QLabel("device:");
    QLabel *l_format=new QLabel("format:");
    QLabel *l_pixel_format=new QLabel("pixel format:");

    QLabel *l_audio_channels=new QLabel("audio channels:");
    QLabel *l_audio_depth=new QLabel("audio sample depth:");

    QPushButton *b_start=new QPushButton("start");
    QPushButton *b_stop=new QPushButton("stop");

    connect(b_start, SIGNAL(clicked(bool)), capture_thread, SLOT(captureStart()), Qt::QueuedConnection);
    connect(b_stop, SIGNAL(clicked(bool)), capture_thread, SLOT(captureStop()), Qt::QueuedConnection);


    l_out_pic=new QLabel();
    l_out_pic->setScaledContents(true);
//    l_out_pic->setFixedSize(800, 500);

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

    la_dev->addWidget(l_audio_depth, row, 0);
    la_dev->addWidget(cb_audio_depth, row, 1);


    QVBoxLayout *la_h=new QVBoxLayout();

    la_h->addLayout(la_dev);
    la_h->addWidget(b_start);
    la_h->addWidget(b_stop);
    la_h->addWidget(l_out_pic);


    QWidget *w_central=new QWidget();
    w_central->setLayout(la_h);

    setCentralWidget(w_central);

    //

    cb_audio_channels->addItem(QString("2"));
    cb_audio_channels->addItem(QString("8"));
    cb_audio_channels->addItem(QString("16"));

    //

    cb_audio_depth->addItem(QString("16"));
    cb_audio_depth->addItem(QString("32"));

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
}

void MainWindow::onInputFrameArrived(QByteArray ba_video, QByteArray ba_audio)
{
/*
    size_t rows=1080;
    size_t cols=1920;

    QByteArray ba_out;

    ba_out.resize(ba_video.size());

    uint32_t *int_in=(uint32_t*)ba_video.data();
    uint32_t *int_out=(uint32_t*)ba_out.data();

    size_t pos;

    for(size_t i_row=0; i_row<rows; ++i_row) {
        for(size_t i_col=0; i_col<cols; ++i_col) {
            pos=rows*i_row + i_col;

            int_out[pos]=
                    (int_in[pos]&0x3f << 4) | (int_in[pos]&0xf000 >> 12)          // R
                    |
                    (int_in[pos]&0xfc0000 >> 8) | (int_in[pos]&0xf00 << 8)        // G
                    |
                    (int_in[pos]&0xff000000 >> 4) | (int_in[pos]&0x30000 << 12)   // B
                    ;

        }
    }
    */


    QImage img=QImage((uchar*)ba_video.data(), 1920, 1080, QImage::Format_ARGB32);

    l_out_pic->setPixmap(QPixmap::fromImage(img));
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

    setup();
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

    setup();
}

void MainWindow::onPixelFormatChanged(int index)
{
    Q_UNUSED(index);

    setup();
}

void MainWindow::setup()
{
    capture_thread->setup(cb_device->currentData().value<DeckLinkDevice>(),
                          cb_format->currentData().value<DeckLinkFormat>(),
                          cb_pixel_format->currentData().value<DeckLinkPixelFormat>());
}
