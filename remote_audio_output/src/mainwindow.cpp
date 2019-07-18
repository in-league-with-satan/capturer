/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QApplication>
#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QNetworkDatagram>
#include <QJsonDocument>
#include <QCborValue>
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

    connect(cb_channels, SIGNAL(currentIndexChanged(int)), SLOT(channelsChanged()));
    connect(cb_sample_rate, SIGNAL(currentIndexChanged(int)), SLOT(sampleRateChanged()));

    //

    cb_normalization=new QCheckBox(QStringLiteral("level normalization"));
    cb_manual_gain_factor=new QCheckBox(QStringLiteral("manual gain factor"));
    le_norm_update_time=new QLineEdit();
    le_norm_gain_change_step=new QLineEdit();
    le_norm_maximum_level_percentage=new QLineEdit();
    le_norm_gain_factor=new QLineEdit(QStringLiteral("1.00"));

    le_norm_gain_factor->setReadOnly(true);

    connect(cb_normalization, SIGNAL(toggled(bool)), SLOT(normalizationModeChanged()));
    connect(cb_manual_gain_factor, SIGNAL(toggled(bool)), SLOT(normalizationModeChanged()));

    connect(le_norm_update_time, SIGNAL(textChanged(QString)), SLOT(setupNormalizer()));
    connect(le_norm_gain_change_step, SIGNAL(textChanged(QString)), SLOT(setupNormalizer()));
    connect(le_norm_maximum_level_percentage, SIGNAL(textChanged(QString)), SLOT(setupNormalizer()));
    connect(le_norm_gain_factor, SIGNAL(textChanged(QString)), SLOT(gainFactorChanged(QString)));

    connect(&normalizer, SIGNAL(gainFactorChanged(double)), SLOT(normalizerGainFactor(double)));

    //

    le_host=new QLineEdit();
    le_port=new QLineEdit();

    QPushButton *b_reload_devices=new QPushButton(QStringLiteral("reload devices"));
    QPushButton *b_start_device=new QPushButton(QStringLiteral("start device"));
    QPushButton *b_connect=new QPushButton(QStringLiteral("connect"));

    connect(b_reload_devices, SIGNAL(clicked(bool)), SLOT(updateDeviceList()));
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

    la_controls->addWidget(b_reload_devices, row++, 1);

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

    QHBoxLayout *la_normalization_cb=new QHBoxLayout();
    la_normalization_cb->addWidget(cb_normalization);
    la_normalization_cb->addWidget(cb_manual_gain_factor);

    la_controls->addWidget(f_line, row++, 0, 1, 2);

    la_controls->addLayout(la_normalization_cb, row++, 1);

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


    updateDeviceList();


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

    dev_settings=map_root.value(QStringLiteral("dev_settings")).toMap();

    le_host->setText(map_root.value(QStringLiteral("host"), QStringLiteral("127.0.0.1")).toString());
    le_port->setText(map_root.value(QStringLiteral("port"), QStringLiteral("4142")).toString());

    cb_normalization->setChecked(map_root.value(QStringLiteral("normalization"), true).toBool());
    le_norm_update_time->setText(map_root.value(QStringLiteral("normalization_update_time"), "2000").toString());
    le_norm_gain_change_step->setText(map_root.value(QStringLiteral("normalization_gain_change_step"), "0.5").toString());
    le_norm_maximum_level_percentage->setText(map_root.value(QStringLiteral("normalization_maximum_level_percentage"), "0.9").toString());

    manual_gain_factor_value=map_root.value(QStringLiteral("manual_gain_factor_value"), "1.0").toDouble();
    cb_manual_gain_factor->setChecked(map_root.value(QStringLiteral("manual_gain_factor"), true).toBool());

    applySettings();

    setupNormalizer();
}

void MainWindow::save()
{
    QFile f(qApp->applicationDirPath() + QStringLiteral("/") + qApp->applicationName() + QStringLiteral(".json"));

    f.open(QFile::ReadWrite | QFile::Truncate);

    QVariantMap map_root;

    map_root.insert(QStringLiteral("audio_device"), cb_audio_device->currentText().trimmed());

    map_root.insert(QStringLiteral("dev_settings"), dev_settings);

    map_root.insert(QStringLiteral("host"), le_host->text().trimmed());
    map_root.insert(QStringLiteral("port"), le_port->text().trimmed());
    map_root.insert(QStringLiteral("normalization"), cb_normalization->isChecked());
    map_root.insert(QStringLiteral("normalization_update_time"), le_norm_update_time->text().trimmed());
    map_root.insert(QStringLiteral("normalization_gain_change_step"), le_norm_gain_change_step->text().trimmed());
    map_root.insert(QStringLiteral("normalization_maximum_level_percentage"), le_norm_maximum_level_percentage->text().trimmed());

    map_root.insert(QStringLiteral("manual_gain_factor_value"), QString::number(manual_gain_factor_value, 'f', 2));
    map_root.insert(QStringLiteral("manual_gain_factor"), cb_manual_gain_factor->isChecked());

    f.write(QJsonDocument::fromVariant(map_root).toJson());
    f.close();
}

