/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#include "magewell_lib.h"

#ifdef LIB_MWCAPTURE
#  ifdef __WIN32__

#include "MWCapture.h"

typedef BOOL TMWCaptureInitInstance();
typedef void TMWCaptureExitInstance();
typedef MW_RESULT TMWRefreshDevice();
typedef int TMWGetChannelCount();
typedef MW_RESULT TMWGetDevicePath(int, WCHAR*);
typedef HCHANNEL TMWOpenChannelByPath(const WCHAR*);
typedef void TMWCloseChannel(HCHANNEL);
typedef MW_RESULT TMWGetChannelInfo(HCHANNEL, MWCAP_CHANNEL_INFO*);
typedef MW_RESULT TMWGetInputSpecificStatus(HCHANNEL, MWCAP_INPUT_SPECIFIC_STATUS*);
typedef MW_RESULT TMWGetVideoSignalStatus(HCHANNEL, MWCAP_VIDEO_SIGNAL_STATUS*);
typedef MW_RESULT TMWGetAudioSignalStatus(HCHANNEL, MWCAP_AUDIO_SIGNAL_STATUS*);
typedef MW_RESULT TMWSetVideoInputColorFormat(HCHANNEL, MWCAP_VIDEO_COLOR_FORMAT);
typedef MW_RESULT TMWSetVideoInputQuantizationRange(HCHANNEL, MWCAP_VIDEO_QUANTIZATION_RANGE);
typedef HCHANNEL TMWOpenChannel(int, int);
typedef MW_RESULT TMWSetDeviceTime(HCHANNEL, LONGLONG);
typedef HNOTIFY TMWRegisterNotify(HCHANNEL, HANDLE, DWORD);
typedef MW_RESULT TMWUnregisterNotify(HCHANNEL, HNOTIFY);
typedef MW_RESULT TMWGetNotifyStatus(HCHANNEL, HNOTIFY, ULONGLONG*);
typedef MW_RESULT TMWStartVideoCapture(HCHANNEL, HANDLE);
typedef MW_RESULT TMWStopVideoCapture(HCHANNEL);
typedef MW_RESULT TMWCaptureVideoFrameToVirtualAddressEx(HCHANNEL, int, LPBYTE, DWORD, DWORD, BOOLEAN, MWCAP_PTR64, DWORD, int, int, DWORD,
                                                         int, HOSD, const RECT*, int, SHORT, SHORT, SHORT, SHORT, MWCAP_VIDEO_DEINTERLACE_MODE, MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE,
                                                         const RECT*, const RECT*, int, int, MWCAP_VIDEO_COLOR_FORMAT, MWCAP_VIDEO_QUANTIZATION_RANGE, MWCAP_VIDEO_SATURATION_RANGE);
typedef MW_RESULT TMWGetVideoBufferInfo(HCHANNEL, MWCAP_VIDEO_BUFFER_INFO*);
typedef MW_RESULT TMWGetVideoFrameInfo(HCHANNEL, BYTE, MWCAP_VIDEO_FRAME_INFO*);
typedef MW_RESULT TMWGetVideoCaptureStatus(HCHANNEL, MWCAP_VIDEO_CAPTURE_STATUS*);
typedef MW_RESULT TMWStartAudioCapture(HCHANNEL);
typedef MW_RESULT TMWCaptureAudioFrame(HCHANNEL, MWCAP_AUDIO_CAPTURE_FRAME*);
typedef MW_RESULT TMWStopAudioCapture(HCHANNEL);
typedef MW_RESULT TMWGetTemperature(HCHANNEL, unsigned int*);

#  endif // __WIN32__

struct MagewellLibPrivate
{
#  ifdef __WIN32__

