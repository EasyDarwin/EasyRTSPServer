#pragma once

#include "libRTSPServerAPI.h"
#include "BaseList.h"

class RtspClientList : public BaseList
{
public:
	RtspClientList(void);
	~RtspClientList(void);


	//int	Add(SIP_DEVICE_INFO_CONTEXT *pSipDeviceInfoContext);
	int Update(RTSP_CLIENT_INFO_T *pClient);
	int Del(char *cameraIndexCode);
	int Find(char *cameraIndexCode, RTSP_CLIENT_INFO_T **pClient);

	int DelNode(void *userPtr);


protected:
};

