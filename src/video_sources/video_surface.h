#ifndef VIDEO_SURFACE_H
#define VIDEO_SURFACE_H

#include <QAbstractVideoSurface>
#include <QList>

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

private:
    QList <FrameBuffer::ptr> subscription_list;

    FFFormatConverter *format_converter;

//signals:
//    void frameAvailable(QImage frame);
};

#endif // VIDEO_SURFACE_H
