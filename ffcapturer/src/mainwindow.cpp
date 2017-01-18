#include <QDebug>
#include <QApplication>
#include <QLayout>
#include <QJsonDocument>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

#include <QTextStream>
#include <QDateTime>
#include <QDir>

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    cb_device=new QComboBox();

    cb_mode=new QComboBox();

    connect(cb_device, SIGNAL(currentTextChanged(QString)), SLOT(onDeviceChanged(QString)));


    cb_restart_rec_on_drop_frames=new QCheckBox("restart rec on drop frames");

    le_quality=new QLineEdit("1");

    te_out=new QTextEdit();
    te_out->setReadOnly(true);
    te_out->document()->setMaximumBlockCount(100);

    b_start_stop=new QPushButton("start");
    connect(b_start_stop, SIGNAL(clicked(bool)), SLOT(startStop()));

    QLabel *l_device=new QLabel("device:");
    QLabel *l_mode=new QLabel("mode:");
    QLabel *l_quality=new QLabel("quality:");


    l_status=new QLabel();

    QGridLayout *la_settings=new QGridLayout();

    la_settings->addWidget(l_device, 0, 0);
    la_settings->addWidget(cb_device, 0, 1);

    la_settings->addWidget(l_mode, 1, 0);
    la_settings->addWidget(cb_mode, 1, 1);

    la_settings->addWidget(l_quality, 2, 0);
    la_settings->addWidget(le_quality, 2, 1);


    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addLayout(la_settings);
    la_main->addWidget(cb_restart_rec_on_drop_frames);
    la_main->addWidget(b_start_stop);
    la_main->addWidget(l_status);
    la_main->addWidget(te_out);


    QWidget *w_central=new QWidget();

    w_central->setLayout(la_main);

    setCentralWidget(w_central);


    connect(&proc, SIGNAL(readyReadStandardOutput()), SLOT(readProcStdOutput()));
    connect(&proc, SIGNAL(readyReadStandardError()), SLOT(readProcErrOutput()));
    connect(&proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(readProcStdOutput()));

    init();

    load();

    if(!QDir().exists(qApp->applicationDirPath() + "/videos"))
        QDir().mkpath(qApp->applicationDirPath() + "/videos");
}

MainWindow::~MainWindow()
{
    stopProc();

    save();
}

QString extractName(const QString &str)
{
    int pos_start=str.indexOf("'") + 1;

    if(pos_start<0)
        return str;

    int pos_end=0;
    int pos=-1;

    while(true) {
        pos=str.indexOf("'", pos + 1);

        if(pos==-1)
            break;

        pos_end=pos;
    }

    return str.mid(pos_start, pos_end - pos_start);
}

QString extractName2(const QString &str)
{
    int pos_start=str.indexOf("]") + 1 + 2;

    if(pos_start<2)
        return str;

    return str.mid(pos_start).replace("\t", " ");
}

void MainWindow::init()
{
    QProcess p;

    p.start(qApp->applicationDirPath() + "/ffmpeg -f decklink -list_devices 1 -i dummy");
    p.waitForFinished();

    QByteArray ba=p.readAllStandardError();

    QTextStream stream(&ba);

    QString line;

    while(!stream.atEnd()) {
        line=stream.readLine();

        if(!line.startsWith("[decklink @"))
            continue;

        QString out=
                extractName(line);

        if(!out.isEmpty())
            cb_device->addItem(out);
    }
}

void MainWindow::stopProc()
{
    while(proc.state()==QProcess::Running) {
        proc.write("q");
        proc.waitForFinished(300);
        QApplication::processEvents();
    }
}

void MainWindow::load()
{
    QFile f;

    f.setFileName(qApp->applicationDirPath() + "/" + qApp->applicationName() + ".json");

    if(!f.open(QFile::ReadOnly))
        return;

    QVariantMap map=
            QJsonDocument::fromJson(f.readAll()).toVariant().toMap();

    f.close();

    cb_device->setCurrentIndex(map.value("device", 0).toInt());

    QApplication::processEvents();

    cb_mode->setCurrentIndex(map.value("mode", 0).toInt());

    le_quality->setText(map.value("quality", 1).toString());

    cb_restart_rec_on_drop_frames->setChecked(map.value("restart_rec_on_drop_frames", true).toBool());

    restoreGeometry(QByteArray::fromBase64(map.value("geometry").toByteArray()));
}

