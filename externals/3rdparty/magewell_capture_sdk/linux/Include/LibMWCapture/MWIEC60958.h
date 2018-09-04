////////////////////////////////////////////////////////////////////////////////
// CONFIDENTIAL and PROPRIETARY software of Magewell Electronics Co., Ltd.
// Copyright (c) 2011-2014 Magewell Electronics Co., Ltd. (Nanjing) 
// All rights reserved.
// This copyright notice MUST be reproduced on all authorized copies.
////////////////////////////////////////////////////////////////////////////////

#pragma once

#pragma pack(push)
#pragma pack(1)

typedef union _IEC60958_CHANNEL_STATUS {
    BYTE			abyData[24];
    WORD			awData[12];
    DWORD			adwData[6];

    struct {
        BYTE		byProfessional : 1;
        BYTE		byNotLPCM : 1;
        BYTE		byEncodedAudioSignalEmphasis : 3;	// 000: Emphasis not indicated, 001: No emphasis, 011: 50/15 us emphasis, 111: CCITT J.17 emphasis
        BYTE		bySourceSamplingFreqUnlocked : 1;
        BYTE		byEncodedSamplingFreq : 2;			// 00: Not indicated, 10: 48, 01: 44.1, 11: 32

        BYTE		byEncodedChannelMode : 4;
        BYTE		byEncodedUserBitsManagement : 4;

        BYTE		byZero : 1;
        BYTE		bySingleCoordinationSignal : 1;
        BYTE		byWordLength : 4;					// 0010: 16, 1100: 17, 0100: 18, 1000: 19, 1010: 20, 0011: 20, 1101: 21, 0101: 22, 1001: 23, 1011: 24
        BYTE		byAlignmentLevel : 2;				// 00: Not indicated, 10: 20 dB, 01: 18.06 dB, 11: Reserved

        BYTE		byChannelNumberOrMultiChannelMode : 7;
        BYTE		byDefinedMultiChannelMode : 1;

        BYTE		byReferenceSignal : 2;				// 00: Not a reference signal, 10: Grade 1, 01: Grade 2, 11: Reserved
        BYTE		byReserved1 : 1;
        BYTE		bySamplingFreq : 4;					// 0000: Not indicated, 0001: 24, 0010: 96, 0011: 192, 1001: 22.05, 1010: 88.2, 1011: 176.4, 1111: User defined
        BYTE		bySamplingFreqScaling : 1;

        BYTE		byReserved2;

        BYTE		achChannelOrigin[4];
        BYTE		achChannelDestination[4];
        DWORD		dwLocalSampleAddress;
        DWORD		dwTimeOfDaySampleAddress;
        BYTE		byReliabilityFlags;

        BYTE		byCRC;
    } Professional;

    struct {
        BYTE		byProfessional : 1;
        BYTE		byNotLPCM : 1;
        BYTE		byNoCopyright : 1;
        BYTE		byAdditionalFormatInfo : 3;
        BYTE		byMode : 2;

        BYTE		byCategoryCode;

        BYTE		bySourceNumber : 4;
        BYTE		byChannelNumber : 4;

        BYTE		bySamplingFreq : 4;					// 0100: 22.05, 0000: 44.1, 1000: 88.2, 1100: 176.4, 0110: 24, 0010: 48, 1010: 96, 1110: 192, 0011: 32, 0001: Not indicated, 1001: 768
        BYTE		byClockAccuracy : 2;				// 00: Level II, 10: Level I, 01: Level III, 11: Not matched
        BYTE		byReserved1 : 2;

        BYTE		byWordLength : 4;					// 0010: 16, 1100: 17, 0100: 18, 1000: 19, 1010: 20, 0011: 20, 1101: 21, 0101: 22, 1001: 23, 1011: 24
        BYTE		byOrigSamplingFreq : 4;				// 1111: 44.1, 0111: 88.2, 1011: 22.05, 0011: 176.4, 1101: 48, 0101: 96, 1001: 24, 0001: 192, 0110: 8, 1010: 11.025, 0010: 12, 1100: 32, 1000: 16, 0000: Not indicated

        BYTE		byCGMS_A;							// 00: Copying permitted, 10: Condition not be used, 01: One generation only, 11: No copying is permitted
    } Consumer;
} IEC60958_CHANNEL_STATUS;

static inline int IEC60958_GetBitsPerSample(BYTE byWordLength)
{
    switch (byWordLength) {
    case 2:		return 16;
    case 3:		return 20;
    case 4:		return 18;
    case 5:		return 22;
    case 8:		return 19;
    case 9:		return 23;
    case 10:	return 20;
    case 11:	return 24;
    case 12:	return 17;
    case 13:	return 21;
    default:	return 16;
    }
}

static inline DWORD IEC60958P_GetSampleRate(IEC60958_CHANNEL_STATUS * pStatus)
{
    switch (pStatus->Professional.byEncodedSamplingFreq) {
    case 0:
        switch (pStatus->Professional.bySamplingFreq) {
        case 1: return 24000;
        case 2: return 96000;
        case 3: return 192000;
        case 5: return 22050;
        case 6: return 88200;
        case 7: return 176400;
        }
        break;

    case 1:
        return 44100;

    case 2:
        return 48000;

    case 3:
        return 32000;
    }

    return 48000;
}

static inline DWORD IEC60958C_GetSampleRate(IEC60958_CHANNEL_STATUS * pStatus)
{
    switch (pStatus->Consumer.bySamplingFreq) {
    case 0:  return 44100;
    case 2:  return 48000;
    case 3:  return 32000;
    case 4:  return 22050;
    case 6:  return 24000;
    case 8:  return 88200;
    case 9:  return 768000;
    case 10: return 96000;
    case 12: return 176400;
    case 14: return 192000;
    }

    return 44100;
}

static inline BOOLEAN IEC60958_ParseChannelStatus(IEC60958_CHANNEL_STATUS * pStatus, BOOLEAN * pbLPCM, DWORD * pdwSampleRate, int * pnBitsPerSample)
{
    BOOLEAN bLPCM;
    DWORD dwSampleRate = 48000;
    int nBitsPerSample = 16;

    if (pStatus->Professional.byProfessional) {
        bLPCM = !pStatus->Professional.byNotLPCM;
        dwSampleRate = IEC60958P_GetSampleRate(pStatus);
        nBitsPerSample = IEC60958_GetBitsPerSample(pStatus->Professional.byWordLength);
    }
    else {
        bLPCM = !pStatus->Consumer.byNotLPCM;
        dwSampleRate = IEC60958C_GetSampleRate(pStatus);
        nBitsPerSample = IEC60958_GetBitsPerSample(pStatus->Consumer.byWordLength);
    }

    if (pbLPCM) *pbLPCM = bLPCM;
    if (pdwSampleRate) *pdwSampleRate = dwSampleRate;
    if (pnBitsPerSample) *pnBitsPerSample = nBitsPerSample;

    return TRUE;
}

#pragma pack(pop)
