/******************************************************************************

Copyright © 2018-2021 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVariant>

#include "data_types.h"
#include "ff_encoder_thread.h"
#include "ff_decoder_thread.h"
#include "ff_encoder_base_filename.h"
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
class TermGui;
class IrcSubtitles;

class MainWindow : public QObject
{
    Q_OBJECT

    friend class TermGui;

public:
    MainWindow(QObject *parent=0);
    ~MainWindow();

private:
    bool recInProgress();

    struct ObjGrp {
        SourceInterface *source_device=nullptr;
        FFEncoderThread *encoder=nullptr;
        AudioSender *audio_sender=nullptr;
    };

    FFEncoderThread *encoder_streaming=nullptr;

    QList <ObjGrp> stream;

    SettingsModel *settings_model=nullptr;

    FFDecoderThread *ff_dec=nullptr;

    QmlMessenger *messenger=nullptr;
    OverlayView *overlay_view=nullptr;

    AudioLevel *audio_level_primary=nullptr;
    AudioLevel *audio_level_secondary=nullptr;

    AudioOutputInterface *audio_output=nullptr;

    HttpServer *http_server=nullptr;

    NvTools *nv_tools=nullptr;
    QStringList cuda_devices;

    FFEncoderBaseFilename enc_streaming_url;

    FFEncoderBaseFilename enc_base_filename;
    FFEncStartSync enc_start_sync;

    TermGui *term=nullptr;

    IrcSubtitles *irc_subtitles=nullptr;

protected:
    virtual bool eventFilter(QObject *object, QEvent *event);
    virtual void closeEvent(QCloseEvent *);

private slots:
    void keyPressed(int code);

    void settingsModelDataChanged(int index, int role, bool qml);

    void setDevice(uint8_t index, SourceInterface::Type::T type);

    void sourceDeviceAddModel(uint8_t index);

    void sourceDeviceAdd();
    void sourceDeviceRemove();

    void deviceStart(uint8_t index);
    void deviceStop(uint8_t index);

    void startStopRecording();
    void startStopStreaming();

    void updateEncList();

    void encoderBufferOverload();

    void previewPrimaryOnOff();
    void previewSecondaryOnOff();

    void encoderStateChanged(bool state);
    void playerStateChanged(int state);

    void updateStats(FFEncoder::Stats s);

    void checkEncoders();
    void reloadFFDevices();

    void checkFreeSpace();

signals:
    void freeSpaceStr(QString size);
    void freeSpace(qint64 size);
    void signalLost(bool state);
    void recStats(NRecStats stats);
};

#endif // MAINWINDOW_H
