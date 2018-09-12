/************************************************************************************************/
// MWProCapture.h : header file

// MAGEWELL PROPRIETARY INFORMATION

// The following license only applies to head files and library within Magewell's SDK 
// and not to Magewell's SDK as a whole. 

// Copyrights © Nanjing Magewell Electronics Co., Ltd. ("Magewell") All rights reserved.

// Magewell grands to any person who obtains the copy of Magewell's head files and library 
// the rights,including without limitation, to use on the condition that the following terms are met:
// - The above copyright notice shall be retained in any circumstances.
// -The following disclaimer shall be included in the software and documentation and/or 
// other materials provided for the purpose of publish, distribution or sublicense.

// THE SOFTWARE IS PROVIDED BY MAGEWELL "AS IS" AND ANY EXPRESS, INCLUDING BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL MAGEWELL BE LIABLE 

// FOR ANY CLAIM, DIRECT OR INDIRECT DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT,
// TORT OR OTHERWISE, ARISING IN ANY WAY OF USING THE SOFTWARE.

// CONTACT INFORMATION:
// SDK@magewell.net
// http://www.magewell.com/
//
/************************************************************************************************/
#pragma once

#ifdef LIBMWCAPTURE_EXPORTS
#define LIBMWCAPTURE_API __declspec(dllexport)
#elif LIBMWCAPTURE_DLL
#define LIBMWCAPTURE_API __declspec(dllimport)
#else
#define LIBMWCAPTURE_API 
#endif

#include "MWCaptureExtension.h"
#include "MWUSBCaptureExtension.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
@brief Open channel according to the nBoardValue and nChannelIndex
@param[in] nBoardValue      The board value of channel, DIP value
@param[in] nChannelIndex      Index of channel
@return Returns the channel handle if succeeded, otherwise returns -1
@note Refers to MWGetChannel()
*/
// Channel
HCHANNEL
LIBMWCAPTURE_API
MWOpenChannel(
	int								nBoardValue,
	int								nChannelIndex
	);

// Device Clock
/**
@brief Gets Device clock according to the channel handle
@param[in] hChannel      Channel handle
@param[out] pllTime      time 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetDeviceTime(
	HCHANNEL						hChannel,
	LONGLONG*						pllTime
	);

/**
@brief Set Device time according to the channel handle
@param[in] hChannel     Channel handle
@param[in] llTime      	time 
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetDeviceTime(
	HCHANNEL						hChannel,
	LONGLONG						llTime
	);

/**
@brief Calibration of the device time with the channel handle
@param[in] hChannel      	Channel handle
@param[in] llTime      		time 
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWRegulateDeviceTime(
	HCHANNEL						hChannel,
	LONGLONG						llTime
	);

// Timer Event
/**
@brief Registers timer according to the channel handle
@param[in] hChannel      	Channel handle
@param[in] hEvent      	  	event
@return Returns timer handle if succeeded, otherwise returns 0
*/
HTIMER
LIBMWCAPTURE_API
MWRegisterTimer(
    HCHANNEL						hChannel,
    MWHANDLE						hEvent
    );

/**
@brief Unregisters timer according to the channel handle
@param[in] hChannel      	Channel handle
@param[in] hTimer      		timer
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUnregisterTimer(
	HCHANNEL 						hChannel,
	HTIMER							hTimer
	);

/**
@brief Schedules timer object according to channel handle
@param[in] hChannel      	Channel handle
@param[in] hTimer      		handle of timer object
@param[in] llExpireTime    	expire time
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWScheduleTimer(
	HCHANNEL 						hChannel,
	HTIMER							hTimer,
	LONGLONG						llExpireTime
	);

// Notify Event
/**
@brief Registers notification object according to channel handle
@param[in] hChannel      	channel handle
@param[in] hEvent      	    handle of the event object
@param[in] dwEnableBits     event bits mask
@return Returns handle of notification object if succeeded, otherwise returns NULL
*/
HNOTIFY
LIBMWCAPTURE_API
MWRegisterNotify(
    HCHANNEL 						hChannel,
    MWHANDLE						hEvent,
    DWORD							dwEnableBits
    );

