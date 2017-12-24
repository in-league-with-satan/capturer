#ifndef TOOLS_VIDEO4LINUX2_H
#define TOOLS_VIDEO4LINUX2_H

#include <QSize>
#include <QString>
#include <QVector>

#include "ff_tools.h"

namespace toolsV4L2 {
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

void testDevList();

}

#endif // TOOLS_VIDEO4LINUX2_H
