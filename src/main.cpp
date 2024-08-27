#if 1

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "libRTSPServerAPI.h"
#include "RtspTypes.h"

#ifdef _DEBUG
#include <vld.h>
#include "BasicHashTable.hh"
#endif
#include "trace.h"

typedef enum __SOURCE_TYPE_ENUM_T
{
	SOURCE_TYPE_FILE		=	0x01,
	SOURCE_TYPE_RTSP
}SOURCE_TYPE_ENUM_T;


typedef struct __LIVE_CHANNEL_INFO_T
{
	int		id;
	char	name[64];
}LIVE_CHANNEL_INFO_T;

typedef struct __RTSP_SOURCE_CHANNEL_T
{
	int			id;
	char		name[36];
	int			pushStream;

	RTSP_MEDIA_INFO_T	mediaInfo;
	void			*rtspHandle;

	SOURCE_TYPE_ENUM_T		sourceType;
	char	source_uri[128];
	char	username[16];
	char	password[16];
}RTSP_SOURCE_CHANNEL_T;


int _libRTSPSvr_Callback(RTSP_SERVER_STATE_T serverStatus, const char *resourceName, RTSP_CHANNEL_HANDLE *channelHandle, RTSP_MEDIA_INFO_T *mediaInfo, void *userPtr, void *channelPtr)
//int __libRTSPSvr_Callback(RTSP_SERVER_STATE_T serverStatus, const char *resourceName, RTSP_CHANNEL_HANDLE *channelHandle, RTSP_MEDIA_INFO_T *mediaInfo, void *userPtr, void *channelPtr)
{

	return 0;
}

//输入的pbuf必须包含start code(00 00 00 01)
int GetH265VPSandSPSandPPS(char *pbuf, int bufsize, char *_vps, int *_vpslen, char *_sps, int *_spslen, char *_pps, int *_ppslen)
{
	char vps[512]={0}, sps[512] = {0}, pps[128] = {0};
	int vpslen=0, spslen=0, ppslen=0, i=0, iStartPos=0, ret=-1;
	int iFoundVPS=0, iFoundSPS=0, iFoundPPS=0, iFoundSEI=0;
	if (NULL == pbuf || bufsize<4)	return -1;

	for (i=0; i<bufsize; i++)
	{
		if ( (unsigned char)pbuf[i] == 0x00 && (unsigned char)pbuf[i+1] == 0x00 && 
			 (unsigned char)pbuf[i+2] == 0x00 && (unsigned char)pbuf[i+3] == 0x01 )
		{
			printf("0x%X\n", (unsigned char)pbuf[i+4]);
			switch ((unsigned char)pbuf[i+4])
			{
			case 0x40:		//VPS
				{
					iFoundVPS = 1;
					iStartPos = i+4;
				}
				break;
			case 0x42:		//SPS
				{
					if (iFoundVPS == 0x01 && i>4)
					{
						vpslen = i-4-iStartPos;
						if (vpslen>256)	return -1;          //vps长度超出范围
						memset(vps, 0x00, sizeof(vps));
						memcpy(vps, pbuf+iStartPos, vpslen);
					}

					iStartPos = i+4;
					iFoundSPS = 1;
				}
				break;
			case 0x44:		//PPS
				{
					if (iFoundSPS == 0x01 && i>4)
					{
						spslen = i-4-iStartPos;
						if (spslen>256)	return -1;
						memset(sps, 0x0, sizeof(sps));
						memcpy(sps, pbuf+iStartPos,  spslen);
					}

					iStartPos = i+4;
					iFoundPPS = 1;
				}
				break;
			case 0x4E:		//SEI
			case 0x50:
				{
					if (iFoundPPS == 0x01 && i>4)
					{
						ppslen = i-4-iStartPos;
						if (ppslen>256)	return -1;
						memset(pps, 0x0, sizeof(pps));
						memcpy(pps, pbuf+iStartPos,  ppslen);
					}
					iStartPos = i+4;
					iFoundSEI = 1;
				}
				break;
			default:
				break;
			}
		}

		if (iFoundSEI == 0x01)		break;
	}


	if (iFoundVPS == 0x01)
    {
        if (vpslen < 1)
        {
            if (bufsize < sizeof(vps))
            {
                vpslen = bufsize-4;
                memset(vps, 0x00, sizeof(vps));
                memcpy(vps, pbuf+4, vpslen);
            }
        }

        if (vpslen > 0)
        {
            if (NULL != _vps)   memcpy(_vps, vps, vpslen);
            if (NULL != _vpslen)    *_vpslen = vpslen;
        }

        ret = 0;
    }

	
	if (iFoundSPS == 0x01)
    {
        if (spslen < 1)
        {
            if (bufsize < sizeof(sps))
            {
                spslen = bufsize-4;
                memset(sps, 0x00, sizeof(sps));
                memcpy(sps, pbuf+4, spslen);
            }
        }

        if (spslen > 0)
        {
            if (NULL != _sps)   memcpy(_sps, sps, spslen);
            if (NULL != _spslen)    *_spslen = spslen;
        }

        ret = 0;
    }

    if (iFoundPPS == 0x01)
    {
        if (ppslen < 1)
        {
            if (bufsize < sizeof(pps))
            {
                ppslen = bufsize-4;
                memset(pps, 0x00, sizeof(pps));
				memcpy(pps, pbuf+4, ppslen);	//pps
            }
        }
        if (ppslen > 0)
        {
            if (NULL != _pps)   memcpy(_pps, pps, ppslen);
            if (NULL != _ppslen)    *_ppslen = ppslen;
        }
        ret = 0;
    }

    return ret;
}


