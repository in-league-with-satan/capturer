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

#include "nv_lib.h"

typedef int TcuGetErrorName(int error, const char **pstr);
typedef int TcuInit(unsigned int flags);
typedef int TcuDeviceGetCount(int *count);
typedef int TcuDeviceGetName(char *name, int len, int dev);
typedef int TcuDeviceGetAttribute(int *pi, int attrib, int dev);

typedef int *Tnvapi_QueryInterface(unsigned int offset);
typedef int Tnvapi_Initialize();
typedef int Tnvapi_EnumPhysicalGPUs(int **handles, int *count);
typedef int Tnvapi_GPU_GetUsages(int *handle, unsigned int *usages);
typedef int Tnvapi_GPU_GetThermalSettings(int *handle, int sensor_index, NV_GPU_THERMAL_SETTINGS *temp);

struct NvLibPrivate
{
    bool initCuda();
    bool initNvApi();

    bool loaded_cuda=false;
    bool loaded_nvapi=false;

    QLibrary lib_nvcuda;
    QLibrary lib_nvapi;

    TcuGetErrorName *f_cuGetErrorName=nullptr;
    TcuInit *f_cuInit=nullptr;
    TcuDeviceGetCount *f_cuDeviceGetCount=nullptr;
    TcuDeviceGetName *f_cuDeviceGetName=nullptr;
    TcuDeviceGetAttribute *f_cuDeviceGetAttribute=nullptr;

    Tnvapi_QueryInterface *f_nvapi_QueryInterface=nullptr;
    Tnvapi_Initialize *f_nvapi_Initialize=nullptr;
    Tnvapi_EnumPhysicalGPUs *f_nvapi_EnumPhysicalGPUs=nullptr;
    Tnvapi_GPU_GetUsages *f_nvapi_GPU_GetUsages=nullptr;
    Tnvapi_GPU_GetThermalSettings *f_nvapi_GPU_GetThermalSettings=nullptr;
};

NvLib::NvLib()
    : d(new NvLibPrivate())
{
    d->loaded_cuda=
            d->initCuda();

    d->loaded_nvapi=
            d->initNvApi();
}

NvLib::~NvLib()
{
    d->lib_nvapi.unload();
    d->lib_nvcuda.unload();

    delete d;
}

bool NvLib::loadedCuda() const
{
    return d->loaded_cuda;
}

bool NvLib::loadedNvApi() const
{
    return d->loaded_nvapi;
}

int NvLib::cuGetErrorName(int error, const char **pstr)
{
    if(!d->loaded_cuda)
        return -1;

    return d->f_cuGetErrorName(error, pstr);
}

int NvLib::cuInit(unsigned int flags)
{
    if(!d->loaded_cuda)
        return -1;

    return d->f_cuInit(flags);
}

int NvLib::cuDeviceGetCount(int *count)
{
    if(!d->loaded_cuda)
        return -1;

    return d->f_cuDeviceGetCount(count);
}

int NvLib::cuDeviceGetName(char *name, int len, int dev)
{
    if(!d->loaded_cuda)
        return -1;

    return d->f_cuDeviceGetName(name, len, dev);
}

int NvLib::cuDeviceGetAttribute(int *pi, int attrib, int dev)
{
    if(!d->loaded_cuda)
        return -1;

    return d->f_cuDeviceGetAttribute(pi, attrib, dev);
}

int *NvLib::nvapiQueryInterface(unsigned int offset)
{
    if(!d->loaded_nvapi)
        return nullptr;

    return d->f_nvapi_QueryInterface(offset);
}

int NvLib::nvapiInitialize()
{
    if(!d->loaded_nvapi)
        return -1;

    return d->f_nvapi_Initialize();
}

int NvLib::nvapiEnumPhysicalGPUs(int **handles, int *count)
{
    if(!d->loaded_nvapi)
        return -1;

    return d->f_nvapi_EnumPhysicalGPUs(handles, count);
}

int NvLib::nvapiGPUGetUsages(int *handle, unsigned int *usages)
{
    if(!d->loaded_nvapi)
        return -1;

    return d->f_nvapi_GPU_GetUsages(handle, usages);
}

int NvLib::nvapiGPUGetThermalSettings(int *handle, int sensor_index, NV_GPU_THERMAL_SETTINGS *temp)
{
    if(!d->loaded_nvapi)
        return -1;

    return d->f_nvapi_GPU_GetThermalSettings(handle, sensor_index, temp);
}

bool NvLibPrivate::initCuda()
{
#ifdef __linux__

    lib_nvcuda.setFileName("libcuda");

#else

    lib_nvcuda.setFileName("nvcuda");

#endif

    if(!lib_nvcuda.load()) {
        qWarning() << "error" << lib_nvcuda.errorString();
        goto err;
    }

    f_cuGetErrorName=(TcuGetErrorName*)lib_nvcuda.resolve("cuGetErrorName");
    f_cuInit=(TcuInit*)lib_nvcuda.resolve("cuInit");
    f_cuDeviceGetCount=(TcuDeviceGetCount*)lib_nvcuda.resolve("cuDeviceGetCount");
    f_cuDeviceGetName=(TcuDeviceGetName*)lib_nvcuda.resolve("cuDeviceGetName");
    f_cuDeviceGetAttribute=(TcuDeviceGetAttribute*)lib_nvcuda.resolve("cuDeviceGetAttribute");

    return true;

err:
    f_cuGetErrorName=nullptr;
    f_cuInit=nullptr;
    f_cuDeviceGetCount=nullptr;
    f_cuDeviceGetName=nullptr;
    f_cuDeviceGetAttribute=nullptr;

    lib_nvcuda.unload();

    return false;
}

bool NvLibPrivate::initNvApi()
{
#ifdef __linux__

    return false;

#endif

    if(sizeof(void*)==8)
        lib_nvapi.setFileName("nvapi64");

    else
        lib_nvapi.setFileName("nvapi");

    if(!lib_nvapi.load()) {
        qWarning() << "error" << lib_nvapi.errorString();
        goto err;
    }

    f_nvapi_QueryInterface=(Tnvapi_QueryInterface*)lib_nvapi.resolve("nvapi_QueryInterface");
    f_nvapi_Initialize=(Tnvapi_Initialize*)(*f_nvapi_QueryInterface)(0x0150e828);
    f_nvapi_EnumPhysicalGPUs=(Tnvapi_EnumPhysicalGPUs*)(*f_nvapi_QueryInterface)(0xe5ac921f);
    f_nvapi_GPU_GetUsages=(Tnvapi_GPU_GetUsages*)(*f_nvapi_QueryInterface)(0x189a1fdf);
    f_nvapi_GPU_GetThermalSettings=(Tnvapi_GPU_GetThermalSettings*)(*f_nvapi_QueryInterface)(0xe3640a56);

    return true;

err:
    f_nvapi_QueryInterface=nullptr;
    f_nvapi_Initialize=nullptr;
    f_nvapi_EnumPhysicalGPUs=nullptr;
    f_nvapi_GPU_GetUsages=nullptr;
    f_nvapi_GPU_GetThermalSettings=nullptr;

    lib_nvapi.unload();

    return false;
}
