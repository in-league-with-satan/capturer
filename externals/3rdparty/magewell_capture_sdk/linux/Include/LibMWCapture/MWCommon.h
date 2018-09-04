////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#pragma pack(push)
#pragma pack(1)

// Firmware header
#define MW_SERIAL_NO_LEN						16
#define MW_FAMILY_NAME_LEN						64
#define MW_PRODUCT_NAME_LEN						64

typedef enum _MW_FAMILY_ID {
	MW_FAMILY_ID_PRO_CAPTURE					= 0x00,
	MW_FAMILY_ID_VALUE_CAPTURE					= 0x01,
	MW_FAMILY_ID_USB_CAPTURE					= 0x02
} MW_FAMILY_ID;

typedef struct _MW_DATE_TIME {
	WORD						wYear;
	BYTE						byMonth;
	BYTE						byDay;
	BYTE						byHour;
	BYTE						byMinute;
	BYTE						bySecond;
	BYTE						byReserved;
} MW_DATE_TIME;

#define MW_FIRMWARE_HEADER_MAGIC                ((DWORD)('M') | ((DWORD)('W') << 8) | ((DWORD)('F') << 16) | ((DWORD)('H') << 24))
#define MW_FIRMWARE_HEADER_VERSION				1
#define MW_FIRMWARE_NAME_LEN					64
#define MW_FIRMWARE_SECTION_NAME_LEN			16

#define MW_MAX_NUM_FIRMWARE_SECTIONS			16

// Compatible ID: wProductID, chHardwareVersion and byFirmwareID
typedef struct _MW_FIRMWARE_INFO_HEADER {
	DWORD						dwMagic;
	DWORD						dwCheckSum;
	WORD						wVersion;
	WORD						cbHeader;
	WORD						wProductID;
	CHAR						chHardwareVersion;
	BYTE						byFirmwareID;
	DWORD						dwFirmwareVersion;
	CHAR						szProductName[MW_PRODUCT_NAME_LEN];
	CHAR						szFirmwareName[MW_FIRMWARE_NAME_LEN];
	MW_DATE_TIME				dtBuild;
	BYTE						cSections;
} MW_FIRMWARE_INFO_HEADER;

typedef struct _MW_FIRMWARE_SECTION_HEADER {
	CHAR						szName[MW_FIRMWARE_SECTION_NAME_LEN];
	DWORD						cbOffset;
	DWORD						cbSection;
	DWORD						dwCheckSum;
} MW_FIRMWARE_SECTION_HEADER;

typedef struct _MW_FIRMWARE_HEADER {
	MW_FIRMWARE_INFO_HEADER		infoHeader;
	MW_FIRMWARE_SECTION_HEADER	aSectionHeaders[MW_MAX_NUM_FIRMWARE_SECTIONS];
} MW_FIRMWARE_HEADER;

#pragma pack(pop)
