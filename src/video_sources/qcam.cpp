#include <QCameraInfo>
#include <QCameraImageCapture>
#include <qcoreapplication.h>

#include "video_surface.h"

#include "qcam.h"

struct QCamPrivate
{
    QCamera *camera;
    QCameraImageCapture *camera_image_capture;

    VideoSurace *surface;

    QList <QCameraViewfinderSettings> supported_viewfinder_settings;
};


QCam::QCam(QObject *parent)
    : QObject(parent)
{
    d=new QCamPrivate();

    d->camera=nullptr;
    d->camera_image_capture=nullptr;
    d->surface=new VideoSurace(this);
}

QCam::~QCam()
{
    delete d;
}

QStringList QCam::availableCameras()
{
    QStringList list;

    foreach(QCameraInfo inf, QCameraInfo::availableCameras())
        list << inf.description();

    return list;
}

QString QCam::pixelFormatToString(QVideoFrame::PixelFormat fmt)
{
    switch(fmt) {
    case QVideoFrame::Format_ARGB32:
        return QStringLiteral("ARGB32");

    case QVideoFrame::Format_ARGB32_Premultiplied:
        return QStringLiteral("ARGB32_Premultiplied");

    case QVideoFrame::Format_RGB32:
        return QStringLiteral("RGB32");

    case QVideoFrame::Format_RGB24:
        return QStringLiteral("RGB24");

    case QVideoFrame::Format_RGB565:
        return QStringLiteral("RGB565");

    case QVideoFrame::Format_RGB555:
        return QStringLiteral("RGB555");

    case QVideoFrame::Format_ARGB8565_Premultiplied:
        return QStringLiteral("ARGB8565_Premultiplied");

    case QVideoFrame::Format_BGRA32:
        return QStringLiteral("BGRA32");

    case QVideoFrame::Format_BGRA32_Premultiplied:
        return QStringLiteral("BGRA32_Premultiplied");

    case QVideoFrame::Format_BGR32:
        return QStringLiteral("BGR32");

    case QVideoFrame::Format_BGR24:
        return QStringLiteral("BGR24");

    case QVideoFrame::Format_BGR565:
        return QStringLiteral("BGR565");

    case QVideoFrame::Format_BGR555:
        return QStringLiteral("BGR555");

    case QVideoFrame::Format_BGRA5658_Premultiplied:
        return QStringLiteral("BGRA5658_Premultiplied");

    case QVideoFrame::Format_AYUV444:
        return QStringLiteral("AYUV444");

    case QVideoFrame::Format_AYUV444_Premultiplied:
        return QStringLiteral("AYUV444_Premultiplied");

    case QVideoFrame::Format_YUV444:
        return QStringLiteral("YUV444");

    case QVideoFrame::Format_YUV420P:
        return QStringLiteral("YUV420P");

    case QVideoFrame::Format_YV12:
        return QStringLiteral("YV12");

    case QVideoFrame::Format_UYVY:
        return QStringLiteral("UYVY");

    case QVideoFrame::Format_YUYV:
        return QStringLiteral("YUYV");

    case QVideoFrame::Format_NV12:
        return QStringLiteral("NV12");

    case QVideoFrame::Format_NV21:
        return QStringLiteral("NV21");

    case QVideoFrame::Format_IMC1:
        return QStringLiteral("IMC1");

    case QVideoFrame::Format_IMC2:
        return QStringLiteral("IMC2");

    case QVideoFrame::Format_IMC3:
        return QStringLiteral("IMC3");

    case QVideoFrame::Format_IMC4:
        return QStringLiteral("IMC4");

    case QVideoFrame::Format_Y8:
        return QStringLiteral("Y8");

    case QVideoFrame::Format_Y16:
        return QStringLiteral("Y16");

    case QVideoFrame::Format_Jpeg:
        return QStringLiteral("Jpeg");

    case QVideoFrame::Format_CameraRaw:
        return QStringLiteral("CameraRaw");
    }

    return QStringLiteral("Invalid");
}

