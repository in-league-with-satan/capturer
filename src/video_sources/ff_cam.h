#ifndef FF_CAM_H
#define FF_CAM_H

#include <QThread>
#include <QAudioFormat>

#include <atomic>


#include "ff_tools.h"
#include "frame_buffer.h"

class FFCamPrivate;


class FFCam : public QThread
{
    Q_OBJECT

public:
    FFCam(QObject *parent=0);
    ~FFCam();

    static QString formatString(const QAudioFormat &format);
    static AVSampleFormat qAudioFormatToAV(const int &depth, const QAudioFormat::SampleType &sample_format);

    static QStringList availableCameras();
    static QStringList availableAudioInput();
    static QString pixelFormatToString(int64_t fmt);

    static void updateDevList();

    bool setVideoDevice(int index);
    void setAudioDevice(int index);

    QList <QSize> supportedResolutions();
    QList <int64_t> supportedPixelFormats(QSize size);
    QList <AVRational> supportedFramerates(QSize size, int64_t fmt);

    void subscribe(FrameBuffer::ptr obj);
    void unsubscribe(FrameBuffer::ptr obj);

    bool isActive();



public slots:
    void setConfig(QSize size, AVRational framerate, int64_t pixel_format);
    void startCam();
    void stop();

protected:
    void run();

private:
    struct Cfg {
        QSize size;
        AVRational framerate;
        int64_t pixel_format;

    } cfg;

    QList <FrameBuffer::ptr> subscription_list;

    int index_device_video=0;
    int index_device_audio=0;

    FFCamPrivate *d;

    std::atomic <bool> running;
};

#endif // FF_CAM_H
