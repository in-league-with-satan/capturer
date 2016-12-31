#ifndef CAPTURE_H
#define CAPTURE_H

#include <QObject>
#include <QThread>
#include <QSize>
#include <QMutex>

#include "frame_buffer.h"

#include "device_list.h"

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

    void setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format, int audio_channels);

    void subscribeForAll(FrameBuffer *obj);
    void subscribeForAudio(FrameBuffer *obj);
    void unsubscribe(FrameBuffer *obj);

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

    DeckLinkCaptureDelegate *deck_link_capture_delegate;

    IDeckLink *decklink;

    IDeckLinkDisplayMode *display_mode;
    IDeckLinkInput *decklink_input;
    IDeckLinkOutput *decklink_output;

    DeckLinkDevice device;
    DeckLinkFormat format;
    DeckLinkPixelFormat pixel_format;
    int audio_channels;

    DlConvertThreadContainer *conv_thread;

    // FF::FormatConverter *ff_converter;

    QMutex mutex_subscription;

signals:
    void noInputSignalDetected();
    void formatChanged(QSize size, int64_t frame_duration, int64_t frame_scale);
};

#endif // CAPTURE_H
