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
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// 'Group sockets'
// Implementation

#include "Groupsock.hh"
#include "GroupsockHelper.hh"
//##### Eventually fix the following #include; we shouldn't know about tunnels
#include "TunnelEncaps.hh"

#ifndef NO_SSTREAM
#include <sstream>
#endif
#include <stdio.h>

#include "../../trace.h"

///////// OutputSocket //////////

OutputSocket::OutputSocket(UsageEnvironment& env, int clientSock, LIVE_IP_VER_ENUM ipVer)
  : Socket(env, 0 /* let kernel choose port */, clientSock, ipVer),
    fSourcePort(0), fLastSentTTL(256/*hack: a deliberately invalid value*/) {
}

OutputSocket::OutputSocket(UsageEnvironment& env, Port port, int clientSock, LIVE_IP_VER_ENUM ipVer)
  : Socket(env, port, clientSock, ipVer),
    fSourcePort(0), fLastSentTTL(256/*hack: a deliberately invalid value*/) {
}

OutputSocket::~OutputSocket() {
}

Boolean OutputSocket::write(LIVE_NET_ADDRESS_INADDR address, portNumBits portNum, u_int8_t ttl,
			    unsigned char* buffer, unsigned bufferSize) {
  if ((unsigned)ttl == fLastSentTTL) {
    // Optimization: Don't do a 'set TTL' system call again
    if (!writeSocket(env(), socketNum(), address, portNum, buffer, bufferSize, socketIPVer())) return False;
  } else {
    if (!writeSocket(env(), socketNum(), address, portNum, ttl, buffer, bufferSize, socketIPVer())) return False;
    fLastSentTTL = (unsigned)ttl;
  }

  if (sourcePortNum() == 0) {
    // Now that we've sent a packet, we can find out what the
    // kernel chose as our ephemeral source port number:
    if (!getSourcePort(env(), socketNum(), fSourcePort, socketIPVer())) {
      if (DebugLevel >= 1)
	env() << *this
	     << ": failed to get source port: "
	     << env().getResultMsg() << "\n";
      return False;
    }
  }

  return True;
}

// By default, we don't do reads:
Boolean OutputSocket
::handleRead(unsigned char* /*buffer*/, unsigned /*bufferMaxSize*/,
	     unsigned& /*bytesRead*/, LIVE_NET_ADDRESS_SOCKADDR_IN& /*fromAddressAndPort*/) {
  return True;
}


///////// destRecord //////////

destRecord
::destRecord(LIVE_NET_ADDRESS_INADDR const& addr, Port const& port, u_int8_t ttl, unsigned sessionId,
	     destRecord* next)
  : fNext(next), fGroupEId(addr, port.num(), ttl), fSessionId(sessionId) {
}

destRecord::~destRecord() {
  delete fNext;
}


///////// Groupsock //////////

NetInterfaceTrafficStats Groupsock::statsIncoming;
NetInterfaceTrafficStats Groupsock::statsOutgoing;
NetInterfaceTrafficStats Groupsock::statsRelayedIncoming;
NetInterfaceTrafficStats Groupsock::statsRelayedOutgoing;

// Constructor for a source-independent multicast group
Groupsock::Groupsock(UsageEnvironment& env, LIVE_NET_ADDRESS_INADDR const& groupAddr,
		     Port port, u_int8_t ttl, int clientSock, LIVE_IP_VER_ENUM ipVer)
  : OutputSocket(env, port, clientSock, ipVer),
    deleteIfNoMembers(False), isSlave(False),
    fDests(new destRecord(groupAddr, port, ttl, 0, NULL)),
    fIncomingGroupEId(groupAddr, port.num(), ttl), mFlag(LIVE_FLAG) {

		if (!socketJoinGroup(env, socketNum(), groupAddr, ipVer)) {
    if (DebugLevel >= 1) {
      env << *this << ": failed to join group: "
	  << env.getResultMsg() << "\n";
    }
  }

  // Make sure we can get our source address:
#if 0		//gavin 2018.03.19
	LIVE_NET_ADDRESS_INADDR tmpInAddr;
	memcpy(&tmpInAddr, &(ourIPAddress(env)), sizeof(LIVE_NET_ADDRESS_INADDR));
	

  if (ourIPAddress(env) == 0) {
    if (DebugLevel >= 0) { // this is a fatal error
      env << "Unable to determine our source address: "
	  << env.getResultMsg() << "\n";
    }
  }
#endif
  if (DebugLevel >= 2) env << *this << ": created\n";
}

