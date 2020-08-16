/******************************************************************************

Copyright © 2020 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef SCREEN_CAPTURE_WORKER_BITBLT_H
#define SCREEN_CAPTURE_WORKER_BITBLT_H

#include <QObject>

#include <atomic>
#include <chrono>

#include <windows.h>

#include "source_interface.h"
#include "screen_capture.h"

class AudioWasapi;

class ScreenCaptureWorkerBitBlt : public QObject, public ScreenCaptureWorkerInterface
{
    Q_OBJECT

public:
    explicit ScreenCaptureWorkerBitBlt(SourceInterface *si, QObject *parent=0);
    ~ScreenCaptureWorkerBitBlt();

    bool step();

    QStringList availableAudioInput();

    void setAudioDevice(QString device_name);

public slots:
    void deviceStart();
    void deviceStop();

private:
    class SourceInterfacePublic : public SourceInterface { friend class ScreenCaptureWorkerBitBlt; } *si=nullptr;

    int screen_width=0;
    int screen_height=0;

    HDC dc_desktop=0;
    HDC dc_capture=0;
    HBITMAP compatible_bitmap=0;

    AudioWasapi *audio_wasapi=nullptr;
    Protect <QString> audio_device_name;

    uint64_t frame_duration;
    std::chrono::high_resolution_clock::time_point frame_time_point;

signals:
    void signalLost(bool value);
    void formatChanged(QString format);
    void errorString(QString err_string);
};

#endif // SCREEN_CAPTURE_WORKER_BITBLT_H
