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

#include "source_interface.h"


class SourceInterface;
class AudioOutputInterface;
class AudioLevel;
class AudioSender;
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
    void setDevicePrimary(SourceInterface::Type::T type);

    SourceInterface *device_primary=nullptr;

    FFEncoderThread *ff_enc;
    FFEncoderThread *ff_enc_cam;
    FFDecoderThread *ff_dec;

    QmlMessenger *messenger;
    OverlayView *overlay_view;

    AudioLevel *audio_level;

    AudioOutputInterface *audio_output;

    AudioSender *audio_sender;

    QMessageBox *mb_rec_stopped;

    HttpServer *http_server;

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

    void settingsModelDataChanged(int index, int role, bool qml);

    void deviceStart();
    void deviceStop();

    void startStopRecording();
    void updateEncList();

    void frameSkipped();
    void encoderBufferOverload();

    void previewOnOff();
    void previewCamOnOff();

    void encoderStateChanged(bool state);
    void playerStateChanged(int state);

    void updateStats(FFEncoder::Stats s);

    void checkEncoders();
};

#endif // MAINWINDOW_H
