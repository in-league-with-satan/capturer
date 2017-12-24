#include <QDebug>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/videodev2.h>
#include <map>
#include <sys/ioctl.h>
#include <unistd.h>

#include "ff_tools.h"

#include "tools_video4linux2.h"

bool supportedPixelFormat(uint64_t pix_fmt)
{
    switch(pix_fmt) {
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_YUYV:
        return true;
    }

    return false;
}

QString toolsV4L2::v4l2PixFmtToString(uint64_t pix_fmt)
{
    switch(pix_fmt) {
    case V4L2_PIX_FMT_YUYV:
        return "YUYV";

    case V4L2_PIX_FMT_MJPEG:
        return "MJPEG";
    }

    return "unknown";
}

QList <toolsV4L2::v4l2_Dev> toolsV4L2::devList()
{
    QList <toolsV4L2::v4l2_Dev> list;

    int fd=0;

    for(int i=0; i<64; ++i) {
        v4l2_Dev dev={};

        dev.dev=QString("/dev/video%1").arg(i);

        if(-1==(fd=open(dev.dev.toLatin1().data(), O_RDONLY))) {
            goto end;
        }

        struct v4l2_capability capability={};

        if(-1==ioctl(fd, VIDIOC_QUERYCAP, &capability)) {
            qCritical() << "Error in ioctl VIDIOC_QUERYCAP";
            goto end;
        }

        dev.name=QString((char*)capability.card);

        v4l2_fmtdesc fmtdesc={};
        fmtdesc.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;

        while(!ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc)) {
            fmtdesc.index++;

            if(!supportedPixelFormat(fmtdesc.pixelformat))
                continue;

            v4l2_Resolution resolution={};

            v4l2_Format format;
            format.pixel_format=fmtdesc.pixelformat;

            v4l2_frmsizeenum frmsizeenum={};
            frmsizeenum.pixel_format=fmtdesc.pixelformat;

            while(!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsizeenum)) {
                switch(frmsizeenum.type) {
                case V4L2_FRMSIZE_TYPE_DISCRETE:
                    resolution.size.setWidth(frmsizeenum.discrete.width);
                    resolution.size.setHeight(frmsizeenum.discrete.height);
                    break;

                case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                case V4L2_FRMSIZE_TYPE_STEPWISE:
                    resolution.size.setWidth(frmsizeenum.stepwise.max_width);
                    resolution.size.setHeight(frmsizeenum.stepwise.max_height);
                    break;

                default:
                    continue;
                }

                v4l2_frmivalenum frmivalenum={};
                frmivalenum.pixel_format=fmtdesc.pixelformat;
                frmivalenum.width=resolution.size.width();
                frmivalenum.height=resolution.size.height();

                while(!ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmivalenum)) {
                    AVRational framerate={};

                    switch(frmivalenum.type) {
                    case V4L2_FRMSIZE_TYPE_DISCRETE:
                        framerate.den=frmivalenum.discrete.denominator;
                        framerate.num=frmivalenum.discrete.numerator;
                        break;

                    case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                    case V4L2_FRMSIZE_TYPE_STEPWISE:
                        framerate.den=frmivalenum.stepwise.min.denominator;
                        framerate.num=frmivalenum.stepwise.min.numerator;
                        break;

                    default:
                        ;
                    }

                    if(!resolution.framerate.contains(framerate) && framerate.den!=0 && framerate.num!=0)
                        resolution.framerate << framerate;

                    frmivalenum.index++;
                }

                format.resolution << resolution;

                frmsizeenum.index++;
            }

            dev.format << format;
        }

        list << dev;

        close(fd);

        fd=0;
    }

end:
    if(fd>0)
        close(fd);

    return list;
}

void toolsV4L2::testDevList()
{
    foreach(toolsV4L2::v4l2_Dev dev, toolsV4L2::devList()) {
        qInfo() << "dev name:" << dev.name << dev.dev;

        foreach(toolsV4L2::v4l2_Format fmt, dev.format) {

            qInfo() << "pixel_format:" << toolsV4L2::v4l2PixFmtToString(fmt.pixel_format);

            foreach(toolsV4L2::v4l2_Resolution res, fmt.resolution) {
                qInfo() << "resolution:" << res.size;

                foreach(AVRational fr, res.framerate) {
                    qInfo() << "framerate:" << fr.num << fr.den;
                }
            }
        }

        qInfo() << "------------";
    }
}