    TMWCaptureInitInstance *f_CaptureInitInstance=nullptr;
    TMWCaptureExitInstance *f_CaptureExitInstance=nullptr;
    TMWRefreshDevice *f_RefreshDevice=nullptr;
    TMWGetChannelCount *f_GetChannelCount=nullptr;
    TMWGetDevicePath *f_GetDevicePath=nullptr;
    TMWOpenChannelByPath *f_OpenChannelByPath=nullptr;
    TMWCloseChannel *f_CloseChannel=nullptr;
    TMWGetChannelInfo *f_GetChannelInfo=nullptr;
    TMWGetInputSpecificStatus *f_GetInputSpecificStatus=nullptr;
    TMWGetVideoSignalStatus *f_GetVideoSignalStatus=nullptr;
    TMWGetAudioSignalStatus *f_GetAudioSignalStatus=nullptr;
    TMWSetVideoInputColorFormat *f_SetVideoInputColorFormat=nullptr;
    TMWSetVideoInputQuantizationRange *f_SetVideoInputQuantizationRange=nullptr;
    TMWOpenChannel *f_OpenChannel=nullptr;
    TMWSetDeviceTime *f_SetDeviceTime=nullptr;
    TMWRegisterNotify *f_RegisterNotify=nullptr;
    TMWUnregisterNotify *f_UnregisterNotify=nullptr;
    TMWGetNotifyStatus *f_GetNotifyStatus=nullptr;
    TMWStartVideoCapture *f_StartVideoCapture=nullptr;
    TMWStopVideoCapture *f_StopVideoCapture=nullptr;
    TMWCaptureVideoFrameToVirtualAddressEx *f_CaptureVideoFrameToVirtualAddressEx=nullptr;
    TMWGetVideoBufferInfo *f_GetVideoBufferInfo=nullptr;
    TMWGetVideoFrameInfo *f_GetVideoFrameInfo=nullptr;
    TMWGetVideoCaptureStatus *f_GetVideoCaptureStatus=nullptr;
    TMWStartAudioCapture *f_StartAudioCapture=nullptr;
    TMWCaptureAudioFrame *f_CaptureAudioFrame=nullptr;
    TMWStopAudioCapture *f_StopAudioCapture=nullptr;
    TMWGetTemperature *f_GetTemperature=nullptr;

    QLibrary lib;

#  endif // __WIN32__
};


MagewellLib *magewell_lib=nullptr;


#  ifdef __WIN32__

BOOL MWCaptureInitInstance()
{
    if(magewell_lib && magewell_lib->d->f_CaptureInitInstance)
        return magewell_lib->d->f_CaptureInitInstance();

    return false;
}

void MWCaptureExitInstance()
{
    if(magewell_lib && magewell_lib->d->f_CaptureExitInstance)
        magewell_lib->d->f_CaptureExitInstance();
}

MW_RESULT MWRefreshDevice()
{
    if(magewell_lib && magewell_lib->d->f_RefreshDevice)
        return magewell_lib->d->f_RefreshDevice();

    return MW_FAILED;
}

int MWGetChannelCount()
{
    if(magewell_lib && magewell_lib->d->f_GetChannelCount)
        return magewell_lib->d->f_GetChannelCount();

    return 0;
}

MW_RESULT MWGetDevicePath(int nIndex, WCHAR *pDevicePath)
{
    if(magewell_lib && magewell_lib->d->f_GetDevicePath)
        return magewell_lib->d->f_GetDevicePath(nIndex, pDevicePath);

    return MW_FAILED;
}

HCHANNEL MWOpenChannelByPath(const WCHAR *pszDevicePath)
{
    if(magewell_lib && magewell_lib->d->f_OpenChannelByPath)
        return magewell_lib->d->f_OpenChannelByPath(pszDevicePath);

    return 0;
}

void MWCloseChannel(HCHANNEL hChannel)
{
    if(magewell_lib && magewell_lib->d->f_CloseChannel)
        magewell_lib->d->f_CloseChannel(hChannel);
}

MW_RESULT MWGetChannelInfo(HCHANNEL hChannel, MWCAP_CHANNEL_INFO *pChannelInfo)
{
    if(magewell_lib && magewell_lib->d->f_GetChannelInfo)
        return magewell_lib->d->f_GetChannelInfo(hChannel, pChannelInfo);

    return MW_FAILED;
}

MW_RESULT MWGetInputSpecificStatus(HCHANNEL hChannel, MWCAP_INPUT_SPECIFIC_STATUS *pInputStatus)
{
    if(magewell_lib && magewell_lib->d->f_GetInputSpecificStatus)
        return magewell_lib->d->f_GetInputSpecificStatus(hChannel, pInputStatus);

    return MW_FAILED;
}

