#ifndef FRAME_H
#define FRAME_H

#include <QtGlobal>
#include <QSize>

#include <memory>

#include "decklink_video_frame.h"

struct Frame
{
    typedef std::shared_ptr<Frame> ptr;

    Frame() {
        reset_counter=false;
    }

    static ptr make() {
        return ptr(new Frame());
    }

    struct DataVideo {
        DataVideo() {
            raw=decklink_frame.getBuffer();
        }

        DeckLinkVideoFrame decklink_frame;

        QByteArray *raw;

    } video;

    struct DataAudio {
        QByteArray raw;
        int channels;
        int sample_size;

    } audio;

    uint8_t counter;
    bool reset_counter;
};

#endif // FRAME_H
