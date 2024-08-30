
#include "EasyStreamingServer.h"
#include <stdio.h>
#ifndef _WIN32
#include <signal.h>
#endif



int main()
{
#ifndef _WIN32
	signal(SIGPIPE, SIG_IGN);
#endif

	EasyStreamingServer		streamingServer;

	streamingServer.Startup(554);

	printf("###############     Press enter to exit     ############\n");
	getchar();

	streamingServer.Shutdown();


	return 0;
}
