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

#include "decklink_global.h"

#include "decklink_device_list.h"

#include "decklink_audio_input_packet.h"

#include "source_interface.h"

class DeckLinkCaptureDelegate;
class DlConvertThreadContainer;


namespace FF {
    class FormatConverter;
}

class DeckLinkThread : public QThread, public SourceInterface
{
    Q_OBJECT
    Q_INTERFACES(SourceInterface)

    friend class DeckLinkCaptureDelegate;

public:
    explicit DeckLinkThread(QObject *parent=0);
    ~DeckLinkThread();

    Type::T type() const;

    struct Device {
        Decklink::Device device;
        bool source_10bit;
        AudioSampleSize::T audio_sample_size;
    };

    void setDevice(void *ptr);

    bool isActive();
    bool gotSignal();

protected:
    void run();

public slots:
    void deviceStart();
    void deviceStop();

    void deviceHold() {}
    void deviceResume() {}

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

    BMDPixelFormat bm_pixel_format;

    QString format;

    Decklink::Device device;

    int64_t frame_scale;

    std::atomic <bool> running;
    std::atomic <bool> running_thread;
    std::atomic <bool> signal_lost;
    std::atomic <bool> skip_frame;
    std::atomic <bool> source_rgb;
    std::atomic <bool> source_10bit;

signals:
    void signalLost(bool value);
    void formatChanged(QString format);
    void errorString(QString err_string);
};

#endif // CAPTURE_H
