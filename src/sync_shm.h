/*
	Copyright (c) 2013-2015 EasyDarwin.ORG.  All rights reserved.
	Github: https://github.com/EasyDarwin
	WEChat: EasyDarwin
	Website: http://www.easydarwin.org
*/
#ifndef __SYNC_SHM_H__
#define __SYNC_SHM_H__

#ifndef ANDROID

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

int shm_create(int _shmkey, int *_shmid, int bufsize, char **pbuf);
int shm_mdt(int shmid, char *pbuf);
int shm_delete(int *shmid, char *pbuf);

#endif

#endif

#endif
