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
#include "MWProCapture.h"
#include "MWCaptureExtension.h"
#include "MWUSBCapture.h"
#include "MWUSBCaptureExtension.h"

#ifdef __cplusplus

extern "C"
{
#endif

/**
@brief get the version of SDK
@param[out] pbyMaj      Major version 
@param[out] pbyMin      Minor version
@param[out] pwBuild     Build version
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or NW_INVALID_PARAMS
@note Always returns MW_SUCCEED. If the parameter is invalid, the value will not be filled in.
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVersion(
	BYTE*							pbyMaj,
	BYTE*							pbyMin,
	WORD*							pwBuild
	);

/**
@brief Initializes MWCapture
*/
BOOL
LIBMWCAPTURE_API
MWCaptureInitInstance(
	);

	
/**
@brief Quits MWCapture
*/
void
LIBMWCAPTURE_API
MWCaptureExitInstance(
	);

/**
@brief Refreshes device list
*/
MW_RESULT
LIBMWCAPTURE_API
MWRefreshDevice(
	);

/**
@brief Gets the number of devices
@return  Returns the number of channels obtained 
*/
int 
LIBMWCAPTURE_API
MWGetChannelCount(
	);

/**
@brief  Gets the channel information based on the enum
@param[in] nIndex      			Channel index
@param[out] pChannelInfo        Channel information structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS

*/
MW_RESULT
LIBMWCAPTURE_API
MWGetChannelInfoByIndex(
	int								nIndex,
	MWCAP_CHANNEL_INFO *			pChannelInfo
	);

/**
@brief Gets the family information of the channel based on the enumeration value
@param[in] nIndex      			Channel index
@param[out] pFamilyInfo      	familyinfo structure
@param[in] dwSize     			size of familyinfo structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetFamilyInfoByIndex(
	int								nIndex,
	LPVOID							pFamilyInfo,
	DWORD							dwSize
	);

/**
@brief Gets the path of the device 
@param[in] nIndex      	 Channel index
@param[out] pDevicePath      path of the device
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetDevicePath(
	int								nIndex,
	char*							pDevicePath
);

/**
@brief Opens the device channel according to the path
@param[in] pszDevicePath      Opened path of the channel
@return  Returns the channel handle, otherwise returns -1
@note  The path value refers to MWGetDevicePath ()
*/
HCHANNEL
LIBMWCAPTURE_API
MWOpenChannelByPath(
	const char*					pszDevicePath
	);

/**
@brief Closes the device channel
@param[in] hChannel      Channel handles
*/
void
LIBMWCAPTURE_API
MWCloseChannel(
	HCHANNEL						hChannel
	);

/**
@brief  Gets the channel information based on the channel handle
@param[out] hChannel      		Channel handle
@param[out] pChannelInfo      	Channel information structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetChannelInfo(
	HCHANNEL						hChannel,
	MWCAP_CHANNEL_INFO *			pChannelInfo
	);

/**
@brief  Gets the channel family information based on the channel handle
@param[in] hChannel      	Channel handle
@param[out] pFamilyInfo     Familyinfo structure
@param[in] dwSize     		Familyinfo structure size
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetFamilyInfo(
	HCHANNEL						hChannel,
	LPVOID							pFamilyInfo,
	DWORD							dwSize
	);

/**
@brief Obtains channel video capability information based on the channel handle
@param[in] hChannel      		 Channel handle
@param[out] pVideoCaps      	The size of video caps
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoCaps(
	HCHANNEL						hChannel,
	MWCAP_VIDEO_CAPS*				pVideoCaps
	);
	
/**
@brief  Obtains channel audio capability information based on the channel handle
@param[in] hChannel      	    Channel handle
@param[out] pAudioCaps      	audio caps structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetAudioCaps(
	HCHANNEL						hChannel,
	MWCAP_AUDIO_CAPS*				pAudioCaps
	);

/**
@brief Obtains channel video capability information based on channel handle
@param[in] hChannel      			Channel handle
@param[out] pdwInputSource      	Stores an array of video input source interface values
@param[out] pdwInputCount     		The number of video input source interfaces 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoInputSourceArray(
	HCHANNEL						hChannel,
	DWORD*							pdwInputSource,
	DWORD*							pdwInputCount
	);

/**
@brief Obtains an audio input array based on the channel handle
@param[in] hChannel      			Channel handle
@param[out] pdwInputSource      	Stores an array of audio input source interfaces
@param[out] pdwInputCount     		The number of audio input source interfaces
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetAudioInputSourceArray(
	HCHANNEL						hChannel,
	DWORD*							pdwInputSource,
	DWORD*							pdwInputCount
	);

	
/**
@brief Gets whether the input source is automatically scanned according to the channel handle
@param[in] hChannel      	Channel handle
@param[out] pbScan      	The query value Whether to automatically scan 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetInputSourceScan(
	HCHANNEL 						hChannel,
	BOOLEAN*						pbScan
	);

/**
@brief Gets whether automatically scans the input signals according to the channel handle
@param[in] hChannel      		Channel handle
@param[in] bScan      			Gets whether the audio input source follows the video input source
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetInputSourceScan(
	HCHANNEL 						hChannel,
	BOOLEAN							bScan
	);

/**
@brief  Sets whether the audio input source Linked with video according to the channel handle
@param[in] hChannel      		Channel handle
@param[out] pbLink      	    Gets whether the audio input source follows the video input source
@return  Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetAVInputSourceLink(
	HCHANNEL 						hChannel,
	BOOLEAN*						pbLink
	);

/**
@brief  Sets whether the audio input source follows the video input source according to the channel handle
@param[in] hChannel      	Channel handle
@param[in] bLink      		Sets whether the audio input source follows the video input source
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetAVInputSourceLink(
	HCHANNEL 						hChannel,
	BOOLEAN							bLink
	);

/**
@brief Gets the video input source according to the channel handle
@param[in] hChannel      		Channel handle
@param[out] pdwSource      		Gets the array of video input sources
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoInputSource(
	HCHANNEL						hChannel,
	DWORD*							pdwSource
	);

/**
@brief Sets the video input source according to the channel handle
@param[in] hChannel     Channel handle
@param[in] dwSource     Sets the video input source
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoInputSource(
	HCHANNEL						hChannel,
	DWORD							dwSource
	);

/**
@brief Gets the audio input source from the channel handle
@param[in] hChannel      	Channel handle
@param[out] pdwSource      	Stores the obtained audio input source
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetAudioInputSource(
	HCHANNEL						hChannel,
	DWORD*							pdwSource
	);

/**
@brief Sets the audio input source according to the channel handle
@param[in] hChannel      Channel handle
@param[in] dwSource      Sets the audio input source
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetAudioInputSource(
	HCHANNEL						hChannel,
	DWORD							dwSource
	);

// EDID
/**
@brief Gets EDID according to the channel handle
@param[in] hChannel     Channel handle
@param[out] pbyData      Stores the obtained EDID value
@param[out] pulSize      The length of EDID 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetEDID(
	HCHANNEL						hChannel,
	BYTE*							pbyData,
	ULONG*							pulSize
	);

/**
@brief Sets edid according to the channel handle
@param[in] hChannel     	Channel handle
@param[in] pbyData          Sets EDID
@param[in] ulSize     		The length of pbyData
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetEDID(
	HCHANNEL						hChannel,
	BYTE*							pbyData,
	ULONG							ulSize
	);

// Signal Status
/**
@brief Gets the input specific status value based on the channel handle
@param[in] hChannel      		Channel handle
@param[out] pInputStatus      	MWCAP_INPUT_SPECIFIC_STATUS structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetInputSpecificStatus(
	HCHANNEL						hChannel,
	MWCAP_INPUT_SPECIFIC_STATUS *	pInputStatus
	);

/**
@brief Gets the video signal status according to the channel handle
@param[in] hChannel      		Channel handle
@param[out] pSignalStatus      	Video signal state structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoSignalStatus(
	HCHANNEL						hChannel,
	MWCAP_VIDEO_SIGNAL_STATUS *		pSignalStatus
	);

/**
@brief Gets the audio signal status according to the channel handle
@param[in] hChannel      		Channel handle
@param[out] pSignalStatus      	Audio signal state structure
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetAudioSignalStatus(
	HCHANNEL						hChannel,
	MWCAP_AUDIO_SIGNAL_STATUS *		pSignalStatus
	);

// HDMI InfoFrame
/**
@brief Gets the HDMI infoframe valid flag according to the channel handle
@param[in] hChannel      	Channel handle
@param[out] pdwValidFlag      	Valid flag
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetHDMIInfoFrameValidFlag(
	HCHANNEL						hChannel,
	DWORD*							pdwValidFlag
	);

/**
@brief Gets the HDMI infoframe data according to the channel handle
@param[in] hChannel      Channel handle
@param[in] id      			Frame id 
@param[out] pPacket     	Frame data
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetHDMIInfoFramePacket(
	HCHANNEL						hChannel,
	MWCAP_HDMI_INFOFRAME_ID			id,
	HDMI_INFOFRAME_PACKET*			pPacket
	);

// Video processing
/**
@brief  Sets the video input aspect ratio according to the channel handle
@param[in] hChannel     Channel handle
@param[in] nAspectX      Width of aspect ratio
@param[in] nAspectY      height of aspect ratio
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoInputAspectRatio(
	HCHANNEL 						hChannel,
	int								nAspectX,
	int								nAspectY
	);

/**
@brief Gets the video input aspect ratio according to the channel handle
@param[in] hChannel     Channel handle
@param[out] pnAspectX    Width of aspect ratio
@param[out] pnAspectY    height of aspect ratio
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoInputAspectRatio(
	HCHANNEL 						hChannel,
	int*							pnAspectX,
	int*							pnAspectY
	);

/**
@brief Sets the video input color format according to the channel handle
@param[in] hChannel      	Channel handle
@param[in] colorFormat     	Color format
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoInputColorFormat(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_COLOR_FORMAT		colorFormat
	);

/**
@brief Gets the video input color format according to the channel handle
@param[in] hChannel      		Channel handle
@param[out] pColorFormat      	Color format
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoInputColorFormat(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_COLOR_FORMAT *		pColorFormat
	);

/**
@brief Sets the video input quantization range according to the channel handle
@param[in] hChannel     Channel handle
@param[in] quantRange    quantization range 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetVideoInputQuantizationRange(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_QUANTIZATION_RANGE	quantRange
	);

/**
@brief Gets the video input quantization range according to the channel handle
@param[in] hChannel      		Channel handle
@param[out] pQuantRange      	the address quantization range stored  
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetVideoInputQuantizationRange(
	HCHANNEL 						hChannel,
	MWCAP_VIDEO_QUANTIZATION_RANGE* pQuantRange
	);

// LED Mode
/**
@brief Sets the led mode according to the channel handle
@param[in] hChannel      	Channel handle
@param[in] dwMode      		mode value set
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWSetLEDMode(
	HCHANNEL 						hChannel,
	DWORD							dwMode
	);

// Upgrade Firmware
/**
@brief Gets firmware storage information according to channel handle
@param[in] hChannel      			Channel handle
@param[out] pFirmwareStorageInfo    the structure contains information about the device firmware
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWGetFirmwareStorageInfo(
	HCHANNEL 						hChannel,
	MWCAP_FIRMWARE_STORAGE *		pFirmwareStorageInfo
	);

/**
@brief  Erases the firmware data according to the channel handle
@param[in] hChannel     Channel handle
@param[in] cbOffset     Offset
@param[in] cbErase     	the data size needed to erase 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWEraseFirmwareData(
	HCHANNEL 						hChannel,
	DWORD							cbOffset,
	DWORD							cbErase
	);

/**
@brief  Obtains the firmware data according to the channel handle
@param[in] hChannel     Channel handle
@param[in] cbOffset     Offset
@param[out] pbyData     the firmware data being read 
@param[in] cbToRead     The length of the data to read
@param[out] pcbRead     The length of the data being read 
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWReadFirmwareData(
	HCHANNEL 						hChannel,
	DWORD							cbOffset,
	BYTE *							pbyData,
	DWORD							cbToRead,
	DWORD *							pcbRead
	);

/**
@brief Writes the firmware data according to the channel handle
@param[in] hChannel     Channel handle
@param[in] cbOffset     Offset
@param[in] pbyData      The firmware data to write
@param[in] cbData       The firmware data being written
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
LIBMWCAPTURE_API
MWWriteFirmwareData(
	HCHANNEL 						hChannel,
	DWORD							cbOffset,
	BYTE *							pbyData,
	DWORD							cbData
	);


/**
@brief  Uses the DShow function to create video capture
@param[in] hChannel			Channel handle opened
@param[in] nWidth			Width of video capture
@param[in] nHeight			Height of video capture
@param[in] nFourcc			Color format 
@param[in] nFrameDuration   Frame rate of video capture
@param[in] callback			Callback video capture
@param[in] pParam			The parameters passed to the callback function
@return  Returns a HANDLE type handle if succeeded, otherwise returns NULL
*/
HANDLE
LIBMWCAPTURE_API
MWCreateVideoCapture(
	HCHANNEL 						hChannel,
	int								nWidth,
	int								nHeight,
	int								nFourcc,
	int								nFrameDuration,
	VIDEO_CAPTURE_CALLBACK			callback,
	void*							pParam
	);

/**
@brief End of video capture
@param[in] hVideo			Video capture handle created
@return  Always return MW_SUCCEED
*/
MW_RESULT
LIBMWCAPTURE_API
MWDestoryVideoCapture(
	HANDLE							hVideo
	);

/**
@brief  create audio capture using the DShow function  
@param[in] hChannel			Channel handle opened
@param[in] callback			Callback function of audio capture 
@param[in] pParam			The parameters passed to the callback function
@return Returns a HANDLE type handle if succeeded, otherwise returns NULL
*/
HANDLE
LIBMWCAPTURE_API
MWCreateAudioCapture(
	HCHANNEL						hChannel,
	MWCAP_AUDIO_CAPTURE_NODE        captureNode,
	DWORD							dwSamplesPerSec,
	WORD							wBitsPerSample,
	WORD							wChannels,
	AUDIO_CAPTURE_CALLBACK			callback,
	void*							pParam
	);

/**
@brief End of audio capture
@param[in] hAudio		an audio capture handle created 
@return  Always return MW_SUCCEED
*/
MW_RESULT
LIBMWCAPTURE_API
MWDestoryAudioCapture(
	HANDLE							hAudio
	);

/**
@brief Obtains audio attribute value of the audio device 
@param[in] hChannel					Channel handle opened
@param[in] audioNode				Audio device type
@param[out] pVolume					Audio attribute value
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
	LIBMWCAPTURE_API
	MWGetAudioVolume(
	HCHANNEL						hChannel,
	MWCAP_AUDIO_NODE				audioNode,
	MWCAP_AUDIO_VOLUME*				pVolume
	);

/**
@brief Sets the audio attribute value of audio device 
@param[in] hChannel					Channel handle opened
@param[in] audioNode				type of audio device
@param[in] pVolume					Audio attribute value
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
	LIBMWCAPTURE_API
	MWSetAudioVolume(
	HCHANNEL						hChannel,
	MWCAP_AUDIO_NODE				audioNode,
	MWCAP_AUDIO_VOLUME*				pVolume
	);


/**
@brief Set the type of ANC packet to capture
@param[in] hChannel					Channel handle opened
@param[in] byIndex					ANC index, the range is 0-3, The capture card can capture up to 4 different types of ANC packages
@param[in] bHANC					Capture in HANC  space or not
@param[in] bVANC					Capture in VANC  space or not
@param[in] byDID/bySDID				ANC ID, refer to SMPTE standard for specific content
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
	LIBMWCAPTURE_API
	MWCaptureSetSDIANCType(
	HCHANNEL										hChannel,
	BYTE											byIndex,
	BOOLEAN											bHANC,
	BOOLEAN											bVANC,
	BYTE											byDID,
	BYTE											bySDID
	);

/**
@brief Get ANC Packet
@param[in] hChannel					Channel handle opened
@param[in] pPacket					ANC packet
           MWCAP_SDI_ANC_PACKET.byDID: ANC DID, byDID == 0 indicates that no valid ANC packet has been read
           MWCAP_SDI_ANC_PACKET.bySDID: ANC SDID
           MWCAP_SDI_ANC_PACKET.byDC: ANC data length
           MWCAP_SDI_ANC_PACKET.abyUDW[255]: ANC data, refer to SMPTE standard for specific content
@return Returns MW_SUCCEED if succeeded, otherwise returns MW_FAILED or MW_INVALID_PARAMS
*/
MW_RESULT
	LIBMWCAPTURE_API
	MWCaptureGetSDIANCPacket(
	HCHANNEL										hChannel,
	MWCAP_SDI_ANC_PACKET*							pPacket
	);

#ifdef __cplusplus
}
#endif
