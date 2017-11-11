#ifndef VIDEO_SURFACE_H
#define VIDEO_SURFACE_H

#include <QAbstractVideoSurface>
#include <QList>
#include <QAudioFormat>

#include "frame_buffer.h"

class FFFormatConverter;

class VideoSurace : public QAbstractVideoSurface
{
    Q_OBJECT

public:
    explicit VideoSurace(QObject *parent=0);

    QList <QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const;

    bool present(const QVideoFrame &video_frame);

    void subscribe(FrameBuffer::ptr obj);
    void unsubscribe(FrameBuffer::ptr obj);

    void setAudioDevice(QIODevice *dev, const QAudioFormat &format);

private:
    QList <FrameBuffer::ptr> subscription_list;

    FFFormatConverter *format_converter;

    //

    QIODevice *audio_device;
    QAudioFormat audio_format;
};

#endif // VIDEO_SURFACE_H
