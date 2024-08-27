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
// Implementation

#include "GroupsockHelper.hh"
#include "../../trace.h"

#if (defined(__WIN32__) || defined(_WIN32)) && !defined(__MINGW32__)
#include <time.h>
extern "C" int initializeWinsockIfNecessary();
#else
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/syscall.h>
#define initializeWinsockIfNecessary() 1
#endif
#if defined(__WIN32__) || defined(_WIN32) || defined(_QNX4)
#else
#include <signal.h>
#define USE_SIGNALS 1
#endif
#include <stdio.h>

// By default, use INADDR_ANY for the sending and receiving interfaces:
#if 1
LIVE_NET_ADDRESS_INADDR SendingInterfaceAddr;
LIVE_NET_ADDRESS_INADDR ReceivingInterfaceAddr;
#else
LIVE_NET_ADDRESS_INADDR SendingInterfaceAddr = INADDR_ANY;
LIVE_NET_ADDRESS_INADDR ReceivingInterfaceAddr = INADDR_ANY;
#endif
static void socketErr(UsageEnvironment& env, char const* errorMsg) {
  env.setResultErrMsg(errorMsg);
}

NoReuse::NoReuse(UsageEnvironment& env)
  : fEnv(env) {
  groupsockPriv(fEnv)->reuseFlag = 0;
}

NoReuse::~NoReuse() {
  groupsockPriv(fEnv)->reuseFlag = 1;
  reclaimGroupsockPriv(fEnv);
}



LIVE_NET_ADDRESS_INADDR our_inet_addr(char const* cp, LIVE_IP_VER_ENUM ipVer)
{
	static LIVE_NET_ADDRESS_INADDR ourInetAddr;
	memset(&ourInetAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
	if (ipVer == LIVE_IP_VER_4)
	{
		char szAddress[64] = {0};
		if (NULL == strstr(cp, "."))			//非IP地址, 为域名
		{
			struct hostent *p= NULL;
#ifdef _WIN32
			p = gethostbyname(cp);
#endif
			if(p==0)
			{

			}
			else  
			{  
			   //本机IP:利用循环,输出本机所有IP
			   for(int i=0;p->h_addr_list[i]!=0;i++)  
			   {  
					struct in_addr in;
					memcpy(&in,p->h_addr_list[i],sizeof(struct in_addr));

					strcpy(szAddress, inet_ntoa(in));

					if (0 != strcmp(szAddress, "0.0.0.0"))	break;
					if (0 != strcmp(szAddress, "127.0.0.1"))	break;

					break;
			   }  
			}
		}
		else
		{
			strcpy(szAddress, cp);
		}
		ourInetAddr.sin_addr.s_addr = inet_addr(szAddress);
	}
	else
	{
		inet_pton(AF_INET6, cp, &ourInetAddr.sin6_addr);
	}
	return ourInetAddr;
}

_groupsockPriv* groupsockPriv(UsageEnvironment& env) {
  if (env.groupsockPriv == NULL) { // We need to create it
    _groupsockPriv* result = new _groupsockPriv;
    result->socketTable = NULL;
    result->reuseFlag = 1; // default value => allow reuse of socket numbers
    env.groupsockPriv = result;
  }
  return (_groupsockPriv*)(env.groupsockPriv);
}

void reclaimGroupsockPriv(UsageEnvironment& env) {
  _groupsockPriv* priv = (_groupsockPriv*)(env.groupsockPriv);
  if (priv->socketTable == NULL && priv->reuseFlag == 1/*default value*/) {
    // We can delete the structure (to save space); it will get created again, if needed:
    delete priv;
    env.groupsockPriv = NULL;
  }
}

static int createSocket(LIVE_IP_VER_ENUM ipver, int type) {
  // Call "socket()" to create a (IPv4) socket of the specified type.
  // But also set it to have the 'close on exec' property (if we can)
  int sock = 0;
	if (ipver == LIVE_IP_VER_4)
	{
		#ifdef SOCK_CLOEXEC
		  sock = socket(AF_INET, type|SOCK_CLOEXEC, 0);
		  if (sock != -1 || errno != EINVAL) return sock;
		  // An "errno" of EINVAL likely means that the system wasn't happy with the SOCK_CLOEXEC; fall through and try again without it:
		#endif

		  sock = socket(AF_INET, type, 0);
		#ifdef FD_CLOEXEC
		  if (sock != -1) fcntl(sock, F_SETFD, FD_CLOEXEC);
		#endif
	}
	else
	{
		#ifdef SOCK_CLOEXEC
		  sock = socket(AF_INET6, type|SOCK_CLOEXEC, 0);
		  if (sock != -1 || errno != EINVAL) return sock;
		  // An "errno" of EINVAL likely means that the system wasn't happy with the SOCK_CLOEXEC; fall through and try again without it:
		#endif

		  sock = socket(AF_INET6, type, 0);
		#ifdef FD_CLOEXEC
		  if (sock != -1) fcntl(sock, F_SETFD, FD_CLOEXEC);
		#endif
	}

  return sock;
}

int setSocketTimeout(int sock, int timeout_msecs)
{
#if defined(__WIN32__) || defined(_WIN32)
    DWORD msto = (DWORD)timeout_msecs;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&msto, sizeof(msto) );
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&msto, sizeof(msto) );
#else
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout_msecs*1000;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof tv);
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv);
#endif

	return 0;
}

int setupDatagramSocket(UsageEnvironment& env, Port port, int clientSock, LIVE_IP_VER_ENUM ipVer) {
  if (!initializeWinsockIfNecessary()) {
    socketErr(env, "Failed to initialize 'winsock': ");
    return -1;
  }

  int newSocket = createSocket(ipVer, SOCK_DGRAM);
  if (newSocket < 0) {
    socketErr(env, "unable to create datagram socket: ");
    return newSocket;
  }

  //_TRACE(TRACE_LOG_INFO, (char *)"setupDatagramSocket: socket[%d]  port[%d]\n", newSocket, port.num());

  setSocketTimeout(newSocket, 500);		//gavin 2019.09.20

  int reuseFlag = groupsockPriv(env)->reuseFlag;
  reclaimGroupsockPriv(env);
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEADDR) error: ");
    closeSocket(newSocket);
    return -1;
  }

#if defined(__WIN32__) || defined(_WIN32)
  // Windoze doesn't properly handle SO_REUSEPORT or IP_MULTICAST_LOOP
#else
#ifdef SO_REUSEPORT
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEPORT) error: ");
    closeSocket(newSocket);
    return -1;
  }
#endif

#ifdef IP_MULTICAST_LOOP
  const u_int8_t loop = 1;
  if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
		 (const char*)&loop, sizeof loop) < 0) {
    socketErr(env, "setsockopt(IP_MULTICAST_LOOP) error: ");
    closeSocket(newSocket);
    return -1;
  }
#endif
#endif

  // Note: Windoze requires binding, even if the port number is 0
  LIVE_NET_ADDRESS_INADDR addr;