/**
@brief Unregisters notification object
@param[in] hChannel      		channel handle
@param[in] hNotify        		event notice
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
	LIBMWCAPTURE_API
	MWUnregisterNotify(
	HCHANNEL 						hChannel,
	HNOTIFY							hNotify
	);

/**
@brief Gets notify status according to channel handle
@param[in] hChannel      	channel handle
@param[in] hNotify      	handle of notify object
@param[out] pullStatus     	notify status 
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetNotifyStatus(
	HCHANNEL 						hChannel,
	HNOTIFY							hNotify,
	ULONGLONG*						pullStatus
	);

// Video Capture
/**
@brief Starts video capture according to channel handle
@param[in] hChannel      	channel handle
@param[in] hEvent      		handle of the event
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWStartVideoCapture(
    HCHANNEL 						hChannel,
    MWHANDLE						hEvent
    );

/**
@brief Stops video capture according to channel handle
@param[in] hChannel      	Channel handle
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWStopVideoCapture(
	HCHANNEL 						hChannel
	);

/**
@brief Pins a video buffer according to channel handle
@param[in] hChannel      	Channel handle
@param[in] pbFrame      	point of virtual memory
@param[in] cbFrame     		size of virtual memory
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWPinVideoBuffer(
    HCHANNEL 						hChannel,
    MWCAP_PTR                       pbFrame,
    DWORD							cbFrame
    );

/**
@brief Unlocks part of video buffer according to channel handle
@param[in] hChannel      	channel handle
@param[in] pbFrame      	pointer of virtual memory
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUnpinVideoBuffer(
	HCHANNEL 						hChannel,
	LPBYTE							pbFrame
	);

/**
@brief captures a video frame, fill it into virtual memory
@param[in] hChannel      	Channel handle
@param[in] iFrame      	    captured frame id
@param[out] pbFrame     	pointer to the memory
@param[in] cbFrame      	capture frame size 
@param[in] cbStride      	stride of the video frame
@param[in] bBottomUp     	bottom up or not
@param[in] pvContext      	pointer of user data
@param[in] dwFOURCC      	capture color format
@param[in] cx     			width
@param[in] cy     			height
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureVideoFrameToVirtualAddress(
	HCHANNEL 						hChannel,
	int								iFrame,
	MWCAP_PTR						pbFrame,
	DWORD							cbFrame,
	DWORD							cbStride,
	BOOLEAN							bBottomUp,
	MWCAP_PTR64						pvContext,
	DWORD							dwFOURCC,
	int								cx,
	int								cy
	);

/**
@brief captures a video frame, fills it into physical memory
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
@note Parameters refer to MWCaptureVideoFrameToVirtualAddress()
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureVideoFrameToPhysicalAddress(
	HCHANNEL 						hChannel,
	int								iFrame,
	LARGE_INTEGER					llFrameAddress,
	DWORD							cbFrame,
	DWORD							cbStride,
	BOOLEAN							bBottomUp,
	MWCAP_PTR64						pvContext,
	DWORD							dwFOURCC,
	int								cx,
	int								cy
	);

/**
@brief Gets a video frame with OSD, fill it into virtual memory
@param[in] pOSDImage      handle of OSD image
@param[in] pOSDRects      pointer of rectangular border of the OSD image 
@param[in] cOSDRects      total counts of OSD images
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
@note Refers to MWCaptureVideoFrameToVirtualAddress()
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureVideoFrameWithOSDToVirtualAddress(
	HCHANNEL 						hChannel,
	int								iFrame,
	MWCAP_PTR						pbFrame,
	DWORD							cbFrame,
	DWORD							cbStride,
	BOOLEAN							bBottomUp,
	MWCAP_PTR64						pvContext,
	DWORD							dwFOURCC,
	int								cx,
	int								cy,
	HOSD							hOSDImage,
	const RECT *					pOSDRects,
	int								cOSDRects
	);

/**
@brief  Gets a video frame with OSD, fills it into physical memory
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
@note Refers to MWCaptureVideoFrameWithOSDToVirtualAddress()
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureVideoFrameWithOSDToPhysicalAddress(
	HCHANNEL 						hChannel,
	int								iFrame,
	LARGE_INTEGER					llFrameAddress,
	DWORD							cbFrame,
	DWORD							cbStride,
	BOOLEAN							bBottomUp,
	MWCAP_PTR64						pvContext,
	DWORD							dwFOURCC,
	int								cx,
	int								cy,
	HOSD							hOSDImage,
	const RECT *					pOSDRects,
	int								cOSDRects
	);

/**
@brief Gets a video frame with customized settings, fill it into virtual memory
@param[in] dwProcessSwitchs      		processing mask
@param[in] cyPartialNotify      		the number of lines of each capture
@param[in] pOSDImage     				handle of OSD image
@param[in] pOSDRects      				pointer of rectangular border of the OSD image 
@param[in] cOSDRects      				total counts of OSD images 
@param[in] sContrast     				contrast
@param[in] sBrightness      			brightness
@param[in] sSaturation      			saturation
@param[in] sHue     					hue
@param[in] deinterlaceMode      		deinterlace mode
@param[in] aspectRatioConvertMode     	convert mode of aspect ration
@param[in] pRectSrc     				source rect of image
@param[in] pRectDest      				target rect of image
@param[in] nAspectX      				width of aspect
@param[in] nAspectY     				height of aspect
@param[in] colorFormat      			color format
@param[in] quantRange      				quantization
@param[in] satRange     				saturation
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
@note Refers to MWCaptureVideoFrameToVirtualAddress()
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureVideoFrameToVirtualAddressEx(
	HCHANNEL 						hChannel,
	int								iFrame,
	LPBYTE							pbFrame,
	DWORD							cbFrame,
	DWORD							cbStride,
	BOOLEAN							bBottomUp,
	MWCAP_PTR64						pvContext,
	DWORD							dwFOURCC,
	int								cx,
	int								cy,
	DWORD							dwProcessSwitchs,
	int								cyParitalNotify,
	HOSD							hOSDImage,
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

/**
@brief Gets a video frame with customize settings, fills it into physical memory
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
@note Refers to MWCaptureVideoFrameToVirtualAddressEx()
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureVideoFrameToPhysicalAddressEx(
	HCHANNEL 						hChannel,
	int								iFrame,
	LARGE_INTEGER					llFrameAddress,
	DWORD							cbFrame,
	DWORD							cbStride,
	BOOLEAN							bBottomUp,
	MWCAP_PTR64						pvContext,
	DWORD							dwFOURCC,
	int								cx,
	int								cy,
	DWORD							dwProcessSwitchs,
	int								cyParitalNotify,
	HOSD							hOSDImage,
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

/**
@brief Gets video buffered in the pro capture card according to channel handle
@param[in] hChannel      			channel handle
@param[out] pVideoBufferInfo      	pointer of video buffer info
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoBufferInfo(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_BUFFER_INFO *		pVideoBufferInfo
	);

/**
@brief Gets video frame info according to channel handle
@param[in] hChannel      		Channel handle
@param[in] i      		 		frame index
@param[out] pVideoFrameInfo     video frame info
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoFrameInfo(
	HCHANNEL 						hChannel,
	BYTE							i,
	MWCAP_VIDEO_FRAME_INFO*			pVideoFrameInfo
	);

/**
@brief Gets video capture status according to channel handle
@param[in] hChannel      channel handle
@param[out] pStatus      video capture status
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoCaptureStatus(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_CAPTURE_STATUS *	pStatus
	);

// Audio Capture
/**
@brief captures audio according to channel handle
@param[in] hChannel      Channel handle
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWStartAudioCapture(
	HCHANNEL 						hChannel
	);	

/**
@brief Stops to capture audio according to channel handle
@param[in] hChannel      Channel handle
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWStopAudioCapture(
	HCHANNEL 						hChannel
	);
	
/**
@brief Captures audio frame according to channel handle
@param[in] hChannel      			Channel handle
@param[out] pAudioCaptureFrame      Captured audio frame
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureAudioFrame(
	HCHANNEL 						hChannel,
	MWCAP_AUDIO_CAPTURE_FRAME*	pAudioCaptureFrame
	);

/**
@brief Sets reconfig delay according to channel handle
@param[in] hChannel      	channel handle
@param[in] dwDelayMS      	time of delay
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetPostReconfig(
	HCHANNEL 						hChannel,
	DWORD							dwDelayMS
	);

// OSD
/**
@brief Creates OSD image according to channel handle
@param[in] hChannel      channel handle
@param[in] cx      		 width
@param[in] cy     		 height	
@return Returns handle of the OSD image if succeeded, otherwise returns NULL
*/
HOSD
LIBMWCAPTURE_API
MWCreateImage(
	HCHANNEL 						hChannel,
	int								cx,
	int								cy
	);

