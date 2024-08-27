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
// Copyright (c) 1996-2018, Live Networks, Inc.  All rights reserved
// "Group Endpoint Id"
// Implementation

#include "GroupEId.hh"
#include "../../live_ipv4_ipv6.h"

GroupEId::GroupEId(LIVE_NET_ADDRESS_INADDR const& groupAddr,
		   portNumBits portNum, u_int8_t ttl) {
  LIVE_NET_ADDRESS_INADDR sourceFilterAddr;
  sourceFilterAddr.sin_addr.s_addr = ~0; // indicates no source filter

  init(groupAddr, sourceFilterAddr, portNum, ttl);
}

GroupEId::GroupEId(LIVE_NET_ADDRESS_INADDR const& groupAddr,
		   LIVE_NET_ADDRESS_INADDR const& sourceFilterAddr,
		   portNumBits portNum) {
  init(groupAddr, sourceFilterAddr, portNum, 255);
}

Boolean GroupEId::isSSM() const {

	LIVE_NET_ADDRESS_INADDR	tmpInAddr;

	if (0 != memcmp(&tmpInAddr, &fSourceFilterAddress, sizeof(LIVE_NET_ADDRESS_INADDR)))		return True;
	//const char *buff = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
	//inet_pton(AF_INET6, buff, &tmpInAddr.sin6_addr);
	//if (0 != memcmp(&tmpInAddr, &fSourceFilterAddress, sizeof(LIVE_NET_ADDRESS_INADDR)))		return True;

	if (live_ip_ver == LIVE_IP_VER_4)
	{
		//return fSourceFilterAddress.sin_addr.s_addr != LIVE_NET_ADDRESS_INADDR(~0);
	}

	//return fSourceFilterAddress.sin6_addr.s_addr != LIVE_NET_ADDRESS_INADDR(~0);
	return False;
}

void GroupEId::init(LIVE_NET_ADDRESS_INADDR const& groupAddr,
		    LIVE_NET_ADDRESS_INADDR const& sourceFilterAddr,
		    portNumBits portNum,
		    u_int8_t ttl) {
  fGroupAddress = groupAddr;
  fSourceFilterAddress = sourceFilterAddr;
  fPortNum = portNum;
  fTTL = ttl;
}
