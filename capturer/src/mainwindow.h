/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QVariant>

#include "ff_encoder_thread.h"
#include "ff_decoder_thread.h"

class DeckLinkCapture;
class AudioOutputInterface;
class AudioLevel;
class QmlMessenger;
class OverlayView;
class Server;
class HttpServer;
class FFCam;

class QMessageBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

private:
    DeckLinkCapture *decklink_thread;
    FFCam *cam_device;

    FFEncoderThread *ff_enc;
    FFEncoderThread *ff_enc_cam;
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

    FFEncoderBaseFilename enc_base_filename;

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
    void updateEncList();

    void frameSkipped();
    void encoderBufferOverload();

    void previewOnOff();
    void previewCamOnOff();

    void encoderStateChanged(bool state);
    void playerStateChanged(int state);

    void updateStats(FFEncoder::Stats s);
};

#endif // MAINWINDOW_H