#if defined(__WIN32__) || defined(_WIN32)
#else
  //if (port.num() != 0 || ReceivingInterfaceAddr != INADDR_ANY) {
  if (port.num() != 0 || (0!=memcmp(&ReceivingInterfaceAddr, &addr, sizeof(LIVE_NET_ADDRESS_INADDR)))) {
#endif
    if (port.num() == 0) addr = ReceivingInterfaceAddr;
	if (ipVer == LIVE_IP_VER_4)		//IPV4
	{
		MAKE_SOCKADDR_IN(name, addr.sin_addr.s_addr, port.num());

#if 1
		if (clientSock > 0)
		{
			socklen_t nSize = sizeof(struct sockaddr_in);
			struct sockaddr_in clientAddr;
			memset(&clientAddr, 0x00, sizeof(clientAddr));
			getsockname(clientSock, (struct sockaddr*)&clientAddr, &nSize);
			//name.sin_addr.s_addr = clientAddr.sin_addr.s_addr;
			memcpy(&name.sin_addr, &clientAddr.sin_addr, sizeof(name.sin_addr));

			//_TRACE(TRACE_LOG_DEBUG, (char *)"Bind Local Addr[%s:%d]\n", inet_ntoa(name.sin_addr), ntohs(name.sin_port));
		}
#endif

		if (bind(newSocket, (struct sockaddr*)&name, sizeof name) != 0) {
		  char tmpBuffer[100];
		  sprintf(tmpBuffer, "bind() error (port number: %d): ",
			  ntohs(port.num()));
		  socketErr(env, tmpBuffer);
		  closeSocket(newSocket);
		  return -1;
		}
	}
	else
	{
		//IPV6
		MAKE_SOCKADDR_IN6(name, addr.sin6_addr, port.num());

		if (clientSock > 0)
		{
			socklen_t nSize = sizeof(struct sockaddr_in6);
			struct sockaddr_in6 clientAddr;
			memset(&clientAddr, 0x00, sizeof(clientAddr));
			getsockname(clientSock, (struct sockaddr*)&clientAddr, &nSize);
			memcpy(&name.sin6_addr, &clientAddr.sin6_addr, sizeof(name.sin6_addr));
		}


		if (bind(newSocket, (struct sockaddr*)&name, sizeof name) != 0) {
		  char tmpBuffer[100];
		  sprintf(tmpBuffer, "bind() error (port number: %d): ",
			  ntohs(port.num()));
		  socketErr(env, tmpBuffer);
		  closeSocket(newSocket);
		  return -1;
		}
	}
#if defined(__WIN32__) || defined(_WIN32)
#else
  }
#endif

  // Set the sending interface for multicasts, if it's not the default:
#if 1
	if (ipVer == LIVE_IP_VER_4)		//IPV4
	{
		LIVE_NET_ADDRESS_INADDR tmpInAddr;
		memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
		if ( 0 != memcmp(&tmpInAddr, &SendingInterfaceAddr, sizeof(LIVE_NET_ADDRESS_INADDR))) {
	  //if (SendingInterfaceAddr != INADDR_ANY) {
		struct in_addr addr;
		addr.s_addr = SendingInterfaceAddr.sin_addr.s_addr;

		if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_IF,
			   (const char*)&addr, sizeof addr) < 0) {
		  socketErr(env, "error setting outgoing multicast interface: ");
		  closeSocket(newSocket);
		  return -1;
		}
	  }
	}
	else			//IPV6
	{

		LIVE_NET_ADDRESS_INADDR tmpInAddr;
		memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
		if ( 0 != memcmp(&tmpInAddr, &SendingInterfaceAddr, sizeof(LIVE_NET_ADDRESS_INADDR))) {
	  //if (SendingInterfaceAddr != INADDR_ANY) {
		struct in6_addr addr;
		memcpy(&addr, &SendingInterfaceAddr.sin6_addr.s6_addr, sizeof(addr));

		if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_IF,
			   (const char*)&addr, sizeof addr) < 0) {
		  socketErr(env, "error setting outgoing multicast interface: ");
		  closeSocket(newSocket);
		  return -1;
		}
	  }
	}
#endif

  return newSocket;
}


int setupDatagramSocket6(UsageEnvironment& env, Port port) {
  if (!initializeWinsockIfNecessary()) {
    socketErr(env, "Failed to initialize 'winsock': ");
    return -1;
  }

  int newSocket = createSocket(LIVE_IP_VER_6, SOCK_DGRAM);
  if (newSocket < 0) {
    socketErr(env, "unable to create datagram socket: ");
    return newSocket;
  }

  setSocketTimeout(newSocket, 500);		//gavin 2019.09.20

  int reuseFlag = groupsockPriv(env)->reuseFlag;
  reclaimGroupsockPriv(env);
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEADDR) error: ");
    closeSocket(newSocket);
    return -1;
  }

#if defined(__WIN32__) || defined(_WIN32)
  // Windoze doesn't properly handle SO_REUSEPORT or IP_MULTICAST_LOOP
#else
#ifdef SO_REUSEPORT
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEPORT) error: ");
    closeSocket(newSocket);
    return -1;
  }
#endif

#ifdef IP_MULTICAST_LOOP
  const u_int8_t loop = 1;
  if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_LOOP,
		 (const char*)&loop, sizeof loop) < 0) {
    socketErr(env, "setsockopt(IP_MULTICAST_LOOP) error: ");
    closeSocket(newSocket);
    return -1;
  }
#endif
#endif

  // Note: Windoze requires binding, even if the port number is 0
  LIVE_NET_ADDRESS_INADDR addr;
#if defined(__WIN32__) || defined(_WIN32)
#else
  //if (port.num() != 0 || ReceivingInterfaceAddr != INADDR_ANY) {
  if (port.num() != 0 || (0!=memcmp(&ReceivingInterfaceAddr, &addr, sizeof(LIVE_NET_ADDRESS_INADDR)))) {
#endif
    if (port.num() == 0) addr = ReceivingInterfaceAddr;
	{
		//IPV6
		MAKE_SOCKADDR_IN6(name, addr.sin6_addr, port.num());
		if (bind(newSocket, (struct sockaddr*)&name, sizeof name) != 0) {
		  char tmpBuffer[100];
		  sprintf(tmpBuffer, "bind() error (port number: %d): ",
			  ntohs(port.num()));
		  socketErr(env, tmpBuffer);
		  closeSocket(newSocket);
		  return -1;
		}
	}
#if defined(__WIN32__) || defined(_WIN32)
#else
  }
#endif

  // Set the sending interface for multicasts, if it's not the default:

	LIVE_NET_ADDRESS_INADDR tmpInAddr;
	memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
	if ( 0 != memcmp(&tmpInAddr, &SendingInterfaceAddr, sizeof(LIVE_NET_ADDRESS_INADDR))) {
	//if (SendingInterfaceAddr != INADDR_ANY) {
	struct in6_addr addr;
	memcpy(&addr, &SendingInterfaceAddr.sin6_addr.s6_addr, sizeof(addr));

	if (setsockopt(newSocket, IPPROTO_IP, IP_MULTICAST_IF,
			(const char*)&addr, sizeof addr) < 0) {
		socketErr(env, "error setting outgoing multicast interface: ");
		closeSocket(newSocket);
		return -1;
	}
	}

  return newSocket;
}


