/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>
#include <map>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __linux__

#include <linux/videodev2.h>
#include <sys/ioctl.h>

#endif


#include "ff_tools.h"

#include "tools_video4linux2.h"


bool supportedPixelFormat(uint64_t pix_fmt)
{
#ifdef __linux__

    switch(pix_fmt) {
    case V4L2_PIX_FMT_MJPEG:
    case V4L2_PIX_FMT_YUYV:
        return true;
    }

#else

    Q_UNUSED(pix_fmt)

#endif

    return false;
}

QList <Cam::Dev> ToolsV4L2::devList()
{
    QList <Cam::Dev> list;

#ifdef __linux__

    int fd=0;

    for(int i=0; i<64; ++i) {
        Cam::Dev dev={};

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

            Cam::Resolution resolution={};

            Cam::Format format;
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

            if(!format.resolution.isEmpty())
                dev.format << format;
        }

        if(!dev.format.isEmpty())
            list << dev;

        close(fd);

        fd=0;
    }

end:
    if(fd>0)
        close(fd);

#endif

    return list;
}

