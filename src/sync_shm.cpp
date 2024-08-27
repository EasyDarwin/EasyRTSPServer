/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef ANDROID
#include "sync_shm.h"

#ifndef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>


int shm_create(int _shmkey, int *_shmid, int bufsize, char **pbuf)
{
	if (NULL == _shmid)			return -1;
	*_shmid = -1;

	printf("shmget..\n");
	int shmid = shmget(_shmkey, bufsize, IPC_CREAT|0666);
	if (shmid < 0)
	{
		printf("shmget fail[%d][%d].  %d\n", _shmkey, shmid, errno);
		return -1;
	}

	*pbuf = (char *)shmat(shmid, 0, 0);
	if ( *pbuf == (char*)-1)
	{
		printf("shmat fail. %d\n", errno);

		shm_delete(&shmid, *pbuf);
		return -1;
	}

	*_shmid = shmid;
	return 0;
}

int shm_mdt(int shmid, char *pbuf)
{
	if (shmid < 0)			return -1;

	if (NULL != pbuf)
	{
		shmdt(pbuf);
		pbuf = NULL;
	}

	return 0;
}

int shm_delete(int *shmid, char *pbuf)
{
	if (NULL == shmid)		return -1;
	if (*shmid < 0)			return -1;

	shm_mdt(*shmid, pbuf);

	shmctl(*shmid, IPC_RMID, 0);
	*shmid = -1;

	return 0;
}



#endif
#endif