// Constructor for a source-specific multicast group
Groupsock::Groupsock(UsageEnvironment& env, LIVE_NET_ADDRESS_INADDR const& groupAddr,
		     LIVE_NET_ADDRESS_INADDR const& sourceFilterAddr,
		     Port port, int clientSock, LIVE_IP_VER_ENUM ipVer)
  : OutputSocket(env, port, clientSock, ipVer),
    deleteIfNoMembers(False), isSlave(False),
    fDests(new destRecord(groupAddr, port, 255, 0, NULL)),
    fIncomingGroupEId(groupAddr, sourceFilterAddr, port.num()), mFlag(LIVE_FLAG) {

		SetSocketIPVer(ipVer);

  // First try a SSM join.  If that fails, try a regular join:
		if (!socketJoinGroupSSM(env, socketNum(), groupAddr,
			sourceFilterAddr, ipVer)) {
    if (DebugLevel >= 3) {
      env << *this << ": SSM join failed: "
	  << env.getResultMsg();
      env << " - trying regular join instead\n";
    }
	if (!socketJoinGroup(env, socketNum(), groupAddr, ipVer)) {
      if (DebugLevel >= 1) {
	env << *this << ": failed to join group: "
	     << env.getResultMsg() << "\n";
      }
    }
  }

  if (DebugLevel >= 2) env << *this << ": created\n";
}

Groupsock::~Groupsock() {
  if (isSSM()) {
	  if (!socketLeaveGroupSSM(env(), socketNum(), groupAddress(),
		  sourceFilterAddress(), socketIPVer())) {
			  socketLeaveGroup(env(), socketNum(), groupAddress(), socketIPVer());
    }
  } else {
    socketLeaveGroup(env(), socketNum(), groupAddress(), socketIPVer());
  }

  delete fDests;

  mFlag = 0x00;

  if (DebugLevel >= 2) env() << *this << ": deleting\n";
}

destRecord* Groupsock
::createNewDestRecord(LIVE_NET_ADDRESS_INADDR const& addr, Port const& port, u_int8_t ttl,
		      unsigned sessionId, destRecord* next) {
  // Default implementation:
  return new destRecord(addr, port, ttl, sessionId, next);
}

void
Groupsock::changeDestinationParameters(LIVE_NET_ADDRESS_INADDR const& newDestAddr,
				       Port newDestPort, int newDestTTL, unsigned sessionId) {
  destRecord* dest;
  for (dest = fDests; dest != NULL && dest->fSessionId != sessionId; dest = dest->fNext) {}

  if (dest == NULL) { // There's no existing 'destRecord' for this "sessionId"; add a new one:
    fDests = createNewDestRecord(newDestAddr, newDestPort, newDestTTL, sessionId, fDests);
    return;
  }

  // "dest" is an existing 'destRecord' for this "sessionId"; change its values to the new ones:
  LIVE_NET_ADDRESS_INADDR destAddr = dest->fGroupEId.groupAddress();

  bool bAddrIsEmpty = false;
  LIVE_NET_ADDRESS_INADDR tmpInAddr;
  if (0 == memcmp(&tmpInAddr, &newDestAddr, sizeof(LIVE_NET_ADDRESS_INADDR)))	bAddrIsEmpty = true;

  bool bCheckIPV6 = true;
  if (0!=memcmp(&newDestAddr.sin6_addr, &destAddr.sin6_addr, sizeof(destAddr.sin6_addr)))	bCheckIPV6 = false;

  if (! bAddrIsEmpty) {
    if ( (! bCheckIPV6)
	&& IsMulticastAddress(newDestAddr, bCheckIPV6?LIVE_IP_VER_6:LIVE_IP_VER_4)) {
      // If the new destination is a multicast address, then we assume that
      // we want to join it also.  (If this is not in fact the case, then
      // call "multicastSendOnly()" afterwards.)
      socketLeaveGroup(env(), socketNum(), destAddr, socketIPVer());
      socketJoinGroup(env(), socketNum(), newDestAddr, socketIPVer());
    }
	memcpy(&destAddr, &newDestAddr, sizeof(newDestAddr));
    //destAddr.sin_addr.s_addr = newDestAddr.sin_addr.s_addr;
  }

  portNumBits destPortNum = dest->fGroupEId.portNum();
  if (newDestPort.num() != 0) {
    if (newDestPort.num() != destPortNum
	&& IsMulticastAddress(destAddr, bCheckIPV6?LIVE_IP_VER_6:LIVE_IP_VER_4)) {
      // Also bind to the new port number:
      changePort(newDestPort);
      // And rejoin the multicast group:
      socketJoinGroup(env(), socketNum(), destAddr, socketIPVer());
    }
    destPortNum = newDestPort.num();
  }

  u_int8_t destTTL = ttl();
  if (newDestTTL != ~0) destTTL = (u_int8_t)newDestTTL;

  dest->fGroupEId = GroupEId(destAddr, destPortNum, destTTL);

  // Finally, remove any other 'destRecord's that might also have this "sessionId":
  removeDestinationFrom(dest->fNext, sessionId);
}

