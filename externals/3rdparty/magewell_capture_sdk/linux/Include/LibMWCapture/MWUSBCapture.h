/************************************************************************************************/
// MWUSBCapture.h : header file

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
#ifndef _MWUSB_CAPTURE_H_
#define _MWUSB_CAPTURE_H_

#ifdef LIBMWCAPTURE_EXPORTS
#define LIBMWCAPTURE_API __declspec(dllexport)
#elif LIBMWCAPTURE_DLL
#define LIBMWCAPTURE_API __declspec(dllimport)
#else
#define LIBMWCAPTURE_API 
#endif

#ifdef _WIN32
#include <Windows.h>
#endif
#include <stdint.h>
#include "MWUSBCaptureExtension.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
@brief Registers notification event of device hot plug
@param[in] lpfnCallback	callback function of device hot plug notification
@return Returns MW_SUCCEED 
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBRegisterHotPlug(
    LPFN_HOT_PLUG_CALLBACK lpfnCallback,
	void *				   pParam
    );

/**
@brief UnRegister notification event of device hot plug
@return return MW_SUCCEED
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBUnRegisterHotPlug(
    );

//Notification
/**
@brief Registers notification
@param[in] hChannel			Input the opened channel handle
@param[in] pNotify			Notification type 
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetNotifyEnable(
	HUSBCHANNEL						hChannel,
	MWCAP_NOTIFY_ENABLE *			pNotify
	);

/**
@brief Get the notify status
@param[in] hChannel					Input the opened channel handle
@param[out] pullStatusBit			The flag bits of notification
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetNotifyStatus(
	HUSBCHANNEL						hChannel,
	uint64_t *						pullStatusBit
	);

//Upgrade
/**
@brief Erases the firmware data
@param[in] hChannel					Input the opened channel handle
@param[in] cbOffset					The offset of address erased
@param[in] cbErase					The size of the data erased
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetFirmwareErase(
	HUSBCHANNEL 					hChannel,
	uint32_t						cbOffset,
	uint32_t						cbErase
	);

/**
@brief Get the firmware data address
@param[in] hChannel					Input the opened channel handle
@param[out] pdwAddress				The address of the firmware data that has been read
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetFirmwareReadAddress(
	HUSBCHANNEL 					hChannel,
	uint32_t *						pdwAddress
	);

//HDMI
/**
@brief Get the loop through EDID 
@param[in] hChannel					Input the opened channel handle
@param[out] pbyEDID					The buffer address to save edid value
@param[out] pcbEDID					The size of edid data that have been read
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetEDIDLoopThrough(
	HUSBCHANNEL						hChannel,
	char *							pbyEDID,
	uint32_t *						pcbEDID
	);

/**
@brief Get the valid flag of the loopthrough
@param[in] hChannel					Input the opened channel handle
@param[out] pbValid					The valid flag of loopthrough			
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetLoopThroughValid(
	HUSBCHANNEL						hChannel,
	bool_t *						pbValid
	);

/**
@brief Get the volume value of the USB audio device
@param[in] hChannel					Input the opened channel handle
@param[in] audioNode				The type of audio device
@param[out] pVolume					The volume value
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetAudioVolume(
	HUSBCHANNEL						hChannel,
	MWCAP_USB_AUDIO_NODE			audioNode,
	MWCAP_AUDIO_VOLUME*				pVolume
	);

/**
@brief Set the volume value to the usb audio device
@param[in] hChannel					Input the opened channel handle
@param[in] audioNode				The type of audio device
@param[in] pVolume					Set volume value
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetAudioVolume(
	HUSBCHANNEL						hChannel,
	MWCAP_USB_AUDIO_NODE			audioNode,
	MWCAP_AUDIO_VOLUME*				pVolume
	);

// Video Processing
/**
@brief Get the capture format related parameters of the video capture device
@param[in] hChannel					Input the opened channel handle
@param[out] pConnFormat				The video connection format information
@return return MW_SUCCEED if succeeded, otherwise return MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoCaptureConnectionFormat(
	HUSBCHANNEL 					hChannel,
	MWCAP_VIDEO_CONNECTION_FORMAT*	pConnFormat
	);

/**
@brief Get the default settings of the video capture device
@param[in] hChannel					Input the opened channel handle
@param[out] pProcSettings			Output the video process settings
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoCaptureProcessSettings(
	HUSBCHANNEL 					hChannel,
	MWCAP_VIDEO_PROCESS_SETTINGS*	pProcSettings
	);

/**
@brief Set the default video capture format
@param[in] hChannel					Input the opened channel handle
@param[in] pProcSettings			The video process settings information
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoCaptureProcessSettings(
	HUSBCHANNEL 					hChannel,
	MWCAP_VIDEO_PROCESS_SETTINGS*	pProcSettings
	);

// Video output format
/**
@brief Get output formate supported by the video capture device
@param[in] hChannel					Input the opened channel handle
@param[out] pOutputFourCC			The output color space format supported 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoOutputFOURCC(
	HUSBCHANNEL 					hChannel,
	MWCAP_VIDEO_OUTPUT_FOURCC*		pOutputFourCC
	);

/**
@brief Set output format of the video capture device
@param[in] hChannel					Input the opened channel handle
@param[in] pOutputFourCC			The value of output format set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoOutputFOURCC(
	HUSBCHANNEL 					hChannel,
	MWCAP_VIDEO_OUTPUT_FOURCC*		pOutputFourCC
	);

/**
@brief Get output frame size 
@param[in] hChannel					Input the opened channel handle
@param[out] pFrameSize				The output frame size supported 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoOutputFrameSize(
	HUSBCHANNEL 						hChannel,
	MWCAP_VIDEO_OUTPUT_FRAME_SIZE*		pFrameSize
	);

/**
@brief Set output frame size 
@param[in] hChannel					Input the opened channel handle
@param[in] pFrameSize				The value of output frame size set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoOutputFrameSize(
	HUSBCHANNEL 						hChannel,
	MWCAP_VIDEO_OUTPUT_FRAME_SIZE*		pFrameSize
	);

/**
@brief Get output frame rate
@param[in] hChannel					Input the opened channel handle
@param[out] pFrameInterval			The video output frame interval supported
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoOutputFrameInterval(
	HUSBCHANNEL 							hChannel,
	MWCAP_VIDEO_OUTPUT_FRAME_INTERVAL*		pFrameInterval
	);

/**
@brief Set output frame rate
@param[in] hChannel					Input the opened channel handle
@param[in] pFrameInterval			The value of output frame interval set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoOutputFrameInterval(
	HUSBCHANNEL 							hChannel,
	MWCAP_VIDEO_OUTPUT_FRAME_INTERVAL*		pFrameInterval
	);

//Image mode
/**
@brief Get the image mode status 
@param[in] hChannel					Input the opened channel handle
@param[out] pImageMode				The image mode of the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetStatusImageMode(
	HUSBCHANNEL						hChannel,
	MWCAP_STATUS_IMAGE_MODE *		pImageMode
	);

/**
@brief Set the image mode status 
@param[in] hChannel					Input the opened channel handle
@param[in] pImageMode				The value of image mode set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetStatusImageMode(
	HUSBCHANNEL						hChannel,
	MWCAP_STATUS_IMAGE_MODE *		pImageMode
	);

//Name mode
/**
@brief Get the value of device name mode
@param[in] hChannel					Input the opened channel handle
@param[out] pNameMode				Output the value of the capture device device name mode
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetDeviceNameMode(
	HUSBCHANNEL						hChannel,
	MWCAP_DEVICE_NAME_MODE *		pNameMode
	);

/**
@brief Set device name mode 
@param[in] hChannel					Input the opened channel handle
@param[in] pNameMode				The name mode set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetDeviceNameMode(
	HUSBCHANNEL						hChannel,
	MWCAP_DEVICE_NAME_MODE *		pNameMode
	);

// HID Options Control
/**
@brief Save the options to the capture device
@param[in] hChannel					Input the opened channel handle
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSaveOptions(
	HUSBCHANNEL		hChannel 
	);

/**
@brief Load the settings to the capture device
@param[in] hChannel					Input the opened channel handle
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBLoadOptions(
	HUSBCHANNEL		hChannel
	);

/**
@brief Reset the settings to the capture device
@param[in] hChannel					Input the opened channel handle
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBResetOptions(
	HUSBCHANNEL		hChannel
	);

//Timing
/**
@brief Get whether video is horizontally aligned 
@param[in] hChannel					Input the opened channel handle
@param[out] pbAutoHAlign			The flag auto align horizontally   
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoAutoHAlign(
	HUSBCHANNEL						hChannel,
	bool_t *						pbAutoHAlign
	);

/**
@brief Set video horizontal alignment
@param[in] hChannel					Input the opened channel handle
@param[out] pbAutoHAlign			The flag of the auto horizontal aligned set to the capture device 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoAutoHAlign(
	HUSBCHANNEL						hChannel,
	bool_t							pbAutoHAlign
	);

/**
@brief Get video sampling phase
@param[in] hChannel					Input the opened channel handle
@param[out] puSamplingPhase			The value of the sampling phase of the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoSamplingPhase(
	HUSBCHANNEL						hChannel,
	uint8_t *						puSamplingPhase
	);

/**
@brief Set video sampling phase
@param[in] hChannel					Input the opened channel handle
@param[in] puSamplingPhase			The value of the sampling phase set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoSamplingPhase(
	HUSBCHANNEL						hChannel,
	uint8_t* 						puSamplingPhase
	);

/**
@brief Get whether video sampling phase is auto adjusted
@param[in] hChannel					Input the opened channel handle
@param[out] pbAutoSamplingPhase		The automatic adjustment flag of the sample phase
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetVideoSamplingPhaseAutoAdjust(
	HUSBCHANNEL						hChannel,
	bool_t * 						pbAutoSamplingPhase
	);

/**
@brief Set video sampling phase auto adjust
@param[in] hChannel					Input the opened channel handle
@param[in] pbAutoSamplingPhase		The automatic adjustment flag of the sample phase set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoSamplingPhaseAutoAdjust(
	HUSBCHANNEL						hChannel,
	bool_t *  						pbAutoSamplingPhase
	);

/**
@brief Set default video timing
@param[in] hChannel					Input the opened channel handle
@param[in] pTiming					The video timing set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetVideoTiming(
	HUSBCHANNEL						hChannel,
	MWCAP_VIDEO_TIMING * 			pTiming
	);

/**
@brief Get video preferred timings
@param[in] hChannel					Input the opened channel handle
@param[out] paTimings				The preferred video timing of the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetPreferredVideoTimings(
	HUSBCHANNEL						hChannel,
	MWCAP_VIDEO_TIMING_ARRAY * 		paTimings
	);

/**
@brief Get customize video timing array
@param[in] hChannel					Input the opened channel handle
@param[out] paCustomTimings			The customize video timing of the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetCustomVideoTimings(
	HUSBCHANNEL							hChannel,
	MWCAP_VIDEO_CUSTOM_TIMING_ARRAY *	paCustomTimings
	);

/**
@brief Set customize video timing array
@param[in] hChannel					Input the opened channel handle
@param[in] paCustomTimings			The customize video timing set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetCustomVideoTimings(
	HUSBCHANNEL							hChannel,
	MWCAP_VIDEO_CUSTOM_TIMING_ARRAY *	paCustomTimings
	);

/**
@brief Set customize video timing
@param[in] hChannel					Input the opened channel handle
@param[in] pCustomTiming			The customize video timing set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetCustomVideoTiming(
	HUSBCHANNEL							hChannel,
	MWCAP_VIDEO_CUSTOM_TIMING *			pCustomTiming
	);

/**
@brief Get customize video resolutions
@param[in] hChannel					Input the opened channel handle
@param[out] paCustomResolutions		The customize resolutions of the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetCustomVideoResolutions(
	HUSBCHANNEL										hChannel,
	MWCAP_VIDEO_CUSTOM_RESOLUTION_ARRAY *			paCustomResolutions
	);

/**
@brief Set customize video resolutions
@param[in] hChannel					Input the opened channel handle
@param[in] paCustomResolutions		The customize resolutions set to the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetCustomVideoResolutions(
	HUSBCHANNEL										hChannel,
	MWCAP_VIDEO_CUSTOM_RESOLUTION_ARRAY *			paCustomResolutions
	);

/**
@brief Get extended support
@param[in] hChannel					Input the opened channel handle
@param[out] pdwFlag					The extended supported flag of the capture device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetExtensionSupported(
	HUSBCHANNEL				hChannel,
	uint32_t *				pdwFlag
	);

/**
@brief Obtains the scan state of input source
@param[in] hChannel					Input the opened channel handle
@param[out] pbScanning	scan state of input source
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or NW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetInputSourceScanState(HUSBCHANNEL hChannel, bool_t * pbScanning);
/**
@brief get current edid mode
@param[in] 	hChannel      	Input the opened channel handle
@param[out] 	pMode      the edid mode of device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or NW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBGetEDIDMode(HUSBCHANNEL hChannel, MWCAP_EDID_MODE * pMode);
/**
@brief set the edid mode
@param[in] 	hChannel      	Input the opened channel handle
@param[out] 	mode      	the edid mode set to device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or NW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWUSBSetEDIDMode(HUSBCHANNEL hChannel, MWCAP_EDID_MODE mode);
#ifdef __cplusplus
}
#endif


#endif //_MWUSB_CAPTURE_H_
