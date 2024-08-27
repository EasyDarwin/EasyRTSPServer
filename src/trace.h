
#ifndef __TRACE_H__
#define __TRACE_H__

#ifdef _WIN32
#include <winsock2.h>
#else
#include <signal.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/socket.h>
#include <fcntl.h>
#define MAX_PATH 260
#endif


typedef enum __TRACE_LOG_LEVEL_ENUM
{
	TRACE_LOG_INFO		=	0,
	TRACE_LOG_WARNING,
	TRACE_LOG_DEBUG,
	TRACE_LOG_ERROR,
}TRACE_LOG_LEVEL_ENUM;


void _TRACE_Init();
void _TRACE_Deinit();

void _TRACE_OpenLogFile(char *filenameprefix);
void _TRACE_CloseLogFile();

void __TRACE_TO_FILE(char *filename, char *pbuf, int bufsize);

void _TRACE(TRACE_LOG_LEVEL_ENUM level, char* szFormat, ...);


#ifdef ANDROID
#include <android/log.h>        
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "EASYDARWIN", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "EASYDARWIN", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "EASYDARWIN", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "EASYDARWIN", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "EASYDARWIN", __VA_ARGS__)
#endif

#endif
