
#include "EasyRTSPServer_Demo.h"
#include <stdio.h>
#ifndef _WIN32
#include <signal.h>

#include <sched.h>
#include <sys/syscall.h>

#endif

int streamingServerStop = 0;

void exit_cleanly(int flag)
{
	streamingServerStop = 1;
	printf("======Receive exit signal, program exit...=======\n");
}


int main()
{
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, exit_cleanly);
	signal(SIGINT, exit_cleanly);
	signal(SIGQUIT, exit_cleanly);
#endif

	EasyStreamingServer		streamingServer;

	streamingServer.Startup(554);

	//getchar();

	printf("###############     Press enter to exit     ############\n");
	while (1)
	{
		if (streamingServerStop == 0x01)
		{
			Sleep(2000);
			break;
		}
		Sleep(1000);
	}
	streamingServer.Shutdown();


	return 0;
}
