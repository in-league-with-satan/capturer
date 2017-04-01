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

    void subscribe(FrameBuffer *obj);
    void unsubscribe(FrameBuffer *obj);

    bool isRunning();

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

    DeckLinkDevice device;
    DeckLinkFormat format;
    DeckLinkPixelFormat pixel_format;
    int audio_channels;

    int64_t frame_scale;
    int64_t frame_time_prev;

    DlConvertThreadContainer *conv_thread;

    // FF::FormatConverter *ff_converter;

    IDeckLinkVideoConversion *video_converter;

    QList <FrameBuffer*> subscription_list;

    uint8_t frame_counter;


    QMutex mutex_subscription;

signals:
    void noInputSignalDetected();
    void formatChanged(int width, int height, quint64 frame_duration, quint64 frame_scale, bool progressive_frame, QString pixel_format);
    void frameSkipped();
};

#endif // CAPTURE_H
