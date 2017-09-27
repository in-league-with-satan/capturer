#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUdpSocket>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>

#include "ff_audio_converter.h"

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
};

#endif // MAINWINDOW_H
