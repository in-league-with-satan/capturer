#ifndef TOOLS_VIDEO4LINUX2_H
#define TOOLS_VIDEO4LINUX2_H

#include "tools_cam.h"

struct ToolsV4L2
{
    static QList <Cam::Dev> devList();
};

#endif // TOOLS_VIDEO4LINUX2_H
