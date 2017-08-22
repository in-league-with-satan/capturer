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

    void setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format, int audio_channels, int audio_sample_size);

    void subscribe(FrameBuffer::ptr obj);
    void unsubscribe(FrameBuffer::ptr obj);

    bool isRunning() const;

    bool gotSignal() const;

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

    DlConvertThreadContainer *conv_thread;

    IDeckLinkVideoConversion *video_converter;

    QList <FrameBuffer::ptr> subscription_list;

    uint8_t frame_counter;

    std::atomic <bool> running;
    std::atomic <bool> half_fps;
    std::atomic <bool> signal_lost;
    std::atomic <bool> skip_frame;

signals:
    void signalLost(const bool &value);
    void formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format);
    void frameSkipped();
};

#endif // CAPTURE_H
