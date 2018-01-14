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

#ifndef CAPTURE_H
#define CAPTURE_H

#include <QObject>
#include <QThread>
#include <QSize>

#include <atomic>

#include "frame_buffer.h"

#include "device_list.h"

#include "decklink_audio_input_packet.h"


class DeckLinkCaptureDelegate;
class DlConvertThreadContainer;

class IDeckLink;
class IDeckLinkDisplayModeIterator;
class IDeckLinkDisplayMode;
class IDeckLinkInput;
class IDeckLinkOutput;
class IDeckLinkIterator;
class IDeckLinkVideoInputFrame;
class IDeckLinkAudioInputPacket;
class IDeckLinkAttributes;
class IDeckLinkVideoConversion;
class IDeckLinkMutableVideoFrame;
class IDeckLinkConfiguration;

namespace FF {
    class FormatConverter;
}

class DeckLinkCapture : public QThread
{
    Q_OBJECT

    friend class DeckLinkCaptureDelegate;

public:
    explicit DeckLinkCapture(QObject *parent=0);
    ~DeckLinkCapture();

    void setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format, int audio_channels, int audio_sample_size, bool rgb_10bit);

    void subscribe(FrameBuffer::ptr obj);
    void unsubscribe(FrameBuffer::ptr obj);

    bool isRunning() const;

    bool gotSignal() const;

    bool rgbSource() const;
    bool rgb10Bit() const;

    void setHalfFps(bool value);

protected:
    void run();

public slots:
    void captureStart();
    void captureStop();

private:
    void videoInputFormatChanged(uint32_t events, IDeckLinkDisplayMode *mode, uint32_t format_flags);
    void videoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet);

    void init();
    void release();

    DeckLinkCaptureDelegate *decklink_capture_delegate;


    IDeckLink *decklink;
    IDeckLinkConfiguration *decklink_configuration;
    IDeckLinkDisplayMode *decklink_display_mode;
    IDeckLinkInput *decklink_input;

    DeckLinkAudioInputPacket *audio_input_packet;

    DeckLinkDevice device;
    DeckLinkFormat format;
    DeckLinkPixelFormat pixel_format;
    int audio_channels;
    int audio_sample_size;

    int64_t frame_scale;
    int64_t frame_time_prev;

    // DlConvertThreadContainer *conv_thread;

    // IDeckLinkVideoConversion *video_converter;

    QList <FrameBuffer::ptr> subscription_list;

    uint8_t frame_counter;

    std::atomic <bool> running;
    std::atomic <bool> running_thread;
    std::atomic <bool> half_fps;
    std::atomic <bool> signal_lost;
    std::atomic <bool> skip_frame;
    std::atomic <bool> rgb_source;
    std::atomic <bool> rgb_10bit;

signals:
    void signalLost(const bool &value);
    void formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format);
    void frameSkipped();
    void errorString(QString err_string);
};

#endif // CAPTURE_H
