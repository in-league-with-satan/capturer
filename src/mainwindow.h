#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>

#include "ff_encoder_thread.h"
#include "ff_decoder_thread.h"
#include "ff_encoder_thread_manager.h"

class DeckLinkCapture;
class AudioOutputInterface;
class AudioLevel;
class QmlMessenger;
class OverlayView;
class Server;
class HttpServer;

class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    DeckLinkCapture *decklink_thread;

    // FFEncoderThread *ff_enc;
    FFEncoderThreadManager *ff_enc;
    FFDecoderThread *ff_dec;

    QmlMessenger *messenger;
    OverlayView *overlay_view;

    AudioLevel *audio_level;

    AudioOutputInterface *audio_output;

    QMessageBox *mb_rec_stopped;

    HttpServer *http_server;

    QSize current_frame_size;
    int64_t current_frame_duration;
    int64_t current_frame_scale;

    uint32_t dropped_frames_counter;

    QPoint pos_mouse_press;

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void closeEvent(QCloseEvent *);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

private slots:
    void keyPressed(int code);

    void formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format);

    void settingsModelDataChanged(int index, int role, bool qml);

    void startStopCapture();
    void captureRestart();
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
