/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#ifndef NV_LIB_H
#define NV_LIB_H

#define NVAPI_MAX_PHYSICAL_GPUS 64
#define NVAPI_MAX_USAGES_PER_GPU 34
#define NVAPI_MAX_THERMAL_SENSORS_PER_GPU 3

typedef enum {
    NVAPI_THERMAL_TARGET_NONE=0,
    NVAPI_THERMAL_TARGET_GPU=1,
    NVAPI_THERMAL_TARGET_MEMORY=2,
    NVAPI_THERMAL_TARGET_POWER_SUPPLY=4,
    NVAPI_THERMAL_TARGET_BOARD=8,
    NVAPI_THERMAL_TARGET_VCD_BOARD=9,
    NVAPI_THERMAL_TARGET_VCD_INLET=10,
    NVAPI_THERMAL_TARGET_VCD_OUTLET=11,

    NVAPI_THERMAL_TARGET_ALL=15,
    NVAPI_THERMAL_TARGET_UNKNOWN=-1,

} NV_THERMAL_TARGET;

typedef struct {
    int version;
    int count;

    struct {
        int controller;
        int temp_default_min;
        int temp_default_max;
        int temp_current;
        NV_THERMAL_TARGET target;

    } sensor[NVAPI_MAX_THERMAL_SENSORS_PER_GPU];

} NV_GPU_THERMAL_SETTINGS;

struct NvLibPrivate;

class NvLib
{
public:
    NvLib();
    ~NvLib();

    bool loadedCuda() const;
    bool loadedNvApi() const;

    int cuGetErrorName(int error, const char **pstr);
    int cuInit(unsigned int flags);
    int cuDeviceGetCount(int *count);
    int cuDeviceGetName(char *name, int len, int dev);
    int cuDeviceGetAttribute(int *pi, int attrib, int dev);

    int *nvapiQueryInterface(unsigned int offset);
    int nvapiInitialize();
    int nvapiEnumPhysicalGPUs(int **handles, int *count);
    int nvapiGPUGetUsages(int *handle, unsigned int *usages);
    int nvapiGPUGetThermalSettings(int *handle, int sensor_index, NV_GPU_THERMAL_SETTINGS *temp);

private:
    NvLibPrivate *d;
};

#endif // NV_LIB_H