Boolean makeSocketNonBlocking(int sock) {
#if defined(__WIN32__) || defined(_WIN32)
  unsigned long arg = 1;
  return ioctlsocket(sock, FIONBIO, &arg) == 0;
#elif defined(VXWORKS)
  int arg = 1;
  return ioctl(sock, FIONBIO, (int)&arg) == 0;
#else
  int curFlags = fcntl(sock, F_GETFL, 0);
  return fcntl(sock, F_SETFL, curFlags|O_NONBLOCK) >= 0;
#endif
}

Boolean makeSocketBlocking(int sock, unsigned writeTimeoutInMilliseconds) {
  Boolean result;
#if defined(__WIN32__) || defined(_WIN32)
  unsigned long arg = 0;
  result = ioctlsocket(sock, FIONBIO, &arg) == 0;
#elif defined(VXWORKS)
  int arg = 0;
  result = ioctl(sock, FIONBIO, (int)&arg) == 0;
#else
  int curFlags = fcntl(sock, F_GETFL, 0);
  result = fcntl(sock, F_SETFL, curFlags&(~O_NONBLOCK)) >= 0;
#endif

  if (writeTimeoutInMilliseconds > 0) {
#ifdef SO_SNDTIMEO
#if defined(__WIN32__) || defined(_WIN32)
    DWORD msto = (DWORD)writeTimeoutInMilliseconds;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&msto, sizeof(msto) );
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&msto, sizeof(msto) );
#else
    struct timeval tv;
    tv.tv_sec = writeTimeoutInMilliseconds/1000;
    tv.tv_usec = (writeTimeoutInMilliseconds%1000)*1000;
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof tv);
    
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv);
#endif
#endif
  }

  return result;
}

int setupStreamSocket4(UsageEnvironment& env,
                      Port port, Boolean makeNonBlocking) {
  if (!initializeWinsockIfNecessary()) {
    socketErr(env, "Failed to initialize 'winsock': ");
    return -1;
  }

  int newSocket = createSocket(LIVE_IP_VER_4, SOCK_STREAM);
  if (newSocket < 0) {
    socketErr(env, "unable to create stream socket: ");
    return newSocket;
  }

  setSocketTimeout(newSocket, 500);		//gavin 2019.09.20

  int reuseFlag = groupsockPriv(env)->reuseFlag;
  reclaimGroupsockPriv(env);
  
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEADDR) error: ");
    closeSocket(newSocket);
    return -1;
  }
  

  // SO_REUSEPORT doesn't really make sense for TCP sockets, so we
  // normally don't set them.  However, if you really want to do this
  // #define REUSE_FOR_TCP
#ifdef REUSE_FOR_TCP
#if defined(__WIN32__) || defined(_WIN32)
  // Windoze doesn't properly handle SO_REUSEPORT
#else
#ifdef SO_REUSEPORT
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEPORT) error: ");
    closeSocket(newSocket);
    return -1;
  }
#endif
#endif
#endif

  // Note: Windoze requires binding, even if the port number is 0
#if defined(__WIN32__) || defined(_WIN32)
#else

  LIVE_NET_ADDRESS_INADDR addr;
  if (port.num() != 0 || (0!=memcmp(&ReceivingInterfaceAddr, &addr, sizeof(LIVE_NET_ADDRESS_INADDR)))) {
//  if (port.num() != 0 || ReceivingInterfaceAddr != INADDR_ANY) {
#endif

	MAKE_SOCKADDR_IN(name, ReceivingInterfaceAddr.sin_addr.s_addr, port.num());
	if (bind(newSocket, (struct sockaddr*)&name, sizeof name) != 0) {
		char tmpBuffer[100];
		sprintf(tmpBuffer, "bind() error (port number: %d): ",
			ntohs(port.num()));
		socketErr(env, tmpBuffer);
		closeSocket(newSocket);
		return -1;
	}

#if defined(__WIN32__) || defined(_WIN32)
#else
  }
#endif

  if (makeNonBlocking) {
    if (!makeSocketNonBlocking(newSocket)) {
      socketErr(env, "failed to make non-blocking: ");
      closeSocket(newSocket);
      return -1;
    }
  }

  return newSocket;
}


int setupStreamSocket6(UsageEnvironment& env,
                      Port port, Boolean makeNonBlocking) {
  if (!initializeWinsockIfNecessary()) {
    socketErr(env, "Failed to initialize 'winsock': ");
    return -1;
  }

  int newSocket = createSocket(LIVE_IP_VER_6, SOCK_STREAM);
  if (newSocket < 0) {
    socketErr(env, "unable to create stream socket: ");
    return newSocket;
  }

  setSocketTimeout(newSocket, 500);		//gavin 2019.09.20

  int reuseFlag = groupsockPriv(env)->reuseFlag;
  reclaimGroupsockPriv(env);
  
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEADDR,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEADDR) error: ");
    closeSocket(newSocket);
    return -1;
  }
  

  // SO_REUSEPORT doesn't really make sense for TCP sockets, so we
  // normally don't set them.  However, if you really want to do this
  // #define REUSE_FOR_TCP
#ifdef REUSE_FOR_TCP
#if defined(__WIN32__) || defined(_WIN32)
  // Windoze doesn't properly handle SO_REUSEPORT
#else
#ifdef SO_REUSEPORT
  if (setsockopt(newSocket, SOL_SOCKET, SO_REUSEPORT,
		 (const char*)&reuseFlag, sizeof reuseFlag) < 0) {
    socketErr(env, "setsockopt(SO_REUSEPORT) error: ");
    closeSocket(newSocket);
    return -1;
  }
#endif
#endif
#endif

#if defined(__WIN32__) || defined(_WIN32)
#else

  LIVE_NET_ADDRESS_INADDR addr;
  if (port.num() != 0 || (0!=memcmp(&ReceivingInterfaceAddr, &addr, sizeof(LIVE_NET_ADDRESS_INADDR)))) {
//  if (port.num() != 0 || ReceivingInterfaceAddr != INADDR_ANY) {
#endif

	{
#if 0
		struct sockaddr_in6 my_addr;
		memset(&my_addr, 0x00, sizeof(my_addr));  
		/* my_addr.sin_family = PF_INET; */ // IPv4  
		my_addr.sin6_family = AF_INET6;    // IPv6  
		/* my_addr.sin_port = htons(myport); */ // IPv4  
		my_addr.sin6_port = htons(0);   // IPv6  
		my_addr.sin6_addr = in6addr_any;            // IPv6  
		/* if (bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr)) */ // IPv4  
		if (bind(newSocket, (struct sockaddr *) &my_addr, sizeof(struct sockaddr_in6))  // IPv6  
			== -1) 
		{  
			perror("bind");  
			exit(1);  
		} else  
			printf("binded/n");  
#endif

		//MAKE_SOCKADDR_IN6(name, ReceivingInterfaceAddr.sin6_addr, port.num());
		MAKE_SOCKADDR_IN6(name, in6addr_any, port.num());

		int on = 1;
		if (setsockopt(newSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *)&on, sizeof(on)) < 0)
		{
			printf("set ipv6 fail.\n");
		}

		
		if (bind(newSocket, (struct sockaddr*)&name, sizeof (struct sockaddr_in6)) != 0) {
			char tmpBuffer[100];
			sprintf(tmpBuffer, "bind() error (port number: %d): ",
				ntohs(port.num()));
			socketErr(env, tmpBuffer);
			closeSocket(newSocket);
			return -1;
		}
		/*
		int on = 1;
		if (setsockopt(fd_v6, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on)) < 0)
		{ 
			//perror("setsockopt\n"); 
			return -1;
		}
		*/
	}
