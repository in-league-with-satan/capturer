#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QFile>

#include "audio_packet.h"
#include "ff_tools.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , audio_output(nullptr)
    , audio_device(nullptr)
{
    av_register_all();


    timer_still_alive=new QTimer(this);
    timer_still_alive->setInterval(1000);

    connect(timer_still_alive, SIGNAL(timeout()), SLOT(connectToHost()));


    socket=new QUdpSocket(this);

    connect(socket, SIGNAL(readyRead()), SLOT(socketRead()));

    if(!socket->bind()) {
        qCritical() << socket->errorString();
        exit(1);
    }

    level_in=new AudioLevelWidget();
    level_out=new AudioLevelWidget();

    cb_audio_device=new QComboBox();

    connect(cb_audio_device, SIGNAL(currentIndexChanged(int)), SLOT(audioDeviceChanged()));


    cb_channels=new QComboBox();
    cb_sample_rate=new QComboBox();

    foreach(QAudioDeviceInfo dev_info, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        cb_audio_device->addItem(dev_info.deviceName(), QVariant::fromValue(dev_info));

    //

    cb_normalization=new QCheckBox(QStringLiteral("level normalization"));
    le_norm_update_time=new QLineEdit();
    le_norm_gain_change_step=new QLineEdit();
    le_norm_maximum_level_percentage=new QLineEdit();
    le_norm_gain_factor=new QLineEdit(QStringLiteral("1.00"));

    le_norm_gain_factor->setReadOnly(true);

    connect(le_norm_update_time, SIGNAL(textChanged(QString)), SLOT(setupNormalizer()));
    connect(le_norm_gain_change_step, SIGNAL(textChanged(QString)), SLOT(setupNormalizer()));
    connect(le_norm_maximum_level_percentage, SIGNAL(textChanged(QString)), SLOT(setupNormalizer()));

    connect(&normalizer, SIGNAL(gainFactorChanged(double)), SLOT(normalizerGainFactor(double)));

    //

    le_host=new QLineEdit();
    le_port=new QLineEdit();

    QPushButton *b_start_device=new QPushButton(QStringLiteral("start device"));
    QPushButton *b_connect=new QPushButton(QStringLiteral("connect"));

    connect(b_start_device, SIGNAL(clicked(bool)), SLOT(startAudioDevice()));
    connect(b_connect, SIGNAL(clicked(bool)), SLOT(connectToHost()));


    QLabel *l_audio_device=new QLabel(QStringLiteral("device:"));
    QLabel *l_channels=new QLabel(QStringLiteral("channels:"));
    QLabel *l_sample_rate=new QLabel(QStringLiteral("sample rate:"));

    QLabel *l_norm_update_time=new QLabel(QStringLiteral("update time:"));
    QLabel *l_norm_gain_change_step=new QLabel(QStringLiteral("gain change step:"));
    QLabel *l_norm_maximum_level_percentage=new QLabel(QStringLiteral("maximum level percentage:"));
    QLabel *l_gain_factor=new QLabel(QStringLiteral("gain factor:"));

    QLabel *l_host=new QLabel(QStringLiteral("host:"));
    QLabel *l_port=new QLabel(QStringLiteral("port:"));
    QLabel *l_in=new QLabel(QStringLiteral("in:"));
    QLabel *l_out=new QLabel(QStringLiteral("out:"));

    QGridLayout *la_controls=new QGridLayout();

    int row=0;

    la_controls->addWidget(l_audio_device, row, 0);
    la_controls->addWidget(cb_audio_device, row++, 1);

    la_controls->addWidget(l_channels, row, 0);
    la_controls->addWidget(cb_channels, row++, 1);

    la_controls->addWidget(l_sample_rate, row, 0);
    la_controls->addWidget(cb_sample_rate, row++, 1);

    la_controls->addWidget(b_start_device, row++, 1);

    QFrame *f_line=new QFrame();
    f_line->setLineWidth(2);
    f_line->setMidLineWidth(1);
    f_line->setFrameShape(QFrame::HLine);
    f_line->setFrameShadow(QFrame::Raised);

    la_controls->addWidget(f_line, row++, 0, 1, 2);

    la_controls->addWidget(l_host, row, 0);
    la_controls->addWidget(le_host, row++, 1);

    la_controls->addWidget(l_port, row, 0);
    la_controls->addWidget(le_port, row++, 1);

    la_controls->addWidget(b_connect, row++, 1);

    f_line=new QFrame();
    f_line->setLineWidth(2);
    f_line->setMidLineWidth(1);
    f_line->setFrameShape(QFrame::HLine);
    f_line->setFrameShadow(QFrame::Raised);

    la_controls->addWidget(f_line, row++, 0, 1, 2);

    la_controls->addWidget(cb_normalization, row++, 1);

    la_controls->addWidget(l_norm_update_time, row, 0);
    la_controls->addWidget(le_norm_update_time, row++, 1);

    la_controls->addWidget(l_norm_gain_change_step, row, 0);
    la_controls->addWidget(le_norm_gain_change_step, row++, 1);

    la_controls->addWidget(l_norm_maximum_level_percentage, row, 0);
    la_controls->addWidget(le_norm_maximum_level_percentage, row++, 1);

    la_controls->addWidget(l_gain_factor, row, 0);
    la_controls->addWidget(le_norm_gain_factor, row++, 1);


    f_line=new QFrame();
    f_line->setLineWidth(2);
    f_line->setMidLineWidth(1);
    f_line->setFrameShape(QFrame::HLine);
    f_line->setFrameShadow(QFrame::Raised);

    la_controls->addWidget(f_line, row++, 0, 1, 2);

    la_controls->addWidget(l_in, row, 0);
    la_controls->addWidget(level_in, row++, 1);

    la_controls->addWidget(l_out, row, 0);
    la_controls->addWidget(level_out, row++, 1);


    QWidget *w_central=new QWidget();

    w_central->setLayout(la_controls);

    setCentralWidget(w_central);


    audio_format.setSampleSize(16);
    audio_format.setCodec(QStringLiteral("audio/pcm"));
    audio_format.setByteOrder(QAudioFormat::LittleEndian);
    audio_format.setSampleType(QAudioFormat::SignedInt);


    load();
}

MainWindow::~MainWindow()
{
    save();
}

void MainWindow::load()
{
    QFile f(qApp->applicationDirPath() + QStringLiteral("/") + qApp->applicationName() + QStringLiteral(".json"));

    f.open(QFile::ReadOnly);

    QVariantMap map_root=QJsonDocument::fromJson(f.readAll()).toVariant().toMap();

    cb_audio_device->setCurrentText(map_root.value(QStringLiteral("audio_device")).toString());

    qApp->processEvents();

    cb_channels->setCurrentText(map_root.value(QStringLiteral("channels"), 2).toString());
    cb_sample_rate->setCurrentText(map_root.value(QStringLiteral("sample_rate"), 48000).toString());


    le_host->setText(map_root.value(QStringLiteral("host"), QStringLiteral("127.0.0.1")).toString());
    le_port->setText(map_root.value(QStringLiteral("port"), QStringLiteral("4142")).toString());

    cb_normalization->setChecked(map_root.value(QStringLiteral("normalization"), true).toBool());
    le_norm_update_time->setText(map_root.value(QStringLiteral("normalization_update_time"), "2000").toString());
    le_norm_gain_change_step->setText(map_root.value(QStringLiteral("normalization_gain_change_step"), "0.5").toString());
    le_norm_maximum_level_percentage->setText(map_root.value(QStringLiteral("normalization_maximum_level_percentage"), "0.9").toString());

    setupNormalizer();
}

void MainWindow::save()
{
    QFile f(qApp->applicationDirPath() + QStringLiteral("/") + qApp->applicationName() + QStringLiteral(".json"));

    f.open(QFile::ReadWrite | QFile::Truncate);

    QVariantMap map_root;

    map_root.insert(QStringLiteral("audio_device"), cb_audio_device->currentText().trimmed());
    map_root.insert(QStringLiteral("channels"), cb_channels->currentText().trimmed().toInt());
    map_root.insert(QStringLiteral("sample_rate"), cb_sample_rate->currentText().trimmed().toInt());
    map_root.insert(QStringLiteral("host"), le_host->text().trimmed());
    map_root.insert(QStringLiteral("port"), le_port->text().trimmed());
    map_root.insert(QStringLiteral("normalization"), cb_normalization->isChecked());
    map_root.insert(QStringLiteral("normalization_update_time"), le_norm_update_time->text().trimmed());
    map_root.insert(QStringLiteral("normalization_gain_change_step"), le_norm_gain_change_step->text().trimmed());
    map_root.insert(QStringLiteral("normalization_maximum_level_percentage"), le_norm_maximum_level_percentage->text().trimmed());

    f.write(QJsonDocument::fromVariant(map_root).toJson());
    f.close();
}

void MainWindow::audioDeviceChanged()
{
   QAudioDeviceInfo dev_info=cb_audio_device->currentData().value<QAudioDeviceInfo>();

   cb_channels->clear();

   foreach(int val, dev_info.supportedChannelCounts()) {
       if(val>8)
           break;

       cb_channels->addItem(QString::number(val));
   }

   cb_sample_rate->clear();

   foreach(int val, dev_info.supportedSampleRates()) {
       cb_sample_rate->addItem(QString::number(val));
   }
}

void MainWindow::socketRead()
{
    AudioPacket packet;

    while(socket->hasPendingDatagrams()) {
        QNetworkDatagram dg=socket->receiveDatagram();

        if(!dg.isValid())
            continue;

        if(!audio_device)
            continue;

        packet.fromExt(QJsonDocument::fromBinaryData(dg.data()).toVariant().toMap());

        if((int)audio_converter.inChannels()!=packet.channels || (int)audio_converter.outChannels()!=audio_format.channelCount()
                || audio_converter.inSampleRate()!=48000 || audio_converter.outSampleRate()!=audio_format.sampleRate()
                || audio_converter.inSampleFormat()!=(packet.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32) || audio_converter.outSampleFormat()!=AV_SAMPLE_FMT_S16)
            audio_converter.init(av_get_default_channel_layout(packet.channels), 48000, (packet.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32),
                                 av_get_default_channel_layout(audio_format.channelCount()), audio_format.sampleRate(), AV_SAMPLE_FMT_S16);


        QByteArray ba_out;

        audio_converter.convert(&packet.data, &ba_out);

        if(cb_normalization->isChecked())
            normalizer.proc(&ba_out, audio_format.channelCount());

        audio_device->write(ba_out);

        level_in->write(packet.data, packet.channels, packet.sample_size);
        level_out->write(ba_out, audio_format.channelCount(), 16);
    }
}

void MainWindow::startAudioDevice()
{
    if(audio_output) {
        audio_output->stop();
        audio_output->deleteLater();

        audio_output=nullptr;
        audio_device=nullptr;
    }

    QAudioDeviceInfo dev_info=cb_audio_device->currentData().value<QAudioDeviceInfo>();

    if(dev_info.isNull())
        return;

    audio_format.setChannelCount(cb_channels->currentText().toInt());
    audio_format.setSampleRate(cb_sample_rate->currentText().toInt());

    audio_output=new QAudioOutput(dev_info, audio_format);

    audio_device=audio_output->start();
}

void MainWindow::connectToHost()
{
    socket->writeDatagram(QByteArray("alive!"), QHostAddress(le_host->text()), le_port->text().toInt());

    if(!timer_still_alive->isActive())
        timer_still_alive->start();
}

void MainWindow::setupNormalizer()
{
    normalizer.setUpdateTime(le_norm_update_time->text().trimmed().toUShort());
    normalizer.setGainChangeStep(le_norm_gain_change_step->text().trimmed().toDouble());
    normalizer.setMaximumLevelPercentage(le_norm_maximum_level_percentage->text().trimmed().toDouble());
}

void MainWindow::normalizerGainFactor(double value)
{
    le_norm_gain_factor->setText(QString::number(value, 'f', 2));
}
