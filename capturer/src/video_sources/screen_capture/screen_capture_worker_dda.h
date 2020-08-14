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

#ifndef SCREEN_CAPTURE_WORKER_DDA_H
#define SCREEN_CAPTURE_WORKER_DDA_H

#include <QObject>

#include <atomic>
#include <chrono>

#include <wincodec.h>
#include <windows.h>

#include <d3d9.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <comdef.h>
#include <shlobj.h>
#include <shellapi.h>

#include "source_interface.h"

class AudioWasapi;

class ScreenCaptureWorkerDda : public QObject
{
    Q_OBJECT

public:
    explicit ScreenCaptureWorkerDda(SourceInterface *si, QObject *parent=0);
    ~ScreenCaptureWorkerDda();

    bool step();

    static QString errorString(HRESULT error_code);

    QStringList availableAudioInput();

    void setAudioDevice(QString device_name);

public slots:
    void deviceStart();
    void deviceStop();

private:
    class SourceInterfacePublic : public SourceInterface { friend class ScreenCaptureWorkerDda; } *si=nullptr;

    ID3D11Device *device=nullptr;
    ID3D11DeviceContext *device_context=nullptr;
    IDXGIOutputDuplication *output_duplication=nullptr;
    DXGI_OUTDUPL_DESC output_duplication_description;

    AudioWasapi *audio_wasapi=nullptr;
    Protect <QString> audio_device_name;

    uint64_t frame_duration;
    std::chrono::high_resolution_clock::time_point frame_time_point;

signals:
    void signalLost(bool value);
    void formatChanged(QString format);
    void errorString(QString err_string);
};

#endif // SCREEN_CAPTURE_WORKER_DDA_H
