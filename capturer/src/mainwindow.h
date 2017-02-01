#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "ffmpeg_thread.h"

class DeckLinkCapture;
class AudioOutputInterface;
class OutWidget;
class Sdl2VideoOutpitThread;
class AudioLevelWidget;

class QComboBox;
class QLabel;
class QLineEdit;
class QCheckBox;
class QMessageBox;

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

    QLineEdit *le_video_mode;

    QComboBox *cb_audio_channels;
    QLineEdit *le_audio_delay;

    QComboBox *cb_rec_pixel_format;
    QComboBox *cb_video_encoder;

    QLineEdit *le_crf;

    QLineEdit *le_stat_size;
    QLineEdit *le_stat_br;
    QLineEdit *le_stat_time;

    QCheckBox *cb_preview;
    QCheckBox *cb_stop_rec_on_frames_drop;
    QCheckBox *cb_half_fps;

    AudioLevelWidget *audio_level;

    AudioOutputInterface *audio_output;

    OutWidget *out_widget;

    Sdl2VideoOutpitThread *out_widget_2;

    QMessageBox *mb_rec_stopped;

    QSize current_frame_size;
    int64_t current_frame_duration;
    int64_t current_frame_scale;

private slots:
    void onFormatChanged(QSize size, int64_t frame_duration, int64_t frame_scale);

    void onDeviceChanged(int index);
    void onFormatChanged(int index);
    void onPixelFormatChanged(int index);

    void onStartCapture();

    void onStartRecording();
    void onStopRecording();

    void onFrameSkipped();

    void onPreviewChanged(int state);

    void updateStats(FFMpeg::Stats s);
};

#endif // MAINWINDOW_H
