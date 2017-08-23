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


enum {
    enc_nv_h264,
    enc_nv_hevc,
    enc_ffvhuff
};

QString presetVisualNameToParamName(const QString &str)
{
    if(str==QLatin1String("default"))
        return QLatin1String("default");

    if(str==QLatin1String("slow"))
        return QLatin1String("slow");

    if(str==QLatin1String("medium"))
        return QLatin1String("medium");

    if(str==QLatin1String("fast"))
        return QLatin1String("fast");

    if(str==QLatin1String("high quality"))
        return QLatin1String("hq");

    if(str==QLatin1String("high performance"))
        return QLatin1String("hp");

    if(str==QLatin1String("bluray disk"))
        return QLatin1String("bd");

    if(str==QLatin1String("low latency"))
        return QLatin1String("ll");

    if(str==QLatin1String("low latency high quality"))
        return QLatin1String("llhq");

    if(str==QLatin1String("low latency high performance"))
        return QLatin1String("llhp");

    return str;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    cb_device=new QComboBox();

    cb_input_format=new QComboBox();

    cb_encoder=new QComboBox();

    cb_preset=new QComboBox();

    cb_pixel_format=new QComboBox();


    connect(cb_device, SIGNAL(currentTextChanged(QString)), SLOT(onDeviceChanged(QString)));
    connect(cb_encoder, SIGNAL(currentIndexChanged(int)), SLOT(onEncoderChanged(int)));
    connect(cb_preset, SIGNAL(currentIndexChanged(int)), SLOT(onPresetChanged(int)));
    connect(cb_pixel_format, SIGNAL(currentIndexChanged(int)), SLOT(onPixelFormatChanged(int)));



    cb_restart_rec_on_drop_frames=new QCheckBox("restart rec on drop frames");

    cb_half_fps=new QCheckBox("half-fps");


    le_quality=new QLineEdit("1");

    te_out=new QTextEdit();
    te_out->setReadOnly(true);
    te_out->document()->setMaximumBlockCount(100);

    b_start_stop=new QPushButton("start");
    connect(b_start_stop, SIGNAL(clicked(bool)), SLOT(startStop()));

    QPushButton *b_export_cmd=new QPushButton("export cmd");
    connect(b_export_cmd, SIGNAL(clicked(bool)), SLOT(exportCmd()));

    QLabel *l_device=new QLabel("device:");
    QLabel *l_input_format=new QLabel("input format:");
    QLabel *l_encoder=new QLabel("encoder:");
    QLabel *l_preset=new QLabel("preset:");
    QLabel *l_pixel_format=new QLabel("pixel format:");
    QLabel *l_quality=new QLabel("quality:");


    l_status=new QLabel();

    QGridLayout *la_settings=new QGridLayout();

    int line=0;

    la_settings->addWidget(l_device, line, 0);
    la_settings->addWidget(cb_device, line, 1);

    line++;

    la_settings->addWidget(l_input_format, line, 0);
    la_settings->addWidget(cb_input_format, line, 1);

    line++;

    la_settings->addWidget(l_encoder, line, 0);
    la_settings->addWidget(cb_encoder, line, 1);

    line++;

    la_settings->addWidget(l_preset, line, 0);
    la_settings->addWidget(cb_preset, line, 1);

    line++;

    la_settings->addWidget(l_pixel_format, line, 0);
    la_settings->addWidget(cb_pixel_format, line, 1);

    line++;

    la_settings->addWidget(l_quality, line, 0);
    la_settings->addWidget(le_quality, line, 1);


    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addLayout(la_settings);
    la_main->addWidget(cb_restart_rec_on_drop_frames);
    la_main->addWidget(cb_half_fps);
    la_main->addWidget(b_start_stop);
    la_main->addWidget(b_export_cmd);
    la_main->addWidget(l_status);
    la_main->addWidget(te_out);


    QWidget *w_central=new QWidget();

    w_central->setLayout(la_main);

    setCentralWidget(w_central);


    connect(&proc, SIGNAL(readyReadStandardOutput()), SLOT(readProcStdOutput()));
    connect(&proc, SIGNAL(readyReadStandardError()), SLOT(readProcErrOutput()));
    connect(&proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(readProcStdOutput()));



    cb_encoder->addItem("h264_nvenc");
    cb_encoder->addItem("hevc_nvenc");
    cb_encoder->addItem("ffvhuff");


    cb_preset->addItems(QStringList()
                        << "default" << "slow" << "medium" << "fast"
                        << "high quality" << "high performance" << "bluray disk"
                        << "low latency" << "low latency high quality" << "low latency high performance");


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
    int pos_start=str.indexOf("\t") + 1;

    if(pos_start<1)
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
        qApp->processEvents();
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

    map_preset=map.value("preset").toMap();

    map_pixel_format=map.value("pixel_format").toMap();

    cb_device->setCurrentIndex(map.value("device", 0).toInt());

    cb_encoder->setCurrentIndex(map.value("encoder", 0).toInt());

    onEncoderChanged(cb_encoder->currentIndex());

    qApp->processEvents();

    cb_input_format->setCurrentIndex(map.value("input_format", 0).toInt());

    le_quality->setText(map.value("quality", 1).toString());

    cb_restart_rec_on_drop_frames->setChecked(map.value("restart_rec_on_drop_frames", true).toBool());

    cb_half_fps->setChecked(map.value("half_fps", false).toBool());

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
    map.insert("input_format", cb_input_format->currentIndex());
    map.insert("encoder", cb_encoder->currentIndex());
    map.insert("preset", map_preset);
    map.insert("pixel_format", map_pixel_format);
    map.insert("quality", le_quality->text().toInt());
    map.insert("restart_rec_on_drop_frames", cb_restart_rec_on_drop_frames->isChecked());
    map.insert("half_fps", cb_half_fps->isChecked());

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

        if(!line.startsWith("[decklink @") && !line.startsWith("\t"))
            continue;

        if(line.contains("format_code") || line.contains("description"))
            continue;

        QString out=
                extractName2(line);

        if(!out.isEmpty())
            cb_input_format->addItem(out);
    }
}

