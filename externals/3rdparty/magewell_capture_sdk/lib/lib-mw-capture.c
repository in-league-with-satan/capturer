
#include "lib-mw-capture.h"
#include "../inc/ProductVer.h"


typedef struct _CHANNEL_INFO_
{
    char                    szDevicePath[MAX_CHANNEL_NAME_LEN];
    MWCAP_CHANNEL_INFO      channelInfo;
    MWCAP_PRO_CAPTURE_INFO  familyInfo;
} CHANNEL_INFO;


static int g_nChannelCount = 0;
static CHANNEL_INFO g_arrChannelInfo[MAX_CHANNEL_COUNT];
static int g_hEventFD = -1;


// Private
int MWCaptureVideoFilter(const struct dirent *pdir)
{
    int ret ;
    ret = fnmatch("video*", pdir->d_name, FNM_PATHNAME);

    if (ret == 0)
        return 1;

    return 0;
}

//lib-mw-capture
MW_RESULT MWGetVersion(unsigned char *pbyMaj, unsigned char *pbyMin, unsigned short *pwBuild)
{
    if (pbyMaj == NULL || pbyMin == NULL || pwBuild == NULL || g_nChannelCount == 0)
    {
        //fprintf(stderr, "Function  %s parameter error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_CHANNEL_INFO channelInfo;
    MWGetChannelInfoByIndex(0, &channelInfo);

    *pbyMaj = ((channelInfo.dwDriverVersion >> 24) & 0xFF);
    *pbyMin = ((channelInfo.dwDriverVersion >> 16) & 0xFF);
    *pwBuild = ((channelInfo.dwDriverVersion) & 0xFFFF);

    return MW_SUCCEEDED;
}


BOOL MWCaptureInitInstance()
{
    g_nChannelCount = 0;

    int i;

    for (i = 0; i < MAX_CHANNEL_COUNT; i++)
    {
        g_arrChannelInfo[i].szDevicePath[0] = '\0';
    }

    if (g_hEventFD == -1)
    {
        g_hEventFD = open("/dev/mw-event", O_RDWR);
        if (g_hEventFD <= 0) {
            //fprintf(stderr,"%s[%d]: open /dev/mw-event failed: %m\n",
                   //__func__, __LINE__);

            g_hEventFD = -1;
            return FALSE;
        }
    }

    return TRUE;
}


void MWCaptureExitInstance()
{
    int i;

    for (i = 0; i < g_nChannelCount; i++)
    {
        g_arrChannelInfo[i].szDevicePath[0] = '\0';
    }

    g_nChannelCount = 0;

    if (g_hEventFD != -1)
    {
        close(g_hEventFD);
        g_hEventFD = -1;
    }
}


MW_RESULT MWRefreshDevice()
{
    int i = 0;
    struct dirent **nameList;
    int nVideoNum;

    g_nChannelCount = 0;

    nVideoNum = scandir("/dev", &nameList, MWCaptureVideoFilter, alphasort);
    if (nVideoNum < 0)
    {
        //fprintf(stderr , "No pro capture device found: %m\n");
        return MW_FAILED;
    }

    for (i = 0; i < nVideoNum; i++)
    {
        char szDevicePath[16];
        sprintf(szDevicePath, "/dev/%s", nameList[i]->d_name);

        HCHANNEL hChannel;
        hChannel = open(szDevicePath, O_RDWR | O_NONBLOCK);
        if ( hChannel == -1)
        {
            //fprintf(stderr, "Open Channel %s failed: %m\n", szDevicePath);
            continue;
        }

        MWCAP_CHANNEL_INFO mci;
        if (ioctl(hChannel, MWCAP_IOCTL_GET_CHANNEL_INFO, &mci) != 0)
        {
            //fprintf(stderr, "ioctl MWCAP_IOCTL_GET_CHANNEL_INFO error: %m\n");
            close(hChannel);
            hChannel = -1;
            continue;
        }

        MWCAP_PRO_CAPTURE_INFO familyInfo;
        if (ioctl(hChannel, MWCAP_IOCTL_GET_FAMILY_INFO, &familyInfo) != 0)
        {
            //fprintf(stderr, "ioctl MWCAP_IOCTL_GET_FAMILY_INFO error: %m\n");
            close(hChannel);
            hChannel = -1;
            continue;
        }

        strcpy(g_arrChannelInfo[g_nChannelCount].szDevicePath, szDevicePath);
        g_arrChannelInfo[g_nChannelCount].channelInfo = mci;
        g_arrChannelInfo[g_nChannelCount].familyInfo = familyInfo;

        g_nChannelCount++;

        close(hChannel);
        hChannel = -1;
    }

    free(nameList);

    return MW_SUCCEEDED;

}

int MWGetChannelCount()
{
    return g_nChannelCount;
}

MW_RESULT MWGetChannelInfoByIndex(int nIndex, MWCAP_CHANNEL_INFO *pChannelInfo)
{
    if (nIndex >= g_nChannelCount)
        {
            //fprintf(stderr, "Function: %s, argument \"nIndex\" error\n", __FUNCTION__);
            return MW_FAILED;
        }

        if (pChannelInfo == NULL)
        {
            //fprintf(stderr, "Function: %s, argument \"pChannelInfo\" error\n", __FUNCTION__);
            return MW_FAILED;
        }

        memcpy(pChannelInfo, &g_arrChannelInfo[nIndex].channelInfo, sizeof(MWCAP_CHANNEL_INFO));
        return MW_SUCCEEDED;
}


MW_RESULT MWGetFamilyInfoByIndex(int nIndex, LPVOID pFamilyInfo, unsigned int dwSize)
{
    if (nIndex >= g_nChannelCount)
        {
            //fprintf(stderr, "Function: %s, argument \"nIndex\" error\n", __FUNCTION__);
            return MW_FAILED;
        }

        if (pFamilyInfo == NULL)
        {
            //fprintf(stderr, "Function: %s, argument \"pFamilyInfo\" error\n", __FUNCTION__);
            return MW_FAILED;
        }

        memcpy(pFamilyInfo, &g_arrChannelInfo[nIndex].familyInfo, dwSize);
        return MW_SUCCEEDED;
}

MW_RESULT MWGetDevicePath(int nIndex, char *pDevicePath)
{
    if (nIndex >= g_nChannelCount || pDevicePath == NULL)
    {
        //fprintf(stderr, "Function: %s, Parameter range overflower!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    strcpy(pDevicePath, g_arrChannelInfo[nIndex].szDevicePath);

    return MW_SUCCEEDED;
}


HCHANNEL MWOpenChannel(int nBoardValue, int nChannelIndex)
{
    HCHANNEL hChannel = -1;
    int i = 0;

    if (nBoardValue < 0 || nBoardValue >=MAX_BOARD_ID || nChannelIndex < 0 || nChannelIndex >= MAX_BOARD_ID )
    {
        //fprintf(stderr, "Function: %s, Parameter range overflower!\n", __FUNCTION__);
        return -1;
    }

    for (i = 0; i < g_nChannelCount; i++)
    {
        MWCAP_CHANNEL_INFO mci;
        if (MWGetChannelInfoByIndex(i, &mci) != MW_SUCCEEDED)
            continue;

        if (mci.byBoardIndex != nBoardValue || mci.byChannelIndex != nChannelIndex)
            continue;

        hChannel = open(g_arrChannelInfo[i].szDevicePath, O_RDWR | O_NONBLOCK);
        if (hChannel == -1)
            continue;

        ////fprintf(stderr,"Open Channel %d!\n", i);

        return hChannel;
    }

    //fprintf(stderr, "Function: %s, No matching channel is found!\n", __FUNCTION__);
    return -1;
}

HCHANNEL MWOpenChannelByPath(const char *pszDevicePath)
{
    int i = 0;
    if (pszDevicePath == NULL)
    {
        //fprintf(stderr, "Function: %s, Device path can't be NULL!\n", __FUNCTION__);
        return -1;
    }

    HCHANNEL hChannel;
    for (i = 0; i < g_nChannelCount; i++)
    {
        if (strcmp(g_arrChannelInfo[i].szDevicePath, pszDevicePath) != 0)
            continue;

        hChannel = open(pszDevicePath, O_RDWR | O_NONBLOCK);
        if (hChannel == -1)
        {
            //fprintf(stderr, "Function: %s, Open channel %s error: %m\n", __FUNCTION__, pszDevicePath);
            return -1;
        }

        return hChannel;
    }

    //fprintf(stderr, "Function: %s, Device path is illegal!\n", __FUNCTION__);
    return -1;
}

void MWCloseChannel(int hChannel)
{
    if (hChannel == -1)
            return;

    if (close(hChannel) != 0)
    {
        //fprintf(stderr, "close Device %d error: %m\n", hChannel);
    }

    return;
}


MW_RESULT MWGetChannelInfo(int hChannel, MWCAP_CHANNEL_INFO *pChannelInfo)
{
    if (hChannel == -1 || pChannelInfo == NULL)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if(ioctl(hChannel, MWCAP_IOCTL_GET_CHANNEL_INFO, pChannelInfo) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_CHANNEL_INFO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetFamilyInfo(int hChannel, LPVOID pFamilyInfo, unsigned int dwSize)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_PRO_CAPTURE_INFO mpci;
    if(ioctl(hChannel, MWCAP_IOCTL_GET_FAMILY_INFO, &mpci) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_FAMILY_INFO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    memcpy(pFamilyInfo, &mpci,dwSize);
    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoCaps(int hChannel, MWCAP_VIDEO_CAPS *pVideoCaps)
{
    if (hChannel == -1 || pVideoCaps == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if(ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_CAPS, pVideoCaps) != 0)
    {
        //fprintf(stderr, "FUnction: %s, ioctl MWCAP_IOCTL_GET_VIDEO_CAPS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetAudioCaps(int hChannel, MWCAP_AUDIO_CAPS *pAudioCaps)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if(ioctl(hChannel, MWCAP_IOCTL_GET_AUDIO_CAPS, pAudioCaps) != 0)
    {
        //fprintf(stderr, "FUnction: %s, ioctl MWCAP_IOCTL_GET_AUDIO_CAPS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoInputSourceArray(int hChannel, unsigned int *pdwInputSource, unsigned int *pdwInputCount)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_INPUT_SOURCE_ARRAY misa;
    misa.count = *pdwInputCount;
    misa.data = (MWCAP_PTR)(unsigned long)pdwInputSource;
    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_INPUT_SOURCE_ARRAY, &misa) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_INPUT_SOURCE_ARRAY error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    if (*pdwInputCount < misa.count)
    {
        //fprintf(stderr, "Function: %s, alloced buffer isn't larger ebough!\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pdwInputCount = misa.count;

    return MW_SUCCEEDED;
}

MW_RESULT MWGetAudioInputSourceArray(int hChannel, unsigned int *pdwInputSource, unsigned int *pdwInputCount)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_INPUT_SOURCE_ARRAY misa;
    misa.count = *pdwInputCount;
    misa.data = (MWCAP_PTR)(unsigned long)pdwInputSource;
    ////fprintf(stderr, "Number of set InputCount = %d\n", *pdwInputCount);
    if (ioctl(hChannel, MWCAP_IOCTL_AUDIO_INPUT_SOURCE_ARRAY, &misa) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl AUDIO_INPUT_SOURCE_ARRAY_NUM error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    if (*pdwInputCount < misa.count)
    {
        //fprintf(stderr, "Function: %s, alloced buffer isn't larger ebough!\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pdwInputCount = misa.count;

    return MW_SUCCEEDED;
}

MW_RESULT MWGetInputSourceScan(int hChannel, BOOLEAN *pbScan)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }


    if (ioctl(hChannel, MWCAP_IOCTL_GET_INPUT_SOURCE_SCAN, pbScan) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_INPUT_SOURCE_SCAN error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetInputSourceScan(int hChannel, BOOLEAN bScan)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_INPUT_SOURCE_SCAN, &bScan) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_INPUT_SOURCE_SCAN error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetAVInputSourceLink(int hChannel, BOOLEAN *pbLink)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_AV_INPUT_SOURCE_LINK, pbLink) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_AV_INPUT_SOURCE_LINK error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetAVInputSourceLink(int hChannel, BOOLEAN bLink)
{
    if (hChannel == -1)
    {
       //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_AV_INPUT_SOURCE_LINK, &bLink) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_AV_INPUT_SOURCE_LINK error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoInputSource(int hChannel, unsigned int *pdwSource)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_INPUT_SOURCE, pdwSource) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_INPUT_SOURCE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoInputSource(int hChannel, unsigned int dwSource)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_INPUT_SOURCE, &dwSource) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_INPUT_SOURCE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetAudioInputSource(int hChannel, unsigned int *pdwSource)
{
    if (hChannel == -1)
    {
       //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_AUDIO_INPUT_SOURCE, pdwSource) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_AUDIO_INPUT_SOURCE error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetAudioInputSource(int hChannel, unsigned int dwSource)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_AUDIO_INPUT_SOURCE, &dwSource) != 0)
    {
        //fprintf(stderr, "FUnction: %s, ioctl MWCAP_IOCTL_SET_AUDIO_INPUT_SOURCE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

//EDID
MW_RESULT MWGetEDID(int hChannel, unsigned char *pbyData, ULONG *pulSize)
{
    if (hChannel == -1 || pbyData == NULL || pulSize == NULL)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_EDID_DATA edidData;
    edidData.data = (MWCAP_PTR)(unsigned long)pbyData;
    edidData.size = *pulSize;
    if (ioctl(hChannel, MWCAP_IOCTL_GET_EDID_DATA, &edidData) != 0)
    {
        //fprintf(stderr, "FUnction: %s, ioctl MWCAP_IOCTL_GET_EDID_DATA error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pulSize = edidData.size;

    return MW_SUCCEEDED;
}

MW_RESULT MWSetEDID(int hChannel, unsigned char *pbyData, ULONG ulSize)
{
    if (hChannel == -1 || pbyData == NULL)
    {
       //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    MWCAP_EDID_DATA edidData;
    edidData.data = (MWCAP_PTR)(unsigned long)pbyData;
    edidData.size = ulSize;

    if (ioctl(hChannel, MWCAP_IOCTL_SET_EDID_DATA, &edidData) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_EDID_DATA error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

//Signal status
MW_RESULT MWGetInputSpecificStatus(int hChannel, MWCAP_INPUT_SPECIFIC_STATUS *pInputStatus)
{
    if (hChannel == -1 || pInputStatus == NULL)
    {
        //fprintf(stderr, "Function  %s parameter error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_INPUT_SPECIFIC_STATUS, pInputStatus) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_INPUT_SPECIFIC_STATUS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT   MWGetVideoSignalStatus(int hChannel, MWCAP_VIDEO_SIGNAL_STATUS *pSignalStatus)
{
    if (hChannel == -1 || pSignalStatus == NULL)
    {
       //fprintf(stderr, "Function  %s parameter error!\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_SIGNAL_STATUS, pSignalStatus) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_SIGNAL_STATUS error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetAudioSignalStatus(int hChannel, MWCAP_AUDIO_SIGNAL_STATUS *pSignalStatus)
{
    if (hChannel == -1 || pSignalStatus == NULL)
    {
        //fprintf(stderr, "Function  %s parameter error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_AUDIO_SIGNAL_STATUS, pSignalStatus) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_AUDIO_SIGNAL_STATUS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// HDMI InfoFrame
MW_RESULT MWGetHDMIInfoFrameValidFlag(int hChannel, unsigned int *pdwValidFlag)
{
    if (hChannel == -1 || NULL == pdwValidFlag)
    {
        //fprintf(stderr, "Function  %s parameter error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_HDMI_INFOFRAME_VALID, pdwValidFlag) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_HDMI_INFOFRAME_VALID error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetHDMIInfoFramePacket(int hChannel, MWCAP_HDMI_INFOFRAME_ID id, HDMI_INFOFRAME_PACKET *pPacket)
{
    if (hChannel == -1 || pPacket == NULL ||
            id < MWCAP_HDMI_INFOFRAME_ID_AVI ||
            id > MWCAP_HDMI_INFOFRAME_COUNT)
    {
        //fprintf(stderr, "Function  %s parameter error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_HDMI_INFOFRAME_PACKET mhip;
    mhip.id = id;
    if (ioctl(hChannel, MWCAP_IOCTL_GET_HDMI_INFOFRAME_PACKET, &mhip) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_HDMI_INFOFRAME_PACKET error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    memcpy(pPacket, &mhip.pkt, sizeof(HDMI_INFOFRAME_PACKET));
    return MW_SUCCEEDED;
}

// Device Time
MW_RESULT MWGetDeviceTime(int hChannel, long long *pllTime)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_TIME, pllTime) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_TIME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetDeviceTime(int hChannel, long long llTime)
{
    if (hChannel == -1)
    {
       //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_TIME, &llTime) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_TIME error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWRegulateDeviceTime(int hChannel, long long llTime)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_TIME_REGULATION, &llTime) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_TIME_REGULATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// Timer Event
HTIMER MWRegisterTimer(int hChannel, HANDLE64 hEvent)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return 0;
    }

    MWCAP_TIMER_REGISTRATION_S timerReg;
    timerReg.pvEvent = hEvent;

    if (ioctl(hChannel, MWCAP_IOCTL_TIMER_REGISTRATION, &timerReg) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_TIMER_REGISTRATION error: %m\n", __FUNCTION__);
        return 0;
    }

    return timerReg.pvTimer;
}

MW_RESULT MWUnregisterTimer(int hChannel, MWCAP_PTR hTimer)
{
    if (hChannel == -1 || hTimer == 0)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_TIMER_DEREGISTRATION, &hTimer) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_TIMER_DEREGISTRATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWScheduleTimer(int hChannel, MWCAP_PTR hTimer, long long llExpireTime)
{
    if (hChannel == -1 || hTimer == 0)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_TIMER_EXPIRE_TIME mtet;
    mtet.llExpireTime = llExpireTime;
    mtet.pvTimer = hTimer;

    if (ioctl(hChannel, MWCAP_IOCTL_TIMER_EXPIRE_TIME, &mtet) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_TIMER_EXPIRE_TIME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


// Notify Event
HNOTIFY MWRegisterNotify(int hChannel, MWCAP_PTR hEvent, unsigned int dwEnableBits)
{
    if (hChannel == -1 || hEvent == 0)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return 0;
    }

    MWCAP_NOTIFY_REGISTRATION_S mnrs;
    mnrs.ullEnableBits = dwEnableBits;
    mnrs.pvEvent = hEvent;

    if (ioctl(hChannel, MWCAP_IOCTL_NOTIFY_REGISTRATION,&mnrs) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_NOTIFY_REGISTRATION error: %m\n", __FUNCTION__);
        return 0;
    }

    return mnrs.pvNotify;
}

MW_RESULT MWUnregisterNotify(int hChannel, MWCAP_PTR hNotify)
{
    if (hChannel == -1 || hNotify == 0)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }


    if (ioctl(hChannel, MWCAP_IOCTL_NOTIFY_DEREGISTRATION, &hNotify) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_NOTIFY_DEREGISTRATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetNotifyStatus(int hChannel, MWCAP_PTR hNotify, unsigned long long *pullStatus)
{
    if (hChannel == -1 || hNotify == 0)
    {
       //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    MWCAP_NOTIFY_STATUS mns;
    mns.pvNotify = hNotify;

    if (ioctl(hChannel, MWCAP_IOCTL_NOTIFY_STATUS, &mns) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_NOTIFY_DEREGISTRATION error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }


    *pullStatus = mns.ullStatusBits;
    return MW_SUCCEEDED;
}

MW_RESULT MWStartVideoCapture(int hChannel, MWCAP_PTR hEvent)
{
    if (hChannel == -1 || hEvent == 0)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_OPEN captureOpen;
    captureOpen.hEvent = hEvent;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_OPEN, &captureOpen) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_OPEN error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWStopVideoCapture(int hChannel)
{
    if (hChannel == -1)
    {
       //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_CLOSE) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_CLOSE error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWPinVideoBuffer(int hChannel, MWCAP_PTR pbFrame, unsigned int cbFrame)
{
    if (hChannel == -1 || pbFrame == 0)
    {
       //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_PIN_BUFFER pinBuffer;
    pinBuffer.pvBuffer = pbFrame;
    pinBuffer.cbBuffer = cbFrame;
    pinBuffer.mem_type = MWCAP_VIDEO_MEMORY_TYPE_USER;


    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_PIN_BUFFER, &pinBuffer) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_PIN_BUFFER error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWUnpinVideoBuffer(int hChannel, MWCAP_PTR pbFrame)
{
    if (hChannel == -1 || pbFrame == 0)
    {
       //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_UNPIN_BUFFER, &pbFrame) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_UNPIN_BUFFER error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWCaptureVideoFrameToVirtualAddress(int           hChannel,
                                              int           iFrame,
                                              MWCAP_PTR     pbFrame,
                                              unsigned int  cbFrame,
                                              unsigned int  cbStride,
                                              BOOLEAN          bBottomUp,
                                              MWCAP_PTR     pvContext,
                                              unsigned int  dwFOURCC,
                                              int           cx,
                                              int           cy
                                              )
{
    if (hChannel == -1 || pbFrame == 0)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_FRAME mvcf = {0};
    mvcf.dwFOURCC = dwFOURCC;
    mvcf.cx = (WORD)cx;
    mvcf.cy = (WORD)cy;

    mvcf.sContrast = 100;
    mvcf.sBrightness = 0;
    mvcf.sSaturation = 100;
    mvcf.sHue = 0;

    mvcf.nAspectX = 0;
    mvcf.nAspectY = 0;
    mvcf.colorFormat = MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;
    mvcf.quantRange = MWCAP_VIDEO_QUANTIZATION_UNKNOWN;

    mvcf.rectSource.top = 0;
    mvcf.rectSource.left = 0;
    mvcf.rectSource.bottom = 0;
    mvcf.rectSource.right = 0;

    mvcf.deinterlaceMode = MWCAP_VIDEO_DEINTERLACE_BLEND;
    mvcf.aspectRatioConvertMode = MWCAP_VIDEO_ASPECT_RATIO_IGNORE;
    mvcf.iSrcFrame = iFrame;

    mvcf.bPhysicalAddress = false;
    mvcf.cbFrame = cbFrame;
    mvcf.pvFrame = pbFrame;

    mvcf.cbStride = cbStride;
    mvcf.bBottomUp = bBottomUp;

    mvcf.pvContext = pvContext;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_FRAME, &mvcf) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWCaptureVideoFrameToPhysicalAddress(int              hChannel,
                                               int              iFrame,
                                               LARGE_INTEGER    liFrameAddress,
                                               unsigned int     cbFrame,
                                               unsigned int     cbStride,
                                               BOOLEAN             bBottomUp,
                                               MWCAP_PTR        pvContext,
                                               unsigned int     dwFOURCC,
                                               int              cx,
                                               int              cy
                                               )
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_FRAME mvcf = {0};
    mvcf.dwFOURCC = dwFOURCC;
    mvcf.cx = (WORD)cx;
    mvcf.cy = (WORD)cy;

    mvcf.sContrast = 100;
    mvcf.sBrightness = 0;
    mvcf.sSaturation = 100;
    mvcf.sHue = 0;


    mvcf.nAspectX = 0;
    mvcf.nAspectY = 0;
    mvcf.colorFormat = MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;
    mvcf.quantRange = MWCAP_VIDEO_QUANTIZATION_UNKNOWN;

    mvcf.rectSource.top = 0;
    mvcf.rectSource.left = 0;
    mvcf.rectSource.bottom = 0;
    mvcf.rectSource.right = 0;

    mvcf.deinterlaceMode = MWCAP_VIDEO_DEINTERLACE_BLEND;
    mvcf.aspectRatioConvertMode = MWCAP_VIDEO_ASPECT_RATIO_IGNORE;
    mvcf.iSrcFrame = iFrame;

    mvcf.bPhysicalAddress = true;
    mvcf.cbFrame = cbFrame;
    mvcf.liPhysicalAddress = liFrameAddress;

    mvcf.cbStride = cbStride;
    mvcf.bBottomUp = bBottomUp;

    mvcf.pvContext = pvContext;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_FRAME, &mvcf) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWCaptureVideoFrameWithOSDToVirtualAddress(int            hChannel,
                                                     int            iFrame,
                                                     MWCAP_PTR      pbFrame,
                                                     unsigned int   cbFrame,
                                                     unsigned int   cbStride,
                                                     BOOLEAN           bBottomUp,
                                                     MWCAP_PTR      pvContext,
                                                     unsigned int   dwFOURCC,
                                                     int            cx,
                                                     int            cy,
                                                     MWCAP_PTR      pOSDImage,
                                                     const RECT *   pOSDRects,
                                                     int            cOSDRects
                                                     )
{
    if (hChannel == -1 || pbFrame == 0 || pOSDImage == 0)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_FRAME mvcf = {0};
    mvcf.dwFOURCC = dwFOURCC;
    mvcf.cx = (WORD)cx;
    mvcf.cy = (WORD)cy;

    mvcf.sContrast = 100;
    mvcf.sBrightness = 0;
    mvcf.sSaturation = 100;
    mvcf.sHue = 0;

    mvcf.nAspectX = 0;
    mvcf.nAspectY = 0;
    mvcf.colorFormat = MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;
    mvcf.quantRange = MWCAP_VIDEO_QUANTIZATION_UNKNOWN;

    mvcf.rectSource.top = 0;
    mvcf.rectSource.left = 0;
    mvcf.rectSource.bottom = 0;
    mvcf.rectSource.right = 0;

    mvcf.deinterlaceMode = MWCAP_VIDEO_DEINTERLACE_BLEND;
    mvcf.aspectRatioConvertMode = MWCAP_VIDEO_ASPECT_RATIO_IGNORE;
    mvcf.iSrcFrame = iFrame;

    mvcf.pOSDImage = pOSDImage;
    memcpy(mvcf.aOSDRects, pOSDRects, cOSDRects * sizeof(RECT));
    mvcf.cOSDRects = cOSDRects;

    mvcf.bPhysicalAddress = false;
    mvcf.cbFrame = cbFrame;
    mvcf.pvFrame = pbFrame;

    mvcf.cbStride = cbStride;
    mvcf.bBottomUp = bBottomUp;

    mvcf.pvContext = pvContext;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_FRAME, &mvcf) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWCaptureVideoFrameWithOSDToPhysicalAddress(int               hChannel,
                                                      int               iFrame,
                                                      LARGE_INTEGER     liFrameAddress,
                                                      unsigned int      cbFrame,
                                                      unsigned int      cbStride,
                                                      BOOLEAN              bBottomUp,
                                                      MWCAP_PTR         pvContext,
                                                      unsigned int      dwFOURCC,
                                                      int               cx,
                                                      int               cy,
                                                      MWCAP_PTR         pOSDImage,
                                                      const RECT *      pOSDRects,
                                                      int               cOSDRects
                                                      )
{
    if (hChannel == -1 || pOSDImage == 0)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_FRAME mvcf = {0};
    mvcf.dwFOURCC = dwFOURCC;
    mvcf.cx = (WORD)cx;
    mvcf.cy = (WORD)cy;

    mvcf.sContrast = 100;
    mvcf.sBrightness = 0;
    mvcf.sSaturation = 100;
    mvcf.sHue = 0;


    mvcf.nAspectX = 0;
    mvcf.nAspectY = 0;
    mvcf.colorFormat = MWCAP_VIDEO_COLOR_FORMAT_UNKNOWN;
    mvcf.quantRange = MWCAP_VIDEO_QUANTIZATION_UNKNOWN;

    mvcf.rectSource.top = 0;
    mvcf.rectSource.left = 0;
    mvcf.rectSource.bottom = 0;
    mvcf.rectSource.right = 0;

    mvcf.deinterlaceMode = MWCAP_VIDEO_DEINTERLACE_BLEND;
    mvcf.aspectRatioConvertMode = MWCAP_VIDEO_ASPECT_RATIO_IGNORE;
    mvcf.iSrcFrame = iFrame;

    mvcf.pOSDImage = pOSDImage;
    memcpy(mvcf.aOSDRects, pOSDRects, cOSDRects * sizeof(RECT));
    mvcf.cOSDRects = cOSDRects;

    mvcf.bPhysicalAddress = true;
    mvcf.cbFrame = cbFrame;
    mvcf.liPhysicalAddress = liFrameAddress;

    mvcf.cbStride = cbStride;
    mvcf.bBottomUp = bBottomUp;

    mvcf.pvContext = pvContext;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_FRAME, &mvcf) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWCaptureVideoFrameToVirtualAddressEx(int                                     hChannel,
                                                int                                     iFrame,
                                                MWCAP_PTR                               pbFrame,
                                                unsigned int                            cbFrame,
                                                unsigned int                            cbStride,
                                                BOOLEAN                                    bBottomUp,
                                                MWCAP_PTR                               pvContext,
                                                unsigned int                            dwFOURCC,
                                                int                                     cx,
                                                int                                     cy,
                                                unsigned int                            dwProcessSwitchs,
                                                int                                     cyPartialNotify,
                                                MWCAP_PTR                               pOSDImage,
                                                const RECT *                            pOSDRects,
                                                int                                     cOSDRects,
                                                short                                   sContrast,
                                                short                                   sBrightness,
                                                short                                   sSaturation,
                                                short                                   sHue,
                                                MWCAP_VIDEO_DEINTERLACE_MODE            deinterlaceMode,
                                                MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE   aspectRatioConvertMode,
                                                const RECT *                            pRectSrc,
                                                const RECT *                            pRectDest,
                                                int                                     nAspectX,
                                                int                                     nAspectY,
                                                MWCAP_VIDEO_COLOR_FORMAT                colorFormat,
                                                MWCAP_VIDEO_QUANTIZATION_RANGE          quantRange,
                                                MWCAP_VIDEO_SATURATION_RANGE            satRange
                                                )
{
    if (hChannel == -1 || pbFrame == 0)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_FRAME mvcf = {0};
    mvcf.dwFOURCC = dwFOURCC;
    mvcf.cx = (WORD)cx;
    mvcf.cy = (WORD)cy;

    mvcf.sContrast = sContrast;
    mvcf.sBrightness = sBrightness;
    mvcf.sSaturation = sSaturation;
    mvcf.sHue = sHue;

    mvcf.nAspectX = nAspectX;
    mvcf.nAspectY = nAspectY;
    mvcf.colorFormat = colorFormat;
    mvcf.quantRange = quantRange;
    mvcf.satRange = satRange;

    if (pRectSrc != 0) {
        mvcf.rectSource.top = pRectSrc->top;
        mvcf.rectSource.left = pRectSrc->left;
        mvcf.rectSource.bottom = pRectSrc->bottom;
        mvcf.rectSource.right = pRectSrc->right;
    }
    if (pRectDest != 0) {
        mvcf.rectTarget.top = pRectDest->top;
        mvcf.rectTarget.left = pRectDest->left;
        mvcf.rectTarget.bottom = pRectDest->bottom;
        mvcf.rectTarget.right = pRectDest->right;
    }

    mvcf.deinterlaceMode = deinterlaceMode;
    mvcf.aspectRatioConvertMode = aspectRatioConvertMode;
    mvcf.iSrcFrame = iFrame;

    mvcf.bPhysicalAddress = false;
    mvcf.cbFrame = cbFrame;
    mvcf.pvFrame = (MWCAP_PTR)(unsigned long)pbFrame;

    mvcf.cbStride = cbStride;
    mvcf.bBottomUp = bBottomUp;

    mvcf.pOSDImage = pOSDImage;
    memcpy(mvcf.aOSDRects, pOSDRects, cOSDRects * sizeof(RECT));
    mvcf.cOSDRects = cOSDRects;

    mvcf.pvContext = pvContext;

    mvcf.dwProcessSwitchs = dwProcessSwitchs;
    mvcf.cyPartialNotify = cyPartialNotify;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_FRAME, &mvcf) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }
    return MW_SUCCEEDED;
}

MW_RESULT MWCaptureVideoFrameToPhysicalAddressEx(int                                            hChannel,
                                                 int                                            iFrame,
                                                 LARGE_INTEGER                                  liFrameAddress,
                                                 unsigned int                                   cbFrame,
                                                 unsigned int                                   cbStride,
                                                 BOOLEAN                                        bBottomUp,
                                                 MWCAP_PTR                                      pvContext,
                                                 unsigned int                                   dwFOURCC,
                                                 int                                            cx,
                                                 int                                            cy,
                                                 unsigned int                                   dwProcessSwitchs,
                                                 int                                            cyPartialNotify,
                                                 MWCAP_PTR                                      pOSDImage,
                                                 const RECT *                                   pOSDRects,
                                                 int                                            cOSDRects,
                                                 short                                          sContrast,
                                                 short                                          sBrightness,
                                                 short                                          sSaturation,
                                                 short                                          sHue,
                                                 MWCAP_VIDEO_DEINTERLACE_MODE                   deinterlaceMode,
                                                 MWCAP_VIDEO_ASPECT_RATIO_CONVERT_MODE          aspectRatioConvertMode,
                                                 const RECT *                                   pRectSrc,
                                                 const RECT *                                   pRectDest,
                                                 int                                            nAspectX,
                                                 int                                            nAspectY,
                                                 MWCAP_VIDEO_COLOR_FORMAT                       colorFormat,
                                                 MWCAP_VIDEO_QUANTIZATION_RANGE                 quantRange,
                                                 MWCAP_VIDEO_SATURATION_RANGE                   satRange
                                                 )
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CAPTURE_FRAME mvcf = {0};
    mvcf.dwFOURCC = dwFOURCC;
    mvcf.cx = (WORD)cx;
    mvcf.cy = (WORD)cy;

    mvcf.sContrast = sContrast;
    mvcf.sBrightness = sBrightness;
    mvcf.sSaturation = sSaturation;
    mvcf.sHue = sHue;

    mvcf.nAspectX = nAspectX;
    mvcf.nAspectY = nAspectY;
    mvcf.colorFormat = colorFormat;
    mvcf.quantRange = quantRange;
    mvcf.satRange = satRange;

    if (pRectSrc != 0) {
        mvcf.rectSource.top = pRectSrc->top;
        mvcf.rectSource.left = pRectSrc->left;
        mvcf.rectSource.bottom = pRectSrc->bottom;
        mvcf.rectSource.right = pRectSrc->right;
    }
    if (pRectDest != 0) {
        mvcf.rectTarget.top = pRectDest->top;
        mvcf.rectTarget.left = pRectDest->left;
        mvcf.rectTarget.bottom = pRectDest->bottom;
        mvcf.rectTarget.right = pRectDest->right;
    }

    mvcf.deinterlaceMode = deinterlaceMode;
    mvcf.aspectRatioConvertMode = aspectRatioConvertMode;
    mvcf.iSrcFrame = iFrame;

    mvcf.bPhysicalAddress = true;
    mvcf.cbFrame = cbFrame;
    mvcf.liPhysicalAddress = liFrameAddress;

    mvcf.cbStride = cbStride;
    mvcf.bBottomUp = bBottomUp;

    mvcf.pOSDImage = pOSDImage;
    memcpy(mvcf.aOSDRects, pOSDRects, cOSDRects * sizeof(RECT));
    mvcf.cOSDRects = cOSDRects;

    mvcf.pvContext = pvContext;

    mvcf.dwProcessSwitchs = dwProcessSwitchs;
    mvcf.cyPartialNotify = cyPartialNotify;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_FRAME, &mvcf) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }
    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoBufferInfo(int hChannel, MWCAP_VIDEO_BUFFER_INFO *pVideoBufferInfo)
{
    if (hChannel == -1 || pVideoBufferInfo == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_BUFFER_INFO, pVideoBufferInfo) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_BUFFER_INFO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoFrameInfo(int hChannel, unsigned char i, MWCAP_VIDEO_FRAME_INFO *pVideoFrameInfo)
{
    if (hChannel == -1 || pVideoFrameInfo == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_FRAME_INFO_PAR mvfip;
    mvfip.iframe = i;


    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_FRAME_INFO, &mvfip) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_FRAME_INFO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    memcpy(pVideoFrameInfo, &(mvfip.info), sizeof(MWCAP_VIDEO_FRAME_INFO));
    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoCaptureStatus(int hChannel, MWCAP_VIDEO_CAPTURE_STATUS *pStatus)
{
    if (hChannel == -1 || pStatus == NULL)
    {
       //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CAPTURE_STATUS, pStatus) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CAPTURE_STATUS error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// Audio Capture
MW_RESULT MWStartAudioCapture(int hChannel)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_AUDIO_CAPTURE_OPEN) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_AUDIO_CAPTURE_OPEN error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWStopAudioCapture(int hChannel)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_AUDIO_CAPTURE_CLOSE) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_AUDIO_CAPTURE_CLOSE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWCaptureAudioFrame(int hChannel, MWCAP_AUDIO_CAPTURE_FRAME *pAudioCaptureFrame)
{
    if (hChannel == -1 || pAudioCaptureFrame == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_AUDIO_CAPTURE_FRAME, pAudioCaptureFrame) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_AUDIO_CAPTURE_FRAME error: %m\n", __FUNCTION__);
	if(errno == ENODATA)
	     return MW_ENODATA;
	else     
	     return MW_FAILED;
    }

    if (pAudioCaptureFrame->dwSyncCode != MWCAP_AUDIO_FRAME_SYNC_CODE)
    {
        //fprintf(stderr, "Function: %s, Audio capture syn code error!\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// Video Process
MW_RESULT MWSetVideoInputAspectRatio(int hChannel, int nAspectX, int nAspectY)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_ASPECT_RATIO mvar;
    mvar.nAspectX = nAspectX;
    mvar.nAspectY = nAspectY;

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_INPUT_ASPECT_RATIO, &mvar) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_INPUT_ASPECT_RATIO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }
    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoInputAspectRatio(int hChannel, int *pnAspectX, int *pnAspectY)
{
    if (hChannel == -1 || NULL == pnAspectX || NULL == pnAspectY)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_ASPECT_RATIO mvar;
    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_INPUT_ASPECT_RATIO, &mvar) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_INPUT_ASPECT_RATIO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pnAspectX = mvar.nAspectX;
    *pnAspectY = mvar.nAspectY;

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoInputColorFormat(int hChannel, MWCAP_VIDEO_COLOR_FORMAT colorFormat)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_INPUT_COLOR_FORMAT, &colorFormat) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_INPUT_COLOR_FORMAT error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoInputColorFormat(int hChannel, MWCAP_VIDEO_COLOR_FORMAT *pColorFormat)
{
    if (hChannel == -1 || NULL == pColorFormat)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_INPUT_COLOR_FORMAT, pColorFormat) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_INPUT_COLOR_FORMAT error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoInputQuantizationRange(int hChannel, MWCAP_VIDEO_QUANTIZATION_RANGE quantRange)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_INPUT_QUANTIZATION_RANGE, &quantRange) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_INPUT_QUANTIZATION_RANGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoInputQuantizationRange(int hChannel, MWCAP_VIDEO_QUANTIZATION_RANGE *pQuantRange)
{
    if (hChannel == -1 || NULL == pQuantRange)
        {
            //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
            return MW_INVALID_PARAMS;
        }

        if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_INPUT_QUANTIZATION_RANGE, pQuantRange) != 0)
        {
            //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_INPUT_QUANTIZATION_RANGE error: %m\n", __FUNCTION__);
            return MW_FAILED;
        }

        return MW_SUCCEEDED;
}


// LED Mode
MW_RESULT MWSetLEDMode(int hChannel, unsigned int dwMode)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_LED_MODE, &dwMode) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_LED_MODE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// upgrade firmware
MW_RESULT MWGetFirmwareStorageInfo(int hChannel, MWCAP_FIRMWARE_STORAGE *pFirmwareStorageInfo)
{
    if (hChannel == -1 || pFirmwareStorageInfo == NULL)
    {
       //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_FIRMWARE_STORAGE, pFirmwareStorageInfo) != 0)
    {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_FIRMWARE_STORAGE error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWEraseFirmwareData(int hChannel, unsigned int cbOffset, unsigned int cbErase)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_FIRMWARE_ERASE mfe;
    mfe.cbOffset = cbOffset;
    mfe.cbErase = cbErase;

    if (ioctl(hChannel, MWCAP_IOCTL_SET_FIRMWARE_ERASE, &mfe) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_FIRMWARE_ERASE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWReadFirmwareData(int hChannel, unsigned int cbOffset, unsigned char *pbyData, unsigned int cbToRead, unsigned int *pcbRead)
{
    int ret;

    if (hChannel == -1 || pbyData == NULL || pcbRead == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_FIRMWARE_DATA mcd;
    mcd.offset = cbOffset;
    mcd.size = cbToRead;
    mcd.data = (MWCAP_PTR)(unsigned long)pbyData;


    if ((ret = ioctl(hChannel, MWCAP_IOCTL_GET_FIRMWARE_DATA, &mcd)) != 0  )
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_FIRMWARE_DATA error: %m ret=%d\n", __FUNCTION__, ret);
        return MW_FAILED;
    }

    *pcbRead = mcd.size;

    return MW_SUCCEEDED;
}

MW_RESULT MWWriteFirmwareData(int hChannel, unsigned int cbOffset, unsigned char *pbyData, unsigned int cbData)
{
    if (hChannel == -1 || pbyData == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    //wirte data
    MWCAP_FIRMWARE_DATA mfd_rd;
    mfd_rd.data = (MWCAP_PTR)(unsigned long)pbyData;
    mfd_rd.offset = cbOffset;
    mfd_rd.size = cbData;

    if (ioctl(hChannel, MWCAP_IOCTL_SET_FIRMWARE_DATA, &mfd_rd) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_FIRMWARE_DATA error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetPostReconfig(int hChannel, unsigned int dwDelayMS)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_POST_RECONFIG, &dwDelayMS) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_POST_RECONFIG error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// OSD
HOSD MWCreateImage(int hChannel, int cx, int cy)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return 0;
    }

    MWCAP_VIDEO_CREATE_IMAGE imagedata = {0};
    imagedata.cx = cx;
    imagedata.cy = cy;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CREATE_IMAGE, &imagedata) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CREATE_IMAGE error: %m\n", __FUNCTION__);
        return 0;
    }

    return imagedata.pvImage;
}

MW_RESULT MWOpenImage(int hChannel, MWCAP_PTR hImage, long *plRet)
{
    if (hChannel == -1 || hImage == 0 || plRet == NULL)
    {
        //fprintf(stderr, "Function  %s parameter error\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_IMAGE_REF imageref = {0};
    imageref.pvImage = hImage;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_OPEN_IMAGE, &imageref) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_OPEN_IMAGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    *plRet = imageref.nRefCount;

    return MW_SUCCEEDED;
}

MW_RESULT MWCloseImage(int hChannel, MWCAP_PTR hImage, long *plRet)
{
    if (hChannel == -1 || hImage == 0 || plRet == NULL)
    {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_IMAGE_REF imageref = {0};
    imageref.pvImage = hImage;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_CLOSE_IMAGE, &imageref) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_CLOSE_IMAGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }


    *plRet = imageref.nRefCount;

    return MW_SUCCEEDED;
}

MW_RESULT MWUploadImageFromVirtualAddress(int                                   hChannel,
                                          MWCAP_PTR                             hImage,
                                          MWCAP_VIDEO_COLOR_FORMAT              cfDest,
                                          MWCAP_VIDEO_QUANTIZATION_RANGE        quantRangeDest,
                                          MWCAP_VIDEO_SATURATION_RANGE          satRangeDest,
                                          unsigned short                        xDest,
                                          unsigned short                        yDest,
                                          unsigned short                        cxDest,
                                          unsigned short                        cyDest,
                                          MWCAP_PTR                             pvSrcFrame,
                                          unsigned int                          cbSrcFrame,
                                          unsigned int                          cbSrcStride,
                                          unsigned short                        cxSrc,
                                          unsigned short                        cySrc,
                                          BOOLEAN                               bSrcBottomUp,
                                          BOOLEAN                               bSrcPixelAlpha,
                                          BOOLEAN                               bSrcPixelXBGR
                                          )
{
    if (hChannel == -1) {
            //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
            return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_UPLOAD_IMAGE imageupload = {0};
    imageupload.pvDestImage		= hImage;

    imageupload.cfDest 			= cfDest;
    imageupload.quantRangeDest	= quantRangeDest;
    imageupload.satRangeDest	= satRangeDest;

    imageupload.xDest			= xDest;
    imageupload.yDest			= yDest;
    imageupload.cxDest			= cxDest;
    imageupload.cyDest			= cyDest;

    imageupload.bSrcPhysicalAddress = FALSE;
    imageupload.pvSrcFrame		= pvSrcFrame;
    imageupload.cbSrcFrame		= cbSrcFrame;
    imageupload.cbSrcStride		= cbSrcStride;

    imageupload.cxSrc			= cxSrc;
    imageupload.cySrc			= cySrc;

    imageupload.bSrcBottomUp	= bSrcBottomUp;
    imageupload.bSrcPixelAlpha	= bSrcPixelAlpha;
    imageupload.bSrcPixelXBGR	= bSrcPixelXBGR;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_UPLOAD_IMAGE, &imageupload) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_UPLOAD_IMAGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWUploadImageFromPhysicalAddress(int                              hChannel,
                                           MWCAP_PTR                        hImage,
                                           MWCAP_VIDEO_COLOR_FORMAT         cfDest,
                                           MWCAP_VIDEO_QUANTIZATION_RANGE   quantRangeDest,
                                           MWCAP_VIDEO_SATURATION_RANGE     satRangeDest,
                                           unsigned short                   xDest,
                                           unsigned short                   yDest,
                                           unsigned short                   cxDest,
                                           unsigned short                   cyDest,
                                           LARGE_INTEGER                    liSrcFrameAddress,
                                           unsigned int                     cbSrcFrame,
                                           unsigned int                     cbSrcStride,
                                           unsigned short                   cxSrc,
                                           unsigned short                   cySrc,
                                           BOOLEAN                          bSrcBottomUp,
                                           BOOLEAN                          bSrcPixelAlpha,
                                           BOOLEAN                          bSrcPixelXBGR
                                           )
{
    if (hChannel == -1) {
            //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
            return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_UPLOAD_IMAGE imageupload = {0};
    imageupload.pvDestImage		= hImage;

    imageupload.cfDest 			= cfDest;
    imageupload.quantRangeDest	= quantRangeDest;
    imageupload.satRangeDest	= satRangeDest;

    imageupload.xDest			= xDest;
    imageupload.yDest			= yDest;
    imageupload.cxDest			= cxDest;
    imageupload.cyDest			= cyDest;

    imageupload.bSrcPhysicalAddress = TRUE;
    imageupload.liSrcPhysicalAddress = liSrcFrameAddress;
    imageupload.cbSrcFrame		= cbSrcFrame;
    imageupload.cbSrcStride		= cbSrcStride;

    imageupload.cxSrc			= cxSrc;
    imageupload.cySrc			= cySrc;

    imageupload.bSrcBottomUp	= bSrcBottomUp;
    imageupload.bSrcPixelAlpha	= bSrcPixelAlpha;
    imageupload.bSrcPixelXBGR	= bSrcPixelXBGR;

    if (ioctl(hChannel, MWCAP_IOCTL_VIDEO_UPLOAD_IMAGE, &imageupload) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_VIDEO_UPLOAD_IMAGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

//Temperature
MW_RESULT MWGetTemperature(int hChannel, unsigned int *pnTemp)
{
    if (hChannel == -1 || NULL == pnTemp)
    {
       //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_CORE_TEMPERATURE, pnTemp) != 0) {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_CORE_TEMPERATURE error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// V4l2
MW_RESULT MWGetStreamCount(int hChannel, int *pnCount)
{
    if (hChannel == -1 || NULL == pnCount)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_STREAMS_COUNT, pnCount) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_STREAMS_COUNT error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWGetStreamInfos(int hChannel, MWCAP_STREAM_INFO *pStreamInfos, int *pnCount)
{
    if (hChannel == -1 || NULL == pStreamInfos || NULL == pnCount)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_STREAMS_INFO infos;
    infos.count = *pnCount;
    infos.infos = (MWCAP_PTR)(unsigned long)pStreamInfos;
    if (ioctl(hChannel, MWCAP_IOCTL_GET_STREAMS_INFO, &infos) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_STREAMS_INFO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pnCount = infos.count;
    return MW_SUCCEEDED;
}

MW_RESULT MWSetCtrlStreamID(int hChannel, int nCrtlID)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_CTRL_STREAM_ID, &nCrtlID) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_CTRL_STREAM_ID error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoConnectFormat(int hChannel, MWCAP_VIDEO_CONNECTION_FORMAT *pConnectFormat)
{
    if (hChannel == -1 || NULL == pConnectFormat)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_CONNECTION_FORMAT, pConnectFormat) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_CONNECTION_FORMAT error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWGetVideoProcessSettings(int hChannel, MWCAP_VIDEO_PROCESS_SETTINGS *pProcessSettings)
{
    if (hChannel == -1 || NULL == pProcessSettings)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_PROCESS_SETTINGS, pProcessSettings) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_PROCESS_SETTINGS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoProcessSettings(int hChannel, MWCAP_VIDEO_PROCESS_SETTINGS processSettings)
{
    if (hChannel == -1) {
       //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
   }

   if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_PROCESS_SETTINGS, &processSettings) != 0) {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_PROCESS_SETTINGS error: %m\n", __FUNCTION__);
       return MW_FAILED;
   }

   return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoOSDSettings(int hChannel, MWCAP_VIDEO_OSD_SETTINGS *pOSDSettings)
{
    if (hChannel == -1 || NULL == pOSDSettings)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_OSD_SETTINGS, pOSDSettings) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_OSD_SETTINGS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWSetVideoOSDSettings(int hChannel, MWCAP_VIDEO_OSD_SETTINGS OSDSettings)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_OSD_SETTINGS, &OSDSettings) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_OSD_SETTINGS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoOSDImage(int hChannel, MWCAP_VIDEO_OSD_IMAGE *pOSDImage)
{
    if (hChannel == -1 || NULL == pOSDImage)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_OSD_IMAGE, pOSDImage) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_OSD_IMAGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoOSDImage(int hChannel, MWCAP_VIDEO_OSD_IMAGE OSGImage)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_OSD_IMAGE, &OSGImage) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_OSD_IMAGE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoBrightness(int hChannel, int *pnBrightness)
{
    if (hChannel == -1 || NULL == pnBrightness)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_BRIGHTNESS, pnBrightness) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_BRIGHTNESS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoBrightness(int hChannel, int nBrightness)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_BRIGHTNESS, &nBrightness) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_BRIGHTNESS error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoContrast(int hChannel, int *pnContrast)
{
    if (hChannel == -1 || NULL == pnContrast)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_CONTRAST, pnContrast) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_CONTRAST error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoContrast(int hChannel, int nContrast)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_CONTRAST, &nContrast) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_CONTRAST error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoHue(int hChannel, int *pnHue)
{
    if (hChannel == -1 || NULL == pnHue)
    {
       //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
       return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_HUE, pnHue) != 0) {
       //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_HUE error: %m\n", __FUNCTION__);
       return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoHue(int hChannel, int nHue)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_HUE, &nHue) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_HUE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoSaturation(int hChannel, int *pnSaturation)
{
    if (hChannel == -1 || NULL == pnSaturation)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_SATURATION, pnSaturation) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_SATURATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoSaturation(int hChannel, int nSaturation)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_SATURATION, &nSaturation) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_SATURATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSaveSettingsAsPreset(int hChannel)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SETTINGS_SAVE_AS_PRESET) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_SATURATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWReloadPreset(int hChannel)
{
    if (hChannel == -1) {
        //fprintf(stderr, "Function  %s parameter \"hChanne\" can't be -1\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SETTINGS_RELOAD_PRESET) != 0) {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_SATURATION error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


// VGA/Component timings
MW_RESULT MWGetVideoAutoHAlign(int hChannel, BOOLEAN *pbAuto)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_AUTO_H_ALIGN, pbAuto) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_AUTO_H_ALIGN error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoAutoHAlign(int hChannel, BOOLEAN bAuto)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_AUTO_H_ALIGN, &bAuto) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_AUTO_H_ALIGN error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoSamplingPhase(int hChannel, unsigned char *pbyValue)
{
    if (hChannel == -1 || pbyValue == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_SAMPLING_PHASE, pbyValue) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_SAMPLING_PHASE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoSamplingPhase(int hChannel, unsigned char byValue)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_SAMPLING_PHASE, &byValue) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_SAMPLING_PHASE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoSamplingPhaseAutoAdjust(int hChannel, BOOLEAN *pbAuto)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_SAMPLING_PHASE_AUTO, pbAuto) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_SAMPLING_PHASE_AUTO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetVideoSamplingPhaseAutoAdjust(int hChannel, BOOLEAN bAuto)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_SAMPLING_PHASE_AUTO, &bAuto) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_SAMPLING_PHASE_AUTO error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}


MW_RESULT MWSetVideoTiming(int hChannel, MWCAP_VIDEO_TIMING videoTiming)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_TIMING, &videoTiming) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_TIMING error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetVideoPreferredTimingArray(int hChannel, MWCAP_VIDEO_TIMING *pVideoTiming, long *plSize)
{
    if (hChannel == -1 || pVideoTiming == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_TIMING_PAR videoTimingPar;
    videoTimingPar.timings = (MWCAP_PTR)(unsigned long)pVideoTiming;
    videoTimingPar.count = *plSize;

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_PREFERRED_TIMING_ARRAY, &videoTimingPar) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_PREFERRED_TIMING_ARRAY error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }
    *plSize = videoTimingPar.count;

    return MW_SUCCEEDED;
}

MW_RESULT MWSetCustomVideoTiming(int hChannel, MWCAP_VIDEO_CUSTOM_TIMING videoTiming)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_CUSTOM_TIMING, &videoTiming) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_CUSTOM_TIMING error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetCustomVideoTimingsCount(int hChannel, unsigned int *pdwCount)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_CUSTOM_TIMING_COUNT, pdwCount) != 0)
    {
        //fprintf(stderr, "FUnction: %s, ioctl MWCAP_IOCTL_GET_VIDEO_CUSTOM_TIMING_COUNT error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetCustomVideoTimingsArray(int hChannel, MWCAP_VIDEO_CUSTOM_TIMING *pVideoCustomTiming, unsigned int *pdwCount)
{
    if (hChannel == -1 || pVideoCustomTiming == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CUSTOM_TIMING_PAR videoCustomTimingPar;
    videoCustomTimingPar.count = *pdwCount;
    videoCustomTimingPar.timings = (MWCAP_PTR)(unsigned long)pVideoCustomTiming;

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_CUSTOM_TIMING_ARRAY, &videoCustomTimingPar) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_CUSTOM_TIMING_ARRAY error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pdwCount = videoCustomTimingPar.count;

    return MW_SUCCEEDED;
}

MW_RESULT MWSetCustomVideoTimingsArray(int hChannel, MWCAP_VIDEO_CUSTOM_TIMING *pVideoCustomTiming, unsigned int dwCount)
{
    if (hChannel == -1 || pVideoCustomTiming == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CUSTOM_TIMING_PAR videoCustomTimingPar;
    videoCustomTimingPar.count = dwCount;
    videoCustomTimingPar.timings = (MWCAP_PTR)(unsigned long)pVideoCustomTiming;

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_CUSTOM_TIMING_ARRAY, &videoCustomTimingPar) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_CUSTOM_TIMING_ARRAY error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetCustomVideoResolutionsCount(int hChannel, unsigned int *pdwCount)
{
    if (hChannel == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_CUSTOM_RESOLUTION_COUNT, pdwCount) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_GET_VIDEO_CUSTOM_RESOLUTION_COUNT error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWGetCustomVideoResolutionsArray(int hChannel, MWCAP_SIZE *pResolutionSize, unsigned int *pdwCount)
{
    if (hChannel == -1 || pResolutionSize == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CUSTOM_RESOLUTION_PAR videoCustomResolutionPar;
    videoCustomResolutionPar.resolutions = (MWCAP_PTR)(unsigned long)pResolutionSize;
    videoCustomResolutionPar.count = *pdwCount;

    if (ioctl(hChannel, MWCAP_IOCTL_GET_VIDEO_CUSTOM_RESOLUTION_ARRAY, &videoCustomResolutionPar) != 0)
    {
        //fprintf(stderr, "FUnction: %s, ioctl MWCAP_IOCTL_GET_VIDEO_CUSTOM_RESOLUTION_ARRAY error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    *pdwCount = videoCustomResolutionPar.count;

    return MW_SUCCEEDED;
}

MW_RESULT MWSetCustomVideoResolutionsArray(int hChannel, MWCAP_SIZE *pResolutionSize, unsigned int dwCount)
{
    if (hChannel == -1 || pResolutionSize == NULL)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    MWCAP_VIDEO_CUSTOM_RESOLUTION_PAR videoCustomResolutionPar;
    videoCustomResolutionPar.resolutions = (MWCAP_PTR)(unsigned long)pResolutionSize;
    videoCustomResolutionPar.count = dwCount;

    if (ioctl(hChannel, MWCAP_IOCTL_SET_VIDEO_CUSTOM_RESOLUTION_ARRAY, &videoCustomResolutionPar) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MWCAP_IOCTL_SET_VIDEO_CUSTOM_RESOLUTION_ARRAY error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

// Event
MWCAP_PTR MWCreateEvent()
{
    MWCAP_PTR hEvent;
    if (g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(g_hEventFD, MW_IOCTL_KEVENT_ALLOC, &hEvent) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MW_IOCTL_KEVENT_ALLOC error: %m\n", __FUNCTION__);
        return 0;
    }

    return hEvent;
}


MW_RESULT MWCloseEvent(MWCAP_PTR hEvent)
{
    if (hEvent == 0 || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(g_hEventFD, MW_IOCTL_KEVENT_FREE, &hEvent) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MW_IOCTL_KEVENT_FREE error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWSetEvent(MWCAP_PTR hEvent)
{
    if (hEvent == 0 || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(g_hEventFD, MW_IOCTL_KEVENT_SET, &hEvent) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MW_IOCTL_KEVENT_SET error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

MW_RESULT MWClearEvent(MWCAP_PTR hEvent)
{
    if (hEvent == 0 || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return MW_INVALID_PARAMS;
    }

    if (ioctl(g_hEventFD, MW_IOCTL_KEVENT_CLEAR, &hEvent) != 0)
    {
        //fprintf(stderr, "Function: %s, ioctl MW_IOCTL_KEVENT_CLEAR error: %m\n", __FUNCTION__);
        return MW_FAILED;
    }

    return MW_SUCCEEDED;
}

BOOLEAN MWIsSetEvent(MWCAP_PTR hEvent)
{
    if (hEvent == 0 || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return FALSE;
    }

    if (ioctl(g_hEventFD, MW_IOCTL_KEVENT_IS_SET, &hEvent) == 0)
        return FALSE;
    else
        return TRUE;

}

int MWTryWaitEvent(MWCAP_PTR hEvent)
{
    if (hEvent == 0 || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return FALSE;
    }

    return ioctl(g_hEventFD, MW_IOCTL_KEVENT_TRY_WAIT, &hEvent);
}

int MWWaitEvent(MWCAP_PTR hEvent, int nTimeout)
{
    if (hEvent == 0 || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return FALSE;
    }

    MW_EVENT_WAIT eventWait;
    eventWait.pvEvent = hEvent;
    eventWait.timeout = nTimeout;

    return ioctl(g_hEventFD, MW_IOCTL_KEVENT_WAIT, &eventWait);
}

DWORD MWMultiWaitEvent(MWCAP_PTR *hEvents, int nCount, int nTimeout)
{
    if (hEvents == NULL || g_hEventFD == -1)
    {
        //fprintf(stderr, "Function  %s parameter Error!\n", __FUNCTION__);
        return FALSE;
    }

    int nRet;
    unsigned int dwReturnValue = 0;

    MW_EVENT_WAIT_MULTI multiWait;
    multiWait.pvEvents = (MWCAP_PTR)(unsigned long)hEvents;
    multiWait.count = nCount;
    multiWait.timeout = nTimeout;

    nRet = ioctl(g_hEventFD, MW_IOCTL_KEVENT_WAIT_MULTI, &multiWait);

    if (nRet <= 0)
        return 0;

    int i;
    for(i = 0; i < nCount; i++)
    {
        if(MWTryWaitEvent(hEvents[i]) == 1)
            dwReturnValue |= (0x01 << i);
    }

    return dwReturnValue;
}