unsigned Groupsock
::lookupSessionIdFromDestination(LIVE_NET_ADDRESS_SOCKADDR_IN const& destAddrAndPort) const {
  destRecord* dest = lookupDestRecordFromDestination(destAddrAndPort);
  if (dest == NULL) return 0;

  return dest->fSessionId;
}

void Groupsock::addDestination(LIVE_NET_ADDRESS_INADDR const& addr, Port const& port, unsigned sessionId) {
  // Default implementation:
  // If there's no existing 'destRecord' with the same "addr", "port", and "sessionId", add a new one:

	LIVE_NET_ADDRESS_INADDR tmpInAddr;


  for (destRecord* dest = fDests; dest != NULL; dest = dest->fNext) 
  {
	  memcpy(&tmpInAddr, &dest->fGroupEId.groupAddress(), sizeof(LIVE_NET_ADDRESS_INADDR));
    if (sessionId == dest->fSessionId && 
		(0==memcmp(&addr, &tmpInAddr, sizeof(LIVE_NET_ADDRESS_INADDR))) &&		//addr.sin_addr.s_addr == dest->fGroupEId.groupAddress().sin_addr.s_addr
		port.num() == dest->fGroupEId.portNum()) {
      return;
    }
  }
  
  _TRACE(TRACE_LOG_INFO, (char*)"createNewDestRecord #######  port: %d\n", port.num());
  fDests = createNewDestRecord(addr, port, 255, sessionId, fDests);
}

void Groupsock::removeDestination(unsigned sessionId) {
  // Default implementation:
  removeDestinationFrom(fDests, sessionId);
}

void Groupsock::removeAllDestinations() {
  delete fDests; fDests = NULL;
}

void Groupsock::multicastSendOnly() {
  // We disable this code for now, because - on some systems - leaving the multicast group seems to cause sent packets
  // to not be received by other applications (at least, on the same host).
#if 0
  socketLeaveGroup(env(), socketNum(), fIncomingGroupEId.groupAddress().s_addr);
  for (destRecord* dests = fDests; dests != NULL; dests = dests->fNext) {
    socketLeaveGroup(env(), socketNum(), dests->fGroupEId.groupAddress().s_addr);
  }
#endif
}

Boolean Groupsock::output(UsageEnvironment& env, unsigned char* buffer, unsigned bufferSize,
			  DirectedNetInterface* interfaceNotToFwdBackTo) {

	if (mFlag != LIVE_FLAG)			return True;

  do {
    // First, do the datagram send, to each destination:
    Boolean writeSuccess = True;
    for (destRecord* dests = fDests; dests != NULL; dests = dests->fNext) 
	{
		if (!write(dests->fGroupEId.groupAddress(), 
						dests->fGroupEId.portNum(), 
						dests->fGroupEId.ttl(),
						buffer, bufferSize)) 
		{
			writeSuccess = False;
			break;
      }
    }

	//_TRACE(TRACE_LOG_DEBUG, (char*)"********************\n");

    if (!writeSuccess) break;
    statsOutgoing.countPacket(bufferSize);
    statsGroupOutgoing.countPacket(bufferSize);

    // Then, forward to our members:
    int numMembers = 0;
    if (!members().IsEmpty()) {
      numMembers =
	outputToAllMembersExcept(interfaceNotToFwdBackTo,
				 ttl(), buffer, bufferSize,
				 ourIPAddress(env, socketIPVer()));
      if (numMembers < 0) break;
    }

    if (DebugLevel >= 3) {
      env << *this << ": wrote " << bufferSize << " bytes, ttl " << (unsigned)ttl();
      if (numMembers > 0) {
	env << "; relayed to " << numMembers << " members";
      }
      env << "\n";
    }
    return True;
  } while (0);

  if (DebugLevel >= 0) { // this is a fatal error
    UsageEnvironment::MsgString msg = strDup(env.getResultMsg());
    env.setResultMsg("Groupsock write failed: ", msg);
    delete[] (char*)msg;
  }
  return False;
}