void MainWindow::onEncoderChanged(int index)
{
    cb_pixel_format->blockSignals(true);

    if(index==enc_nv_h264) {
        le_quality->setEnabled(true);
        cb_preset->setEnabled(true);

        cb_pixel_format->clear();
        cb_pixel_format->addItems(QStringList() << "yuv420p" << "yuv444p");

    } else if(index==enc_nv_hevc) {
        le_quality->setEnabled(true);
        cb_preset->setEnabled(true);

        cb_pixel_format->clear();
        cb_pixel_format->addItems(QStringList() << "yuv420p");

    } else {
        le_quality->setEnabled(false);
        cb_preset->setEnabled(false);

        cb_pixel_format->clear();
        cb_pixel_format->addItems(QStringList() << "rgb24" << "yuv420p" << "yuv422p" << "yuv444p");
    }

    cb_pixel_format->blockSignals(false);

    cb_preset->setCurrentIndex(map_preset.value(QString::number(index), 0).toInt());
    cb_pixel_format->setCurrentIndex(map_pixel_format.value(QString::number(index), 0).toInt());
}

void MainWindow::onPresetChanged(int index)
{
    map_preset.insert(QString::number(cb_encoder->currentIndex()), index);
}

void MainWindow::onPixelFormatChanged(int index)
{
    map_pixel_format.insert(QString::number(cb_encoder->currentIndex()), index);
}

void MainWindow::exportCmd()
{
    QFile f;

    f.setFileName(qApp->applicationDirPath() + "/" + qApp->applicationName() + ".sh");

    if(!f.open(QFile::ReadWrite | QFile::Text | QFile::Truncate))
        return;


    QString args=buildArgs();

    QTextStream ts(&f);

    ts << "#!/bin/bash";
    ts << "\n";
    ts << "\n";
    ts << "DATE=`date +%Y-%m-%d_%H-%M-%S`";
    ts << "\n";
    ts << "\n";
    ts << "./" << args << " " << "videos/$DATE.mkv";
    ts << "\n";

    f.close();
}

QString MainWindow::buildArgs()
{
    QString args=QString("ffmpeg -rtbufsize 2000M -format_code %2 -channels 8 -f decklink -i \"%1\" -acodec pcm_s16le -af pan=7.1|c0=c0|c1=c1|c2=c3|c3=c2|c4=c6|c5=c7|c6=c4|c7=c5")
            .arg(cb_device->currentText())
            .arg(cb_input_format->currentText().split(" ").value(0, "null"));


    args+= " -vcodec " + cb_encoder->currentText();

    args+= " -pix_fmt " + cb_pixel_format->currentText();


    if(cb_half_fps->isChecked()) {
        foreach(QString str, cb_input_format->currentText().split(" ")) {
            if(str.contains("/")) {
                QStringList sl=str.split("/");
                args+=QString(" -r %1/%2").arg(sl[0].toInt()*.5).arg(sl[1]);
                break;
            }
        }
    }


    if(cb_encoder->currentIndex()!=enc_ffvhuff) {
        int quality=le_quality->text().toInt();

        if(quality==0 && cb_encoder->currentIndex()==enc_nv_h264)
            args+=QString(" -preset lossless");

        else
            args+=QString(" -preset %1 -qp %2 -threads 0")
                    .arg(presetVisualNameToParamName(cb_preset->currentText()))
                    .arg(quality);
    }

    return args;
}

void MainWindow::startStop()
{
    if(!cb_device->count())
        return;

    if(proc.state()==QProcess::Running) {
        stopProc();
        return;
    }


    QString args=buildArgs();


    if(cb_encoder->currentIndex()!=enc_ffvhuff) {
        int quality=le_quality->text().toInt();

        args+=QString(" %1/videos/%2_q%3.mkv")
                .arg(qApp->applicationDirPath())
                .arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd_hh-mm-ss"))
                .arg(quality);

    } else {
        args+=QString(" %1/videos/%2.mkv")
                .arg(qApp->applicationDirPath())
                .arg(QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd_hh-mm-ss"));
    }

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
