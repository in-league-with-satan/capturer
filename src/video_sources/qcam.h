#ifndef Q_CAM_H
#define Q_CAM_H

#include <QThread>

#include <QVideoFrame>



#include "ff_tools.h"
#include "frame_buffer.h"

class QCamPrivate;


class QCam : public QObject
{
    Q_OBJECT

public:
    QCam(QObject *parent=0);
    ~QCam();

    static QStringList availableCameras();
    static QStringList availableAudioInput();
    static QString pixelFormatToString(QVideoFrame::PixelFormat fmt);
    static qreal rationalToFramerate(AVRational value);
    static AVRational framrateToRational(qreal fr);

    void setVideoDevice(size_t index);
    void setAudioDevice(size_t index);


    QList <QSize> supportedResolutions();
    QList <QVideoFrame::PixelFormat> pixelFormats(QSize size);
    QList <AVRational> frameRateRanges(QSize size);

    void start(QSize size, QVideoFrame::PixelFormat pixel_format, AVRational framerate);
    void stop();

    void subscribe(FrameBuffer::ptr obj);
    void unsubscribe(FrameBuffer::ptr obj);

    bool isActive();

private:
    QCamPrivate *d;
};

#endif // Q_CAM_H
