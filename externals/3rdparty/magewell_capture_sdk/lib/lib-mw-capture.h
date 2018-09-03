#ifndef LIBMWCAPTURE_H
#define LIBMWCAPTURE_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <fnmatch.h>
#include <stdbool.h>
#include <sys/types.h>
#include <dirent.h>

#include "../inc/mw-linux.h"
#include "../inc/mw-dma-mem.h"
#include "../inc/mw-event-ioctl.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CHANNEL_COUNT       64
#define MAX_BOARD_ID            16
#define MAX_CHANNEL_ID          4

#define MAX_CHANNEL_NAME_LEN    16

#ifndef HCHANNEL
#define HCHANNEL int
#endif

#ifndef HTIMER
#define HTIMER MWCAP_PTR
#endif

#ifndef HNOTIFY
#define HNOTIFY MWCAP_PTR
#endif

#ifndef HOSD
#define HOSD MWCAP_PTR
#endif

#ifndef LPVOID
#define LPVOID void *
#endif

#ifndef ULONG
#define ULONG unsigned long
#endif

#ifndef HANDLE64
#define HANDLE64 MWCAP_PTR
#endif


typedef enum _MW_RESULT_ {
    MW_SUCCEEDED = 0x00,
    MW_FAILED,
    MW_ENODATA,
    MW_INVALID_PARAMS,
} MW_RESULT;


MW_RESULT
MWGetVersion(
    BYTE*							pbyMaj,
    BYTE*							pbyMin,
    WORD*							pwBuild
    );

BOOL
MWCaptureInitInstance(
    );

void
MWCaptureExitInstance(
    );

MW_RESULT
MWRefreshDevice(
    );


int
MWGetChannelCount(
    );


MW_RESULT
MWGetChannelInfoByIndex(
    int								nIndex,
    MWCAP_CHANNEL_INFO *			pChannelInfo
    );

MW_RESULT
MWGetFamilyInfoByIndex(
    int								nIndex,
    LPVOID							pFamilyInfo,
    DWORD							dwSize
    );

MW_RESULT
MWGetDevicePath(
    int								nIndex,
    char*							pDevicePath
);

//channel
HCHANNEL
MWOpenChannel(
    int								nBoardValue,
    int								nChannelIndex
    );

HCHANNEL
MWOpenChannelByPath(
    const char *					pszDevicePath
    );


void
MWCloseChannel(
    HCHANNEL						hChannel
    );

MW_RESULT
MWGetChannelInfo(
    HCHANNEL						hChannel,
    MWCAP_CHANNEL_INFO *			pChannelInfo
    );

MW_RESULT
MWGetFamilyInfo(
    HCHANNEL						hChannel,
    LPVOID							pFamilyInfo,
    DWORD							dwSize
    );

MW_RESULT
MWGetVideoCaps(
    HCHANNEL						hChannel,
    MWCAP_VIDEO_CAPS*				pVideoCaps
    );

MW_RESULT
MWGetAudioCaps(
    HCHANNEL						hChannel,
    MWCAP_AUDIO_CAPS*				pAudioCaps
    );

MW_RESULT
MWGetVideoInputSourceArray(
    HCHANNEL						hChannel,
    DWORD*							pdwInputSource,
    DWORD*							pdwInputCount
    );

MW_RESULT
MWGetAudioInputSourceArray(
    HCHANNEL						hChannel,
    DWORD*							pdwInputSource,
    DWORD*							pdwInputCount
    );

MW_RESULT
MWGetInputSourceScan(
    HCHANNEL 						hChannel,
    BOOLEAN*						pbScan
    );

MW_RESULT
MWSetInputSourceScan(
    HCHANNEL 						hChannel,
    BOOLEAN							bScan
    );

MW_RESULT
MWGetAVInputSourceLink(
    HCHANNEL 						hChannel,
    BOOLEAN*						pbLink
    );

MW_RESULT
MWSetAVInputSourceLink(
    HCHANNEL 						hChannel,
    BOOLEAN							bLink
    );

MW_RESULT
MWGetVideoInputSource(
    HCHANNEL						hChannel,
    DWORD*							pdwSource
    );

MW_RESULT
MWSetVideoInputSource(
    HCHANNEL						hChannel,
    DWORD							dwSource
    );

MW_RESULT
MWGetAudioInputSource(
    HCHANNEL						hChannel,
    DWORD*							pdwSource
    );

MW_RESULT
MWSetAudioInputSource(
    HCHANNEL						hChannel,
    DWORD							dwSource
    );

//EDID
MW_RESULT
MWGetEDID(
    HCHANNEL						hChannel,
    BYTE*							pbyData,
    ULONG*							pulSize
    );