#if defined(__WIN32__) || defined(_WIN32)
#else
  }
#endif

  if (makeNonBlocking) {
    if (!makeSocketNonBlocking(newSocket)) {
      socketErr(env, "failed to make non-blocking: ");
      closeSocket(newSocket);
      return -1;
    }
  }

  return newSocket;
}


int readSocket(UsageEnvironment& env,
	       int socket, unsigned char* buffer, unsigned bufferSize,
	       LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddress) {
  SOCKLEN_T addressSize = sizeof fromAddress;
  int bytesRead = recvfrom(socket, (char*)buffer, bufferSize, 0,
			   (struct sockaddr*)&fromAddress,
			   &addressSize);
  if (bytesRead < 0) {
    //##### HACK to work around bugs in Linux and Windows:
    int err = env.getErrno();
    if (err == 111 /*ECONNREFUSED (Linux)*/
#if defined(__WIN32__) || defined(_WIN32)
	// What a piece of crap Windows is.  Sometimes
	// recvfrom() returns -1, but with an 'errno' of 0.
	// This appears not to be a real error; just treat
	// it as if it were a read of zero bytes, and hope
	// we don't have to do anything else to 'reset'
	// this alleged error:
	|| err == 0 || err == EWOULDBLOCK
#else
	|| err == EAGAIN
#endif
	|| err == 113 /*EHOSTUNREACH (Linux)*/) { // Why does Linux return this for datagram sock?
		fromAddress.saddr4.sin_addr.s_addr = 0;
      return 0;
    }
    //##### END HACK
    socketErr(env, "recvfrom() error: ");
  } else if (bytesRead == 0) {
    // "recvfrom()" on a stream socket can return 0 if the remote end has closed the connection.  Treat this as an error:
    return -1;
  }

  return bytesRead;
}

Boolean writeSocket(UsageEnvironment& env,
		    int socket, LIVE_NET_ADDRESS_INADDR address, portNumBits portNum,
		    u_int8_t ttlArg,
		    unsigned char* buffer, unsigned bufferSize, LIVE_IP_VER_ENUM ipVer) {
  // Before sending, set the socket's TTL:
#if defined(__WIN32__) || defined(_WIN32)
#define TTL_TYPE int
#else
#define TTL_TYPE u_int8_t
#endif

	
  TTL_TYPE ttl = (TTL_TYPE)ttlArg;
  if (setsockopt(socket, IPPROTO_IP, IP_MULTICAST_TTL,
		 (const char*)&ttl, sizeof ttl) < 0) {
    socketErr(env, "setsockopt(IP_MULTICAST_TTL) error: ");
    return False;
  }

  return writeSocket(env, socket, address, portNum, buffer, bufferSize, ipVer);
}

Boolean writeSocket(UsageEnvironment& env,
		    int socket, LIVE_NET_ADDRESS_INADDR address, portNumBits portNum,
		    unsigned char* buffer, unsigned bufferSize, LIVE_IP_VER_ENUM ipVer) {
	if (ipVer == LIVE_IP_VER_4)		//IPV4
	{
	  do {
		  MAKE_SOCKADDR_IN(dest, address.sin_addr.s_addr, portNum);
		int bytesSent = sendto(socket, (char*)buffer, bufferSize, 0,
				   (struct sockaddr*)&dest, sizeof dest);
		if (bytesSent != (int)bufferSize) {
		  char tmpBuf[100];
		  sprintf(tmpBuf, "writeSocket(%d), sendTo() error: wrote %d bytes instead of %u: ", socket, bytesSent, bufferSize);
		  socketErr(env, tmpBuf);
		  break;
		}


#if 0
                {
			socklen_t nSize = sizeof(struct sockaddr_in);
			struct sockaddr_in clientAddr;
			memset(&clientAddr, 0x00, sizeof(clientAddr));
			getsockname(socket, (struct sockaddr*)&clientAddr, &nSize);
                    
			
                    
                    char *srcAddr = inet_ntoa(clientAddr.sin_addr);
					char szSrcAddr[64] = {0};
					if (NULL != srcAddr)	strcpy(szSrcAddr, srcAddr);
                    char *dstAddr = inet_ntoa(dest.sin_addr);
                    
                    printf("writeSocket: socket[%d]  [%s : %d] --> [%s : %d]  datasize:%d\n", 
                            socket, 
                            szSrcAddr, ntohs(clientAddr.sin_port),
                            dstAddr, ntohs(dest.sin_port),
                            bytesSent);
                }
#endif
    
		return True;
	  } while (0);
	}
	else							//IPV6
	{
	  do {
		  MAKE_SOCKADDR_IN6(dest, address.sin6_addr, portNum);

		 // inet_pton(AF_INET6, "2001:0:9d38:953c:38b0:d177:88d8:a5e1", &dest.sin6_addr);

		  //char ipv6Address[128] = {0};
		  //inet_ntop(AF_INET6, &dest.sin6_addr, ipv6Address, INET6_ADDRSTRLEN);

		int bytesSent = sendto(socket, (char*)buffer, bufferSize, 0,
				   (struct sockaddr*)&dest, sizeof dest);
		if (bytesSent != (int)bufferSize) {
		  char tmpBuf[100];
		  sprintf(tmpBuf, "writeSocket(%d), sendTo() error: wrote %d bytes instead of %u: ", socket, bytesSent, bufferSize);
		  socketErr(env, tmpBuf);
		  break;
		}
    


		return True;
	  } while (0);

	}
  return False;
}

void ignoreSigPipeOnSocket(int socketNum) {
  #ifdef USE_SIGNALS
  #ifdef SO_NOSIGPIPE
  int set_option = 1;
  setsockopt(socketNum, SOL_SOCKET, SO_NOSIGPIPE, &set_option, sizeof set_option);
  #else
  signal(SIGPIPE, SIG_IGN);
  #endif
  #endif
}

static unsigned getBufferSize(UsageEnvironment& env, int bufOptName,
			      int socket) {
  unsigned curSize;
  SOCKLEN_T sizeSize = sizeof curSize;
  if (getsockopt(socket, SOL_SOCKET, bufOptName,
		 (char*)&curSize, &sizeSize) < 0) {
    socketErr(env, "getBufferSize() error: ");
    return 0;
  }

  return curSize;
}
unsigned getSendBufferSize(UsageEnvironment& env, int socket) {
  return getBufferSize(env, SO_SNDBUF, socket);
}
unsigned getReceiveBufferSize(UsageEnvironment& env, int socket) {
  return getBufferSize(env, SO_RCVBUF, socket);
}

