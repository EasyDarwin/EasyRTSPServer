/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "mTunnel" multicast access service
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// Helper routines to implement 'group sockets'
// C++ header

#ifndef _GROUPSOCK_HELPER_HH
#define _GROUPSOCK_HELPER_HH

#ifndef _NET_ADDRESS_HH
#include "NetAddress.hh"
#endif
#include "../../../live_ipv4_ipv6.h"

int setupDatagramSocket(UsageEnvironment& env, Port port, int clientSock, LIVE_IP_VER_ENUM ipVer);

int setupDatagramSocket6(UsageEnvironment& env, Port port);

int setupStreamSocket4(UsageEnvironment& env,
		      Port port, Boolean makeNonBlocking = True);
int setupStreamSocket6(UsageEnvironment& env,
		      Port port, Boolean makeNonBlocking = True);

int setSocketTimeout(int sock, int timeout_msecs);

int readSocket(UsageEnvironment& env,
	       int socket, unsigned char* buffer, unsigned bufferSize,
	       LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddress);

Boolean writeSocket(UsageEnvironment& env,
		    int socket, LIVE_NET_ADDRESS_INADDR address, portNumBits portNum/*network byte order*/,
		    u_int8_t ttlArg,
		    unsigned char* buffer, unsigned bufferSize, LIVE_IP_VER_ENUM ipVer);

Boolean writeSocket(UsageEnvironment& env,
		    int socket, LIVE_NET_ADDRESS_INADDR address, portNumBits portNum/*network byte order*/,
		    unsigned char* buffer, unsigned bufferSize, LIVE_IP_VER_ENUM ipVer);
    // An optimized version of "writeSocket" that omits the "setsockopt()" call to set the TTL.

void ignoreSigPipeOnSocket(int socketNum);

unsigned getSendBufferSize(UsageEnvironment& env, int socket);
unsigned getReceiveBufferSize(UsageEnvironment& env, int socket);
unsigned setSendBufferTo(UsageEnvironment& env,
			 int socket, unsigned requestedSize);
unsigned setReceiveBufferTo(UsageEnvironment& env,
			    int socket, unsigned requestedSize);
unsigned increaseSendBufferTo(UsageEnvironment& env,
			      int socket, unsigned requestedSize);
unsigned increaseReceiveBufferTo(UsageEnvironment& env,
				 int socket, unsigned requestedSize);

Boolean makeSocketNonBlocking(int sock);
Boolean makeSocketBlocking(int sock, unsigned writeTimeoutInMilliseconds = 0);
  // A "writeTimeoutInMilliseconds" value of 0 means: Don't timeout

Boolean socketJoinGroup(UsageEnvironment& env, int socket,
			LIVE_NET_ADDRESS_INADDR groupAddress, LIVE_IP_VER_ENUM ipVer);
Boolean socketLeaveGroup(UsageEnvironment&, int socket,
			 LIVE_NET_ADDRESS_INADDR groupAddress, LIVE_IP_VER_ENUM ipVer);

// source-specific multicast join/leave
Boolean socketJoinGroupSSM(UsageEnvironment& env, int socket,
			   LIVE_NET_ADDRESS_INADDR groupAddress,
			   LIVE_NET_ADDRESS_INADDR sourceFilterAddr, LIVE_IP_VER_ENUM ipVer);
Boolean socketLeaveGroupSSM(UsageEnvironment&, int socket,
			    LIVE_NET_ADDRESS_INADDR groupAddress,
			    LIVE_NET_ADDRESS_INADDR sourceFilterAddr, LIVE_IP_VER_ENUM ipVer);

Boolean getSourcePort(UsageEnvironment& env, int socket, Port& port, LIVE_IP_VER_ENUM ipVer);

LIVE_NET_ADDRESS_INADDR ourIPAddress(UsageEnvironment& env, LIVE_IP_VER_ENUM ipVer); // in network order

// IP addresses of our sending and receiving interfaces.  (By default, these
// are INADDR_ANY (i.e., 0), specifying the default interface.)
extern LIVE_NET_ADDRESS_INADDR SendingInterfaceAddr;
extern LIVE_NET_ADDRESS_INADDR ReceivingInterfaceAddr;

// Allocates a randomly-chosen IPv4 SSM (multicast) address:
LIVE_NET_ADDRESS_INADDR chooseRandomIPv4SSMAddress(UsageEnvironment& env);

// Returns a simple "hh:mm:ss" string, for use in debugging output (e.g.)
char const* timestampString();


#ifdef HAVE_SOCKADDR_LEN
#define SET_SOCKADDR_SIN_LEN(var) var.sin_len = sizeof var
#else
#define SET_SOCKADDR_SIN_LEN(var)
#endif

#if 0
#define MAKE_SOCKADDR_IN(var,adr,prt) /*adr,prt must be in network order*/\
    LIVE_NET_ADDRESS_SOCKADDR_IN var;\
	if (live_ip_ver == LIVE_IP_VER_4) \
	{	\
		var.saddr4.sin_family = AF_INET;	\
		var.saddr4.sin_addr.s_addr = (adr); \
		var.saddr4.sin_port = (prt);	\
		SET_SOCKADDR_SIN_LEN(var.saddr4);	\
	}	\
	else	\
	{	\
    var.saddr6.sin6_family = AF_INET6; \
    var.saddr6.sin6_addr.s6_addr = (adr);\
    var.saddr6.sin6_port = (prt);\
    SET_SOCKADDR_SIN_LEN(var.saddr6); \
	}

#else
#define MAKE_SOCKADDR_IN(var,adr,prt) /*adr,prt must be in network order*/\
    struct sockaddr_in var;\
    var.sin_family = AF_INET;\
    var.sin_addr.s_addr = (adr);\
    var.sin_port = (prt);\
    SET_SOCKADDR_SIN_LEN(var);

#define MAKE_SOCKADDR_IN6(var,adr,prt) /*adr,prt must be in network order*/\
    struct sockaddr_in6 var;\
	memset(&var, 0x00, sizeof(var)); /*¡§¡ã???¡ì¡§¡ãamemset*/ \
    var.sin6_family = AF_INET6;\
    var.sin6_addr = (adr);\
    var.sin6_port = (prt);\
    SET_SOCKADDR_SIN_LEN(var);

#endif


// By default, we create sockets with the SO_REUSE_* flag set.
// If, instead, you want to create sockets without the SO_REUSE_* flags,
// Then enclose the creation code with:
//          {
//            NoReuse dummy;
//            ...
//          }
class NoReuse {
public:
  NoReuse(UsageEnvironment& env);
  ~NoReuse();

private:
  UsageEnvironment& fEnv;
};


// Define the "UsageEnvironment"-specific "groupsockPriv" structure:

struct _groupsockPriv { // There should be only one of these allocated
  HashTable* socketTable;
  int reuseFlag;
};
_groupsockPriv* groupsockPriv(UsageEnvironment& env); // allocates it if necessary
void reclaimGroupsockPriv(UsageEnvironment& env);


extern int gettimeofdayEx(struct timeval*, int*);
#if (defined(__WIN32__) || defined(_WIN32)) && !defined(__MINGW32__)
// For Windoze, we need to implement our own gettimeofday()
#endif

// The following are implemented in inet.c:
//extern "C" LIVE_NET_ADDRESS_INADDR our_inet_addr(char const*);
LIVE_NET_ADDRESS_INADDR our_inet_addr(char const*, LIVE_IP_VER_ENUM ipVer);
extern "C" void our_srandom(int x);
extern "C" long our_random();
extern "C" u_int32_t our_random32(); // because "our_random()" returns a 31-bit number

#endif
