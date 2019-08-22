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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QComboBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QTimer>

#include "ff_audio_converter.h"
#include "audio_level_widget.h"
#include "audio_normalization.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    void load();
    void save();

    QUdpSocket *socket;

    QComboBox *cb_audio_device;
    QComboBox *cb_channels;
    QComboBox *cb_sample_rate;

    QLineEdit *le_host;
    QLineEdit *le_port;

    QCheckBox *cb_normalization;
    QCheckBox *cb_manual_gain_factor;
    QLineEdit *le_norm_update_time;
    QLineEdit *le_norm_gain_change_step;
    QLineEdit *le_norm_level_percentage_maximum;
    QLineEdit *le_norm_gain_factor_maximum;
    QLineEdit *le_norm_gain_factor;

    AudioLevelWidget *level_in;
    AudioLevelWidget *level_out;

    AudioNormalization normalizer;

    QAudioOutput *audio_output;
    QAudioFormat audio_format;

    QIODevice *audio_device;

    QTimer *timer_still_alive;

    AudioConverter audio_converter;

    QVariantMap dev_settings;

    double manual_gain_factor_value=1.;

    struct Dev {
        QVariantMap toExt() {
            QVariantMap map;

            map.insert("channels", channels);
            map.insert("sample_rate", sample_rate);

            return map;
        }

        void fromExt(const QVariantMap &map) {
            channels=map.value("channels", 0).toInt();
            sample_rate=map.value("sample_rate", 0).toInt();
        }

        int channels=0;
        int sample_rate=0;
    };

private slots:
    void updateDeviceList();

    void audioDeviceChanged();
    void channelsChanged();
    void sampleRateChanged();

    void applySettings();

    void socketRead();
    void startAudioDevice();
    void connectToHost();

    void setupNormalizer();
    void normalizerGainFactor(double value);

    void normalizationModeChanged();

    void gainFactorChanged(const QString &value);
};

#endif // MAINWINDOW_H