static unsigned setBufferTo(UsageEnvironment& env, int bufOptName,
			    int socket, unsigned requestedSize) {
  SOCKLEN_T sizeSize = sizeof requestedSize;
  setsockopt(socket, SOL_SOCKET, bufOptName, (char*)&requestedSize, sizeSize);

  // Get and return the actual, resulting buffer size:
  return getBufferSize(env, bufOptName, socket);
}
unsigned setSendBufferTo(UsageEnvironment& env,
			 int socket, unsigned requestedSize) {
	return setBufferTo(env, SO_SNDBUF, socket, requestedSize);
}
unsigned setReceiveBufferTo(UsageEnvironment& env,
			    int socket, unsigned requestedSize) {
	return setBufferTo(env, SO_RCVBUF, socket, requestedSize);
}

static unsigned increaseBufferTo(UsageEnvironment& env, int bufOptName,
				 int socket, unsigned requestedSize) {
  // First, get the current buffer size.  If it's already at least
  // as big as what we're requesting, do nothing.
  unsigned curSize = getBufferSize(env, bufOptName, socket);

  // Next, try to increase the buffer to the requested size,
  // or to some smaller size, if that's not possible:
  while (requestedSize > curSize) {
    SOCKLEN_T sizeSize = sizeof requestedSize;
    if (setsockopt(socket, SOL_SOCKET, bufOptName,
		   (char*)&requestedSize, sizeSize) >= 0) {
      // success
      return requestedSize;
    }
    requestedSize = (requestedSize+curSize)/2;
  }

  return getBufferSize(env, bufOptName, socket);
}
unsigned increaseSendBufferTo(UsageEnvironment& env,
			      int socket, unsigned requestedSize) {
  return increaseBufferTo(env, SO_SNDBUF, socket, requestedSize);
}
unsigned increaseReceiveBufferTo(UsageEnvironment& env,
				 int socket, unsigned requestedSize) {
  return increaseBufferTo(env, SO_RCVBUF, socket, requestedSize);
}

static void clearMulticastAllSocketOption(int socket) {
#ifdef IP_MULTICAST_ALL
  // This option is defined in modern versions of Linux to overcome a bug in the Linux kernel's default behavior.
  // When set to 0, it ensures that we receive only packets that were sent to the specified IP multicast address,
  // even if some other process on the same system has joined a different multicast group with the same port number.
  int multicastAll = 0;
  (void)setsockopt(socket, IPPROTO_IP, IP_MULTICAST_ALL, (void*)&multicastAll, sizeof multicastAll);
  // Ignore the call's result.  Should it fail, we'll still receive packets (just perhaps more than intended)
#endif
}

Boolean socketJoinGroup(UsageEnvironment& env, int socket,
			LIVE_NET_ADDRESS_INADDR groupAddress, LIVE_IP_VER_ENUM ipVer){
  if (!IsMulticastAddress(groupAddress, ipVer)) return True; // ignore this case

	if (ipVer == LIVE_IP_VER_4)
	{
		  struct ip_mreq imr;
		  imr.imr_multiaddr.s_addr = groupAddress.sin_addr.s_addr;
		  imr.imr_interface.s_addr = ReceivingInterfaceAddr.sin_addr.s_addr;
		  if (setsockopt(socket, IPPROTO_IP, IP_ADD_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct ip_mreq)) < 0) {
		#if defined(__WIN32__) || defined(_WIN32)
			if (env.getErrno() != 0) {
			  // That piece-of-shit toy operating system (Windows) sometimes lies
			  // about setsockopt() failing!
		#endif
			  socketErr(env, "setsockopt(IP_ADD_MEMBERSHIP) error: ");
			  return False;
		#if defined(__WIN32__) || defined(_WIN32)
			}
		#endif
		  }
	}
	else		//IPV6
	{
      struct ipv6_mreq imr; 
	  memcpy(&imr.ipv6mr_multiaddr, &groupAddress.sin6_addr, sizeof(struct in6_addr)); 
      imr.ipv6mr_interface = 0; 

		  if (setsockopt(socket, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct ipv6_mreq)) < 0) {
		#if defined(__WIN32__) || defined(_WIN32)
			if (env.getErrno() != 0) {
			  // That piece-of-shit toy operating system (Windows) sometimes lies
			  // about setsockopt() failing!
		#endif
			  socketErr(env, "setsockopt(IPV6_ADD_MEMBERSHIP) error: ");
			  return False;
		#if defined(__WIN32__) || defined(_WIN32)
			}
		#endif
		  }
	}

  clearMulticastAllSocketOption(socket);

  return True;
}

Boolean socketLeaveGroup(UsageEnvironment&, int socket,
			 LIVE_NET_ADDRESS_INADDR groupAddress, LIVE_IP_VER_ENUM ipVer) {
  if (!IsMulticastAddress(groupAddress, ipVer)) return True; // ignore this case

	if (ipVer == LIVE_IP_VER_4)
	{
		  struct ip_mreq imr;
		  imr.imr_multiaddr.s_addr = groupAddress.sin_addr.s_addr;
		  imr.imr_interface.s_addr = ReceivingInterfaceAddr.sin_addr.s_addr;
		  if (setsockopt(socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct ip_mreq)) < 0) {
			return False;
		  }
	}
	else
	{
      struct ipv6_mreq imr; 
	  memcpy(&imr.ipv6mr_multiaddr, &groupAddress.sin6_addr, sizeof(struct in6_addr)); 
      imr.ipv6mr_interface = 0; 
      if (setsockopt(socket, IPPROTO_IPV6, IPV6_DROP_MEMBERSHIP, 
         (const char*)&imr, sizeof (struct ipv6_mreq)) < 0) { 
			return False;
		  }
	}

  return True;
}

// The source-specific join/leave operations require special setsockopt()
// commands, and a special structure (ip_mreq_source).  If the include files
// didn't define these, we do so here:
#if !defined(IP_ADD_SOURCE_MEMBERSHIP)
struct ip_mreq_source {
  struct  in_addr imr_multiaddr;  /* IP multicast address of group */
  struct  in_addr imr_sourceaddr; /* IP address of source */
  struct  in_addr imr_interface;  /* local IP address of interface */
};
#endif

#ifndef IP_ADD_SOURCE_MEMBERSHIP

#ifdef LINUX
#define IP_ADD_SOURCE_MEMBERSHIP   39
#define IP_DROP_SOURCE_MEMBERSHIP 40
#else
#define IP_ADD_SOURCE_MEMBERSHIP   25
#define IP_DROP_SOURCE_MEMBERSHIP 26
#endif

#endif