Boolean Groupsock::handleRead(unsigned char* buffer, unsigned bufferMaxSize,
			      unsigned& bytesRead,
			      LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddressAndPort) {
  // Read data from the socket, and relay it across any attached tunnels
  //##### later make this code more general - independent of tunnels

  bytesRead = 0;

  int maxBytesToRead = bufferMaxSize - TunnelEncapsulationTrailerMaxSize;
  int numBytes = readSocket(env(), socketNum(),
			    buffer, maxBytesToRead, fromAddressAndPort);
  if (numBytes < 0) {
    if (DebugLevel >= 0) { // this is a fatal error
      UsageEnvironment::MsgString msg = strDup(env().getResultMsg());
      env().setResultMsg("Groupsock read failed: ", msg);
      delete[] (char*)msg;
    }
    return False;
  }

  // If we're a SSM group, make sure the source address matches:

	if (socketIPVer() == LIVE_IP_VER_4)
	{
	  if (isSSM()
		  && fromAddressAndPort.saddr4.sin_addr.s_addr != sourceFilterAddress().sin_addr.s_addr) {
		return True;
	  }
	}
	else
	{
	  if (isSSM()
		  && (0!=memcmp(&(fromAddressAndPort.saddr6.sin6_addr), &(sourceFilterAddress().sin6_addr), sizeof(fromAddressAndPort.saddr6.sin6_addr)))) {
		return True;
	  }
	}

  // We'll handle this data.
  // Also write it (with the encapsulation trailer) to each member,
  // unless the packet was originally sent by us to begin with.
  bytesRead = numBytes;

  int numMembers = 0;
  if (!wasLoopedBackFromUs(env(), fromAddressAndPort, socketIPVer())) {
    statsIncoming.countPacket(numBytes);
    statsGroupIncoming.countPacket(numBytes);

	LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	memcpy(&tmpInAddr.sin_addr, &fromAddressAndPort.saddr4.sin_addr, sizeof(tmpInAddr.sin_addr));
	memcpy(&tmpInAddr.sin6_addr, &fromAddressAndPort.saddr6.sin6_addr, sizeof(tmpInAddr.sin6_addr));

    numMembers =
      outputToAllMembersExcept(NULL, ttl(),
			       buffer, bytesRead,
			       tmpInAddr);
    if (numMembers > 0) {
      statsRelayedIncoming.countPacket(numBytes);
      statsGroupRelayedIncoming.countPacket(numBytes);
    }
  }
  if (DebugLevel >= 3) {
	  env() << *this << ": read " << bytesRead << " bytes from " << AddressString(fromAddressAndPort.saddr4.sin_addr).val() << ", port " << ntohs(fromAddressAndPort.saddr4.sin_port);
    if (numMembers > 0) {
      env() << "; relayed to " << numMembers << " members";
    }
    env() << "\n";
  }

  return True;
}

Boolean Groupsock::wasLoopedBackFromUs(UsageEnvironment& env,
				       LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddressAndPort,LIVE_IP_VER_ENUM ipVer) {

	LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	LIVE_NET_ADDRESS_INADDR pInAddr = ourIPAddress(env, ipVer);
	memcpy(&tmpInAddr, &pInAddr, sizeof(LIVE_NET_ADDRESS_INADDR));

	if (ipVer == LIVE_IP_VER_4)
	{
			if ( (0 == memcmp(&fromAddressAndPort.saddr4.sin_addr, &tmpInAddr.sin_addr, sizeof(tmpInAddr.sin_addr))) ||
				(fromAddressAndPort.saddr4.sin_addr.s_addr == 0x7F000001/*127.0.0.1*/) ) {
			if (fromAddressAndPort.saddr4.sin_port == sourcePortNum()) {
		#ifdef DEBUG_LOOPBACK_CHECKING
			  if (DebugLevel >= 3) {
			env() << *this << ": got looped-back packet\n";
			  }
		#endif
			  return True;
			}
		  }
	}
	else
	{
			if ( 0 == memcmp(&fromAddressAndPort.saddr6.sin6_addr, &tmpInAddr.sin6_addr, sizeof(tmpInAddr.sin6_addr)))
				 {
			if (fromAddressAndPort.saddr6.sin6_port == sourcePortNum()) {
		#ifdef DEBUG_LOOPBACK_CHECKING
			  if (DebugLevel >= 3) {
			env() << *this << ": got looped-back packet\n";
			  }
		#endif
			  return True;
			}
		  }
	}
  return False;
}

