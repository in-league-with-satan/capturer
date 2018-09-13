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

#ifndef FF_SOURCE_H
#define FF_SOURCE_H

#include <QThread>

#include <atomic>

#include "source_interface.h"
#include "pixel_format.h"
#include "ff_tools.h"
#include "frame_buffer.h"

class FFSourceWorker;

class FFSource : public QThread, public SourceInterface
{
    Q_OBJECT
    Q_INTERFACES(SourceInterface)

public:
    FFSource(QObject *parent=0);
    ~FFSource();

    Type::T type() const;

    struct Device {
        QSize size;
        AVRational framerate;
        int pixel_format;
    };

    static QStringList availableCameras();
    static QStringList availableAudioInput();

    static void updateDevList();

    bool setVideoDevice(int index);
    void setAudioDevice(int index);

    void setDevice(void *ptr);

    QList <QSize> supportedResolutions();
    QList <int> supportedPixelFormats(QSize size);
    QList <AVRational> supportedFramerates(QSize size, int fmt);

    void subscribe(FrameBuffer<Frame::ptr>::ptr obj);
    void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj);

    bool isActive();
    bool gotSignal();

    AVRational currentFrameRate();
    PixelFormat pixelFormat();

public slots:

protected:
    void run();

private:
    int index_device_video=0;

    FFSourceWorker *d;

    std::atomic <bool> running;

    QMutex mutex;

signals:
    void setConfig(QSize size, AVRational framerate, int64_t pixel_format);
    void deviceStart();
    void deviceStop();

    void signalLost(bool value);
    void formatChanged(QString format);
    void frameSkipped();
    void errorString(QString err_string);
};

#endif // FF_SOURCE_H