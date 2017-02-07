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





    QVBoxLayout *la_main=new QVBoxLayout();

    la_main->addWidget(b_start_rec);
    la_main->addWidget(b_stop_rec);

    QWidget *w_central=new QWidget();

    w_central->setLayout(la_main);

    setCentralWidget(w_central);

    //

    messenger=new QmlMessenger();

    connect(b_start_rec, SIGNAL(clicked(bool)), messenger, SIGNAL(recStarted()));
    connect(b_start_rec, SIGNAL(clicked(bool)), SLOT(onRecStarted()));

    connect(b_stop_rec, SIGNAL(clicked(bool)), messenger, SIGNAL(recStopped()));

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
    // overlay_view.showFullScreen();
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