MW_RESULT
MWSetEDID(
    HCHANNEL						hChannel,
    BYTE*							pbyData,
    ULONG							ulSize
    );

// Signal Status
MW_RESULT
MWGetInputSpecificStatus(
    HCHANNEL						hChannel,
    MWCAP_INPUT_SPECIFIC_STATUS *	pInputStatus
    );

MW_RESULT
MWGetVideoSignalStatus(
    HCHANNEL						hChannel,
    MWCAP_VIDEO_SIGNAL_STATUS *		pSignalStatus
    );

MW_RESULT
MWGetAudioSignalStatus(
    HCHANNEL						hChannel,
    MWCAP_AUDIO_SIGNAL_STATUS *		pSignalStatus
    );

//HDMI InfoFrame
MW_RESULT
MWGetHDMIInfoFrameValidFlag(
    HCHANNEL						hChannel,
    DWORD*							pdwValidFlag
    );

MW_RESULT
MWGetHDMIInfoFramePacket(
    HCHANNEL						hChannel,
    MWCAP_HDMI_INFOFRAME_ID			id,
    HDMI_INFOFRAME_PACKET*			pPacket
    );

// Device Clock
MW_RESULT
MWGetDeviceTime(
    HCHANNEL						hChannel,
    LONGLONG*						pllTime
    );

MW_RESULT
MWSetDeviceTime(
    HCHANNEL						hChannel,
    LONGLONG						llTime
    );

MW_RESULT
MWRegulateDeviceTime(
    HCHANNEL						hChannel,
    LONGLONG						llTime
    );

// Timer Event
HTIMER
MWRegisterTimer(
    HCHANNEL						hChannel,
    HANDLE64						hEvent
    );

MW_RESULT
MWUnregisterTimer(
    HCHANNEL 						hChannel,
    HTIMER							hTimer
    );


MW_RESULT
MWScheduleTimer(
    HCHANNEL 						hChannel,
    HTIMER							hTimer,
    LONGLONG						llExpireTime
    );

// Notify Event
HNOTIFY
MWRegisterNotify(
    HCHANNEL 						hChannel,
    HANDLE64						hEvent,
    DWORD							dwEnableBits
    );

MW_RESULT
MWUnregisterNotify(
    HCHANNEL 						hChannel,
    HNOTIFY							hNotify
    );

MW_RESULT
MWGetNotifyStatus(
    HCHANNEL 						hChannel,
    HNOTIFY							hNotify,
    ULONGLONG*						pullStatus
    );

// Video Capture
MW_RESULT
MWStartVideoCapture(
    HCHANNEL 						hChannel,
    HANDLE64						hEvent
    );

MW_RESULT
MWStopVideoCapture(
    HCHANNEL 						hChannel
    );


MW_RESULT
MWPinVideoBuffer(
    HCHANNEL 						hChannel,
    MWCAP_PTR						pbFrame,
    DWORD							cbFrame
    );

MW_RESULT
MWUnpinVideoBuffer(
    HCHANNEL 						hChannel,
    MWCAP_PTR						pbFrame
    );

MW_RESULT
MWCaptureVideoFrameToVirtualAddress(
    HCHANNEL 						hChannel,
    int								iFrame,
    MWCAP_PTR						pbFrame,
    DWORD							cbFrame,
    DWORD							cbStride,
    BOOLEAN							bBottomUp,
    MWCAP_PTR						pvContext,
    DWORD							dwFOURCC,
    int								cx,
    int								cy
    );

MW_RESULT
MWCaptureVideoFrameToPhysicalAddress(
    HCHANNEL 						hChannel,
    int								iFrame,
    LARGE_INTEGER					liFrameAddress,
    DWORD							cbFrame,
    DWORD							cbStride,
    BOOLEAN							bBottomUp,
    MWCAP_PTR						pvContext,
    DWORD							dwFOURCC,
    int								cx,
    int								cy
    );

MW_RESULT
MWCaptureVideoFrameWithOSDToVirtualAddress(
    HCHANNEL 						hChannel,
    int								iFrame,
    MWCAP_PTR						pbFrame,
    DWORD							cbFrame,
    DWORD							cbStride,
    BOOLEAN							bBottomUp,
    MWCAP_PTR       				pvContext,
    DWORD							dwFOURCC,
    int								cx,
    int								cy,
    MWCAP_PTR						pOSDImage,
    const RECT *					pOSDRects,
    int								cOSDRects
    );

