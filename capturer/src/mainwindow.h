#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>

#include "ffmpeg_thread.h"

class DeckLinkCapture;
class AudioOutputInterface;
class OutWidget2;
class Sdl2VideoOutpitThread;
class AudioLevelWidget;
class QmlMessenger;
class OverlayView;

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

    QmlMessenger *messenger;
    OverlayView *overlay_view;


    QComboBox *cb_device;

    QLineEdit *le_video_mode;

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

    OutWidget2 *out_widget;

    QMessageBox *mb_rec_stopped;

    QSize current_frame_size;
    int64_t current_frame_duration;
    int64_t current_frame_scale;

    QVariantMap map_pixel_format;

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void load();
    void save();

    void onEncoderChanged(const int &index);
    void onPixelFormatChanged(const int &index);

    void onFormatChanged(QSize size, int64_t frame_duration, int64_t frame_scale);

    void onCrfChanged(const QString &text);
    void onCrfChanged(const int &crf);

    void onStartCapture();

    void onStartStopRecording();

    void onFrameSkipped();
    void onEncBufferOverload();

    void onPreviewChanged(int state);

    void updateStats(FFMpeg::Stats s);
};

#endif // MAINWINDOW_H
