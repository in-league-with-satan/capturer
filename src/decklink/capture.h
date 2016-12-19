#ifndef CAPTURE_H
#define CAPTURE_H

#include <QObject>
#include <QThread>

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
    void videoInputFrameArrived(IDeckLinkVideoInputFrame *video_frame, IDeckLinkAudioInputPacket *audio_packet);

    void release();

    DeckLinkCaptureDelegate *deck_link_capture_delegate;

    IDeckLink *decklink;
    IDeckLinkDisplayModeIterator *display_mode_iterator;
    IDeckLinkDisplayMode *display_mode;
    IDeckLinkInput *decklink_input;
    IDeckLinkOutput *decklink_output;
    IDeckLinkIterator *decklink_iterator;
    IDeckLinkAttributes *decklink_attributes;
    IDeckLinkVideoConversion *video_converter;
    IDeckLinkMutableVideoFrame *video_frame_converted;

    DeckLinkDevice device;
    DeckLinkFormat format;
    DeckLinkPixelFormat pixel_format;

signals:
    void inputFrameArrived(QByteArray ba_video, QByteArray ba_audio);
};

#endif // CAPTURE_H