MW_RESULT MWGetVideoSignalStatus(HCHANNEL hChannel, MWCAP_VIDEO_SIGNAL_STATUS *pSignalStatus)
{
    if(magewell_lib && magewell_lib->d->f_GetVideoSignalStatus)
        return magewell_lib->d->f_GetVideoSignalStatus(hChannel, pSignalStatus);

    return MW_FAILED;
}

MW_RESULT MWGetAudioSignalStatus(HCHANNEL hChannel, MWCAP_AUDIO_SIGNAL_STATUS *pSignalStatus)
{
    if(magewell_lib && magewell_lib->d->f_GetAudioSignalStatus)
        return magewell_lib->d->f_GetAudioSignalStatus(hChannel, pSignalStatus);

    return MW_FAILED;
}

MW_RESULT MWSetVideoInputColorFormat(HCHANNEL hChannel, MWCAP_VIDEO_COLOR_FORMAT colorFormat)
{
    if(magewell_lib && magewell_lib->d->f_SetVideoInputColorFormat)
        return magewell_lib->d->f_SetVideoInputColorFormat(hChannel, colorFormat);

    return MW_FAILED;
}

MW_RESULT MWSetVideoInputQuantizationRange(HCHANNEL hChannel, MWCAP_VIDEO_QUANTIZATION_RANGE quantRange)
{
    if(magewell_lib && magewell_lib->d->f_SetVideoInputQuantizationRange)
        return magewell_lib->d->f_SetVideoInputQuantizationRange(hChannel, quantRange);

    return MW_FAILED;
}

HCHANNEL MWOpenChannel(int nBoardValue, int nChannelIndex)
{
    if(magewell_lib && magewell_lib->d->f_OpenChannel)
        return magewell_lib->d->f_OpenChannel(nBoardValue, nChannelIndex);

    return 0;
}

MW_RESULT MWSetDeviceTime(HCHANNEL hChannel, LONGLONG llTime)
{
    if(magewell_lib && magewell_lib->d->f_SetDeviceTime)
        return magewell_lib->d->f_SetDeviceTime(hChannel, llTime);

    return MW_FAILED;
}

HNOTIFY MWRegisterNotify(HCHANNEL hChannel, HANDLE hEvent, DWORD dwEnableBits)
{
    if(magewell_lib && magewell_lib->d->f_RegisterNotify)
        return magewell_lib->d->f_RegisterNotify(hChannel, hEvent, dwEnableBits);

    return 0;
}

MW_RESULT MWUnregisterNotify(HCHANNEL hChannel, HNOTIFY hNotify)
{
    if(magewell_lib && magewell_lib->d->f_UnregisterNotify)
        return magewell_lib->d->f_UnregisterNotify(hChannel, hNotify);

    return MW_FAILED;
}

MW_RESULT MWGetNotifyStatus(HCHANNEL hChannel, HNOTIFY hNotify, ULONGLONG *pullStatus)
{
    if(magewell_lib && magewell_lib->d->f_GetNotifyStatus)
        return magewell_lib->d->f_GetNotifyStatus(hChannel, hNotify, pullStatus);

    return MW_FAILED;
}

MW_RESULT MWStartVideoCapture(HCHANNEL hChannel, HANDLE hEvent)
{
    if(magewell_lib && magewell_lib->d->f_StartVideoCapture)
        return magewell_lib->d->f_StartVideoCapture(hChannel, hEvent);

    return MW_FAILED;
}

MW_RESULT MWStopVideoCapture(HCHANNEL hChannel)
{
    if(magewell_lib && magewell_lib->d->f_StopVideoCapture)
        return magewell_lib->d->f_StopVideoCapture(hChannel);

    return MW_FAILED;
}

