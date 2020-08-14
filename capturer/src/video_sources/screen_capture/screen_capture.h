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


#ifndef SCREEN_CAPTURE_DDA_H
#define SCREEN_CAPTURE_DDA_H

#include <QObject>
#include <QThread>
#include <QSize>

#include <atomic>

#include "magewell_global.h"
#include "source_interface.h"
#include "frame_buffer.h"

class ScreenCaptureWorkerDda;

class ScreenCapture : public QThread, public SourceInterface
{
    Q_OBJECT
    Q_INTERFACES(SourceInterface)

public:
    struct FramerateLimit {
        enum T {
            l_15,
            l_25,
            l_30,
            l_50,
            l_60,
            l_90,
            l_120,
            l_144,

            size
        };

        static int value(int index) {
            switch(index) {
            case l_15: return 15;
            case l_25: return 25;
            case l_30: return 30;
            case l_50: return 50;
            case l_60: return 60;
            case l_90: return 90;
            case l_120: return 120;
            case l_144: return 144;
            default:
                break;
            }

            return 60;
        }
    };

    struct Device {
        QString audio_device_name;
        FramerateLimit::T framerate_limit;
    };

    explicit ScreenCapture(int device_index, QObject *parent=0);
    ~ScreenCapture();

    QStringList availableAudioInput() const;
    int indexAudioInput(const QString &name);

    Type::T type() const;

    bool isImplemented() const;

    bool isActive();
    bool gotSignal();

    void deviceHold();
    void deviceResume();

    void setDevice(void *ptr);

    void setAudioDevice(QString device_name);
    void setUpperFramerateLimit(ScreenCapture::FramerateLimit::T lim);

protected:
    void run();

private:
    QMutex mutex;
    std::atomic <bool> running;
    std::atomic <bool> on_hold;
    ScreenCaptureWorkerDda *d;

signals:
    void deviceStart();
    void deviceStop();

    void signalLost(bool value);
    void formatChanged(QString format);
    void temperatureChanged(double temperature);
    void errorString(QString err_string);
};

#endif // SCREEN_CAPTURE_DDA_H
