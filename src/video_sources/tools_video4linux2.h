#ifndef TOOLS_VIDEO4LINUX2_H
#define TOOLS_VIDEO4LINUX2_H

#include <QSize>
#include <QString>
#include <QVector>

#include "ff_tools.h"

#ifndef __linux__

  #define V4L2_PIX_FMT_YUYV 1
  #define V4L2_PIX_FMT_MJPEG 2

#endif


namespace ToolsV4L2 {
struct v4l2_Resolution {
    QSize size;
    QList <AVRational> framerate;
};

struct v4l2_Format {
    int64_t pixel_format;
    QList <v4l2_Resolution> resolution;
};

struct v4l2_Dev {
    QString name;
    QString dev;
    QList <v4l2_Format> format;
};

QString v4l2PixFmtToString(uint64_t pix_fmt);

QList <v4l2_Dev> devList();

void testDevList(const QList <ToolsV4L2::v4l2_Dev> &list);

}

#endif // TOOLS_VIDEO4LINUX2_H
