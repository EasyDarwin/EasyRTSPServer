#ifndef __LIVE_IPV4_IPV6
#define __LIVE_IPV4_IPV6

#ifndef _WIN32
#define	Sleep(x)	{usleep(x*1000);}
#endif


#define	LIVE_FLAG		0xFFFFFFFF

typedef enum LIVE_IPVER_ENUM
{
	LIVE_IP_VER_4	=	0,
	LIVE_IP_VER_6
}LIVE_IP_VER_ENUM;

extern LIVE_IP_VER_ENUM live_ip_ver;










#endif