MW_RESULT MWCaptureVideoFrameToVirtualAddressEx(HCHANNEL hChannel, int iFrame, LPBYTE pbFrame, DWORD cbFrame,
                                                DWORD cbStride, BOOLEAN bBottomUp, MWCAP_PTR64 pvContext, DWORD dwFOURCC, int cx, int cy, DWORD dwProcessSwitchs,
                                                int cyParitalNotify, HOSD hOSDImage, const RECT *pOSDRects, int cOSDRects, SHORT sContrast, SHORT sBrightness,
                                                SHORT sSaturation, SHORT sHue, MWCAP_VIDEO_DEINTERLACE_MODE deinterlaceMode, MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE aspectRatioConvertMode,
                                                const RECT *pRectSrc, const RECT *pRectDest, int nAspectX, int nAspectY,
                                                MWCAP_VIDEO_COLOR_FORMAT colorFormat, MWCAP_VIDEO_QUANTIZATION_RANGE quantRange, MWCAP_VIDEO_SATURATION_RANGE satRange)
{
    if(magewell_lib && magewell_lib->d->f_CaptureVideoFrameToVirtualAddressEx)
        return magewell_lib->d->f_CaptureVideoFrameToVirtualAddressEx(hChannel, iFrame, pbFrame, cbFrame, cbStride, bBottomUp, pvContext, dwFOURCC, cx, cy, dwProcessSwitchs,
                                                                      cyParitalNotify, hOSDImage, pOSDRects, cOSDRects, sContrast, sBrightness, sSaturation, sHue, deinterlaceMode,
                                                                      aspectRatioConvertMode, pRectSrc, pRectDest, nAspectX, nAspectY, colorFormat, quantRange, satRange);

    return MW_FAILED;
}

MW_RESULT MWGetVideoBufferInfo(HCHANNEL hChannel, MWCAP_VIDEO_BUFFER_INFO *pVideoBufferInfo)
{
    if(magewell_lib && magewell_lib->d->f_GetVideoBufferInfo)
        return magewell_lib->d->f_GetVideoBufferInfo(hChannel, pVideoBufferInfo);

    return MW_FAILED;
}

MW_RESULT MWGetVideoFrameInfo(HCHANNEL hChannel, BYTE i, MWCAP_VIDEO_FRAME_INFO *pVideoFrameInfo)
{
    if(magewell_lib && magewell_lib->d->f_GetVideoFrameInfo)
        return magewell_lib->d->f_GetVideoFrameInfo(hChannel, i, pVideoFrameInfo);

    return MW_FAILED;
}

MW_RESULT MWGetVideoCaptureStatus(HCHANNEL hChannel, MWCAP_VIDEO_CAPTURE_STATUS *pStatus)
{
    if(magewell_lib && magewell_lib->d->f_GetVideoCaptureStatus)
        return magewell_lib->d->f_GetVideoCaptureStatus(hChannel, pStatus);

    return MW_FAILED;
}

MW_RESULT MWStartAudioCapture(HCHANNEL hChannel)
{
    if(magewell_lib && magewell_lib->d->f_StartAudioCapture)
        return magewell_lib->d->f_StartAudioCapture(hChannel);

    return MW_FAILED;
}

MW_RESULT MWCaptureAudioFrame(HCHANNEL hChannel, MWCAP_AUDIO_CAPTURE_FRAME *pAudioCaptureFrame)
{
    if(magewell_lib && magewell_lib->d->f_CaptureAudioFrame)
        return magewell_lib->d->f_CaptureAudioFrame(hChannel, pAudioCaptureFrame);

    return MW_FAILED;
}

MW_RESULT MWStopAudioCapture(HCHANNEL hChannel)
{
    if(magewell_lib && magewell_lib->d->f_StopAudioCapture)
        return magewell_lib->d->f_StopAudioCapture(hChannel);

    return MW_FAILED;
}

MW_RESULT MWGetTemperature(HCHANNEL hChannel, unsigned int *pnTemp)
{
    if(magewell_lib && magewell_lib->d->f_GetTemperature)
        return magewell_lib->d->f_GetTemperature(hChannel, pnTemp);

    return MW_FAILED;
}

#  endif // __WIN32__
#endif // LIB_MWCAPTURE


MagewellLib::MagewellLib()
#ifdef LIB_MWCAPTURE
    : d(new MagewellLibPrivate())
#endif
{
    load();
}

void MagewellLib::init()
{
#ifdef LIB_MWCAPTURE

    magewell_lib=new MagewellLib();

#endif
}

bool MagewellLib::isLoaded()
{
#ifdef LIB_MWCAPTURE
#  ifdef __WIN32__

    if(magewell_lib)
        return magewell_lib->d->lib.isLoaded();

    return false;

#  else

    return true;

#  endif
#endif

    return false;
}