/**
@brief Opens image according to channel handle
@param[in] hChannel      channel handle
@param[in] hImage      	 handle of OSD image
@param[out] plRet        reference count of OSD image 
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWOpenImage(
	HCHANNEL 						hChannel,
	HOSD							hImage,
	LONG*							plRet
	);

/**
@brief Closes image according to channel handle
@param[in] hChannel      Channel handle
@param[in] hImage        handle of OSD image
@param[out] plRet     	 reference count of OSD image
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWCloseImage(
	HCHANNEL 						hChannel,
	HOSD							hImage,
	LONG*							plRet
	);

/**
@brief Uploads a OSD image from the virtual memory
@param[in] hChannel      			Channel handle
@param[in] hImage      	 			handle of OSD image
@param[in] cfDest     	 			color format of the target image
@param[in] quantRangeDest      		quantization of the target image
@param[in] satRangeDest      		saturation of the target image
@param[in] xDest     				x
@param[in] yDest      				y
@param[in] cxDest      				target width
@param[in] cyDest     				target height
@param[in] pvSrcFrame      			pointer of source image
@param[in] cbSrcFrame      			size of source image
@param[in] cbSrcStride     			stride of source image
@param[in] cxSrc      				source width
@param[in] cySrc      				source height
@param[in] bSrcBottomUp     		bottom up or not
@param[in] bSrcPixelAlpha      		with alpha or not
@param[in] bSrcPixelXBGR      		XBGR or not
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
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
	MWCAP_PTR64						pvSrcFrame,
	DWORD							cbSrcFrame,
	DWORD							cbSrcStride,
	WORD							cxSrc,
	WORD							cySrc,
	BOOLEAN							bSrcBottomUp,
	BOOLEAN							bSrcPixelAlpha,
	BOOLEAN							bSrcPixelXBGR
	);

/**
@brief Uploads an OSD image physical memory
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
@note Refers to MWUploadImageFromVirtualAddress()
*/
MW_RESULT
LIBMWCAPTURE_API
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
	LARGE_INTEGER					llSrcFrameAddress,
	DWORD							cbSrcFrame,
	DWORD							cbSrcStride,
	WORD							cxSrc,
	WORD							cySrc,
	BOOLEAN							bSrcBottomUp,
	BOOLEAN							bSrcPixelAlpha,
	BOOLEAN							bSrcPixelXBGR
	);