destRecord* Groupsock
::lookupDestRecordFromDestination(LIVE_NET_ADDRESS_SOCKADDR_IN const& destAddrAndPort) const {

	if (socketIPVer() == LIVE_IP_VER_4)
	{
	  for (destRecord* dest = fDests; dest != NULL; dest = dest->fNext) {
		if (destAddrAndPort.saddr4.sin_addr.s_addr == dest->fGroupEId.groupAddress().sin_addr.s_addr
		&& destAddrAndPort.saddr4.sin_port == dest->fGroupEId.portNum()) {
		  return dest;
		}
	  }
	}
	else		//IPV6
	{
	  for (destRecord* dest = fDests; dest != NULL; dest = dest->fNext) {
		if (0==memcmp(&(destAddrAndPort.saddr6.sin6_addr), &(dest->fGroupEId.groupAddress().sin6_addr), sizeof(destAddrAndPort.saddr6.sin6_addr))
		&& destAddrAndPort.saddr4.sin_port == dest->fGroupEId.portNum()) {
		  return dest;
		}
	  }

	}
	return NULL;
}

void Groupsock::removeDestinationFrom(destRecord*& dests, unsigned sessionId) {
  destRecord** destsPtr = &dests;
  while (*destsPtr != NULL) {
    if (sessionId == (*destsPtr)->fSessionId) {
      // Remove the record pointed to by *destsPtr :
      destRecord* next = (*destsPtr)->fNext;
      (*destsPtr)->fNext = NULL;
      delete (*destsPtr);
      *destsPtr = next;
    } else {
      destsPtr = &((*destsPtr)->fNext);
    }
  }
}

int Groupsock::outputToAllMembersExcept(DirectedNetInterface* exceptInterface,
					u_int8_t ttlToFwd,
					unsigned char* data, unsigned size,
					LIVE_NET_ADDRESS_INADDR sourceAddr) {
  // Don't forward TTL-0 packets
  if (ttlToFwd == 0) return 0;

  DirectedNetInterfaceSet::Iterator iter(members());
  unsigned numMembers = 0;
  DirectedNetInterface* interf;
  while ((interf = iter.next()) != NULL) {
    // Check whether we've asked to exclude this interface:
    if (interf == exceptInterface)
      continue;

    // Check that the packet's source address makes it OK to
    // be relayed across this interface:
    UsageEnvironment& saveEnv = env();
    // because the following call may delete "this"
    if (!interf->SourceAddrOKForRelaying(saveEnv, sourceAddr)) {
      if (strcmp(saveEnv.getResultMsg(), "") != 0) {
				// Treat this as a fatal error
	return -1;
      } else {
	continue;
      }
    }

    if (numMembers == 0) {
      // We know that we're going to forward to at least one
      // member, so fill in the tunnel encapsulation trailer.
      // (Note: Allow for it not being 4-byte-aligned.)
      TunnelEncapsulationTrailer* trailerInPacket
	= (TunnelEncapsulationTrailer*)&data[size];
      TunnelEncapsulationTrailer* trailer;

      Boolean misaligned = ((uintptr_t)trailerInPacket & 3) != 0;
      unsigned trailerOffset;
      u_int8_t tunnelCmd;
      if (isSSM()) {
	// add an 'auxilliary address' before the trailer
	trailerOffset = TunnelEncapsulationTrailerAuxSize;
	tunnelCmd = TunnelDataAuxCmd;
      } else {
	trailerOffset = 0;
	tunnelCmd = TunnelDataCmd;
      }
      unsigned trailerSize = TunnelEncapsulationTrailerSize + trailerOffset;
      unsigned tmpTr[TunnelEncapsulationTrailerMaxSize];
      if (misaligned) {
	trailer = (TunnelEncapsulationTrailer*)&tmpTr;
      } else {
	trailer = trailerInPacket;
      }
      trailer += trailerOffset;

      if (fDests != NULL) {
	trailer->address() = fDests->fGroupEId.groupAddress().sin_addr.s_addr;		//gavin 2018.03.21  ??yDT??  o????Y???：??|━?
	Port destPort(ntohs(fDests->fGroupEId.portNum()));
	trailer->port() = destPort; // structure copy
      }
      trailer->ttl() = ttlToFwd;
      trailer->command() = tunnelCmd;

      if (isSSM()) {
	trailer->auxAddress() = sourceFilterAddress().sin_addr.s_addr;				//gavin 2018.03.21  ??yDT??  o????Y???：??|━?
      }

      if (misaligned) {
	memmove(trailerInPacket, trailer-trailerOffset, trailerSize);
      }

      size += trailerSize;
    }

    interf->write(data, size);
    ++numMembers;
  }

  return numMembers;
}

