#include "trace.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#ifndef _WIN32
#include <pthread.h>
#endif


static int trace_mutex_init = 0;
#ifdef _WIN32
	static CRITICAL_SECTION		trace_mutex;
#else
	static pthread_mutex_t			trace_mutex;
#endif

void _TRACE_Init()
{
	trace_mutex_init = 0x01;
	#ifdef _WIN32
		InitializeCriticalSection(&trace_mutex);
	#else
		pthread_mutex_init(&trace_mutex, NULL);
	#endif
}
void _TRACE_Lock()
{
	if (trace_mutex_init == 0x00)		return;

	#ifdef _WIN32
		EnterCriticalSection(&trace_mutex);
	#else
		(void)pthread_mutex_lock(&trace_mutex);
	#endif
}
void _TRACE_Unlock()
{
	if (trace_mutex_init == 0x00)		return;

	#ifdef _WIN32
		LeaveCriticalSection(&trace_mutex);
	#else
		pthread_mutex_unlock(&trace_mutex);
	#endif
}

void _TRACE_Deinit()
{
	#ifdef _WIN32
		DeleteCriticalSection(&trace_mutex);
	#else
		pthread_mutex_destroy(&trace_mutex);
	#endif

	trace_mutex_init = 0x00;
}


FILE *fLog = NULL;
char szLogFilename[MAX_PATH] = {0};
void _TRACE_OpenLogFile(char *filenameprefix)
{
	memset(szLogFilename, 0x00, sizeof(szLogFilename));
	strcpy(szLogFilename, filenameprefix);

    char szTime[64] = {0};
    time_t tt = time(NULL);
    struct tm *_timetmp = NULL;
    _timetmp = localtime(&tt);
    //if (NULL != _timetmp)   strftime(szTime, 32, "%Y%m%d_%H%M%S",_timetmp);
	if (NULL != _timetmp)   strftime(szTime, 32, "%Y%m%d",_timetmp);

    if (NULL == fLog)
    {
        char szFile[MAX_PATH] = {0};
        sprintf(szFile, "%s.%s.log", filenameprefix,szTime);
        fLog = fopen(szFile, "wb");
    }
}

void _TRACE_CloseLogFile()
{
    if (NULL != fLog)
    {
        fclose(fLog);
        fLog = NULL;
    }
}

void _TRACE(TRACE_LOG_LEVEL_ENUM level, char* szFormat, ...)
{
#ifndef _WIN32
	return;
#else
#ifndef _DEBUG
	return;
#endif
#endif


#ifdef _WIN32
	HANDLE h = NULL;
	WORD wOldColorAttrs = 0;
	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
#endif
	char buff[1024] = {0,};
	va_list args;
	va_start(args,szFormat);
#ifdef _WIN32
	_vsnprintf(buff, 1023, szFormat,args);
#else
	vsnprintf(buff, 1023, szFormat,args);
#endif
	va_end(args);


	_TRACE_Lock();

#ifdef _WIN32
	if (level != TRACE_LOG_DEBUG)
	{
		h = GetStdHandle(STD_OUTPUT_HANDLE);  
		// Save the current color  
		GetConsoleScreenBufferInfo(h, &csbiInfo);
		wOldColorAttrs = csbiInfo.wAttributes;
		// Set the new color  
    
	/*
	system（"color 0A"）；
	其中color后面的0是背景色代号，A是前景色代号

		0=黑色
　　	  1=蓝色
　　	  2=绿色
　　	  3=湖蓝色
　　	  4=红色
　　	  5=紫色
　　	  6=黄色
　　	  7=白色
　　	  8=灰色
　　	  9=淡蓝色
　　	  A=淡绿色
　　	  B=淡浅绿色
　　	  C=淡红色
　　	  D=淡紫色
　　	  E=淡黄色
　　	  F=亮白色
	*/  

		if (level == TRACE_LOG_INFO)
		{
			SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		}
		else if (level == TRACE_LOG_WARNING)
		{
			SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
		}
		else if (level == TRACE_LOG_DEBUG)
		{
			SetConsoleTextAttribute(h, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		}
		else if (level == TRACE_LOG_ERROR)
		{
			SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY | BACKGROUND_RED);
		}
	}
#endif

	//if (NULL == fLog)     _TRACE_OpenLogFile("");
	if (NULL != fLog)
	{
		char szTime[64] = {0,};
		time_t tt = time(NULL);
		struct tm *_timetmp = NULL;
		_timetmp = localtime(&tt);
		if (NULL != _timetmp)	strftime(szTime, 32, "%Y-%m-%d %H:%M:%S ", _timetmp);

		static int filesize = 0;

		filesize += (int)fwrite(szTime, 1, (int)strlen(szTime), fLog);
		filesize += (int)fwrite(buff, 1, (int)strlen(buff), fLog);

/*
		if (filesize >= 10485760)			//10M
		{
			filesize = 0;
			_TRACE_CloseLogFile();

			char filename[MAX_PATH] = {0};
			strcpy(filename, szLogFilename);
			_TRACE_OpenLogFile(filename);
		}
*/
	}

#ifdef _WIN32
	{
        char szTime[64] = {0,};
        time_t tt = time(NULL);
        struct tm *_timetmp = NULL;
        _timetmp = localtime(&tt);
        if (NULL != _timetmp)   strftime(szTime, 32, "%Y%m%d %H%M%S ",_timetmp);
		printf("%s", szTime);

		OutputDebugString(szTime);
	}

	printf("%s", buff);
#endif
	//printf("StreamingServer::%s", buff);

#ifdef _WIN32
	// Restore the original color
	OutputDebugString(buff);
	if (NULL != h)
	{
		SetConsoleTextAttribute(h, wOldColorAttrs);
	}
#endif

	_TRACE_Unlock();
}


void __TRACE_TO_FILE(char *filename, char *pbuf, int bufsize)
{
#ifdef _DEBUG
	FILE *f = fopen(filename, "wb");
	if (NULL != f)
	{
		fwrite(pbuf, 1, bufsize, f);
		fclose(f);
	}
#endif
}