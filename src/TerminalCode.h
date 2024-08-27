#ifndef __TERMINAL_CODE_H__
#define __TERMINAL_CODE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "enc_dec.h"

#define	CODEING_TIME		"20171030 1000"			//年月日时分

#define COMPANY_EASYDARWIN	"Gavin&Babosa@2016EasyDarwinTeam"
#define COMPANY_BJ_LSXJ		"BJLSXJ_4C53584A"		//0x424A4C53584A			//BJLSXJ		北京蓝色星际		2016.04.11


/****************************************************************/
/****************************************************************/
/****************************************************************/
/**************注意: 每次发版本，请修改下面的公司名称************/
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

	unsigned int	modulelist[MAX_MODULE_NUM];			//模块列表
	unsigned int	platformlist[MAX_PLATFORM_NUM];		//平台列表
	int			validperiod;		//最大有效期

}TERMINAL_NODE_T;










#endif
