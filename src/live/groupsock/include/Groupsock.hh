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
// 'Group sockets'
// C++ header

#ifndef _GROUPSOCK_HH
#define _GROUPSOCK_HH

#ifndef _GROUPSOCK_VERSION_HH
#include "groupsock_version.hh"
#endif

#ifndef _NET_INTERFACE_HH
#include "NetInterface.hh"
#endif

#ifndef _GROUPEID_HH
#include "GroupEId.hh"
#endif

#include "../../../live_ipv4_ipv6.h"

// An "OutputSocket" is (by default) used only to send packets.
// No packets are received on it (unless a subclass arranges this)

class OutputSocket: public Socket {
public:
  OutputSocket(UsageEnvironment& env, int clientSock, LIVE_IP_VER_ENUM ipVer);
  virtual ~OutputSocket();

  virtual Boolean write(LIVE_NET_ADDRESS_INADDR address, portNumBits portNum/*in network order*/, u_int8_t ttl,
			unsigned char* buffer, unsigned bufferSize);
  Boolean write(LIVE_NET_ADDRESS_SOCKADDR_IN& addressAndPort, u_int8_t ttl,
		unsigned char* buffer, unsigned bufferSize) {


			LIVE_NET_ADDRESS_INADDR inAddr;
			memset(&inAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
			

	if (socketIPVer() == LIVE_IP_VER_4)
	{
		memcpy(&inAddr.sin_addr, &addressAndPort.saddr4.sin_addr, sizeof(inAddr.sin_addr));
		return write(inAddr, addressAndPort.saddr4.sin_port, ttl, buffer, bufferSize);
	}
	else
	{
		memcpy(&inAddr.sin6_addr, &addressAndPort.saddr6.sin6_addr, sizeof(inAddr.sin6_addr));
		return write(inAddr, addressAndPort.saddr6.sin6_port, ttl, buffer, bufferSize);		//gavin ??yDT??  2018.03.15
	}
  }

protected:
  OutputSocket(UsageEnvironment& env, Port port, int clientSock, LIVE_IP_VER_ENUM ipVer);

  portNumBits sourcePortNum() const {return fSourcePort.num();}

private: // redefined virtual function
  virtual Boolean handleRead(unsigned char* buffer, unsigned bufferMaxSize,
			     unsigned& bytesRead,
			     LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddressAndPort);

private:
  Port fSourcePort;
  unsigned fLastSentTTL;
};

class destRecord {
public:
  destRecord(LIVE_NET_ADDRESS_INADDR const& addr, Port const& port, u_int8_t ttl, unsigned sessionId,
	     destRecord* next);
  virtual ~destRecord();

public:
  destRecord* fNext;
  GroupEId fGroupEId;
  unsigned fSessionId;
};

// A "Groupsock" is used to both send and receive packets.
// As the name suggests, it was originally designed to send/receive
// multicast, but it can send/receive unicast as well.

class Groupsock: public OutputSocket {
public:
  Groupsock(UsageEnvironment& env, LIVE_NET_ADDRESS_INADDR const& groupAddr,
	    Port port, u_int8_t ttl, int clientSock, LIVE_IP_VER_ENUM ipVer);
      // used for a 'source-independent multicast' group
  Groupsock(UsageEnvironment& env, LIVE_NET_ADDRESS_INADDR const& groupAddr,
	    LIVE_NET_ADDRESS_INADDR const& sourceFilterAddr,
	    Port port, int clientSock, LIVE_IP_VER_ENUM ipVer);
      // used for a 'source-specific multicast' group
  virtual ~Groupsock();

  virtual destRecord* createNewDestRecord(LIVE_NET_ADDRESS_INADDR const& addr, Port const& port, u_int8_t ttl, unsigned sessionId, destRecord* next);
      // Can be redefined by subclasses that also subclass "destRecord"

  void changeDestinationParameters(LIVE_NET_ADDRESS_INADDR const& newDestAddr,
				   Port newDestPort, int newDestTTL,
				   unsigned sessionId = 0);
      // By default, the destination address, port and ttl for
      // outgoing packets are those that were specified in
      // the constructor.  This works OK for multicast sockets,
      // but for unicast we usually want the destination port
      // number, at least, to be different from the source port.
      // (If a parameter is 0 (or ~0 for ttl), then no change is made to that parameter.)
      // (If no existing "destRecord" exists with this "sessionId", then we add a new "destRecord".)
  unsigned lookupSessionIdFromDestination(LIVE_NET_ADDRESS_SOCKADDR_IN const& destAddrAndPort) const;
      // returns 0 if not found

  // As a special case, we also allow multiple destinations (addresses & ports)
  // (This can be used to implement multi-unicast.)
  virtual void addDestination(LIVE_NET_ADDRESS_INADDR const& addr, Port const& port, unsigned sessionId);
  virtual void removeDestination(unsigned sessionId);
  void removeAllDestinations();
  Boolean hasMultipleDestinations() const { return fDests != NULL && fDests->fNext != NULL; }