//int GetVPSandSPSandPPS(char *pbuf, int bufsize, char *_vps, int *_vpslen, char *_sps, int *_spslen, char *_pps, int *_ppslen)
int GetH264SPSandPPS(char *pbuf, int bufsize, char *_sps, int *_spslen, char *_pps, int *_ppslen)
{
    char sps[512] = {0,}, pps[128] = {0,};
	int spslen=0, ppslen=0, i=0, iStartPos=0, ret=-1;
	int iFoundSPS=0, iFoundPPS=0, iFoundIDR=0;
	if (NULL == pbuf || bufsize<4)	return -1;

	for (i=0; i<bufsize; i++)
	{
		if ( (unsigned char)pbuf[i] == 0x00 && (unsigned char)pbuf[i+1] == 0x00 && 
			 (unsigned char)pbuf[i+2] == 0x00 && (unsigned char)pbuf[i+3] == 0x01 )
		{
			unsigned char naltype = ((unsigned char)pbuf[i+4] & 0x1F);
			if (naltype == 7)       //sps
			{
			    iFoundSPS = 1;
                iStartPos = i+4;
			}
			else if (naltype == 8)	//pps
			{
                //copy sps
                if (iFoundSPS == 0x01 && i>4)
                {
                    spslen = i-4;
                    if (spslen>256)	return -1;          //sps长度超出范围
                    memset(sps, 0x00, sizeof(sps));
                    memcpy(sps, pbuf+4, spslen);
                }

                iFoundPPS = 1;
			}
			else if (naltype == 5)	//idr
			{
                iFoundIDR = 1;

				ppslen = i-spslen-4-4;
				if (ppslen>0 && ppslen+ppslen<sizeof(pps))
				{
                    memset(pps, 0x00, sizeof(pps));
					memcpy(pps, pbuf+spslen+8, ppslen);	//pps
				}
                break;
			}
		}
		else
		{

		}
	}


    if (iFoundSPS == 0x01)
    {
        if (spslen < 1)
        {
            if (bufsize < sizeof(sps))
            {
                spslen = bufsize-4;
                memset(sps, 0x00, sizeof(sps));
                memcpy(sps, pbuf+4, spslen);
            }
        }

        if (spslen > 0)
        {
            if (NULL != _sps)   memcpy(_sps, sps, spslen);
            if (NULL != _spslen)    *_spslen = spslen;
        }

        ret = 0;
    }

    if (iFoundPPS == 0x01)
    {
        if (ppslen < 1)
        {
            if (bufsize < sizeof(pps))
            {
                ppslen = bufsize-4;
                memset(pps, 0x00, sizeof(pps));
				memcpy(pps, pbuf+4, ppslen);	//pps
            }
        }
        if (ppslen > 0)
        {
            if (NULL != _pps)   memcpy(_pps, pps, ppslen);
            if (NULL != _ppslen)    *_ppslen = ppslen;
        }
        ret = 0;
    }

    return ret;
}