UsageEnvironment& operator<<(UsageEnvironment& s, const Groupsock& g) {
  UsageEnvironment& s1 = s << timestampString() << " Groupsock("
			   << g.socketNum() << ": "
			   << AddressString(g.groupAddress(), g.socketIPVer()).val()
			   << ", " << g.port() << ", ";
  if (g.isSSM()) {
    return s1 << "SSM source: "
		<<  AddressString(g.sourceFilterAddress(), g.socketIPVer()).val() << ")";
  } else {
    return s1 << (unsigned)(g.ttl()) << ")";
  }
}


////////// GroupsockLookupTable //////////


// A hash table used to index Groupsocks by socket number.

static HashTable*& getSocketTable(UsageEnvironment& env) {
  _groupsockPriv* priv = groupsockPriv(env);
  if (priv->socketTable == NULL) { // We need to create it
    priv->socketTable = HashTable::create(ONE_WORD_HASH_KEYS);
  }
  return priv->socketTable;
}

static Boolean unsetGroupsockBySocket(Groupsock const* groupsock) {
  do {
    if (groupsock == NULL) break;

    int sock = groupsock->socketNum();
    // Make sure "sock" is in bounds:
    if (sock < 0) break;

    HashTable*& sockets = getSocketTable(groupsock->env());

    Groupsock* gs = (Groupsock*)sockets->Lookup((char*)(long)sock);
    if (gs == NULL || gs != groupsock) break;
    sockets->Remove((char*)(long)sock);

    if (sockets->IsEmpty()) {
      // We can also delete the table (to reclaim space):
      delete sockets; sockets = NULL;
      reclaimGroupsockPriv(gs->env());
    }

    return True;
  } while (0);

  return False;
}

static Boolean setGroupsockBySocket(UsageEnvironment& env, int sock,
				    Groupsock* groupsock) {
  do {
    // Make sure the "sock" parameter is in bounds:
    if (sock < 0) {
      char buf[100];
      sprintf(buf, "trying to use bad socket (%d)", sock);
      env.setResultMsg(buf);
      break;
    }

    HashTable* sockets = getSocketTable(env);

    // Make sure we're not replacing an existing Groupsock (although that shouldn't happen)
    Boolean alreadyExists
      = (sockets->Lookup((char*)(long)sock) != 0);
    if (alreadyExists) {
      char buf[100];
      sprintf(buf, "Attempting to replace an existing socket (%d)", sock);
      env.setResultMsg(buf);
      break;
    }

    sockets->Add((char*)(long)sock, groupsock);
    return True;
  } while (0);

  return False;
}

static Groupsock* getGroupsockBySocket(UsageEnvironment& env, int sock) {
  do {
    // Make sure the "sock" parameter is in bounds:
    if (sock < 0) break;

    HashTable* sockets = getSocketTable(env);
    return (Groupsock*)sockets->Lookup((char*)(long)sock);
  } while (0);

  return NULL;
}

Groupsock*
GroupsockLookupTable::Fetch(UsageEnvironment& env,
			    LIVE_NET_ADDRESS_INADDR groupAddress,
			    Port port, u_int8_t ttl,
			    Boolean& isNew) {
  isNew = False;
  Groupsock* groupsock;
  do {
	  //gavin 2018.03.19
	  LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	  memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));

	const char *buff = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"; 
	inet_pton(AF_INET6, buff, &tmpInAddr.sin6_addr); 

    groupsock = (Groupsock*) fTable.Lookup(groupAddress, tmpInAddr, port, LIVE_IP_VER_4);
    if (groupsock == NULL) { // we need to create one:
      groupsock = AddNew(env, groupAddress, tmpInAddr, port, ttl, LIVE_IP_VER_4);
      if (groupsock == NULL) break;
      isNew = True;
    }
  } while (0);

  return groupsock;
}

