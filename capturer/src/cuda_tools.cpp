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
#include <QLibrary>

#include "cuda_tools.h"


#if defined(_WIN32) || defined(__CYGWIN__)

#define CUDAAPI __stdcall

#else

#define CUDAAPI

#endif

typedef enum cudaError_enum {
    CUDA_SUCCESS=0

} CUresult;

typedef CUresult CUDAAPI TcuGetErrorName(CUresult error, const char **pstr);
typedef CUresult CUDAAPI TcuInit(unsigned int flags);
typedef CUresult CUDAAPI TcuDeviceGetCount(int *count);
typedef CUresult CUDAAPI TcuDeviceGetName(char *name, int len, int dev);


QStringList cuda::availableDevices()
{
    static QStringList devices;
    static bool ready=false;

    if(ready)
        return devices;

#ifdef __linux__

    QLibrary lib("libcuda");

#else

    QLibrary lib("nvcuda");

#endif

    if(!lib.load()) {
        qWarning() << "error" << lib.errorString();

        ready=true;

        return devices;
    }

    TcuGetErrorName *cuGetErrorName=(TcuGetErrorName*)lib.resolve("cuGetErrorName");
    TcuInit *cuInit=(TcuInit*)lib.resolve("cuInit");
    TcuDeviceGetCount *cuDeviceGetCount=(TcuDeviceGetCount*)lib.resolve("cuDeviceGetCount");
    TcuDeviceGetName *cuDeviceGetName=(TcuDeviceGetName*)lib.resolve("cuDeviceGetName");

    if(cuInit) {
        cuInit(0);

        int cnt=0;
        int result=cuDeviceGetCount(&cnt);

        const char *resstr;

        cuGetErrorName((CUresult)result, &resstr);

        for(int i=0; i<cnt; i++) {
            char devname[255];

            cuDeviceGetName(devname, 255, i);

            devices << QString(devname);
        }

    } else {
        qWarning() << "error" << lib.errorString();
    }

    ready=true;

    lib.unload();

    return devices;
}