Boolean socketJoinGroupSSM(UsageEnvironment& env, int socket,
			   LIVE_NET_ADDRESS_INADDR groupAddress,
			   LIVE_NET_ADDRESS_INADDR sourceFilterAddr, LIVE_IP_VER_ENUM ipVer) {
  if (!IsMulticastAddress(groupAddress, ipVer)) return True; // ignore this case

	if (ipVer == LIVE_IP_VER_4)
	{
		  struct ip_mreq_source imr;
		#ifdef __ANDROID__
			imr.imr_multiaddr.s_addr = groupAddress.sin_addr.s_addr;
			imr.imr_sourceaddr.s_addr = sourceFilterAddr.sin_addr.s_addr;
			imr.imr_interface.s_addr = ReceivingInterfaceAddr.sin_addr.s_addr;
		#else
		  imr.imr_multiaddr.s_addr = groupAddress.sin_addr.s_addr;
			imr.imr_sourceaddr.s_addr = sourceFilterAddr.sin_addr.s_addr;
			imr.imr_interface.s_addr = ReceivingInterfaceAddr.sin_addr.s_addr;
		#endif
		  if (setsockopt(socket, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct ip_mreq_source)) < 0) {
			socketErr(env, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP) error: ");
			return False;
		  }

		  clearMulticastAllSocketOption(socket);
	}
	else
	{
		struct group_source_req imr; 
		#ifdef __ANDROID__
			//imr.gsr_group.__data = (struct sockaddr_in6)groupAddress;
			//imr.gsr_source.__data = (struct sockaddr_in6)sourceFilterAddr;
			//imr.gsr_interface = (struct sockaddr_in6)ReceivingInterfaceAddr;
		#else
			memcpy(&imr.gsr_group, &groupAddress.sin6_addr, sizeof(struct sockaddr_in6));
			memcpy(&imr.gsr_source, &sourceFilterAddr.sin6_addr, sizeof(struct sockaddr_in6));
			memcpy(&imr.gsr_interface, &ReceivingInterfaceAddr.sin6_addr, sizeof(struct sockaddr_in6));
		#endif
		  if (setsockopt(socket, IPPROTO_IPV6, IPV6_ADD_SOURCE_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct ip_mreq_source)) < 0) {
			socketErr(env, "setsockopt(IP_ADD_SOURCE_MEMBERSHIP) error: ");
			return False;
		  }

		  clearMulticastAllSocketOption(socket);
	}

  return True;
}

Boolean socketLeaveGroupSSM(UsageEnvironment& /*env*/, int socket,
			    LIVE_NET_ADDRESS_INADDR groupAddress,
			    LIVE_NET_ADDRESS_INADDR sourceFilterAddr, LIVE_IP_VER_ENUM ipVer) {
  if (!IsMulticastAddress(groupAddress, ipVer)) return True; // ignore this case

	if (ipVer == LIVE_IP_VER_4)
	{
		  struct ip_mreq_source imr;
		#ifdef __ANDROID__
			//imr.imr_multiaddr = groupAddress;
			//imr.imr_sourceaddr = sourceFilterAddr;
			//imr.imr_interface = ReceivingInterfaceAddr;
		#else
		  imr.imr_multiaddr.s_addr = groupAddress.sin_addr.s_addr;
			imr.imr_sourceaddr.s_addr = sourceFilterAddr.sin_addr.s_addr;
			imr.imr_interface.s_addr = ReceivingInterfaceAddr.sin_addr.s_addr;
		#endif
		  if (setsockopt(socket, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct ip_mreq_source)) < 0) {
			return False;
		  }
	}
	else
	{
		  struct group_source_req imr; 
		#ifdef __ANDROID__
			//imr.imr_multiaddr = groupAddress.sin6_addr.s6_addr;
			//imr.imr_sourceaddr = sourceFilterAddr.sin6_addr.s6_addr;
			//imr.imr_interface = ReceivingInterfaceAddr.sin6_addr.s6_addr;
		#else
			memcpy(&imr.gsr_group, &groupAddress, sizeof(struct sockaddr_in6));
			memcpy(&imr.gsr_source, &sourceFilterAddr, sizeof(struct sockaddr_in6));
			memcpy(&imr.gsr_interface, &ReceivingInterfaceAddr, sizeof(struct sockaddr_in6));
		#endif
		  if (setsockopt(socket, IPPROTO_IPV6, IPV6_DROP_SOURCE_MEMBERSHIP,
				 (const char*)&imr, sizeof (struct group_source_req)) < 0) {
			return False;
		  }
	}



  return True;
}

static Boolean getSourcePort0(int socket, portNumBits& resultPortNum/*host order*/, LIVE_IP_VER_ENUM ipVer) {
	if (ipVer == LIVE_IP_VER_4)
	{
	  sockaddr_in test; test.sin_port = 0;
	  SOCKLEN_T len = sizeof test;
	  if (getsockname(socket, (struct sockaddr*)&test, &len) < 0) return False;

	  resultPortNum = ntohs(test.sin_port);
	}
	else
	{
	  sockaddr_in6 test; test.sin6_port = 0;
	  SOCKLEN_T len = sizeof test;
	  if (getsockname(socket, (struct sockaddr*)&test, &len) < 0) return False;

	  resultPortNum = ntohs(test.sin6_port);
	}
  return True;
}

Boolean getSourcePort(UsageEnvironment& env, int socket, Port& port, LIVE_IP_VER_ENUM ipVer) {
  portNumBits portNum = 0;
  if (!getSourcePort0(socket, portNum, ipVer) || portNum == 0) {
    // Hack - call bind(), then try again:
	  
	  //gavin 2018.03.20
	if (ipVer == LIVE_IP_VER_4)
	{
			MAKE_SOCKADDR_IN(name, INADDR_ANY, 0);
			bind(socket, (struct sockaddr*)&name, sizeof name);

			if (!getSourcePort0(socket, portNum, ipVer) || portNum == 0) {
			  socketErr(env, "getsockname() error: ");
			  return False;
			}
	  }
	  else
	  {
		  LIVE_NET_ADDRESS_INADDR tmpInAddr;
		  memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
		  tmpInAddr.sin6_addr = in6addr_any;
			MAKE_SOCKADDR_IN6(name, tmpInAddr.sin6_addr, 0);
			bind(socket, (struct sockaddr*)&name, sizeof name);

			if (!getSourcePort0(socket, portNum, ipVer) || portNum == 0) {
				socketErr(env, "getsockname() error: ");
				return False;
			}
	  }
  }
  port = Port(portNum);
  return True;
}

static Boolean badAddressForUs(LIVE_NET_ADDRESS_INADDR addr, LIVE_IP_VER_ENUM ipVer) {
  // Check for some possible erroneous addresses:

	if (ipVer == LIVE_IP_VER_4)
	{
	  //nAddr= htonl(addr);

	  unsigned int nAddr = htonl(addr.sin_addr.s_addr);

	  return (nAddr == 0x7F000001 /* 127.0.0.1 */
		  || nAddr == 0
		  || nAddr == (u_int32_t)(~0));
	 }

	//IPV6
	char ntopaddr[128]; 
	inet_ntop(AF_INET6, &addr, ntopaddr, sizeof(struct in6_addr));  
	return  (memcmp(ntopaddr, "::1", sizeof(struct in6_addr))==0  
			|| memcmp(ntopaddr, "0:0:0:0:0:0:0:0", sizeof(struct in6_addr))==0  
			|| memcmp(ntopaddr, "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff", sizeof(struct in6_addr))==0      
	); 
}

