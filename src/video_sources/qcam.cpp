#include <QAudioInput>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include <qcoreapplication.h>

#include <cmath>

#include "video_surface.h"

#include "qcam.h"

struct QCamPrivate
{
    QCamera *camera;
    QCameraImageCapture *camera_image_capture;

    VideoSurace *surface;

    QList <QCameraViewfinderSettings> supported_viewfinder_settings;

    //

    QAudioInput *audio_input;
};


QCam::QCam(QObject *parent)
    : QObject(parent)
{
    d=new QCamPrivate();

    d->camera=nullptr;
    d->camera_image_capture=nullptr;
    d->surface=new VideoSurace(this);

    d->audio_input=nullptr;
}

QCam::~QCam()
{
    if(d->camera) {
        d->camera->stop();
        d->camera->unload();
        d->camera->deleteLater();
        d->camera=nullptr;
    }

    if(d->audio_input) {
        d->audio_input->stop();
        d->audio_input=nullptr;
    }

    delete d;
}

QStringList QCam::availableCameras()
{
    QStringList list;

    foreach(QCameraInfo inf, QCameraInfo::availableCameras())
        list << inf.description();

    return list;
}

QStringList QCam::availableAudioInput()
{
     QStringList list;

     foreach(QAudioDeviceInfo di, QAudioDeviceInfo::availableDevices(QAudio::AudioInput)) {
         list << di.deviceName();
     }

     return list;
}

QString QCam::pixelFormatToString(QVideoFrame::PixelFormat fmt)
{
    switch((uint32_t)fmt) {
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

void QCam::setVideoDevice(size_t index)
{
    QList <QCameraInfo> dev_list=QCameraInfo::availableCameras();

    if(dev_list.size()<=(int)index) {
        qCritical() << "index out of range";
        return;
    }

    if(d->camera) {
        d->camera->stop();
        d->camera->unload();
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

void QCam::setAudioDevice(size_t index)
{
    QList <QAudioDeviceInfo> dev_list=QAudioDeviceInfo::availableDevices(QAudio::AudioInput);

    if(dev_list.size()<=(int)index) {
        qCritical() << "index out of range";
        return;
    }

    if(d->audio_input) {
        d->audio_input->stop();
        d->audio_input->deleteLater();
    }

    QAudioFormat format;
    format.setSampleRate(48000);
    format.setChannelCount(2);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::UnSignedInt);

    if(!dev_list[index].isFormatSupported(format)) {
        qWarning() << "Default format not supported, trying to use the nearest.";
        format=dev_list[index].nearestFormat(format);
    }

    d->audio_input=new QAudioInput(dev_list[index], format, this);
}

QList <QSize> QCam::supportedResolutions()
{
#ifdef __WIN32__
    return QList<QSize>() << QSize(640, 480) << QSize(1280, 720) << QSize(1920, 1080);
#endif

    if(!d->camera_image_capture) {
        qCritical() << "camera_image_capture nullptr";
        return QList <QSize>();
    }

    qInfo() << "supportedImageCodecs" << d->camera_image_capture->supportedImageCodecs();

    return d->camera_image_capture->supportedResolutions();
}

QList <QVideoFrame::PixelFormat> QCam::pixelFormats(QSize size)
{
    QSet <QVideoFrame::PixelFormat> res;

    foreach(QCameraViewfinderSettings set, d->supported_viewfinder_settings) {
        if(set.resolution()==size)
            // if(QVideoFrame::Format_Jpeg!=set.pixelFormat())
                res << set.pixelFormat();
    }

    qInfo() << "QCam::pixelFormats:" << res.toList();

    return res.toList();
}

AVRational QCam::framrateToRational(qreal fr)
{
    static const double eps=.0001;

    static auto rnd2=[](const double &value)->double {
       return round(value*100)/100.;
    };

    static auto rnd3=[](const double &value)->double {
       return round(value*1000)/1000.;
    };

    static auto rnd4=[](const double &value)->double {
       return round(value*10000)/10000.;
    };

    if(std::abs(fr - 15.)<eps)
        return { 1000, 15000 };

    if(std::abs(rnd3(fr) - 23.976)<eps)
        return { 1001, 24000 };

    if(std::abs(fr - 24.)<eps)
        return { 1000, 24000 };

    if(std::abs(fr - 25.)<eps)
        return { 1000, 25000 };

    if(std::abs(rnd2(fr) - 29.97)<eps)
        return { 1001, 30000 };

    if(std::abs(fr - 30.)<eps)
        return { 1000, 30000 };

    if(std::abs(fr - 50.)<eps)
        return { 1000, 50000 };

    if(std::abs(rnd4(fr) - 59.9398)<eps)
        return { 1001, 60000 };

    if(std::abs(rnd2(fr) - 59.94)<eps)
        return { 1001, 60000 };

    if(std::abs(fr - 60.)<eps)
        return { 1000, 60000 };

    qWarning() << "framrateToRational unknown fr:" << fr;

    return { 1000, 30000 };
}

qreal QCam::rationalToFramerate(AVRational value)
{
    if(value.num==1000 && value.den==15000)
        return 15.;

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

    foreach(QCameraViewfinderSettings set, d->supported_viewfinder_settings) {
        if(set.resolution()==size) {
            res_set << set.minimumFrameRate() << set.maximumFrameRate();
        }
    }

    QList <AVRational> res_list;

    foreach(qreal val, res_set) {
        AVRational r=framrateToRational(val);

        if(!res_list.contains(r))
            res_list << r;
    }

    return res_list;
}

void QCam::start(QSize size, QVideoFrame::PixelFormat pixel_format, AVRational framerate)
{
    if(!d->camera)
        return;


    stop();

    qApp->processEvents();


    QCameraViewfinderSettings vfs;

    vfs.setPixelFormat(pixel_format);
    vfs.setResolution(size);
    vfs.setMinimumFrameRate(rationalToFramerate(framerate));
    vfs.setMaximumFrameRate(rationalToFramerate(framerate));


    d->camera->setCaptureMode(QCamera::CaptureVideo);
    d->camera->setViewfinderSettings(vfs);

    d->camera->start();

    if(d->audio_input)
        d->surface->setAudioDevice(d->audio_input->start(), d->audio_input->format());

    else
        d->surface->setAudioDevice(0, QAudioFormat());
}

void QCam::stop()
{
    if(d->camera)
        d->camera->stop();

    if(d->audio_input)
        d->audio_input->stop();
}

void QCam::subscribe(FrameBuffer::ptr obj)
{
    d->surface->subscribe(obj);
}

void QCam::unsubscribe(FrameBuffer::ptr obj)
{
    d->surface->unsubscribe(obj);
}

bool QCam::isActive()
{
    if(!d->camera)
        return false;

    if(d->camera->state()==QCamera::ActiveState)
        return true;

    return false;
}

