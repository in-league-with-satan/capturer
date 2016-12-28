#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QComboBox;
class QLabel;
class QLineEdit;

class DeckLinkCapture;
class AudioOutputThread;
class OutWidget;
class FFMpegThread;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    DeckLinkCapture *decklink_thread;

    FFMpegThread *ffmpeg;

    QComboBox *cb_device;
    QComboBox *cb_format;
    QComboBox *cb_pixel_format;

    QComboBox *cb_audio_channels;

    QComboBox *cb_rec_fps;
    QComboBox *cb_rec_pixel_format;

    QLineEdit *le_crf;

    AudioOutputThread *audio_output;

    OutWidget *out_widget;

    QSize last_frame_size;

private slots:
    void onFrameVideo(QByteArray ba_data, QSize size);

    void onFormatChanged(QSize size, int64_t frame_duration, int64_t frame_scale);

    void onDeviceChanged(int index);
    void onFormatChanged(int index);
    void onPixelFormatChanged(int index);

    void onStartCapture();

    void onStartRecording();
    void onStopRecording();
};

#endif // MAINWINDOW_H