int get_aac_data(unsigned char **framedata, int *framesize)
{
	char *filename = "C:\\test\\rawdata.adts";

	static int iFlag = 0x00;
	static int offset = 0;
	static unsigned char *aac_framedata = NULL;
	static FILE *f = NULL;


	if (iFlag > 0)	return -1;

	if (NULL==f)	f = fopen(filename, "rb");

	if (NULL == aac_framedata)	aac_framedata = (unsigned char *)malloc(1024*8);

	memset(aac_framedata, 0x00, 1024*8);
	while (! feof(f) )
	{
		int readbytes = 0;
		readbytes = fread(aac_framedata+offset, 1, 1, f);
		if (offset > 2 && aac_framedata[offset]==0xF1 && aac_framedata[offset-1] == 0xFF)
		{
			FILE *ff = fopen("1_org.txt", "wb");
			if (NULL != ff)
			{
				fwrite(aac_framedata, 1, offset-1, ff);
				fclose(ff);
			}

			if (NULL != framesize)		*framesize = offset-1;
			if (NULL != framedata)		*framedata = aac_framedata;

			offset = 0;
			aac_framedata[offset++] = 0xFF;
			aac_framedata[offset++] = 0xF1;
			break;
		}
		offset ++;

	}
	if (feof(f) )
	{
		//fclose(f);
		//free(aac_framedata);
		//aac_framedata = NULL;
		//iFlag ++;

		fseek(f, 0, SEEK_SET);

		return -1;
	}
	return 0;
}


//rtspserver Callback
int __libRTSPSvr_Callback(int channelId, RTSP_SERVER_STATE_T channelState, RTSP_MEDIA_INFO_T *_mediaInfo, void *userPtr)
{
	RTSP_SOURCE_CHANNEL_T	*pChannel = (RTSP_SOURCE_CHANNEL_T *)userPtr;

	if (channelId < 0)		return -1;

	if (channelState == RTSP_CHANNEL_OPEN_STREAM)
	{
		if (pChannel[channelId].sourceType == SOURCE_TYPE_FILE)		//文件
		{
		}
		else if (pChannel[channelId].sourceType == SOURCE_TYPE_RTSP)					//摄像机
		{
			if (NULL != pChannel[channelId].rtspHandle)
			{
				//EasyRTSP_CloseStream(pChannel[channelId].rtspHandle);
				//EasyRTSP_Deinit(&pChannel[channelId].rtspHandle);
				pChannel[channelId].rtspHandle = NULL;
			}}

		_TRACE(TRACE_LOG_INFO, (char*)"[channel %d] Get media info...\n", channelId);
		for (int i=0; i<6; i++)
		{
			//注:   此处视情况而定，判断是否需有音频
			Sleep(500);
		}
	}
	else if (channelState == RTSP_CHANNEL_START_STREAM)
	{
		_TRACE(TRACE_LOG_INFO, "PlayStream...\n");

		pChannel[channelId].pushStream = 0x01;
		if (pChannel[channelId].sourceType == SOURCE_TYPE_FILE)
		{
		}
		else if (pChannel[channelId].sourceType == SOURCE_TYPE_RTSP)
		{
		}
	}
	else if (channelState == RTSP_CHANNEL_STOP_STREAM)
	{
		_TRACE(TRACE_LOG_INFO, "StopStream...channel[%d]\n", channelId);

		if (pChannel[channelId].sourceType == SOURCE_TYPE_FILE)
		{
		}
		else if (pChannel[channelId].sourceType == SOURCE_TYPE_RTSP)
		{
			pChannel[channelId].pushStream = 0x00;
			//EasyRTSP_CloseStream(pChannel[channelId].rtspHandle);
			//EasyRTSP_Deinit(&pChannel[channelId].rtspHandle);
			pChannel[channelId].rtspHandle = NULL;
		}

		memset(&pChannel[channelId].mediaInfo, 0x00, sizeof(RTSP_MEDIA_INFO_T));
	}

	return 0;
}


