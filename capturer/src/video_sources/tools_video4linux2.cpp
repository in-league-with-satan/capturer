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
#include "pixel_format.h"

#include "tools_video4linux2.h"


QList <Cam::Dev> ToolsV4L2::devList()
{
    QList <Cam::Dev> list;

#ifdef __linux__

    int fd=0;

    for(int i=0; i<64; ++i) {
        Cam::Dev dev={};

        dev.dev=QString("/dev/video%1").arg(i);

        if(-1==(fd=open(dev.dev.toLatin1().data(), O_RDONLY))) {
            continue;
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

            // qInfo() << "fmtdesc.description" << fmtdesc.pixelformat << QString((char*)fmtdesc.description);

            Cam::Format format;

            if(!format.pixel_format.fromV4L2PixelFormat(fmtdesc.pixelformat))
                continue;


            Cam::Resolution resolution={};
            v4l2_frmsizeenum frmsizeenum={};

            frmsizeenum.pixel_format=fmtdesc.pixelformat;

            while(!ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsizeenum)) {
                frmsizeenum.index++;

                QList <QSize> res;

                switch(frmsizeenum.type) {
                case V4L2_FRMSIZE_TYPE_DISCRETE:
                    res << QSize(frmsizeenum.discrete.width, frmsizeenum.discrete.height);
                    break;

                case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                case V4L2_FRMSIZE_TYPE_STEPWISE:
                    res=ToolsCam::resBuildSequence(QSize(frmsizeenum.stepwise.min_width, frmsizeenum.stepwise.min_height),
                                                   QSize(frmsizeenum.stepwise.max_width, frmsizeenum.stepwise.max_height));
                    break;

                default:
                    continue;
                }

                foreach(QSize size, res) {
                    resolution.size=size;
                    resolution.framerate.clear();

                    v4l2_frmivalenum frmivalenum={};
                    frmivalenum.pixel_format=fmtdesc.pixelformat;
                    frmivalenum.width=resolution.size.width();
                    frmivalenum.height=resolution.size.height();

                    while(!ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmivalenum)) {
                        frmivalenum.index++;

                        QList <AVRational> framerate;

                        switch(frmivalenum.type) {
                        case V4L2_FRMSIZE_TYPE_DISCRETE:
                            framerate.append(ToolsCam::framerateToRational((double)frmivalenum.discrete.denominator/(double)frmivalenum.discrete.numerator));
                            break;

                        case V4L2_FRMSIZE_TYPE_CONTINUOUS:
                        case V4L2_FRMSIZE_TYPE_STEPWISE:
                            // framerate << AVRational({ frmivalenum.stepwise.min.numerator, frmivalenum.stepwise.min.denominator });
                            framerate=ToolsCam::framerateBuildSequence(25., (double)frmivalenum.stepwise.min.denominator/(double)frmivalenum.stepwise.min.numerator);
                            break;

                        default:
                            ;
                        }

                        foreach(const AVRational &rational, framerate) {
                            if(!resolution.framerate.contains(rational))
                                resolution.framerate << rational;
                        }
                    }

                    std::sort(resolution.framerate.begin(), resolution.framerate.end(), [](const AVRational &l, const AVRational &r) { return ToolsCam::rationalToFramerate(l)<ToolsCam::rationalToFramerate(r); });

                    format.resolution << resolution;
                }
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

