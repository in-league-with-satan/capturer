/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
    explicit MagewellDevice(int device_index, QObject *parent=0);
    ~MagewellDevice();

    Type::T type() const;

    bool isImplemented() const;

    static void init();
    static void release();

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

        struct AudioRemapMode {
            enum {
                disabled,
                sides_drop,
                sides_to_rear,
                size
            };

            static QString toString(int value) {
                switch(value) {
                case disabled: return QStringLiteral("disabled");
                case sides_drop: return QStringLiteral("sides drop");
                case sides_to_rear: return QStringLiteral("sides to rear");
                }
                return QStringLiteral("unknown");
            }
        };

        struct PtsMode {
            enum {
                disabled,
                device,
                audio,

                size
            };

            static QString toString(int value) {
                switch(value) {
                case disabled: return QStringLiteral("disabled");
                case device: return QStringLiteral("device");
                case audio: return QStringLiteral("audio");
                }
                return QStringLiteral("unknown");
            }
        };

        ColorFormat::T color_format_in=ColorFormat::unknown;
        ColorFormat::T color_format_out=ColorFormat::unknown;
        QuantizationRange::T quantization_range_in=QuantizationRange::unknown;
        QuantizationRange::T quantization_range_out=QuantizationRange::unknown;

        QString name;
        QString path;
        int index_channel;
        int index_board;
        int audio_remap_mode;
        int pts_mode;
        PixelFormat pixel_format;
        bool half_fps=false;
        bool low_latency=false;
        QSize framesize;
    };

    typedef QList <Device> Devices;

    static Devices availableDevices();
    static PixelFormats supportedPixelFormats();

    bool isActive();

public slots:
    void subscribe(FrameBuffer<Frame::ptr>::ptr obj);
    void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj);

    void setDevice(void *ptr);

    void deviceHold();
    void deviceResume();

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

    std::atomic <bool> on_hold;

signals:
    void deviceStart();
    void deviceStop();

    void updateDevice(MagewellDevice::Device dev);

    //

    void signalLost(bool value);
    void formatChanged(QString format);
    void errorString(QString err_string);
};

Q_DECLARE_METATYPE(MagewellDevice::Device)
Q_DECLARE_METATYPE(MagewellDevice::Devices)

#endif // MAGEWELL_DEVICE_H
