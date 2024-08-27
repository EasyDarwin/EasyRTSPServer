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
// Network Interfaces
// Implementation

#include "NetInterface.hh"
#include "GroupsockHelper.hh"

#ifndef NO_SSTREAM
#include <sstream>
#endif
#include <stdio.h>
#include "../../trace.h"
////////// NetInterface //////////

UsageEnvironment* NetInterface::DefaultUsageEnvironment = NULL;

NetInterface::NetInterface() {
}

NetInterface::~NetInterface() {
}


////////// NetInterface //////////

DirectedNetInterface::DirectedNetInterface() {
}

DirectedNetInterface::~DirectedNetInterface() {
}


////////// DirectedNetInterfaceSet //////////

DirectedNetInterfaceSet::DirectedNetInterfaceSet()
	: fTable(HashTable::create(ONE_WORD_HASH_KEYS)) {
}

DirectedNetInterfaceSet::~DirectedNetInterfaceSet() {
	delete fTable;
}

DirectedNetInterface*
DirectedNetInterfaceSet::Add(DirectedNetInterface const* interf) {
  return (DirectedNetInterface*) fTable->Add((char*)interf, (void*)interf);
}

Boolean
DirectedNetInterfaceSet::Remove(DirectedNetInterface const* interf) {
  return fTable->Remove((char*)interf);
}

DirectedNetInterfaceSet::Iterator::
Iterator(DirectedNetInterfaceSet& interfaces)
  : fIter(HashTable::Iterator::create(*(interfaces.fTable))) {
}

DirectedNetInterfaceSet::Iterator::~Iterator() {
  delete fIter;
}

DirectedNetInterface* DirectedNetInterfaceSet::Iterator::next() {
  char const* key; // dummy
  return (DirectedNetInterface*) fIter->next(key);
};


////////// Socket //////////

int Socket::DebugLevel = 1; // default value

Socket::Socket(UsageEnvironment& env, Port port, int clientSock, LIVE_IP_VER_ENUM ipVer)
  : fEnv(DefaultUsageEnvironment != NULL ? *DefaultUsageEnvironment : env), fPort(port), fSocketIPVer(ipVer) {
  fSocketNum = setupDatagramSocket(fEnv, port, clientSock, ipVer);
  unsigned oldSendBufferSize = getSendBufferSize(fEnv, fSocketNum);
  setSendBufferTo(fEnv, fSocketNum, 512*1024);
  unsigned newSendBufferSize = getSendBufferSize(fEnv, fSocketNum);
}

void Socket::reset() {
  if (fSocketNum >= 0)
  {
	  //_TRACE(TRACE_LOG_DEBUG, (char *)"Socket::reset() ¹Ø±Õsocket: %d\n", fSocketNum);
	  closeSocket(fSocketNum);
  }
  fSocketNum = -1;
}

Socket::~Socket() {
  reset();
}

Boolean Socket::changePort(Port newPort) {
  int oldSocketNum = fSocketNum;
  unsigned oldReceiveBufferSize = getReceiveBufferSize(fEnv, fSocketNum);
  unsigned oldSendBufferSize = getSendBufferSize(fEnv, fSocketNum);
  closeSocket(fSocketNum);

  fSocketNum = setupDatagramSocket(fEnv, newPort, 0, fSocketIPVer);
  if (fSocketNum < 0) {
    fEnv.taskScheduler().turnOffBackgroundReadHandling(oldSocketNum);
    return False;
  }

  setReceiveBufferTo(fEnv, fSocketNum, oldReceiveBufferSize);
  setSendBufferTo(fEnv, fSocketNum, oldSendBufferSize);
  if (fSocketNum != oldSocketNum) { // the socket number has changed, so move any event handling for it:
    fEnv.taskScheduler().moveSocketHandling(oldSocketNum, fSocketNum);
  }
  return True;
}

UsageEnvironment& operator<<(UsageEnvironment& s, const Socket& sock) {
	return s << timestampString() << " Socket(" << sock.socketNum() << ")";
}

////////// SocketLookupTable //////////

SocketLookupTable::SocketLookupTable()
  : fTable(HashTable::create(ONE_WORD_HASH_KEYS)) {
}

SocketLookupTable::~SocketLookupTable() {
  delete fTable;
}

Socket* SocketLookupTable::Fetch(UsageEnvironment& env, Port port,
				 Boolean& isNew) {
  isNew = False;
  Socket* sock;
  do {
    sock = (Socket*) fTable->Lookup((char*)(long)(port.num()));
    if (sock == NULL) { // we need to create one:
      sock = CreateNew(env, port);
      if (sock == NULL || sock->socketNum() < 0) break;

      fTable->Add((char*)(long)(port.num()), (void*)sock);
      isNew = True;
    }

    return sock;
  } while (0);

  delete sock;
  return NULL;
}

Boolean SocketLookupTable::Remove(Socket const* sock) {
  return fTable->Remove( (char*)(long)(sock->port().num()) );
}

////////// NetInterfaceTrafficStats //////////

NetInterfaceTrafficStats::NetInterfaceTrafficStats() {
  fTotNumPackets = fTotNumBytes = 0.0;
}

void NetInterfaceTrafficStats::countPacket(unsigned packetSize) {
  fTotNumPackets += 1.0;
  fTotNumBytes += packetSize;
}

Boolean NetInterfaceTrafficStats::haveSeenTraffic() const {
  return fTotNumPackets != 0.0;
}
