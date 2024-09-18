
#ifndef __GET_SPS_H__
#define __GET_SPS_H__


int GetH265VPSandSPSandPPS(char *pbuf, int bufsize, char *_vps, int *_vpslen, char *_sps, int *_spslen, char *_pps, int *_ppslen);
//int GetH264SPSandPPS(char *pbuf, int bufsize, char *_sps, int *_spslen, char *_pps, int *_ppslen);
int GetH264SPSandPPS(char* pbuf, int bufsize, char* _sps, int* _spslen, char* _pps, int* _ppslen, char* _sei, int* _seilen);


#endif