/**
@brief Gets temperature according to channel handle
@param[in] hChannel      		channel handle
@param[out] pnTemp     	  		pointer of temperature
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetTemperature(
    HCHANNEL                        hChannel,
    unsigned int*                   pnTemp
    );
	
//linux
// V4l2
/**
@brief Gets the number of streams according to channel handle
@param[in] hChannel      		channel handle
@param[in] pnCount      	 	the number of streams
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetStreamCount(
    HCHANNEL                        hChannel,
    int *                           pnCount
    );

/**
@brief  Gets the information of streams according to channel handle
@param[in] hChannel      		channel handle
@param[in] pStreamInfos      	 	information of streams
@param[in] pnCount      	 	the number of streams
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetStreamInfos(
    HCHANNEL                        hChannel,
    MWCAP_STREAM_INFO *             pStreamInfos,
    int *                           pnCount
    );

/**
@brief Sets the ID of controlled stream according to channel handle
@param[in] hChannel      		channel handle
@param[in] nCrtlID      	 	ID of control stream
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetCtrlStreamID(
    HCHANNEL                        hChannel,
    int                             nCrtlID
    );

/**
@brief  Gets the video capture format according to channel handle
@param[in] hChannel      		channel handle
@param[out] pConnectFormat      	 	video capture format
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoConnectFormat(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CONNECTION_FORMAT * pConnectFormat
    );

/**
@brief   Gets the video capture settings according to channel handle
@param[in] hChannel      		channel handle
@param[out] pProcessSettings      	video capture settings
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoProcessSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_PROCESS_SETTINGS *  pProcessSettings
    );

/**
@brief  Sets video capture configuration according to channel handle
@param[in] hChannel      		channel handle
@param[in] processSettings      	video capture settings
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoProcessSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_PROCESS_SETTINGS    processSettings
    );

/**
@brief Gets OSD settings according to channel handle
@param[in] hChannel      		channel handle
@param[out] pOSDSettings      	 	OSD settings of video capture
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoOSDSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_SETTINGS *      pOSDSettings
    );

/**
@brief Sets OSD settings according to channel handle
@param[in] hChannel      		channel handle
@param[in] OSDSettings      	 OSD settings of video capture
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoOSDSettings(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_SETTINGS        OSDSettings
    );

/**
@brief  Gets OSD location according to channel handle
@param[in] hChannel      		channel handle
@param[out] pOSDImage      	 OSD image
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoOSDImage(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_IMAGE *         pOSDImage
    );

/**
@brief  Sets OSD location according to channel handle
@param[in] hChannel      		channel handle
@param[in] OSGImage      	 	OSD image
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoOSDImage(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_OSD_IMAGE           OSDImage
    );

/**
@brief  Gets the brightness of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[out] pnBrightness      	 	brightness of video capture
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoBrightness(
    HCHANNEL hChannel,
    int *pnBrightness
    );

/**
@brief  Sets the brightness of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[in] nBrightness      	 	brightness of video captured
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoBrightness(
    HCHANNEL                        hChannel,
    int                             nBrightness
    );

/**
@brief  Gets the contrast of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[out] pnContrast      	 	contrast of video captured 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoContrast(
    HCHANNEL                        hChannel,
    int *                           pnContrast
    );

/**
@brief Sets the contrast of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[in] nContrast      	 	contrast of video captured
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoContrast(
    HCHANNEL                        hChannel,
    int                             nContrast
    );

/**
@brief Gets the hue of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[in] pnHue      	 		hue of video captured
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoHue(
    HCHANNEL                        hChannel,
    int *                           pnHue
    );

/**
@brief Sets the hue of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[in] nHue      	 		hue of video captured
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoHue(
    HCHANNEL                        hChannel,
    int                             nHue
    );

/**
@brief Gets the saturation of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[out] pnSaturation      	 	saturation of video captured
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoSaturation(
    HCHANNEL                        hChannel,
    int *                           pnSaturation
    );

/**
@brief Sets the saturation of video captured according to channel handle
@param[in] hChannel      		channel handle
@param[in] nSaturation      	 	saturation of video captured
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoSaturation(
    HCHANNEL                        hChannel,
    int                             nSaturation
    );

/**
@brief  Saves the presetting configurations of video capture according to channel handle.
@param[in] hChannel      		channel handle
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSaveSettingsAsPreset(
    HCHANNEL                        hChannel
    );

/**
@brief Reloads the presetting configurations of video capture according to channel handle.
@param[in] hChannel      		channel handle
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWReloadPreset(
    HCHANNEL                        hChannel
    );

// VGA/Component timings
/**
@brief Gets whether the horizontal direction of the video is automatically adjusted according to channel handle.
@param[in] 	hChannel      	channel handle
@param[out] 	pbAuto      	whether the horizontal direction of the video is automatically adjusted 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoAutoHAlign(
    HCHANNEL                        hChannel,
    BOOLEAN *                       pbAuto
    );

/**
@brief Sets whether the horizontal direction of the video is automatically adjusted according to channel handle.
@param[in] hChannel      	channel handle
@param[in] bAuto      		whether the horizontal direction of the video is automatically adjusted 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoAutoHAlign(
    HCHANNEL                        hChannel,
    BOOLEAN                         bAuto
    );

/**
@brief Gets the sampling phase according to channel handle
@param[in] hChannel      	channel handle
@param[out] pbyValue      	sampling phase
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoSamplingPhase(
    HCHANNEL                        hChannel,
    BYTE *                          pbyValue
    );

/**
@brief Sets the sampling phase according to channel handle
@param[in] hChannel      	channel handle
@param[in] byValue      	sampling phase
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoSamplingPhase(
    HCHANNEL                        hChannel,
    BYTE                            byValue
    );

/**
@brief Gets whether the video sampling phase is automatically adjusted according to channel handle
@param[in] hChannel      	channel handle
@param[out] pbAuto      	whether the video sampling phase is automatically adjusted
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoSamplingPhaseAutoAdjust(
    HCHANNEL                        hChannel,
    BOOLEAN *                       pbAuto
    );

/**
@brief Sets whether the video sampling phase is automatically adjusted according to channel handle
@param[in] hChannel      	channel handle
@param[in] bAuto      		whether the video sampling phase is automatically adjusted
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoSamplingPhaseAutoAdjust(
    HCHANNEL                        hChannel,
    BOOLEAN                         bAuto
    );

/**
@brief Sets the video timing according to channel handle
@param[in] hChannel      	channel handle
@param[in] videoTiming      	video timing
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoTiming(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_TIMING              videoTiming
    );

/**
@brief Gets the preset video timing according to channel handle
@param[in] hChannel      	channel handle
@param[out] pVideoTiming      	preset video timing
@param[out] plCount     		时序个数
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoPreferredTimingArray(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_TIMING *            pVideoTiming,
    long *                          plCount
    );

/**
@brief Sets the customized video timing according to channel handle
@param[in] hChannel      	channel handle
@param[in] videoTiming      	customized video timing
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetCustomVideoTiming(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CUSTOM_TIMING       videoTiming
    );

/**
@brief Gets the number of customized video timing according to channel handle
@param[in] hChannel      	channel handle
@param[out] pdwCount      	the number of customized video timing
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetCustomVideoTimingsCount(
    HCHANNEL                        hChannel,
    DWORD   *                       pdwCount
    );

/**
@brief  Gets customized video timing according to channel handle
@param[in] hChannel      		channel handle
@param[out] pVideoCustomTiming  	customized video timing
@param[out] pdwCount     		the number of customized video timing
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetCustomVideoTimingsArray(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CUSTOM_TIMING *     pVideoCustomTiming,
    DWORD *                         pdwCount
    );

/**
@brief Sets customized video timing according to channel handle
@param[in] hChannel      		channel handle
@param[in] pVideoCustomTiming   	customized video timing
@param[in] dwCount     		the number of customized video timing
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetCustomVideoTimingsArray(
    HCHANNEL                        hChannel,
    MWCAP_VIDEO_CUSTOM_TIMING *     pVideoCustomTiming,
    DWORD                           dwCount
    );

/**
@brief  Gets the number of customized video resolution according to channel handle
@param[in] hChannel      	channel handle
@param[out] pdwCount      	the number of customized video resolution
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetCustomVideoResolutionsCount(
    HCHANNEL                        hChannel,
    DWORD *                         pdwCount
    );

/**
@brief  Gets customized video resolution according to channel handle
@param[in] hChannel      	channel handle
@param[out] pResolutionSize    customized video resolution
@param[out] pdwCount     	the number of customized video resolution
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetCustomVideoResolutionsArray(
    HCHANNEL                        hChannel,
    MWCAP_SIZE *                    pResolutionSize,
    DWORD *                         pdwCount
    );

/**
@brief  Sets customized video resolution according to channel handle
@param[in] hChannel      	channel handle
@param[in] pResolutionSize      customized video resolution
@param[in] dwCount     		the number of customized video resolution
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetCustomVideoResolutionsArray(
    HCHANNEL                        hChannel,
    MWCAP_SIZE *                    pResolutionSize,
    DWORD                           dwCount
    );

// Event
/**
@brief Creates events 
@return  Returns event handle if succeeded, otherwise return 0 or MW_INVALID_PARAMS
*/
MWCAP_PTR
LIBMWCAPTURE_API
MWCreateEvent(
    );

