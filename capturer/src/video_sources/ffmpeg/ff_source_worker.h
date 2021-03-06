/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef FF_SOURCE_WORKER_H
#define FF_SOURCE_WORKER_H

#include <QAudioFormat>

#include "source_interface.h"
#include "tools_ff_source.h"
#include "pixel_format.h"
#include "ff_tools.h"
#include "frame_buffer.h"

class FFSourceContext;
class FFSource;

class FFSourceWorker : public QObject
{
    Q_OBJECT

public:
    FFSourceWorker(FFSource *parent_interface, QObject *parent=0);
    ~FFSourceWorker();

    static QString formatString(const QAudioFormat &format);
    static AVSampleFormat qAudioFormatToAV(const int &depth, const QAudioFormat::SampleType &sample_format);

    void setVideoDevice(FFDevice::Dev video_device);
    void setAudioDevice(int index);

    void subscribe(FrameBuffer<Frame::ptr>::ptr obj);
    void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj);

    bool isActive();

    AVRational currentFrameRate() const;
    PixelFormat pixelFormat() const;

    bool step();

public slots:
    void setConfig(QSize size, AVRational framerate, int64_t pixel_format, bool high_depth_audio);
    void deviceStart();
    void deviceStop();

    void deviceHold();
    void deviceResume();

private:
    struct Cfg {
        QSize size;
        AVRational framerate;
        PixelFormat pixel_format;
        bool high_depth_audio;

    } cfg;

    QList <FrameBuffer<Frame::ptr>::ptr> subscription_list;

    int index_device_audio=0;

    QAudioFormat default_format;

    FFDevice::Dev video_device;

    FFSourceContext *d;
    FFSource *parent_interface;

    bool on_hold=false;

signals:
    void signalLost(bool value);
    void formatChanged(QString format);
    void errorString(QString err_string);
    void sampleSizeChanged(int value);
};

#endif // FF_SOURCE_WORKER_H
