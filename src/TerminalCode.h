#ifndef __TERMINAL_CODE_H__
#define __TERMINAL_CODE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enc_dec.h"

#define	CODEING_TIME		"20171030 1000"			//������ʱ��

#define COMPANY_EASYDARWIN	"Gavin&Babosa@2016EasyDarwinTeam"
#define COMPANY_BJ_LSXJ		"BJLSXJ_4C53584A"		//0x424A4C53584A			//BJLSXJ		������ɫ�Ǽ�		2016.04.11


/****************************************************************/
/****************************************************************/
/****************************************************************/
/**************ע��: ÿ�η��汾�����޸�����Ĺ�˾����************/
#define	COMPANY_NAME		COMPANY_EASYDARWIN
/****************************************************************/
/****************************************************************/
/****************************************************************/

#define	MAX_MODULE_NUM			3
#define	MAX_PLATFORM_NUM		5

typedef struct __TERMAINAL_NODE_T
{
	char		companyname[36];
	char		password[36];

	unsigned int	modulelist[MAX_MODULE_NUM];			//ģ���б�
	unsigned int	platformlist[MAX_PLATFORM_NUM];		//ƽ̨�б�
	int			validperiod;		//�����Ч��

}TERMINAL_NODE_T;










#endif
