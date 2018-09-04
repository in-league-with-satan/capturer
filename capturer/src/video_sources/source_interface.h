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

#ifndef SOURCE_INTERFACE_H
#define SOURCE_INTERFACE_H

#include <QDebug>
#include <QObject>
#include <QSize>
#include <QMutex>
#include <atomic>

#include "frame_buffer.h"
#include "ff_tools.h"

class SourceInterface
{
public:
    struct Type {
        enum T {
            disabled,
            dummy,
            ffmpeg,
            magewell,
            decklink,

            size
        };
    };

    struct AudioSampleSize {
        enum T {
            bitdepth_null=0,
            bitdepth_16=16,
            bitdepth_32=32
        };
    };

    struct AudioChannels {
        enum T {
            ch_2=2,
            ch_6=6,
            ch_8=8
        };
    };

    SourceInterface() {
        pixel_format=PixelFormat::undefined;
        framerate={ 60000, 1000 };
        framesize=QSize(640, 480);
        half_fps=false;

        audio_sample_size=AudioSampleSize::bitdepth_null;
        audio_channels=AudioChannels::ch_8;
    }

    virtual ~SourceInterface() {}

    virtual Type::T type() const {
        return Type::disabled;
    }

    bool isImplemented() const {
        return isImplemented(type());
    }

    static bool isImplemented(int type) {
        switch(type) {
        case Type::disabled:
            return true;

        case Type::dummy:
            return true;

        case Type::ffmpeg:
            return true;

        case Type::magewell:
#ifdef __linux__
            return true;
#endif
            return false;

        case Type::decklink:
            return true;

        default:
            break;
        }

        return false;
    }

    QString title() const {
        return title(type());
    }

    static QString title(int type) {
        switch(type) {
        case Type::dummy:
            return QStringLiteral("dummy");

        case Type::ffmpeg:
#ifdef __linux__
            return QStringLiteral("v4l2");
#else
            return QStringLiteral("dshow");
#endif

        case Type::magewell:
            return QStringLiteral("magewell");

        case Type::decklink:
            return QStringLiteral("decklink");

        default:
            break;
        }

        return QStringLiteral("disabled");
    }

    virtual void subscribe(FrameBuffer<Frame::ptr>::ptr obj) {
        if(!subscription_list.contains(obj))
            subscription_list.append(obj);
    }

    virtual void unsubscribe(FrameBuffer<Frame::ptr>::ptr obj) {
        subscription_list.removeAll(obj);
    }

    virtual bool isActive()=0;

    virtual bool gotSignal() {
        return !signal_lost;
    }

    //

    virtual void deviceStart()=0;
    virtual void deviceStop()=0;

    virtual void setDevice(void *dev)=0;

    virtual void setFramerate(AVRational fr) {
        framerate=fr;
    }

    virtual AVRational currentFramerate() {
        return framerate;
    }

    virtual void setPixelFormat(PixelFormat fmt) {
        pixel_format=(int)fmt;
    }

    virtual PixelFormat currentPixelFormat() {
        return PixelFormat(pixel_format);
    }

    virtual void setFramesize(QSize r) {
        framesize=r;
    }

    virtual QSize currentFramesize() {
        return framesize;
    }

    virtual void setHalfFps(bool value) {
        half_fps=value;
    }

    //

    virtual void setAudioSampleSize(AudioSampleSize::T value) {
        audio_sample_size=value;
    }

    virtual AudioSampleSize::T currentAudioSampleSize() {
        return audio_sample_size;
    }

    virtual void setAudioChannels(AudioChannels::T value) {
        audio_channels=value;
    }

    virtual AudioChannels::T currentAudioChannels() {
        return audio_channels;
    }

    //

    virtual void signalLost(bool value)=0;
    virtual void formatChanged(QString format)=0;
    virtual void frameSkipped()=0;
    virtual void errorString(QString err_string)=0;

protected:
    QList <FrameBuffer<Frame::ptr>::ptr> subscription_list;

    std::atomic <bool> signal_lost;

    std::atomic <int> pixel_format;
    std::atomic <AVRational> framerate;
    std::atomic <QSize> framesize;
    std::atomic <bool> half_fps;

    std::atomic <AudioSampleSize::T> audio_sample_size;
    std::atomic <AudioChannels::T> audio_channels;
};

Q_DECLARE_INTERFACE(SourceInterface, "SourceInterface")

#endif // SOURCE_INTERFACE_H