void QCam::setDevice(size_t index)
{
    QList <QCameraInfo> dev_list=QCameraInfo::availableCameras();

    if(dev_list.size()<=(int)index) {
        qCritical() << "index out of range";
        return;
    }

    if(d->camera) {
        d->camera->stop();
        d->camera->deleteLater();
    }

    if(d->camera_image_capture)
        d->camera_image_capture->deleteLater();

    qInfo() << "cam info:" << dev_list[index].description();

    d->camera=new QCamera(dev_list[index]);

    d->camera->setViewfinder(d->surface);

    d->camera->setCaptureMode(QCamera::CaptureVideo);

    d->camera->load();

    while(d->camera->status()!=QCamera::LoadedStatus) {
        qApp->processEvents();
        QThread::usleep(10);
    }

    d->supported_viewfinder_settings=d->camera->supportedViewfinderSettings();

    d->camera_image_capture=new QCameraImageCapture(d->camera);

}

QList <QSize> QCam::supportedResolutions()
{
    QList <QSize> lst;

    if(!d->camera_image_capture) {
        qCritical() << "camera_image_capture nullptr";
        return lst;
    }

    qInfo() << "supportedImageCodecs" << d->camera_image_capture->supportedImageCodecs();

    return d->camera_image_capture->supportedResolutions();
}

QList <QVideoFrame::PixelFormat> QCam::pixelFormats(QSize size)
{
    // qInfo()<<"QCam::pixelFormats" << size;
    QSet <QVideoFrame::PixelFormat> res;

    foreach(QCameraViewfinderSettings set, d->supported_viewfinder_settings) {
        if(set.resolution()==size)
            res << set.pixelFormat();
    }

    return res.toList();
}

AVRational QCam::framrateToRational(qreal fr)
{
    static const double df=.0001;

    if(std::abs(fr - 23.976)<df)
        return { 1001, 24000 };

    if(std::abs(fr - 24.)<df)
        return { 1000, 24000 };

    if(std::abs(fr - 25.)<df)
        return { 1000, 25000 };

    if(std::abs(fr - 29.97)<df)
        return { 1001, 30000 };

    if(std::abs(fr - 30.)<df)
        return { 1000, 30000 };

    if(std::abs(fr - 50.)<df)
        return { 1000, 50000 };

    if(std::abs(fr - 59.9398)<df)
        return { 1001, 60000 };

    if(std::abs(fr - 60.)<df)
        return { 1000, 60000 };

    qWarning() << "framrateToRational unknown fr:" << fr;

    return { 1000, 30000 };
}

qreal QCam::rationalToFramerate(AVRational value)
{
    if(value.num==1001 && value.den==24000)
        return 23.976;

    if(value.num==1000 && value.den==24000)
        return 24.;

    if(value.num==1000 && value.den==25000)
        return 25.;

    if(value.num==1001 && value.den==30000)
        return 29.97;

    if(value.num==1000 && value.den==30000)
        return 30.;

    if(value.num==1000 && value.den==50000)
        return 50.;

    if(value.num==1001 && value.den==60000)
        return 59.9398;

    if(value.num==1000 && value.den==60000)
        return 60.;

    return 30.;
}

QList <AVRational> QCam::frameRateRanges(QSize size)
{
    QSet <qreal> res_set;

    //qInfo() << "m2" << d->supported_viewfinder_settings.size();

    foreach(QCameraViewfinderSettings set, d->supported_viewfinder_settings) {
        if(set.resolution()==size) {
            // qInfo() << set.resolution() << set.minimumFrameRate() << set.maximumFrameRate();
            res_set << set.minimumFrameRate() << set.maximumFrameRate();
        }
    }

    QList <AVRational> res_list;

    foreach(qreal val, res_set) {
        res_list << framrateToRational(val);
    }

    return res_list;
}

void QCam::start(QSize size, QVideoFrame::PixelFormat pixel_format, AVRational framerate)
{
    if(!d->camera)
        return;

    QCameraViewfinderSettings vfs;

    vfs.setPixelFormat(pixel_format);
    vfs.setResolution(size);
    vfs.setMinimumFrameRate(rationalToFramerate(framerate));
    vfs.setMaximumFrameRate(rationalToFramerate(framerate));

    qInfo() << "rationalToFramerate:" << rationalToFramerate(framerate);

    //    vfs.setMinimumFrameRate(30);
//    vfs.setMaximumFrameRate(30);


    d->camera->setCaptureMode(QCamera::CaptureVideo);
    d->camera->setViewfinderSettings(vfs);

    d->camera->start();
}

void QCam::subscribe(FrameBuffer::ptr obj)
{
    d->surface->subscribe(obj);
}

void QCam::unsubscribe(FrameBuffer::ptr obj)
{
    d->surface->unsubscribe(obj);
}