MW_RESULT
MWCaptureVideoFrameWithOSDToPhysicalAddress(
    HCHANNEL 						hChannel,
    int								iFrame,
    LARGE_INTEGER					liFrameAddress,
    DWORD							cbFrame,
    DWORD							cbStride,
    BOOLEAN							bBottomUp,
    MWCAP_PTR						pvContext,
    DWORD							dwFOURCC,
    int								cx,
    int								cy,
    MWCAP_PTR						pOSDImage,
    const RECT *					pOSDRects,
    int								cOSDRects
    );

MW_RESULT
MWCaptureVideoFrameToVirtualAddressEx(
    HCHANNEL 						hChannel,
    int								iFrame,
    MWCAP_PTR						pbFrame,
    DWORD							cbFrame,
    DWORD							cbStride,
    BOOLEAN							bBottomUp,
    MWCAP_PTR						pvContext,
    DWORD							dwFOURCC,
    int								cx,
    int								cy,
    DWORD							dwProcessSwitchs,
    int								cyPartialNotify,
    MWCAP_PTR						pOSDImage,
    const RECT *					pOSDRects,
    int								cOSDRects,
    SHORT							sContrast,
    SHORT							sBrightness,
    SHORT							sSaturation,
    SHORT							sHue,
    MWCAP_VIDEO_DEINTERLACE_MODE			deinterlaceMode,
    MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE	aspectRatioConvertMode,
    const RECT *							pRectSrc,
    const RECT *							pRectDest,
    int										nAspectX,
    int										nAspectY,
    MWCAP_VIDEO_COLOR_FORMAT				colorFormat,
    MWCAP_VIDEO_QUANTIZATION_RANGE			quantRange,
    MWCAP_VIDEO_SATURATION_RANGE			satRange
    );


MW_RESULT
MWCaptureVideoFrameToPhysicalAddressEx(
    HCHANNEL 						hChannel,
    int								iFrame,
    LARGE_INTEGER					liFrameAddress,
    DWORD							cbFrame,
    DWORD							cbStride,
    BOOLEAN							bBottomUp,
    MWCAP_PTR						pvContext,
    DWORD							dwFOURCC,
    int								cx,
    int								cy,
    DWORD							dwProcessSwitchs,
    int								cyPartialNotify,
    MWCAP_PTR						pOSDImage,
    const RECT *					pOSDRects,
    int								cOSDRects,
    SHORT							sContrast,
    SHORT							sBrightness,
    SHORT							sSaturation,
    SHORT							sHue,
    MWCAP_VIDEO_DEINTERLACE_MODE			deinterlaceMode,
    MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE	aspectRatioConvertMode,
    const RECT *							pRectSrc,
    const RECT *							pRectDest,
    int										nAspectX,
    int										nAspectY,
    MWCAP_VIDEO_COLOR_FORMAT				colorFormat,
    MWCAP_VIDEO_QUANTIZATION_RANGE			quantRange,
    MWCAP_VIDEO_SATURATION_RANGE			satRange
    );

MW_RESULT
MWGetVideoBufferInfo(
    HCHANNEL 						hChannel,
    MWCAP_VIDEO_BUFFER_INFO *		pVideoBufferInfo
    );

MW_RESULT
MWGetVideoFrameInfo(
    HCHANNEL 						hChannel,
    BYTE							i,
    MWCAP_VIDEO_FRAME_INFO*			pVideoFrameInfo
    );

MW_RESULT
MWGetVideoCaptureStatus(
    HCHANNEL 						hChannel,
    MWCAP_VIDEO_CAPTURE_STATUS *	pStatus
    );

// Audio Capture
MW_RESULT
MWStartAudioCapture(
    HCHANNEL 						hChannel
    );

MW_RESULT
MWStopAudioCapture(
    HCHANNEL 						hChannel
    );

MW_RESULT
MWCaptureAudioFrame(
    HCHANNEL 						hChannel,
    MWCAP_AUDIO_CAPTURE_FRAME *     pAudioCaptureFrame
    );


// Video processing
MW_RESULT
MWSetVideoInputAspectRatio(
    HCHANNEL 						hChannel,
    int								nAspectX,
    int								nAspectY
    );

MW_RESULT
MWGetVideoInputAspectRatio(
    HCHANNEL 						hChannel,
    int*							pnAspectX,
    int*							pnAspectY
    );

MW_RESULT
MWSetVideoInputColorFormat(
    HCHANNEL 						hChannel,
    MWCAP_VIDEO_COLOR_FORMAT		colorFormat
    );

MW_RESULT
MWGetVideoInputColorFormat(
    HCHANNEL 						hChannel,
    MWCAP_VIDEO_COLOR_FORMAT *		pColorFormat
    );