void MainWindow::save()
{
    QFile f;

    f.setFileName(qApp->applicationDirPath() + "/" + qApp->applicationName() + ".json");

    if(!f.open(QFile::ReadWrite | QFile::Truncate))
        return;

    QVariantMap map;

    map.insert("device", cb_device->currentIndex());
    map.insert("mode", cb_mode->currentIndex());
    map.insert("quality", le_quality->text().toInt());
    map.insert("restart_rec_on_drop_frames", cb_restart_rec_on_drop_frames->isChecked());
    map.insert("geometry", QString(saveGeometry().toBase64()));

    f.write(QJsonDocument::fromVariant(map).toJson());

    f.close();
}

void MainWindow::onDeviceChanged(const QString &device)
{
    QProcess p;

    p.start(qApp->applicationDirPath() + QString("/ffmpeg -f decklink -list_formats 1 -i \"%1\"").arg(device));
    p.waitForFinished();


    QByteArray ba=p.readAllStandardError();

    QTextStream stream(&ba);

    QString line;

    while(!stream.atEnd()) {
        line=stream.readLine();

        if(line.contains(":"))
            continue;

        if(!line.startsWith("[decklink @"))
            continue;

        QString out=
                extractName2(line);

        if(!out.isEmpty())
            cb_mode->addItem(out);
    }
}

void MainWindow::startStop()
{
    if(!cb_device->count())
        return;

    if(proc.state()==QProcess::Running) {
        stopProc();
        return;
    }


    QString args=QString("ffmpeg -channels 8 -f decklink -i \"%1@%2\" -acodec copy -vcodec h264_nvenc")
            .arg(cb_device->currentText())
            .arg(cb_mode->currentIndex() + 1);


    int quality=le_quality->text().toInt();

    if(quality==0)
        args+=QString(" -preset lossless");

    else
        args+=QString(" -preset fast -global_quality %1").arg(quality);


    args+=QString(" %1/videos/%2.mkv").arg(qApp->applicationDirPath()).arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd_hh-mm-ss"));

    qInfo() << args;

    proc.start(qApp->applicationDirPath() + "/" + args);
}

QString getParam(const QString &name, const QString &src)
{
    QString src_simple=src.simplified();

    int pos_start=src_simple.indexOf(name);

    if(pos_start<0)
        return QString();

    pos_start+=name.size();

    int pos_end=
            src_simple.indexOf(" ", pos_start);

    if(pos_end==pos_start)
        pos_end=
                src_simple.indexOf(" ", pos_start + 1);

    if(pos_end<0)
        return QString();

    return src_simple.mid(pos_start, pos_end - pos_start).trimmed();
}

void MainWindow::readProcStdOutput()
{
    if(proc.state()==QProcess::Running)
        b_start_stop->setText("stop");

    else
        b_start_stop->setText("start");


    QString str=QString(proc.readAllStandardOutput());

    if(str.isEmpty())
        return;

    te_out->setTextColor(Qt::black);
    te_out->append(str);
}

void MainWindow::readProcErrOutput()
{
    QString str=QString(proc.readAllStandardError());

    if(str.isEmpty())
        return;

    QString status;

    if(str.contains("frame="))
        status+="  frame: " + getParam("frame=", str);

    if(str.contains("fps="))
        status+="  fps: " + getParam("fps=", str);

    if(str.contains("q="))
        status+="  q: " + getParam("q=", str);

    if(str.contains("size="))
        status+="  size: " + getParam("size=", str);

    if(str.contains("time="))
        status+="  time: " + getParam("time=", str);

    if(str.contains("bitrate="))
        status+="  bitrate: " + getParam("bitrate=", str);


    if(!status.isEmpty())
        l_status->setText(status);

    te_out->setTextColor(Qt::red);
    te_out->append(str);

    if(cb_restart_rec_on_drop_frames->isChecked()) {
        if(str.contains("Decklink input buffer overrun!", Qt::CaseInsensitive)) {
            startStop();
            startStop();
        }
    }
}

void MainWindow::stateChanged(const QProcess::ProcessState &state)
{
    if(state==QProcess::Running) {
        b_start_stop->setText("stop");

    } else {
        b_start_stop->setText("start");
    }
}