Boolean loopbackWorks = 1;

LIVE_NET_ADDRESS_INADDR ourIPAddress(UsageEnvironment& env,LIVE_IP_VER_ENUM ipVer) {


  static int iOurAddressInit = 0;
  static LIVE_NET_ADDRESS_INADDR ourAddress;
  int sock = -1;
  LIVE_NET_ADDRESS_INADDR testAddr;

  if (iOurAddressInit == 0)
  {
	  iOurAddressInit = 0x01;
	  //memset(&ourAddress, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
  }

  LIVE_NET_ADDRESS_INADDR	tmpInAddr;
  //memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
  if (0 == memcmp(&tmpInAddr, &ReceivingInterfaceAddr, sizeof(LIVE_NET_ADDRESS_INADDR)) ) {
  //if (ReceivingInterfaceAddr != INADDR_ANY) {
    // Hack: If we were told to receive on a specific interface address, then 
    // define this to be our ip address:
    //ourAddress = ReceivingInterfaceAddr;
	memcpy(&ourAddress, &ReceivingInterfaceAddr, sizeof(LIVE_NET_ADDRESS_INADDR));
  }

  //LIVE_NET_ADDRESS_INADDR	tmpInAddr;
  //memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
  if (0 == memcmp(&tmpInAddr, &ourAddress, sizeof(LIVE_NET_ADDRESS_INADDR))) {
  //if (ourAddress == 0) {
    // We need to find our source address
    LIVE_NET_ADDRESS_SOCKADDR_IN fromAddr;
	memset(&fromAddr, 0x00, sizeof(LIVE_NET_ADDRESS_SOCKADDR_IN));
    //fromAddr.sin_addr.s_addr = 0;
	//fromAddr.saddr6 = in6addr0;		//gavin 2018.03.19

    // Get our address by sending a (0-TTL) multicast packet,
    // receiving it, and looking at the source address used.
    // (This is kinda bogus, but it provides the best guarantee
    // that other nodes will think our address is the same as we do.)
    do {
      loopbackWorks = 0; // until we learn otherwise
#if 0	//gavin 2018.06.27
	  LIVE_NET_ADDRESS_INADDR	tmpInAddr4;
	  LIVE_NET_ADDRESS_INADDR        inaddr4 = our_inet_addr("228.67.43.91", LIVE_IP_VER_4);
	  memcpy(&tmpInAddr4, &inaddr4, sizeof(LIVE_NET_ADDRESS_INADDR));

	  LIVE_NET_ADDRESS_INADDR	tmpInAddr6;
      LIVE_NET_ADDRESS_INADDR       inaddr6 = our_inet_addr("ff0e::2", LIVE_IP_VER_6);
	  memcpy(&tmpInAddr6, &inaddr6, sizeof(LIVE_NET_ADDRESS_INADDR));

	  memcpy(&testAddr.sin_addr, &tmpInAddr.sin_addr, sizeof(tmpInAddr.sin_addr));
	  memcpy(&testAddr.sin6_addr, &tmpInAddr.sin6_addr, sizeof(tmpInAddr.sin6_addr));

      Port testPort(15947); // ditto

      sock = setupDatagramSocket(env, testPort, 0, ipVer);
      if (sock < 0) break;

	  if (!socketJoinGroup(env, sock, testAddr, ipVer)) break;

      unsigned char testString[] = "hostIdTest";
      unsigned testStringLength = sizeof testString;

      if (!writeSocket(env, sock, testAddr, testPort.num(), 0,
		       testString, testStringLength, ipVer)) break;

      // Block until the socket is readable (with a 5-second timeout):
      fd_set rd_set;
      FD_ZERO(&rd_set);
      FD_SET((unsigned)sock, &rd_set);
      const unsigned numFds = sock+1;
      struct timeval timeout;
      timeout.tv_sec = 5;
      timeout.tv_usec = 0;
      int result = select(numFds, &rd_set, NULL, NULL, &timeout);
      if (result <= 0) break;

      unsigned char readBuffer[20];
      int bytesRead = readSocket(env, sock,
				 readBuffer, sizeof readBuffer,
				 fromAddr);
      if (bytesRead != (int)testStringLength
	  || strncmp((char*)readBuffer, (char*)testString, testStringLength) != 0) {
	break;
      }

      // We use this packet's source address, if it's good:

	  LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	  memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
	  memcpy(&tmpInAddr.sin_addr, &fromAddr.saddr4.sin_addr, sizeof(tmpInAddr.sin_addr));
	  memcpy(&tmpInAddr.sin6_addr, &fromAddr.saddr6.sin6_addr, sizeof(tmpInAddr.sin6_addr));

      loopbackWorks = !badAddressForUs(tmpInAddr, ipVer);
#endif
    } while (0);

    if (sock >= 0) {
		socketLeaveGroup(env, sock, testAddr, ipVer);
      closeSocket(sock);
    }

	LIVE_NET_ADDRESS_INADDR from;
    if (!loopbackWorks) do {
      // We couldn't find our address using multicast loopback,
      // so try instead to look it up directly - by first getting our host name, and then resolving this host name
      char hostname[100];
      hostname[0] = '\0';
      int result = gethostname(hostname, sizeof hostname);
      if (result != 0 || hostname[0] == '\0') {
	env.setResultErrMsg("initial gethostname() failed");
	break;
      }

      // Try to resolve "hostname" to an IP address:
      NetAddressList addresses(hostname, ipVer);
      NetAddressList::Iterator iter(addresses);
      NetAddress const* address;

      // Take the first address that's not bad:
      LIVE_NET_ADDRESS_INADDR addr;
      while ((address = iter.nextAddress()) != NULL) 
	  {
			LIVE_NET_ADDRESS_INADDR a = *(LIVE_NET_ADDRESS_INADDR*)(address->data());

#ifdef _DEBUG
			char sztmp[128 ] = {0};
			strcpy(sztmp, inet_ntoa(a.sin_addr));
#endif
			if (!badAddressForUs(a, ipVer)) 
			{
				from = a;
				break;
			}
      }

      // Assign the address that we found to "fromAddr" (as if the 'loopback' method had worked), to simplify the code below: 
      //fromAddr.saddr4.sin_addr.s_addr = addr;
	  /*
	  if (ipVer == LIVE_IP_VER_4)
	  {
		  memcpy(&fromAddr.saddr4.sin_addr, &addr.sin_addr, sizeof(struct sockaddr_in));
	  }
	  else if (ipVer == LIVE_IP_VER_6)
	  {
		  memcpy(&fromAddr.saddr6, &addr.sin6_addr, sizeof(struct in6_addr));
	  }
	  */
    } while (0);

    // Make sure we have a good address:
    //LIVE_NET_ADDRESS_INADDR from;
    if (badAddressForUs(from, ipVer)) {
      //char tmp[100];
      //sprintf(tmp, "This computer has an invalid IP address: %s", AddressString(from, ipVer).val());
      //env.setResultMsg(tmp);
	  memset(&from, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
      //from = 0;
    }

    //ourAddress = from;
	memcpy(&ourAddress, &from, sizeof(LIVE_NET_ADDRESS_INADDR));

    // Use our newly-discovered IP address, and the current time,
    // to initialize the random number generator's seed:
    struct timeval timeNow;
    gettimeofdayEx(&timeNow, NULL);
	unsigned seed = 0U;
	if (ipVer == LIVE_IP_VER_4)
	{
		seed = ourAddress.sin_addr.s_addr^timeNow.tv_sec^timeNow.tv_usec;
	}
	else
	{
		seed = timeNow.tv_sec^timeNow.tv_usec;
	}
    our_srandom(seed);
  }
  return ourAddress;
}

LIVE_NET_ADDRESS_INADDR chooseRandomIPv4SSMAddress(UsageEnvironment& env) {
#if 0
  // First, a hack to ensure that our random number generator is seeded:
  (void) ourIPAddress(env);

  // Choose a random address in the range [232.0.1.0, 232.255.255.255)
  // i.e., [0xE8000100, 0xE8FFFFFF)
  LIVE_NET_ADDRESS_INADDR const first = 0xE8000100, lastPlus1 = 0xE8FFFFFF;
  LIVE_NET_ADDRESS_INADDR const range = lastPlus1 - first;

  return ntohl(first + ((LIVE_NET_ADDRESS_INADDR)our_random())%range);
#else

	//gavin 2018.03.19   ???????o?￥¨oy2?é???¨o1¨??

	static LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));

  // Choose a random address in the range [232.0.1.0, 232.255.255.255)
  // i.e., [0xE8000100, 0xE8FFFFFF)
  unsigned int const first = 0xE8000100, lastPlus1 = 0xE8FFFFFF;
  unsigned int const range = lastPlus1 - first;

  unsigned long ulTmp = ntohl(first + ((unsigned int)our_random())%range);
  tmpInAddr.sin_addr.s_addr = ntohl(first + ((unsigned int)our_random())%range);
  return tmpInAddr;