void MainWindow::updateDeviceList()
{
    QString current_dev=cb_audio_device->currentText();

    cb_audio_device->clear();

    foreach(QAudioDeviceInfo dev_info, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput))
        cb_audio_device->addItem(dev_info.deviceName(), QVariant::fromValue(dev_info));

    if(cb_audio_device->count()) {
        if(current_dev.isEmpty())
            cb_audio_device->setCurrentIndex(0);

        else
            cb_audio_device->setCurrentText(current_dev);
    }

    applySettings();
}

void MainWindow::audioDeviceChanged()
{
   QAudioDeviceInfo dev_info=cb_audio_device->currentData().value<QAudioDeviceInfo>();

   cb_channels->blockSignals(true);
   cb_sample_rate->blockSignals(true);

   cb_channels->clear();
    cb_sample_rate->clear();


   foreach(int val, dev_info.supportedChannelCounts()) {
       if(val>8)
           break;

       cb_channels->addItem(QString::number(val));
   }

   cb_sample_rate->clear();

   foreach(int val, dev_info.supportedSampleRates()) {
       cb_sample_rate->addItem(QString::number(val));
   }

   cb_channels->blockSignals(false);
   cb_sample_rate->blockSignals(false);

   applySettings();
}

void MainWindow::channelsChanged()
{
    dev_settings[cb_audio_device->currentText()]=Dev({ cb_channels->currentIndex(), cb_sample_rate->currentIndex() }).toExt();
}

void MainWindow::sampleRateChanged()
{
    dev_settings[cb_audio_device->currentText()]=Dev({ cb_channels->currentIndex(), cb_sample_rate->currentIndex() }).toExt();
}

void MainWindow::applySettings()
{
    if(dev_settings.contains(cb_audio_device->currentText())) {
        Dev dev;
        dev.fromExt(dev_settings.value(cb_audio_device->currentText()).toMap());

        cb_channels->setCurrentIndex(dev.channels);
        cb_sample_rate->setCurrentIndex(dev.sample_rate);
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

        packet.fromExt(QCborValue::fromCbor(dg.data()).toVariant().toMap());

        if((int)audio_converter.inChannels()!=packet.channels || (int)audio_converter.outChannels()!=audio_format.channelCount()
                || audio_converter.inSampleRate()!=48000 || audio_converter.outSampleRate()!=audio_format.sampleRate()
                || audio_converter.inSampleFormat()!=(packet.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32) || audio_converter.outSampleFormat()!=AV_SAMPLE_FMT_S16)
            audio_converter.init(av_get_default_channel_layout(packet.channels), 48000, (packet.sample_size==16 ? AV_SAMPLE_FMT_S16 : AV_SAMPLE_FMT_S32),
                                 av_get_default_channel_layout(audio_format.channelCount()), audio_format.sampleRate(), AV_SAMPLE_FMT_S16);


        QByteArray ba_out;

        audio_converter.convert(&packet.data, &ba_out);

        if(cb_normalization->isChecked())
            normalizer.proc(&ba_out, audio_format.channelCount());

        if(cb_manual_gain_factor->isChecked())
            normalizer.proc(&ba_out, audio_format.channelCount(), true);

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

void MainWindow::normalizationModeChanged()
{
    cb_normalization->blockSignals(true);
    cb_manual_gain_factor->blockSignals(true);

    if(sender()==cb_normalization) {
        if(cb_normalization->isChecked()) {
            if(cb_manual_gain_factor->isChecked()) {
                manual_gain_factor_value=le_norm_gain_factor->text().toDouble();
            }

            cb_manual_gain_factor->setChecked(false);

            le_norm_update_time->setEnabled(true);
            le_norm_gain_change_step->setEnabled(true);
            le_norm_maximum_level_percentage->setEnabled(true);
            le_norm_gain_factor->setEnabled(true);

            le_norm_gain_factor->setReadOnly(true);

        } else {
            le_norm_update_time->setEnabled(false);
            le_norm_gain_change_step->setEnabled(false);
            le_norm_maximum_level_percentage->setEnabled(false);
            le_norm_gain_factor->setEnabled(false);

        }
    }

    if(sender()==cb_manual_gain_factor) {
        if(cb_manual_gain_factor->isChecked()) {
            le_norm_gain_factor->setText(QString::number(manual_gain_factor_value, 'f', 2));
            normalizer.setGainFactor(manual_gain_factor_value);

            cb_normalization->setChecked(false);

            le_norm_update_time->setEnabled(false);
            le_norm_gain_change_step->setEnabled(false);
            le_norm_maximum_level_percentage->setEnabled(false);
            le_norm_gain_factor->setEnabled(true);

            le_norm_gain_factor->setReadOnly(false);

        } else {
            le_norm_update_time->setEnabled(false);
            le_norm_gain_change_step->setEnabled(false);
            le_norm_maximum_level_percentage->setEnabled(false);
            le_norm_gain_factor->setEnabled(false);

        }
    }

    cb_normalization->blockSignals(false);
    cb_manual_gain_factor->blockSignals(false);
}

void MainWindow::gainFactorChanged(const QString &value)
{
    if(!cb_manual_gain_factor->isChecked())
        return;

    bool ok;

    double double_value=value.toDouble(&ok);

    if(!ok)
        le_norm_gain_factor->setText(QString::number(manual_gain_factor_value, 'f', 2));

    else {
        manual_gain_factor_value=double_value;

        normalizer.setGainFactor(manual_gain_factor_value);
    }
}
