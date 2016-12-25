#ifndef CAPTURE_H
#define CAPTURE_H

#include <QObject>
#include <QThread>
#include <QSize>

#include "device_list.h"

class DeckLinkCaptureDelegate;

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

    void setup(DeckLinkDevice device, DeckLinkFormat format, DeckLinkPixelFormat pixel_format);

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
    IDeckLinkVideoConversion *video_converter;
    IDeckLinkMutableVideoFrame *video_frame_converted_720p;
    IDeckLinkMutableVideoFrame *video_frame_converted_1080p;
    IDeckLinkMutableVideoFrame *video_frame_converted_2160p;

    DeckLinkDevice device;
    DeckLinkFormat format;
    DeckLinkPixelFormat pixel_format;

    FF::FormatConverter *ff_converter;

signals:
    void frameVideo(QByteArray ba_data, QSize size);
    void frameAudio(QByteArray ba_data);
    void frame(QByteArray ba_video, QSize size, QByteArray ba_audio);
    void noInputSignalDetected();
};

#endif // CAPTURE_H