Groupsock*
GroupsockLookupTable::Fetch(UsageEnvironment& env,
			    LIVE_NET_ADDRESS_INADDR groupAddress,
			    LIVE_NET_ADDRESS_INADDR sourceFilterAddr, Port port,
			    Boolean& isNew) {
  isNew = False;
  Groupsock* groupsock;
  do {
    groupsock
      = (Groupsock*) fTable.Lookup(groupAddress, sourceFilterAddr, port, LIVE_IP_VER_4);
    if (groupsock == NULL) { // we need to create one:
      groupsock = AddNew(env, groupAddress, sourceFilterAddr, port, 0, LIVE_IP_VER_4);
      if (groupsock == NULL) break;
      isNew = True;
    }
  } while (0);

  return groupsock;
}

Groupsock*
GroupsockLookupTable::Lookup(LIVE_NET_ADDRESS_INADDR groupAddress, Port port, LIVE_IP_VER_ENUM ipVer) {

	  LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	  memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));

	const char *buff = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"; 
	inet_pton(AF_INET6, buff, &tmpInAddr.sin6_addr); 

  return (Groupsock*) fTable.Lookup(groupAddress, tmpInAddr, port, ipVer);
}

Groupsock*
GroupsockLookupTable::Lookup(LIVE_NET_ADDRESS_INADDR groupAddress,
			     LIVE_NET_ADDRESS_INADDR sourceFilterAddr, Port port, LIVE_IP_VER_ENUM ipVer) {
  return (Groupsock*) fTable.Lookup(groupAddress, sourceFilterAddr, port, ipVer);
}

Groupsock* GroupsockLookupTable::Lookup(UsageEnvironment& env, int sock) {
  return getGroupsockBySocket(env, sock);
}

Boolean GroupsockLookupTable::Remove(Groupsock const* groupsock) {
  unsetGroupsockBySocket(groupsock);

  return fTable.Remove(groupsock->groupAddress(),
		       groupsock->sourceFilterAddress(),
		       groupsock->port(),
			   groupsock->socketIPVer());
}

Groupsock* GroupsockLookupTable::AddNew(UsageEnvironment& env,
					LIVE_NET_ADDRESS_INADDR groupAddress,
					LIVE_NET_ADDRESS_INADDR sourceFilterAddress,
					Port port, u_int8_t ttl, LIVE_IP_VER_ENUM ipVer) {
  Groupsock* groupsock;
  do {
    LIVE_NET_ADDRESS_INADDR groupAddr;
	//groupAddr.sin_addr.s_addr = groupAddress;
	memcpy(&groupAddr, &groupAddress, sizeof(LIVE_NET_ADDRESS_INADDR));
    //if (sourceFilterAddress == LIVE_NET_ADDRESS_INADDR(~0)) {
	if (0 == memcmp(&groupAddr, &sourceFilterAddress, sizeof(LIVE_NET_ADDRESS_INADDR))) {
      // regular, ISM groupsock
      groupsock = new Groupsock(env, groupAddr, port, ttl, 0, ipVer);
    } else {
      // SSM groupsock
      LIVE_NET_ADDRESS_INADDR sourceFilterAddr;
      //sourceFilterAddr.sin_addr.s_addr = sourceFilterAddress;
	  memcpy(&sourceFilterAddr, &sourceFilterAddress, sizeof(LIVE_NET_ADDRESS_INADDR));
      groupsock = new Groupsock(env, groupAddr, sourceFilterAddr, port, 0, ipVer);
    }

    if (groupsock == NULL || groupsock->socketNum() < 0) break;

    if (!setGroupsockBySocket(env, groupsock->socketNum(), groupsock)) break;

    fTable.Add(groupAddress, sourceFilterAddress, port, (void*)groupsock, ipVer);
  } while (0);

  return groupsock;
}

GroupsockLookupTable::Iterator::Iterator(GroupsockLookupTable& groupsocks)
  : fIter(AddressPortLookupTable::Iterator(groupsocks.fTable)) {
}

Groupsock* GroupsockLookupTable::Iterator::next() {
  return (Groupsock*) fIter.next();
};
