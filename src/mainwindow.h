#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>

#include "ff_encoder_thread.h"
#include "ff_decoder_thread.h"

class DeckLinkCapture;
class AudioOutputInterface;
class VideoWidget;
class Sdl2VideoOutpitThread;
class AudioLevel;
class QmlMessenger;
class OverlayView;

class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    DeckLinkCapture *decklink_thread;

    FFEncoderThread *ff_enc;
    FFDecoderThread *ff_dec;

    QmlMessenger *messenger;
    OverlayView *overlay_view;

    AudioLevel *audio_level;

    AudioOutputInterface *audio_output;

    VideoWidget *out_widget;

    QMessageBox *mb_rec_stopped;

    QSize current_frame_size;
    int64_t current_frame_duration;
    int64_t current_frame_scale;

    uint32_t dropped_frames_counter;

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format);

    void settingsModelDataChanged(int index, int role, bool qml);

    void startStopCapture();
    void captureStart();
    void captureStop();

    void startStopRecording();

    void frameSkipped();
    void encoderBufferOverload();

    void previewOnOff();

    void encoderStateChanged(bool state);
    void playerStateChanged(int state);

    void updateStats(FFEncoder::Stats s);
};

#endif // MAINWINDOW_H