MW_RESULT
MWSetVideoInputQuantizationRange(
    HCHANNEL 						hChannel,
    MWCAP_VIDEO_QUANTIZATION_RANGE	quantRange
    );

MW_RESULT
MWGetVideoInputQuantizationRange(
    HCHANNEL 						hChannel,
    MWCAP_VIDEO_QUANTIZATION_RANGE* pQuantRange
    );


// LED Mode
MW_RESULT
MWSetLEDMode(
    HCHANNEL 						hChannel,
    DWORD							dwMode
    );

// Upgrade Firmware
MW_RESULT
MWGetFirmwareStorageInfo(
    HCHANNEL 						hChannel,
    MWCAP_FIRMWARE_STORAGE *		pFirmwareStorageInfo
    );

MW_RESULT
MWEraseFirmwareData(
    HCHANNEL 						hChannel,
    DWORD							cbOffset,
    DWORD							cbErase
    );

MW_RESULT
MWReadFirmwareData(
    HCHANNEL 						hChannel,
    DWORD							cbOffset,
    BYTE *							pbyData,
    DWORD							cbToRead,
    DWORD *							pcbRead
    );

MW_RESULT
MWWriteFirmwareData(
    HCHANNEL 						hChannel,
    DWORD							cbOffset,
    BYTE *							pbyData,
    DWORD							cbData
    );

MW_RESULT
MWSetPostReconfig(
    HCHANNEL 						hChannel,
    DWORD							dwDelayMS
    );

// OSD
HOSD
MWCreateImage(
    HCHANNEL 						hChannel,
    int								cx,
    int								cy
    );

MW_RESULT
MWOpenImage(
    HCHANNEL 						hChannel,
    HOSD							hImage,
    LONG*							plRet
    );

MW_RESULT
MWCloseImage(
    HCHANNEL 						hChannel,
    HOSD							hImage,
    LONG*							plRet
    );

MW_RESULT
MWUploadImageFromVirtualAddress(
    HCHANNEL 						hChannel,
    HOSD							hImage,
    MWCAP_VIDEO_COLOR_FORMAT		cfDest,
    MWCAP_VIDEO_QUANTIZATION_RANGE	quantRangeDest,
    MWCAP_VIDEO_SATURATION_RANGE	satRangeDest,
    WORD							xDest,
    WORD							yDest,
    WORD							cxDest,
    WORD							cyDest,
    MWCAP_PTR						pvSrcFrame,
    DWORD							cbSrcFrame,
    DWORD							cbSrcStride,
    WORD							cxSrc,
    WORD							cySrc,
    BOOLEAN							bSrcBottomUp,
    BOOLEAN							bSrcPixelAlpha,
    BOOLEAN							bSrcPixelXBGR
    );

MW_RESULT
MWUploadImageFromPhysicalAddress(
    HCHANNEL 						hChannel,
    HOSD							hImage,
    MWCAP_VIDEO_COLOR_FORMAT		cfDest,
    MWCAP_VIDEO_QUANTIZATION_RANGE	quantRangeDest,
    MWCAP_VIDEO_SATURATION_RANGE	satRangeDest,
    WORD							xDest,
    WORD							yDest,
    WORD							cxDest,
    WORD							cyDest,
    LARGE_INTEGER					liSrcFrameAddress,
    DWORD							cbSrcFrame,
    DWORD							cbSrcStride,
    WORD							cxSrc,
    WORD							cySrc,
    BOOLEAN							bSrcBottomUp,
    BOOLEAN							bSrcPixelAlpha,
    BOOLEAN							bSrcPixelXBGR
    );

//Temperature
MW_RESULT MWGetTemperature(
    HCHANNEL                        hChannel,
    DWORD *                         pnTemp
    );

// V4l2
MW_RESULT MWGetStreamCount(
    HCHANNEL                        hChannel,
    int *                           pnCount
    );

MW_RESULT MWGetStreamInfos(
    HCHANNEL                        hChannel,
    MWCAP_STREAM_INFO *             pStreamInfos,
    int *                           pnCount
    );

MW_RESULT MWSetCtrlStreamID(
    HCHANNEL                        hChannel,
    int                             nCrtlID
    );

MW_RESULT MWGetVideoConnectFormat(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CONNECTION_FORMAT * pConnectFormat
    );

MW_RESULT MWGetVideoProcessSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_PROCESS_SETTINGS *  pProcessSettings
    );

MW_RESULT MWSetVideoProcessSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_PROCESS_SETTINGS    processSettings
    );

MW_RESULT MWGetVideoOSDSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_SETTINGS *      pOSDSettings
    );

