#ifndef TOOLS_CAM_H
#define TOOLS_CAM_H

#include <QSize>
#include <QString>
#include <QVector>

#include "ff_tools.h"

#ifdef __linux__

#include <linux/videodev2.h>

#endif

struct Cam {
    struct Resolution {
        QSize size;
        QList <AVRational> framerate;
    };

    struct Format {
        int64_t pixel_format;
        QList <Resolution> resolution;
    };

    struct Dev {
        QString name;
        QString dev;
        QList <Format> format;
    };

    struct PixelFormat {
        enum {
#ifdef __linux__

            YUYV=V4L2_PIX_FMT_YUYV,
            MJPEG=V4L2_PIX_FMT_MJPEG

#else

            YUYV,
            MJPEG

#endif
        };

        static QString toString(const uint64_t &pix_fmt) {
            switch(pix_fmt) {
            case YUYV:
                return QStringLiteral("YUYV");

            case MJPEG:
                return QStringLiteral("MJPEG");
            }

            return QStringLiteral("unknown");
        }
    };
};

struct ToolsCam
{
    static QList <Cam::Dev> devList();

    static AVRational framrateToRational(const qreal &fr);
    static double rationalToFramerate(const AVRational &value);

    static void testDevList(const QList <Cam::Dev> &list);
};

#endif // TOOLS_CAM_H
