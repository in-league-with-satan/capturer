/************************************************************************************************/
// MWCapture.h : header file

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

#include <stdint.h>
#include "MWLinux.h"
#include "MWCaptureExtension.h"

#ifdef __cplusplus

extern "C"
{
#endif

/**
@brief Start video Eco capture according to channel handle
@param[in] hChannel      	channel handle
@param[in] pEcoCaptureOpen  open parameter
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWStartVideoEcoCapture(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_ECO_CAPTURE_OPEN	*pEcoCaptureOpen
	);

/**
@brief Set AMP value of video Eco capture
@param[in] hChannel      	channel handle
@param[in] pSettings        Eco capture settings
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoEcoCaptureSettings(
	HCHANNEL 							hChannel,
	MWCAP_VIDEO_ECO_CAPTURE_SETTINGS	*pSettings
	);

/**
@brief Set video Eco capture frame
@param[in] hChannel      	Channel handle
@param[in] pFrame           capture frame
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWCaptureSetVideoEcoFrame(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_ECO_CAPTURE_FRAME   *pFrame
	);

/**
@brief Get video Eco capture status according to channel handle
@param[in] hChannel      channel handle
@param[out] pStatus      video capture status
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoEcoCaptureStatus(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_ECO_CAPTURE_STATUS  *pStatus
	);

/**
@brief Stop video Eco capture according to channel handle
@param[in] hChannel      	Channel handle
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWStopVideoEcoCapture(
	HCHANNEL 						hChannel
	);

#ifdef __cplusplus
}
#endif
