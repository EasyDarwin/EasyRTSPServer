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
// Network Addresses
// C++ header

#ifndef _NET_ADDRESS_HH
#define _NET_ADDRESS_HH

#ifndef _HASH_TABLE_HH
#include "HashTable.hh"
#endif

#ifndef _NET_COMMON_H
#include "NetCommon.h"
#endif

#ifndef _USAGE_ENVIRONMENT_HH
#include "UsageEnvironment.hh"
#endif

#include "../../../live_ipv4_ipv6.h"


#ifndef IPV6_ADD_SOURCE_MEMBERSHIP 
 
#ifdef LINUX 
#define IPV6_ADD_SOURCE_MEMBERSHIP   39 
#define IPV6_DROP_SOURCE_MEMBERSHIP 40 
#else 
#define IPV6_ADD_SOURCE_MEMBERSHIP   25 
#define IPV6_DROP_SOURCE_MEMBERSHIP 26 
#endif 
#endif

// Definition of a type representing a low-level network address.
// At present, this is 32-bits, for IPv4.  Later, generalize it,
// to allow for IPv6.

typedef struct __LIVE_NET_ADDRESS_INADDR
{
	struct in_addr sin_addr;
	struct in6_addr sin6_addr;

	__LIVE_NET_ADDRESS_INADDR()
	{
		sin_addr.s_addr = INADDR_ANY;
		sin6_addr = in6addr_any;
	};

}LIVE_NET_ADDRESS_INADDR;

typedef struct __LIVE_NET_ADDRESS_SOCKADDR_IN
{
	struct sockaddr_in	saddr4;
	struct sockaddr_in6	saddr6;

	//LIVE_NET_ADDRESS_INADDR	inaddr;
}LIVE_NET_ADDRESS_SOCKADDR_IN;

//typedef u_int32_t netAddressBits;

//typedef LIVE_NET_ADDRESS_INADDR netAddressBits;


class NetAddress {
public:
  NetAddress(u_int8_t const* data,
	     unsigned length = 4 /* default: 32 bits */);
  NetAddress(unsigned length = 4); // sets address data to all-zeros
  NetAddress(NetAddress const& orig);
  NetAddress& operator=(NetAddress const& rightSide);
  virtual ~NetAddress();
  
  unsigned length() const { return fLength; }
  u_int8_t const* data() const // always in network byte order
  { return fData; }
  
private:
  void assign(u_int8_t const* data, unsigned length);
  void clean();
  
  unsigned fLength;
  u_int8_t* fData;
};

class NetAddressList {
public:
  NetAddressList(char const* hostname, LIVE_IP_VER_ENUM ipVer);
  NetAddressList(NetAddressList const& orig);
  NetAddressList& operator=(NetAddressList const& rightSide);
  virtual ~NetAddressList();
  
  unsigned numAddresses() const { return fNumAddresses; }
  
  NetAddress const* firstAddress() const;
  
  // Used to iterate through the addresses in a list:
  class Iterator {
  public:
    Iterator(NetAddressList const& addressList);
    NetAddress const* nextAddress(); // NULL iff none
  private:
    NetAddressList const& fAddressList;
    unsigned fNextIndex;
  };
  
private:
  void assign(unsigned numAddresses, NetAddress** addressArray);
  void clean();
  
  friend class Iterator;
  unsigned fNumAddresses;
  NetAddress** fAddressArray;
};

typedef u_int16_t portNumBits;

class Port {
public:
  Port(portNumBits num /* in host byte order */);
  
  portNumBits num() const { return fPortNum; } // in network byte order
  
private:
  portNumBits fPortNum; // stored in network byte order
#ifdef IRIX
  portNumBits filler; // hack to overcome a bug in IRIX C++ compiler
#endif
};

UsageEnvironment& operator<<(UsageEnvironment& s, const Port& p);


// A generic table for looking up objects by (address1, address2, port)
class AddressPortLookupTable {
public:
  AddressPortLookupTable();
  virtual ~AddressPortLookupTable();
  
  void* Add(LIVE_NET_ADDRESS_INADDR address1, LIVE_NET_ADDRESS_INADDR address2, Port port, void* value, LIVE_IP_VER_ENUM ipVer);
      // Returns the old value if different, otherwise 0
  Boolean Remove(LIVE_NET_ADDRESS_INADDR address1, LIVE_NET_ADDRESS_INADDR address2, Port port, LIVE_IP_VER_ENUM ipVer);
  void* Lookup(LIVE_NET_ADDRESS_INADDR address1, LIVE_NET_ADDRESS_INADDR address2, Port port, LIVE_IP_VER_ENUM ipVer);
      // Returns 0 if not found
  void* RemoveNext() { return fTable->RemoveNext(); }

  // Used to iterate through the entries in the table
  class Iterator {
  public:
    Iterator(AddressPortLookupTable& table);
    virtual ~Iterator();
    
    void* next(); // NULL iff none
    
  private:
    HashTable::Iterator* fIter;
  };
  
private:
  friend class Iterator;
  HashTable* fTable;
};


Boolean IsMulticastAddress(LIVE_NET_ADDRESS_INADDR address, LIVE_IP_VER_ENUM ipVer);


// A mechanism for displaying an IPv4 address in ASCII.  This is intended to replace "inet_ntoa()", which is not thread-safe.
class AddressString {
public:
  AddressString(LIVE_NET_ADDRESS_SOCKADDR_IN const& addr, LIVE_IP_VER_ENUM ipVer);
  AddressString(LIVE_NET_ADDRESS_INADDR const& addr, LIVE_IP_VER_ENUM ipVer);
  AddressString(struct in_addr const& addr);
  AddressString(struct in6_addr const& addr);
  AddressString(char *ipAddrStr);

  virtual ~AddressString();

  char const* val() const { return fVal; }

private:
  void init(struct in_addr addr); // used to implement each of the constructors
  void init6(struct in6_addr);
private:
  char* fVal; // The result ASCII string: allocated by the constructor; deleted by the destructor
};

#endif
