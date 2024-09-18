#include "getsps.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>



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
			//printf("0x%X\n", (unsigned char)pbuf[i+4]);
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
						vpslen = i-4-iStartPos+4;
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
						spslen = i-4-iStartPos+4;
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
						ppslen = i-4-iStartPos+4;
						if (ppslen>256)	return -1;
						memset(pps, 0x0, sizeof(pps));
						memcpy(pps, pbuf+iStartPos,  ppslen);
					}
					iStartPos = i+4;
					iFoundSEI = 1;
				}
				break;
			default:
				{
					iStartPos = i+4;
				}
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



int GetH264SPSandPPS(char* pbuf, int bufsize, char* _sps, int* _spslen, char* _pps, int* _ppslen, char* _sei, int* _seilen)
{
	char sps[512] = { 0 }, pps[128] = { 0 }, sei[128] = { 0 };
	int spslen = 0, ppslen = 0, seilen = 0, i = 0, iStartPos = 0, ret = -1;
	int iFoundSPS = 0, iFoundPPS = 0, iFoundSEI = 0, iFoundIDR = 0;
	if (NULL == pbuf || bufsize < 4)	return -1;


	int startCodeOffset = -1;
	int offsetLen = 0;
	for (i = 0; i < bufsize; i++)
	{

		if ((unsigned char)pbuf[i] == 0x00 && (unsigned char)pbuf[i + 1] == 0x00 &&
			(unsigned char)pbuf[i + 2] == 0x00 && (unsigned char)pbuf[i + 3] == 0x01)
		{
			startCodeOffset = 4;
		}
		else if ((unsigned char)pbuf[i] == 0x00 && (unsigned char)pbuf[i + 1] == 0x00 &&
			(unsigned char)pbuf[i + 2] == 0x01)
		{
			startCodeOffset = 3;
		}

		if (startCodeOffset >= 0)
		{
			unsigned char naltype = ((unsigned char)pbuf[i + startCodeOffset] & 0x1F);
			if (naltype == 7)       //sps
			{
				iFoundSPS = 1;
				iStartPos = i + startCodeOffset;

				offsetLen += startCodeOffset;
				i += 1;
			}
			else if (naltype == 8)	//pps
			{
				//copy sps
				if (iFoundSPS == 0x01 && i > 4)
				{
					spslen = i - startCodeOffset;
					if (spslen > 256)	return -1;          //sps3¤?è3?3?·??§
					memset(sps, 0x00, sizeof(sps));
					memcpy(sps, pbuf + startCodeOffset, spslen);
				}

				iFoundPPS = 1;
				offsetLen += startCodeOffset;
				i += 1;
			}
			else if (naltype == 6)		//sei
			{
				if (iFoundPPS == 0x01 && i > 4)
				{
					ppslen = i - spslen - offsetLen;
					if (ppslen > 0 && ppslen < sizeof(pps))
					{
						memset(pps, 0x00, sizeof(pps));
						memcpy(pps, pbuf + spslen + offsetLen, ppslen);	//pps
					}
				}
				iFoundSEI = 1;
				offsetLen += startCodeOffset;
				i += 1;
			}
			else if (naltype == 5)	//idr
			{
				iFoundIDR = 1;

				if (iFoundPPS == 1 && iFoundSEI == 0)
				{
					ppslen = i - spslen - offsetLen;
					if (ppslen > 0 && ppslen < sizeof(pps))
					{
						memset(pps, 0x00, sizeof(pps));
						memcpy(pps, pbuf + spslen + offsetLen, ppslen);	//pps
					}
				}
				else if (iFoundSEI==1)
				{
					seilen = i - spslen - offsetLen - ppslen;
					if (seilen > 0 && seilen < sizeof(sei))
					{
						memset(sei, 0x00, sizeof(sei));
						memcpy(sei, pbuf + spslen + offsetLen + ppslen, seilen);	//sei
					}
					else
					{
						seilen = 0;
					}
				}
				break;
			}


			
			startCodeOffset = -1;

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
				spslen = bufsize - 4;
				memset(sps, 0x00, sizeof(sps));
				memcpy(sps, pbuf + 4, spslen);
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
				ppslen = bufsize - 4;
				memset(pps, 0x00, sizeof(pps));
				memcpy(pps, pbuf + 4, ppslen);	//pps
			}
		}
		if (ppslen > 0)
		{
			if (NULL != _pps)   memcpy(_pps, pps, ppslen);
			if (NULL != _ppslen)    *_ppslen = ppslen;
		}
		ret = 0;
	}

	if (iFoundSEI == 0x01)
	{
		if (seilen > 0)
		{
			if (NULL != _sei)   memcpy(_sei, sei, seilen);
			if (NULL != _seilen)    *_seilen = seilen;
		}
		ret = 0;
	}
    return ret;
}