void MagewellLib::load()
{
#ifdef LIB_MWCAPTURE
#  ifdef __WIN32__

    d->lib.setFileName("LibMWCapture");

    if(!d->lib.load()) {
        qWarning() << "error" << d->lib.errorString();
        goto err;
    }

    d->f_CaptureInitInstance=(TMWCaptureInitInstance*)d->lib.resolve("MWCaptureInitInstance");

    if(!d->f_CaptureInitInstance) {
        qWarning() << "MWCaptureInitInstance resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_CaptureExitInstance=(TMWCaptureExitInstance*)d->lib.resolve("MWCaptureExitInstance");

    if(!d->f_CaptureExitInstance) {
        qWarning() << "MWCaptureExitInstance resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_RefreshDevice=(TMWRefreshDevice*)d->lib.resolve("MWRefreshDevice");

    if(!d->f_RefreshDevice) {
        qWarning() << "MWRefreshDevice resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetChannelCount=(TMWGetChannelCount*)d->lib.resolve("MWGetChannelCount");

    if(!d->f_GetChannelCount) {
        qWarning() << "MWGetChannelCount resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetDevicePath=(TMWGetDevicePath*)d->lib.resolve("MWGetDevicePath");

    if(!d->f_GetDevicePath) {
        qWarning() << "MWGetDevicePath resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_OpenChannelByPath=(TMWOpenChannelByPath*)d->lib.resolve("MWOpenChannelByPath");

    if(!d->f_OpenChannelByPath) {
        qWarning() << "MWOpenChannelByPath resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_CloseChannel=(TMWCloseChannel*)d->lib.resolve("MWCloseChannel");

    if(!d->f_CloseChannel) {
        qWarning() << "MWCloseChannel resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetChannelInfo=(TMWGetChannelInfo*)d->lib.resolve("MWGetChannelInfo");

    if(!d->f_GetChannelInfo) {
        qWarning() << "MWGetChannelInfo resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetInputSpecificStatus=(TMWGetInputSpecificStatus*)d->lib.resolve("MWGetInputSpecificStatus");

    if(!d->f_GetInputSpecificStatus) {
        qWarning() << "MWGetInputSpecificStatus resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetVideoSignalStatus=(TMWGetVideoSignalStatus*)d->lib.resolve("MWGetVideoSignalStatus");

    if(!d->f_GetVideoSignalStatus) {
        qWarning() << "MWGetVideoSignalStatus resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetAudioSignalStatus=(TMWGetAudioSignalStatus*)d->lib.resolve("MWGetAudioSignalStatus");

    if(!d->f_GetAudioSignalStatus) {
        qWarning() << "MWGetAudioSignalStatus resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_SetVideoInputColorFormat=(TMWSetVideoInputColorFormat*)d->lib.resolve("MWSetVideoInputColorFormat");

    if(!d->f_SetVideoInputColorFormat) {
        qWarning() << "MWSetVideoInputColorFormat resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_SetVideoInputQuantizationRange=(TMWSetVideoInputQuantizationRange*)d->lib.resolve("MWSetVideoInputQuantizationRange");

    if(!d->f_SetVideoInputQuantizationRange) {
        qWarning() << "MWSetVideoInputQuantizationRange resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_OpenChannel=(TMWOpenChannel*)d->lib.resolve("MWOpenChannel");

    if(!d->f_OpenChannel) {
        qWarning() << "MWOpenChannel resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_SetDeviceTime=(TMWSetDeviceTime*)d->lib.resolve("MWSetDeviceTime");

    if(!d->f_SetDeviceTime) {
        qWarning() << "MWSetDeviceTime resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_RegisterNotify=(TMWRegisterNotify*)d->lib.resolve("MWRegisterNotify");

    if(!d->f_RegisterNotify) {
        qWarning() << "MWRegisterNotify resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_UnregisterNotify=(TMWUnregisterNotify*)d->lib.resolve("MWUnregisterNotify");

    if(!d->f_UnregisterNotify) {
        qWarning() << "MWUnregisterNotify resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetNotifyStatus=(TMWGetNotifyStatus*)d->lib.resolve("MWGetNotifyStatus");

    if(!d->f_GetNotifyStatus) {
        qWarning() << "MWGetNotifyStatus resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_StartVideoCapture=(TMWStartVideoCapture*)d->lib.resolve("MWStartVideoCapture");

    if(!d->f_StartVideoCapture) {
        qWarning() << "MWStartVideoCapture resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_StopVideoCapture=(TMWStopVideoCapture*)d->lib.resolve("MWStopVideoCapture");

    if(!d->f_StopVideoCapture) {
        qWarning() << "MWStopVideoCapture resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_CaptureVideoFrameToVirtualAddressEx=(TMWCaptureVideoFrameToVirtualAddressEx*)d->lib.resolve("MWCaptureVideoFrameToVirtualAddressEx");

    if(!d->f_CaptureVideoFrameToVirtualAddressEx) {
        qWarning() << "MWCaptureVideoFrameToVirtualAddressEx resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetVideoBufferInfo=(TMWGetVideoBufferInfo*)d->lib.resolve("MWGetVideoBufferInfo");

    if(!d->f_GetVideoBufferInfo) {
        qWarning() << "MWGetVideoBufferInfo resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetVideoFrameInfo=(TMWGetVideoFrameInfo*)d->lib.resolve("MWGetVideoFrameInfo");

    if(!d->f_GetVideoFrameInfo) {
        qWarning() << "MWGetVideoFrameInfo resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetVideoCaptureStatus=(TMWGetVideoCaptureStatus*)d->lib.resolve("MWGetVideoCaptureStatus");

    if(!d->f_GetVideoCaptureStatus) {
        qWarning() << "MWGetVideoCaptureStatus resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_StartAudioCapture=(TMWStartAudioCapture*)d->lib.resolve("MWStartAudioCapture");

    if(!d->f_StartAudioCapture) {
        qWarning() << "MWStartAudioCapture resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_CaptureAudioFrame=(TMWCaptureAudioFrame*)d->lib.resolve("MWCaptureAudioFrame");

    if(!d->f_CaptureAudioFrame) {
        qWarning() << "MWCaptureAudioFrame resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_StopAudioCapture=(TMWStopAudioCapture*)d->lib.resolve("MWStopAudioCapture");

    if(!d->f_StopAudioCapture) {
        qWarning() << "MWStopAudioCapture resolve error:" << d->lib.errorString();
        goto err;
    }

    d->f_GetTemperature=(TMWGetTemperature*)d->lib.resolve("MWGetTemperature");

    if(!d->f_GetTemperature) {
        qWarning() << "MWGetTemperature resolve error:" << d->lib.errorString();
        goto err;
    }

    return;

err:

    unload();

#  endif // __WIN32__
#endif // LIB_MWCAPTURE
}

void MagewellLib::unload()
{
#ifdef LIB_MWCAPTURE
#  ifdef __WIN32__

    d->f_CaptureInitInstance=nullptr;
    d->f_CaptureExitInstance=nullptr;
    d->f_RefreshDevice=nullptr;
    d->f_GetChannelCount=nullptr;
    d->f_GetDevicePath=nullptr;
    d->f_OpenChannelByPath=nullptr;
    d->f_CloseChannel=nullptr;
    d->f_GetChannelInfo=nullptr;
    d->f_GetInputSpecificStatus=nullptr;
    d->f_GetVideoSignalStatus=nullptr;
    d->f_GetAudioSignalStatus=nullptr;
    d->f_SetVideoInputColorFormat=nullptr;
    d->f_SetVideoInputQuantizationRange=nullptr;
    d->f_OpenChannel=nullptr;
    d->f_SetDeviceTime=nullptr;
    d->f_RegisterNotify=nullptr;
    d->f_UnregisterNotify=nullptr;
    d->f_GetNotifyStatus=nullptr;
    d->f_StartVideoCapture=nullptr;
    d->f_StopVideoCapture=nullptr;
    d->f_CaptureVideoFrameToVirtualAddressEx=nullptr;
    d->f_GetVideoBufferInfo=nullptr;
    d->f_GetVideoFrameInfo=nullptr;
    d->f_GetVideoCaptureStatus=nullptr;
    d->f_StartAudioCapture=nullptr;
    d->f_CaptureAudioFrame=nullptr;
    d->f_StopAudioCapture=nullptr;
    d->f_GetTemperature=nullptr;

    if(d->lib.isLoaded())
        d->lib.unload();

#  endif // __WIN32__
#endif // LIB_MWCAPTURE
}