void write_file(char *filename, char *pbuf, int bufsize)
{
	FILE *f = fopen(filename, "wb");
	if (NULL == f)		return;

	fwrite(pbuf, 1, bufsize, f);
	fclose(f);
}
int __parseH265()
{
	FILE *f = fopen("C:\\test\\1.h265", "rb");
	if (NULL == f)		return -1;

	fseek(f, 0, SEEK_END);
	long lSize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *pbuf = new char[lSize+1];
	memset(pbuf, 0x00, lSize+1);
	fread(pbuf, 1, lSize, f );
	fclose(f);


	char vps[512]={0}, sps[512]={0}, pps[512]={0};
	int vpslen=0, spslen=0, ppslen=0;

	GetH265VPSandSPSandPPS(pbuf, lSize, vps, &vpslen, sps, &spslen, pps, &ppslen);

	if (vpslen > 0)		write_file("vps.txt", vps, vpslen);
	if (spslen > 0)		write_file("sps.txt", sps, spslen);
	if (ppslen > 0)		write_file("pps.txt", pps, ppslen);

	
	return 0;
}




int main_(int argc, char *argv[])
{
	RTSP_SVR_HANDLE		rtspSvrHandle = NULL;
	libRTSPSvr_Create(&rtspSvrHandle);

	for (int i=0; i<1; i++)
	{

		//初始化各通道信息
	#define	MAX_CHANNEL_NUM			4
		RTSP_SOURCE_CHANNEL_T	channel[MAX_CHANNEL_NUM];
		memset(&channel[0], 0x00, sizeof(RTSP_SOURCE_CHANNEL_T) * MAX_CHANNEL_NUM);
		for (int i=0; i<MAX_CHANNEL_NUM; i++)
		{
			channel[i].id = i;
			sprintf(channel[i].name, "channel=%d", i+1);

			if (i==0)
			{
#if 0
				channel[i].sourceType = SOURCE_TYPE_FILE;
				strcpy(channel[i].source_uri, "C:\\20160519 155015.mpg");
#else
				channel[i].sourceType = SOURCE_TYPE_RTSP;

				FILE *f = fopen("url.txt", "rb");
				if (NULL != f)
				{
					fseek(f, 0, SEEK_END);
					long lSize = ftell(f);
					fread(channel[i].source_uri, 1, lSize, f);
					fclose(f);

					printf("rtsp://ip:8554/%s\n", channel[i].name);
				}
				else
				{
					strcpy(channel[i].source_uri, "rtsp://192.168.66.222/11");
				}
				strcpy(channel[i].username, "admin");
				strcpy(channel[i].password, "admin");
#endif
			}
			else if (i==1)
			{
				channel[i].sourceType = SOURCE_TYPE_RTSP;
				strcpy(channel[i].source_uri, "rtsp://190.168.7.186:554/Streaming/channels/101");
				strcpy(channel[i].username, "admin");
				strcpy(channel[i].password, "admin");
			}
			else if (i==2)
			{
				channel[i].sourceType = SOURCE_TYPE_RTSP;
				strcpy(channel[i].source_uri, "rtsp://192.168.10.29/ch0_unicast_firststream");
				strcpy(channel[i].username, "admin");
				strcpy(channel[i].password, "12345");
			}
			else if (i==3)
			{
				channel[i].sourceType = SOURCE_TYPE_RTSP;
				strcpy(channel[i].source_uri, "rtsp://190.168.7.177:8557");
				strcpy(channel[i].username, "admin");
				strcpy(channel[i].password, "12345");
			}
		}


		LIVE_CHANNEL_INFO_T	liveChannel[MAX_CHANNEL_NUM];
		memset(&liveChannel[0], 0x00, sizeof(LIVE_CHANNEL_INFO_T)*MAX_CHANNEL_NUM);
		for (int i=0; i<MAX_CHANNEL_NUM; i++)
		{
			liveChannel[i].id = channel[i].id;
			strcpy(liveChannel[i].name, channel[i].name);
		}

		libRTSPSvr_Startup(rtspSvrHandle, 8554,  "rtspserver", AUTHENTICATION_TYPE_NONE,(char*)"admin", (char*)"12345", (RTSPSvrCallBack)_libRTSPSvr_Callback, (void *)&channel[0], 0);
		//libRTSPSvr_AddUser((unsigned char*)"abc", (unsigned char*)"123");

		getchar();

	
		//先关闭源
		for (int i=0; i<MAX_CHANNEL_NUM; i++)
		{
			if (NULL != channel[i].rtspHandle)
			{
				//EasyRTSP_CloseStream(channel[i].rtspHandle);
				//EasyRTSP_Deinit(&channel[i].rtspHandle);
				channel[i].rtspHandle = NULL;
			}
		}

		//再关闭rtsp server
		libRTSPSvr_Shutdown(rtspSvrHandle);
		libRTSPSvr_Release(&rtspSvrHandle);

		getchar();
	}


	return 0;
}



