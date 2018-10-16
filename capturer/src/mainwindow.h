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
class SettingsModel;
class NvTools;

class MainWindow : public QObject
{
    Q_OBJECT

public:
    MainWindow(QObject *parent=0);
    ~MainWindow();

private:
    void setDevicePrimary(SourceInterface::Type::T type);
    void setDeviceSecondary(SourceInterface::Type::T type);
    void setDevice(bool primary, SourceInterface::Type::T type);

    SourceInterface *device_primary=nullptr;
    SourceInterface *device_secondary=nullptr;

    SettingsModel *settings_model;

    FFEncoderThread *ff_enc_primary;
    FFEncoderThread *ff_enc_secondary;

    FFDecoderThread *ff_dec;

    QmlMessenger *messenger;
    OverlayView *overlay_view;

    AudioLevel *audio_level_primary;
    AudioLevel *audio_level_secondary;

    AudioOutputInterface *audio_output;

    AudioSender *audio_sender;

    HttpServer *http_server;

    NvTools *nv_tools;

    FFEncoderBaseFilename enc_base_filename;

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void keyPressed(int code);

    void settingsModelDataChanged(int index, int role, bool qml);

    void deviceStart(bool primary);
    void deviceStop(bool primary);

    void startStopRecording();
    void updateEncList();

    void encoderBufferOverload();

    void previewPrimaryOnOff();
    void previewSecondaryOnOff();

    void encoderStateChanged(bool state);
    void playerStateChanged(int state);

    void updateStats(FFEncoder::Stats s);

    void checkEncoders();

    void checkFreeSpace();

signals:
    void freeSpaceStr(QString size);
    void freeSpace(qint64 size);
    void signalLost(bool state);
};

#endif // MAINWINDOW_H
