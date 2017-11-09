#include <QDebug>

#include "ff_format_converter.h"

#include "video_surface.h"

VideoSurace::VideoSurace(QObject *parent) :
    QAbstractVideoSurface(parent)
{
    format_converter=new FFFormatConverter();
}

QList <QVideoFrame::PixelFormat> VideoSurace::supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const
{
    Q_UNUSED(handleType);

    return QList <QVideoFrame::PixelFormat>()
            << QVideoFrame::Format_YUV420P;
    /*
            << QVideoFrame::Format_ARGB32
            << QVideoFrame::Format_ARGB32_Premultiplied
            << QVideoFrame::Format_RGB32
            << QVideoFrame::Format_RGB24
            << QVideoFrame::Format_RGB565
            << QVideoFrame::Format_RGB555
            << QVideoFrame::Format_ARGB8565_Premultiplied
            << QVideoFrame::Format_BGRA32
            << QVideoFrame::Format_BGRA32_Premultiplied
            << QVideoFrame::Format_BGR32
            << QVideoFrame::Format_BGR24
            << QVideoFrame::Format_BGR565
            << QVideoFrame::Format_BGR555
            << QVideoFrame::Format_BGRA5658_Premultiplied
            << QVideoFrame::Format_AYUV444
            << QVideoFrame::Format_AYUV444_Premultiplied
            << QVideoFrame::Format_YUV444
            << QVideoFrame::Format_YUV420P
            << QVideoFrame::Format_YV12
            << QVideoFrame::Format_UYVY
            << QVideoFrame::Format_YUYV
            << QVideoFrame::Format_NV12
            << QVideoFrame::Format_NV21
            << QVideoFrame::Format_IMC1
            << QVideoFrame::Format_IMC2
            << QVideoFrame::Format_IMC3
            << QVideoFrame::Format_IMC4
            << QVideoFrame::Format_Y8
            << QVideoFrame::Format_Y16
            << QVideoFrame::Format_Jpeg
            << QVideoFrame::Format_CameraRaw
            << QVideoFrame::Format_AdobeDng;
    */
}

bool VideoSurace::present(const QVideoFrame &video_frame)
{
    if(video_frame.isValid()) {
        QVideoFrame frame_clone(video_frame);

        frame_clone.map(QAbstractVideoBuffer::ReadOnly);

        Frame::ptr frame=Frame::make();

        QByteArray ba_video_frame_src;
        QByteArray ba_video_frame_dst;

        ba_video_frame_src.resize(frame_clone.mappedBytes());

        memcpy((char*)ba_video_frame_src.constData(), frame_clone.bits(), ba_video_frame_src.size());

        format_converter->setup(AV_PIX_FMT_YUV420P, video_frame.size(), AV_PIX_FMT_BGRA, video_frame.size());
        // format_converter->setup(AV_PIX_FMT_YUV420P, video_frame.size(), AV_PIX_FMT_ARGB, video_frame.size());
        // format_converter->setup(AV_PIX_FMT_YUV420P, video_frame.size(), AV_PIX_FMT_ABGR, video_frame.size());

        format_converter->convert(&ba_video_frame_src, &ba_video_frame_dst);

        frame->setData(ba_video_frame_dst, frame_clone.size(), QByteArray(), 0, 0);

        frame_clone.unmap();

        foreach(FrameBuffer::ptr buf, subscription_list)
            buf->append(frame);

        return true;
    }

    return false;
}

void VideoSurace::subscribe(FrameBuffer::ptr obj)
{
    if(!subscription_list.contains(obj))
        subscription_list.append(obj);
}

void VideoSurace::unsubscribe(FrameBuffer::ptr obj)
{
    subscription_list.removeAll(obj);
}
