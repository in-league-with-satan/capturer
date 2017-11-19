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
    QLineEdit *le_norm_update_time;
    QLineEdit *le_norm_gain_change_step;
    QLineEdit *le_norm_maximum_level_percentage;
    QLineEdit *le_norm_gain_factor;

    AudioLevelWidget *level_in;
    AudioLevelWidget *level_out;

    AudioNormalization normalizer;

    QAudioOutput *audio_output;
    QAudioFormat audio_format;

    QIODevice *audio_device;

    QTimer *timer_still_alive;

    AudioConverter audio_converter;

private slots:
    void audioDeviceChanged();
    void socketRead();
    void startAudioDevice();
    void connectToHost();

    void setupNormalizer();
    void normalizerGainFactor(double value);

};

#endif // MAINWINDOW_H
