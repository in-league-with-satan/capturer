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

#ifndef MAGEWELL_DEVICE_H
#define MAGEWELL_DEVICE_H

#include <QObject>
#include <QThread>
#include <QSize>

#include <atomic>

#include "magewell_global.h"
#include "source_interface.h"
#include "frame_buffer.h"


class MagewellDeviceWorker;

class MagewellDevice : public QThread, public SourceInterface
{
    Q_OBJECT
    Q_INTERFACES(SourceInterface)

public:
    explicit MagewellDevice(QObject *parent=0);
    ~MagewellDevice();

    Type::T type() const;

    static void init();

    struct Device {
        struct ColorFormat {
            enum T {
                unknown,
                rgb,
                bt601,
                bt709,
                bt2020,
                size
            };

            static QString toString(int value) {
                switch(value) {
                case rgb: return QStringLiteral("rgb");
                case bt601: return QStringLiteral("bt601");
                case bt709: return QStringLiteral("bt709");
                case bt2020: return QStringLiteral("bt2020");
                }
                return QStringLiteral("unknown");
            }
        };

        struct QuantizationRange {
            enum T {
                unknown,
                full,
                limited,
                size
            };

            static QString toString(int value) {
                switch(value) {
                case full: return QStringLiteral("full");
                case limited: return QStringLiteral("limited");
                }
                return QStringLiteral("unknown");
            }
        };

        ColorFormat::T color_format=ColorFormat::unknown;
        QuantizationRange::T quantization_range=QuantizationRange::unknown;

        QString name;
        QString path;
        int index_channel;
        int index_board;
        PixelFormat pixel_format;
        bool pts_enabled=false;
    };

    typedef QList <Device> Devices;

    static Devices availableDevices();
    static PixelFormats supportedPixelFormats();

    bool isActive();

public slots:
    void subscribe(FrameBuffer<Frame::ptr>::ptr obj);
    void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj);

    void setDevice(void *ptr);

private slots:
    void setFramerate(AVRational fr);
    void setFramesize(QSize r);

    void setAudioSampleSize(SourceInterface::AudioSampleSize::T value);
    void setAudioChannels(SourceInterface::AudioChannels::T value);

protected:
    virtual void run();

private:
    Device device;

    MagewellDeviceWorker *d;

    QMutex mutex;

protected:
    std::atomic <bool> running;

signals:
    void deviceStart();
    void deviceStop();
    void setDevice(QSize board_index);
    void setPixelFormat(PixelFormat fmt);
    void setColorFormat(int value);
    void setQuantizationRange(int value);
    void setPtsEnabled(bool value);

    //

    void signalLost(bool value);
    void formatChanged(QString format);
    void frameSkipped();
    void errorString(QString err_string);
};

Q_DECLARE_METATYPE(MagewellDevice::Device)
Q_DECLARE_METATYPE(MagewellDevice::Devices)

#endif // MAGEWELL_DEVICE_H