/**
@brief Destroys the event 
@param[in] hEvent      	event handle 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWCloseEvent(
    MWCAP_PTR                       hEvent
    );

/**
@brief trigger the event
@param[in] hEvent      		 event 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetEvent(
    MWCAP_PTR                       hEvent
    );

/**
@brief  Resets the event 
@param[in] hEvent      		 event 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWClearEvent(
    MWCAP_PTR                       hEvent
    );

/**
@brief Querys whether the event is triggered
@param[in] hEvent      		event 
@return  Returns TRUE if succeeded, otherwise returns FALSE
*/
BOOLEAN
LIBMWCAPTURE_API
MWIsSetEvent(
    MWCAP_PTR                       hEvent
    );

/**
@brief Waits for the event
@param[in] hEvent      	event 
@return Returns TRUE if succeeded, otherwise returns FALSE
*/
int
LIBMWCAPTURE_API
MWTryWaitEvent(
    MWCAP_PTR                       hEvent
    );

/**
@brief  Waits for the event
@param[in] hEvent      		event 
@param[in] nTimeout     	timeout period
@return Returns TRUE if succeeded, otherwise returns FALSE
*/
int
LIBMWCAPTURE_API
MWWaitEvent(
    MWCAP_PTR                       hEvent,
    int                             nTimeout
    );

/**
@brief Waits for the events
@param[in] hEvents      	events
@param[in] nCount      		the number of event objects
@param[in] nTimeout     		timeout period
@return Returns event flag if succeeded, otherwise return FALSE
*/
DWORD
LIBMWCAPTURE_API
MWMultiWaitEvent(
    MWCAP_PTR *                 hEvents,
    int                         nCount,
    int                         nTimeout
    );

#ifdef __cplusplus
}
#endif