#endif
}

char const* timestampString() {
  struct timeval tvNow;
  gettimeofdayEx(&tvNow, NULL);

#if !defined(_WIN32_WCE)
  static char timeString[9]; // holds hh:mm:ss plus trailing '\0'

  time_t tvNow_t = tvNow.tv_sec;
  char const* ctimeResult = ctime(&tvNow_t);
  if (ctimeResult == NULL) {
    sprintf(timeString, "??:??:??");
  } else {
    char const* from = &ctimeResult[11];
    int i;
    for (i = 0; i < 8; ++i) {
      timeString[i] = from[i];
    }
    timeString[i] = '\0';
  }
#else
  // WinCE apparently doesn't have "ctime()", so instead, construct
  // a timestamp string just using the integer and fractional parts
  // of "tvNow":
  static char timeString[50];
  sprintf(timeString, "%lu.%06ld", tvNow.tv_sec, tvNow.tv_usec);
#endif

  return (char const*)&timeString;
}

#if (defined(__WIN32__) || defined(_WIN32)) && !defined(__MINGW32__)
// For Windoze, we need to implement our own gettimeofday()

// used to make sure that static variables in gettimeofday() aren't initialized simultaneously by multiple threads
static LONG initializeLock_gettimeofday = 0;  

#if !defined(_WIN32_WCE)
#include <sys/timeb.h>
#endif

int gettimeofdayEx(struct timeval* tp, int* /*tz*/) {
  static LARGE_INTEGER tickFrequency, epochOffset;

  static Boolean isInitialized = False;

  LARGE_INTEGER tickNow;

#if !defined(_WIN32_WCE)
  QueryPerformanceCounter(&tickNow);
#else
  tickNow.QuadPart = GetTickCount();
#endif
 
  if (!isInitialized) {
    if(1 == InterlockedIncrement(&initializeLock_gettimeofday)) {
#if !defined(_WIN32_WCE)
      // For our first call, use "ftime()", so that we get a time with a proper epoch.
      // For subsequent calls, use "QueryPerformanceCount()", because it's more fine-grain.
      struct timeb tb;
      ftime(&tb);
      tp->tv_sec = (long)tb.time;
      tp->tv_usec = 1000*tb.millitm;

      // Also get our counter frequency:
      QueryPerformanceFrequency(&tickFrequency);
#else
      /* FILETIME of Jan 1 1970 00:00:00. */
      const LONGLONG epoch = 116444736000000000LL;
      FILETIME fileTime;
      LARGE_INTEGER time;
      GetSystemTimeAsFileTime(&fileTime);

      time.HighPart = fileTime.dwHighDateTime;
      time.LowPart = fileTime.dwLowDateTime;

      // convert to from 100ns time to unix timestamp in seconds, 1000*1000*10
      tp->tv_sec = (long)((time.QuadPart - epoch) / 10000000L);

      /*
        GetSystemTimeAsFileTime has just a seconds resolution,
        thats why wince-version of gettimeofday is not 100% accurate, usec accuracy would be calculated like this:
        // convert 100 nanoseconds to usec
        tp->tv_usec= (long)((time.QuadPart - epoch)%10000000L) / 10L;
      */
      tp->tv_usec = 0;

      // resolution of GetTickCounter() is always milliseconds
      tickFrequency.QuadPart = 1000;
#endif     
      // compute an offset to add to subsequent counter times, so we get a proper epoch:
      epochOffset.QuadPart
          = tp->tv_sec * tickFrequency.QuadPart + (tp->tv_usec * tickFrequency.QuadPart) / 1000000L - tickNow.QuadPart;
      
      // next caller can use ticks for time calculation
      isInitialized = True; 
      return 0;
    } else {
        InterlockedDecrement(&initializeLock_gettimeofday);
        // wait until first caller has initialized static values
        while(!isInitialized){
          Sleep(1);
        }
    }
  }

  // adjust our tick count so that we get a proper epoch:
  tickNow.QuadPart += epochOffset.QuadPart;

  tp->tv_sec =  (long)(tickNow.QuadPart / tickFrequency.QuadPart);
  tp->tv_usec = (long)(((tickNow.QuadPart % tickFrequency.QuadPart) * 1000000L) / tickFrequency.QuadPart);

  return 0;
}

#elif defined ANDROID
int gettimeofdayEx(struct timeval* tp, int* /*tz*/) 
{
  return gettimeofday(tp, NULL);
}

#else

int gettimeofdayEx(struct timeval* tp, int* /*tz*/) 
{
	struct timespec monotonic_time;
	memset(&monotonic_time, 0, sizeof(monotonic_time));
	syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &monotonic_time);
    //syscall(__NR_clock_gettime, CLOCK_MONOTONIC, &tSpec);
	if (NULL != tp)
	{
		tp->tv_sec = monotonic_time.tv_sec;
		tp->tv_usec = monotonic_time.tv_nsec/1000;
	}
  
	return 0;
}

#endif