MW_RESULT MWSetVideoOSDSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_SETTINGS        OSDSettings
    );

MW_RESULT MWGetVideoOSDImage(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_IMAGE *         pOSDImage
    );

MW_RESULT MWSetVideoOSDImage(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_IMAGE           OSGImage
    );

MW_RESULT MWGetVideoBrightness(
    HCHANNEL hChannel,
    int *pnBrightness
    );

MW_RESULT MWSetVideoBrightness(
    HCHANNEL                        hChannel,
    int                             nBrightness
    );

MW_RESULT MWGetVideoContrast(
    HCHANNEL                        hChannel,
    int *                           pnContrast
    );

MW_RESULT MWSetVideoContrast(
    HCHANNEL                        hChannel,
    int                             nContrast
    );

MW_RESULT MWGetVideoHue(
    HCHANNEL                        hChannel,
    int *                           pnHue
    );

MW_RESULT MWSetVideoHue(
    HCHANNEL                        hChannel,
    int                             nHue
    );

MW_RESULT MWGetVideoSaturation(
    HCHANNEL                        hChannel,
    int *                           pnSaturation
    );

MW_RESULT MWSetVideoSaturation(
    HCHANNEL                        hChannel,
    int                             nSaturation
    );


MW_RESULT MWSaveSettingsAsPreset(
    HCHANNEL                        hChannel
    );

MW_RESULT MWReloadPreset(
    HCHANNEL                        hChannel
    );

// VGA/Component timings
MW_RESULT MWGetVideoAutoHAlign(
    HCHANNEL                        hChannel,
    BOOLEAN *                       pbAuto
    );

MW_RESULT MWSetVideoAutoHAlign(
    HCHANNEL                        hChannel,
    BOOLEAN                         bAuto
    );

MW_RESULT MWGetVideoSamplingPhase(
    HCHANNEL                        hChannel,
    BYTE *                          pbyValue
    );

MW_RESULT MWSetVideoSamplingPhase(
    HCHANNEL                        hChannel,
    BYTE                            byValue
    );

MW_RESULT MWGetVideoSamplingPhaseAutoAdjust(
    HCHANNEL                        hChannel,
    BOOLEAN *                       pbAuto
    );

MW_RESULT MWSetVideoSamplingPhaseAutoAdjust(
    HCHANNEL                        hChannel,
    BOOLEAN                         bAuto
    );

MW_RESULT MWSetVideoTiming(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_TIMING              videoTiming
    );

MW_RESULT MWGetVideoPreferredTimingArray(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_TIMING *            pVideoTiming,
    long *                          plSize
    );

MW_RESULT MWSetCustomVideoTiming(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CUSTOM_TIMING       videoTiming
    );

MW_RESULT MWGetCustomVideoTimingsCount(
    HCHANNEL                        hChannel,
    DWORD   *                       pdwCount
    );

MW_RESULT MWGetCustomVideoTimingsArray(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CUSTOM_TIMING *     pVideoCustomTiming,
    DWORD *                         pdwCount
    );

MW_RESULT MWSetCustomVideoTimingsArray(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CUSTOM_TIMING *     pVideoCustomTiming,
    DWORD                           dwCount
    );

MW_RESULT MWGetCustomVideoResolutionsCount(
    HCHANNEL                        hChannel,
    DWORD *                         pdwCount
    );

MW_RESULT MWGetCustomVideoResolutionsArray(
    HCHANNEL                        hChannel,
    MWCAP_SIZE *                    pResolutionSize,
    DWORD *                         pdwCount
    );

MW_RESULT MWSetCustomVideoResolutionsArray(
    HCHANNEL                        hChannel,
    MWCAP_SIZE *                    pResolutionSize,
    DWORD                           dwCount
    );

// Event
MWCAP_PTR MWCreateEvent(
    );

MW_RESULT MWCloseEvent(
    MWCAP_PTR                       hEvent
    );

MW_RESULT MWSetEvent(
    MWCAP_PTR                       hEvent
    );

MW_RESULT MWClearEvent(
    MWCAP_PTR                       hEvent
    );

BOOLEAN MWIsSetEvent(
    MWCAP_PTR                       hEvent
    );

int MWTryWaitEvent(
    MWCAP_PTR                       hEvent
    );

int MWWaitEvent(
    MWCAP_PTR                       hEvent,
    int                             nTimeout
    );

DWORD MWMultiWaitEvent(
    MWCAP_PTR *                 hEvents,
    int                         nCount,
    int                         nTimeout
    );


#ifdef __cplusplus
}
#endif



#endif // LIBMWCAPTURE_H
