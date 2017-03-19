#ifndef FRAME_H
#define FRAME_H

#include <QtGlobal>
#include <QSize>

#include <memory>

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
        QByteArray raw;
        QSize size;
        uint32_t bmd_pixel_format;

    } video;

    struct DataAudio {
        QByteArray raw;
        int channels;

    } audio;

    uint8_t counter;
    bool reset_counter;
};

#endif // FRAME_H
