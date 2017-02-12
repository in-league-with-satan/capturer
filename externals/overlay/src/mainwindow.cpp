#include <QDebug>
#include <QApplication>
#include <QLayout>
#include <QTimer>
#include <QLabel>
#include <QPushButton>

#include "qml_messenger.h"
#include "overlay_view.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    rec_progress_timer=new QTimer();

    rec_progress_timer->setInterval(1000);

    connect(rec_progress_timer, SIGNAL(timeout()), SLOT(onRecProgressTimer()));


    QPushButton *b_start_rec=new QPushButton("start rec");
    QPushButton *b_stop_rec=new QPushButton("stop rec");
    QPushButton *b_menu=new QPushButton("menu");
    QPushButton *b_back=new QPushButton("back");

    QPushButton *b_up=new QPushButton("up");
    QPushButton *b_down=new QPushButton("down");
    QPushButton *b_left=new QPushButton("left");
    QPushButton *b_right=new QPushButton("right");

    QGridLayout *la_arrows=new QGridLayout();
    la_arrows->addWidget(b_up, 0, 1);
    la_arrows->addWidget(b_down, 2, 1);
    la_arrows->addWidget(b_left, 1, 0);
    la_arrows->addWidget(b_right, 1, 2);


    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(b_start_rec);
    la_main->addWidget(b_stop_rec);
    la_main->addWidget(b_menu);
    la_main->addWidget(b_back);
    la_main->addLayout(la_arrows);
    la_main->addStretch(1);


    QWidget *w_central=new QWidget();

    w_central->setLayout(la_main);

    setCentralWidget(w_central);

    //

    messenger=new QmlMessenger();

    connect(b_start_rec, SIGNAL(clicked(bool)), messenger, SIGNAL(recStarted()));
    connect(b_start_rec, SIGNAL(clicked(bool)), SLOT(onRecStarted()));

    connect(b_stop_rec, SIGNAL(clicked(bool)), messenger, SIGNAL(recStopped()));

    connect(b_menu, SIGNAL(clicked(bool)), messenger, SIGNAL(showMenu()));

    connect(b_back, SIGNAL(clicked(bool)), messenger, SIGNAL(back()));

    connect(b_up, SIGNAL(clicked(bool)), SLOT(keyUp()));
    connect(b_down, SIGNAL(clicked(bool)), SLOT(keyDown()));
    connect(b_left, SIGNAL(clicked(bool)), SLOT(keyLeft()));
    connect(b_right, SIGNAL(clicked(bool)), SLOT(keyRight()));

    OverlayView *overlay_view=new OverlayView();

    overlay_view->setMessenger(messenger);

/*
#ifdef QT_DEBUG

    if(QFile::exists("qml/Root.qml"))
        overlay_view.setSource(QStringLiteral("qml/Root.qml"));

    else if(QFile::exists("../qml/Root.qml"))
        overlay_view.setSource(QStringLiteral("../qml/Root.qml"));

    else {
        qCritical() << "qml path not found";
        exit(1);
    }

#else
*/
    overlay_view->setSource(QStringLiteral("qrc:/qml/Root.qml"));

// #endif


    overlay_view->show();
    overlay_view->resize(overlay_view->size() - QSize(1, 1));
    // overlay_view.showFullScreen();

    //

    messenger->setModelVideoEncoder(QStringList() << "libx264" << "libx264rgb" << "nvenc_h264" << "nvenc_hevc");

    messenger->setModelPixelFormat(QStringList() << "yuv420p" << "yuv444p" << "yuv422p10" << "rgb");

    messenger->videoEncoderIndexSet(2);

    messenger->crfSet(4);

    messenger->halfFpsSet(true);

    messenger->stopOnDropSet(true);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent*)
{
    QApplication::exit();
}

void MainWindow::onRecStarted()
{
    rec_progress_start_point=QDateTime::currentDateTimeUtc();
    rec_progress_size=0;

    rec_progress_timer->start();

    onRecProgressTimer();
}

void MainWindow::onRecProgressTimer()
{
    messenger->recStats(QDateTime::fromMSecsSinceEpoch(QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() - rec_progress_start_point.toMSecsSinceEpoch()).toUTC().toString("HH:mm:ss.zzz"),
                        QString::number(48*1024 + qrand()%128*1024),
                        QString::number(rec_progress_size));

    rec_progress_size+=qrand()%128*1024;
}

void MainWindow::keyUp()
{
    messenger->keyPressed(Qt::Key_Up);
}

void MainWindow::keyDown()
{
    messenger->keyPressed(Qt::Key_Down);
}

void MainWindow::keyLeft()
{
    messenger->keyPressed(Qt::Key_Left);
}

void MainWindow::keyRight()
{
    messenger->keyPressed(Qt::Key_Right);
}