int main()
{
#if 0
	typedef struct __ClientSession
	{
		int			id;
		char		name[16];
	}ClientSession;
	{
  u_int32_t sessionId;
  char sessionIdStr[16] = {0};
  for (int i=0; i<5; i++)
  {
		static u_int32_t clientSessionId = 0x7FFFFFFF-1;//100;
		if (clientSessionId < 0x7FFFFFFF)
		{
			sessionId = (clientSessionId++);
		}
		else
		{
			clientSessionId = 100;
			sessionId = (clientSessionId++);
		}

		_snprintf(sessionIdStr, sizeof sessionIdStr, "%08X", sessionId);
		printf("sessionIdStr: %s\n", sessionIdStr);
  }
}

	HashTable* fClientSessions; // 

	fClientSessions = HashTable::create(STRING_HASH_KEYS);
	//===============================================================
	char sessionIdStr[128] = {0};
	for (int i=0; i<5; i++)
	{
		sprintf(sessionIdStr, "%d", i+1);
		ClientSession	 *pClientSession = new ClientSession();
		memset(pClientSession, 0x00, sizeof(ClientSession));
		pClientSession->id = i+1;
		sprintf(pClientSession->name, "user%d", i+1);
		fClientSessions->Add(sessionIdStr, pClientSession);
	}

	//===============================================================
  HashTable::Iterator* iter = HashTable::Iterator::create(*fClientSessions);
  ClientSession* clientSession;
  char const* key; // dummy
  while ((clientSession = (ClientSession*)(iter->next(key))) != NULL) {

	  if (clientSession->id == 2)
	  {
		  char id[128] = {0};
		  sprintf(id, "%d", clientSession->id);
		  fClientSessions->Remove((const char *)key);
		  delete clientSession;
	  }
  }
  delete iter;

  //===============================================================
  {
	char sessionIdStr[128] = {0};
	for (int i=10; i<15; i++)
	{
		sprintf(sessionIdStr, "%d", i+1);
		ClientSession	 *pClientSession = new ClientSession();
		memset(pClientSession, 0x00, sizeof(ClientSession));
		pClientSession->id = i+1;
		sprintf(pClientSession->name, "user%d", i+1);
		fClientSessions->Add(sessionIdStr, pClientSession);
	}

  }
  //===============================================================
  {
	  HashTable::Iterator* iter = HashTable::Iterator::create(*fClientSessions);
	  ClientSession* clientSession;
	  char const* key; // dummy
	  while ((clientSession = (ClientSession*)(iter->next(key))) != NULL) {

		  printf("Id: %d   name: %s\n", clientSession->id, clientSession->name);
		  delete clientSession;

	  }
	  delete iter;

	  delete fClientSessions;
  }
#endif

	return 0;
}

#endif