  LIVE_NET_ADDRESS_INADDR const& groupAddress() const {
    return fIncomingGroupEId.groupAddress();
  }
  LIVE_NET_ADDRESS_INADDR const& sourceFilterAddress() const {
    return fIncomingGroupEId.sourceFilterAddress();
  }

  Boolean isSSM() const {
    return fIncomingGroupEId.isSSM();
  }

  u_int8_t ttl() const { return fIncomingGroupEId.ttl(); }

  void multicastSendOnly(); // send, but don't receive any multicast packets

  virtual Boolean output(UsageEnvironment& env, unsigned char* buffer, unsigned bufferSize,
			 DirectedNetInterface* interfaceNotToFwdBackTo = NULL);

  DirectedNetInterfaceSet& members() { return fMembers; }

  Boolean deleteIfNoMembers;
  Boolean isSlave; // for tunneling

  static NetInterfaceTrafficStats statsIncoming;
  static NetInterfaceTrafficStats statsOutgoing;
  static NetInterfaceTrafficStats statsRelayedIncoming;
  static NetInterfaceTrafficStats statsRelayedOutgoing;
  NetInterfaceTrafficStats statsGroupIncoming; // *not* static
  NetInterfaceTrafficStats statsGroupOutgoing; // *not* static
  NetInterfaceTrafficStats statsGroupRelayedIncoming; // *not* static
  NetInterfaceTrafficStats statsGroupRelayedOutgoing; // *not* static

  Boolean wasLoopedBackFromUs(UsageEnvironment& env, LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddressAndPort,LIVE_IP_VER_ENUM ipVer);

public: // redefined virtual functions
  virtual Boolean handleRead(unsigned char* buffer, unsigned bufferMaxSize,
			     unsigned& bytesRead,
			     LIVE_NET_ADDRESS_SOCKADDR_IN& fromAddressAndPort);

protected:
  destRecord* lookupDestRecordFromDestination(LIVE_NET_ADDRESS_SOCKADDR_IN const& destAddrAndPort) const;

private:
  void removeDestinationFrom(destRecord*& dests, unsigned sessionId);
    // used to implement (the public) "removeDestination()", and "changeDestinationParameters()"
  int outputToAllMembersExcept(DirectedNetInterface* exceptInterface,
			       u_int8_t ttlToFwd,
			       unsigned char* data, unsigned size,
			       LIVE_NET_ADDRESS_INADDR sourceAddr);

protected:
  destRecord* fDests;
private:
  GroupEId fIncomingGroupEId;
  DirectedNetInterfaceSet fMembers;

  unsigned int	mFlag;
};

UsageEnvironment& operator<<(UsageEnvironment& s, const Groupsock& g);

// A data structure for looking up a 'groupsock'
// by (multicast address, port), or by socket number
class GroupsockLookupTable {
public:
  Groupsock* Fetch(UsageEnvironment& env, LIVE_NET_ADDRESS_INADDR groupAddress,
		   Port port, u_int8_t ttl, Boolean& isNew);
      // Creates a new Groupsock if none already exists
  Groupsock* Fetch(UsageEnvironment& env, LIVE_NET_ADDRESS_INADDR groupAddress,
		   LIVE_NET_ADDRESS_INADDR sourceFilterAddr,
		   Port port, Boolean& isNew);
      // Creates a new Groupsock if none already exists
  Groupsock* Lookup(LIVE_NET_ADDRESS_INADDR groupAddress, Port port, LIVE_IP_VER_ENUM ipVer);
      // Returns NULL if none already exists
  Groupsock* Lookup(LIVE_NET_ADDRESS_INADDR groupAddress,
		    LIVE_NET_ADDRESS_INADDR sourceFilterAddr,
		    Port port, LIVE_IP_VER_ENUM ipVer);
      // Returns NULL if none already exists
  Groupsock* Lookup(UsageEnvironment& env, int sock);
      // Returns NULL if none already exists
  Boolean Remove(Groupsock const* groupsock);

  // Used to iterate through the groupsocks in the table
  class Iterator {
  public:
    Iterator(GroupsockLookupTable& groupsocks);

    Groupsock* next(); // NULL iff none

  private:
    AddressPortLookupTable::Iterator fIter;
  };

private:
  Groupsock* AddNew(UsageEnvironment& env,
		    LIVE_NET_ADDRESS_INADDR groupAddress,
		    LIVE_NET_ADDRESS_INADDR sourceFilterAddress,
		    Port port, u_int8_t ttl, LIVE_IP_VER_ENUM ipVer);

private:
  friend class Iterator;
  AddressPortLookupTable fTable;
};

#endif
