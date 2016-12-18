#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QComboBox;
class QLabel;

class DeckLinkCapture;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();


private:
    DeckLinkCapture *capture_thread;

    QComboBox *cb_device;
    QComboBox *cb_format;
    QComboBox *cb_pixel_format;

    QComboBox *cb_audio_channels;
    QComboBox *cb_audio_depth;


    QLabel *l_out_pic;

private slots:
    void onInputFrameArrived(QByteArray ba_video, QByteArray ba_audio);

    void onDeviceChanged(int index);
    void onFormatChanged(int index);
    void onPixelFormatChanged(int index);

    void setup();

};

#endif // MAINWINDOW_H
