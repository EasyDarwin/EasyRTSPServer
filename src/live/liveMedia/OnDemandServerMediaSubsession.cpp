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
// "liveMedia"
// Copyright (c) 1996-2018 Live Networks, Inc.  All rights reserved.
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand.
// Implementation

#include "OnDemandServerMediaSubsession.hh"
#include <GroupsockHelper.hh>
#include "../../trace.h"

OnDemandServerMediaSubsession
::OnDemandServerMediaSubsession(UsageEnvironment& env,
				Boolean reuseFirstSource,
				char *streamName, char *mediaTypeName,
				portNumBits initialPortNum,
				Boolean multiplexRTCPWithRTP)
  : ServerMediaSubsession(env),
    fSDPLines(NULL), fReuseFirstSource(reuseFirstSource), 
    fMultiplexRTCPWithRTP(multiplexRTCPWithRTP), fLastStreamToken(NULL),
    fAppHandlerTask(NULL), fAppHandlerClientData(NULL), fPauseStatus(False){
  fDestinationsHashTable = HashTable::create(ONE_WORD_HASH_KEYS);
  if (fMultiplexRTCPWithRTP) {
    fInitialPortNum = initialPortNum;
  } else {
    // Make sure RTP ports are even-numbered:
    fInitialPortNum = (initialPortNum+1)&~1;
  }
  gethostname(fCNAME, sizeof fCNAME);
  fCNAME[sizeof fCNAME-1] = '\0'; // just in case

  memset(mStreamName, 0x00, sizeof(mStreamName));
  strcpy(mStreamName, streamName);
  memset(mMediaTypeName, 0x00, sizeof(mMediaTypeName));
  strcpy(mMediaTypeName, mediaTypeName);

  //_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] OnDemandServerMediaSubsession::OnDemandServerMediaSubsession()   0x%X  mStreamName[%s] mediaType[%s]\n", env.GetEnvirName(), this, mStreamName, mMediaTypeName);

  tally = 0;
}

OnDemandServerMediaSubsession::~OnDemandServerMediaSubsession() {
  delete[] fSDPLines;

  // Clean out the destinations hash table:
  while (1) {
    Destinations* destinations
      = (Destinations*)(fDestinationsHashTable->RemoveNext());
    if (destinations == NULL) break;
    delete destinations;
  }
  delete fDestinationsHashTable;

  _TRACE(TRACE_LOG_DEBUG, (char *)"[%s] OnDemandServerMediaSubsession::~OnDemandServerMediaSubsession() 0x%X mStreamName[%s] mediaType[%s]\n", this->envir().GetEnvirName(), this, mStreamName, mMediaTypeName);
}

char const*
OnDemandServerMediaSubsession::sdpLines(int clientSock, LIVE_IP_VER_ENUM ipVer) {
  if (fSDPLines == NULL) {
    // We need to construct a set of SDP lines that describe this
    // subsession (as a unicast stream).  To do so, we first create
    // dummy (unused) source and "RTPSink" objects,
    // whose parameters we use for the SDP lines:
    unsigned estBitrate;
    FramedSource* inputSource = createNewStreamSource(0, estBitrate);
    if (inputSource == NULL) return NULL; // file not found

	LIVE_NET_ADDRESS_INADDR dummyAddr;
	dummyAddr.sin_addr.s_addr = 0;

	//_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::sdpLines createGroupSock...\n");
    Groupsock* dummyGroupsock = createGroupsock(dummyAddr, 0, clientSock, ipVer);
    unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic
    RTPSink* dummyRTPSink = createNewRTPSink(dummyGroupsock, rtpPayloadType, inputSource);
    if (dummyRTPSink != NULL && dummyRTPSink->estimatedBitrate() > 0) estBitrate = dummyRTPSink->estimatedBitrate();

    setSDPLinesFromRTPSink(dummyRTPSink, inputSource, estBitrate, ipVer);
    Medium::close(dummyRTPSink);
    delete dummyGroupsock;
    closeStreamSource(inputSource);

	//_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::sdpLines deleteGroupSock...\n");
  }

  return fSDPLines;
}

void OnDemandServerMediaSubsession::getStreamParameters(UsageEnvironment *pEnv, unsigned clientSessionId,
		      LIVE_NET_ADDRESS_INADDR clientAddress,
		      Port const& clientRTPPort,
		      Port const& clientRTCPPort,
		      int tcpSocketNum,
		      unsigned char rtpChannelId,
		      unsigned char rtcpChannelId,
		      LIVE_NET_ADDRESS_INADDR& destinationAddress,
		      u_int8_t& /*destinationTTL*/,
		      Boolean& isMulticast,
		      Port& serverRTPPort,
		      Port& serverRTCPPort,
		      void*& streamToken, 
			  int clientSock, LIVE_IP_VER_ENUM ipVer, char *requestStr) {


	LIVE_NET_ADDRESS_INADDR	tmpInAddr;
	if (0 == memcmp(&tmpInAddr, &destinationAddress, sizeof(LIVE_NET_ADDRESS_INADDR)))
	{
		memcpy(&destinationAddress, &clientAddress, sizeof(LIVE_NET_ADDRESS_INADDR));
	}

	

	LIVE_NET_ADDRESS_INADDR destinationAddr;
	memcpy(&destinationAddr, &destinationAddress, sizeof(LIVE_NET_ADDRESS_INADDR));
  isMulticast = False;

  if (fLastStreamToken != NULL && fReuseFirstSource) {
    // Special case: Rather than creating a new 'StreamState',
    // we reuse the one that we've already created:
    serverRTPPort = ((StreamState*)fLastStreamToken)->serverRTPPort();
    serverRTCPPort = ((StreamState*)fLastStreamToken)->serverRTCPPort();
    ++((StreamState*)fLastStreamToken)->referenceCount();
    streamToken = fLastStreamToken;

	StreamState *pTmp = (StreamState *)streamToken;

	if (pTmp->GetEnvir() != pEnv)
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"#######  复用异常  OnDemandServerMediaSubsession::getStreamParameters ERROR.  StreamState: 0x%X\n", pTmp);
	}


  } else {
    // Normal case: Create a new media source:
    unsigned streamBitrate;
    FramedSource* mediaSource
      = createNewStreamSource(clientSessionId, streamBitrate);

    // Create 'groupsock' and 'sink' objects for the destination,
    // using previously unused server port numbers:
    RTPSink* rtpSink = NULL;
    BasicUDPSink* udpSink = NULL;
    Groupsock* rtpGroupsock = NULL;
    Groupsock* rtcpGroupsock = NULL;

    if (clientRTPPort.num() != 0 || tcpSocketNum >= 0) { // Normal case: Create destinations
      portNumBits serverPortNum;
      if (clientRTCPPort.num() == 0) {
	// We're streaming raw UDP (not RTP). Create a single groupsock:
	NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
	for (serverPortNum = fInitialPortNum; ; ++serverPortNum) {
  		LIVE_NET_ADDRESS_INADDR dummyAddr;
	  
	  serverRTPPort = serverPortNum;
	  rtpGroupsock = createGroupsock(dummyAddr, serverRTPPort, 0, ipVer);
	  if (rtpGroupsock->socketNum() >= 0) break; // success
	}

#ifdef _DEBUG
	  sockaddr_in test; test.sin_port = 0;
	  SOCKLEN_T len = sizeof test;
	  getpeername(clientSock, (struct sockaddr*)&test, &len);
	  char *p = inet_ntoa(test.sin_addr);
	  FILE *f = fopen("udpsink.txt", "wb");
	  if (NULL != f)
	  {
		  if (NULL != p)		fwrite(p, 1, (int)strlen(p), f);

		  if (NULL != requestStr)
		  {
			  fwrite(requestStr, 1, (int)strlen(requestStr), f);
		  }

		  char sztmp[128] = {0};
		  sprintf(sztmp, "clientSessionId: %d\n", clientSessionId);


		  fwrite(sztmp, 1, (int)strlen(sztmp), f);

		  fclose(f);
	  }
#endif

	udpSink = BasicUDPSink::createNew(envir(), rtpGroupsock);
      } else {
	// Normal case: We're streaming RTP (over UDP or TCP).  Create a pair of
	// groupsocks (RTP and RTCP), with adjacent port numbers (RTP port number even).
	// (If we're multiplexing RTCP and RTP over the same port number, it can be odd or even.)
	NoReuse dummy(envir()); // ensures that we skip over ports that are already in use
	for (portNumBits serverPortNum = fInitialPortNum; ; ++serverPortNum) {
	  LIVE_NET_ADDRESS_INADDR dummyAddr; dummyAddr.sin_addr.s_addr = 0;


	 // _TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::getStreamParameters createGroupSock[RTP]...\n");

	  serverRTPPort = serverPortNum;
	  rtpGroupsock = createGroupsock(dummyAddr, serverRTPPort, clientSock, ipVer);
	  if (rtpGroupsock->socketNum() < 0) {
	    delete rtpGroupsock;
		//_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::getStreamParameters createGroupSock[RTP] fail...\n");
	    continue; // try again
	  }

	  //_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::getStreamParameters createGroupSock[RTP] ok.. sock[%d]\n", rtpGroupsock->socketNum());

	  if (fMultiplexRTCPWithRTP) {
	    // Use the RTP 'groupsock' object for RTCP as well:
	    serverRTCPPort = serverRTPPort;
	    rtcpGroupsock = rtpGroupsock;
	  } else {
	    // Create a separate 'groupsock' object (with the next (odd) port number) for RTCP:

		  //_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::getStreamParameters createGroupSock[RTCP]...\n");

	    serverRTCPPort = ++serverPortNum;
	    rtcpGroupsock = createGroupsock(dummyAddr, serverRTCPPort, clientSock, ipVer);
	    if (rtcpGroupsock->socketNum() < 0) {
	      delete rtpGroupsock;
	      delete rtcpGroupsock;

		  //_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::getStreamParameters createGroupSock[RTCP] fail...\n");

	      continue; // try again
	    }

		//_TRACE(TRACE_LOG_DEBUG, (char *)"OnDemandServerMediaSubsession::getStreamParameters createGroupSock[RTCP] ok.. sock[%d]\n", rtcpGroupsock->socketNum());
	  }

	  break; // success
	}

	unsigned char rtpPayloadType = 96 + trackNumber()-1; // if dynamic
	rtpSink = createNewRTPSink(rtpGroupsock, rtpPayloadType, mediaSource);
	if (rtpSink != NULL && rtpSink->estimatedBitrate() > 0) streamBitrate = rtpSink->estimatedBitrate();
      }

      // Turn off the destinations for each groupsock.  They'll get set later
      // (unless TCP is used instead):
      if (rtpGroupsock != NULL) rtpGroupsock->removeAllDestinations();
      if (rtcpGroupsock != NULL) rtcpGroupsock->removeAllDestinations();

      if (rtpGroupsock != NULL) {
	// Try to use a big send buffer for RTP -  at least 0.1 second of
	// specified bandwidth and at least 50 KB
	unsigned rtpBufSize = streamBitrate * 25 / 2; // 1 kbps * 0.1 s = 12.5 bytes
	//if (rtpBufSize < 50 * 1024) rtpBufSize = 50 * 1024;
	if (rtpBufSize < 512 * 1024) rtpBufSize = 512 * 1024;
	increaseSendBufferTo(envir(), rtpGroupsock->socketNum(), rtpBufSize);
      }
    }

    // Set up the state of the stream.  The stream will get started later:
    streamToken = fLastStreamToken
      = new StreamState(pEnv, *this, serverRTPPort, serverRTCPPort, rtpSink, udpSink,
			streamBitrate, mediaSource,
			rtpGroupsock, rtcpGroupsock);

	_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] new streamToken: 0x%X\n", pEnv->GetEnvirName(), streamToken);

  }

  // Record these destinations as being for this client session id:
  Destinations* destinations;
  if (tcpSocketNum < 0) { // UDP
    destinations = new Destinations(destinationAddr, ipVer, clientRTPPort, clientRTCPPort);
  } else { // TCP
    destinations = new Destinations(tcpSocketNum, ipVer, rtpChannelId, rtcpChannelId);
  }

  fDestinationsHashTable->Add((char const*)clientSessionId, destinations);
}

void OnDemandServerMediaSubsession::startStream(UsageEnvironment& env, unsigned clientSessionId,
						void* streamToken,
						TaskFunc* rtcpRRHandler,
						void* rtcpRRHandlerClientData,
						unsigned short& rtpSeqNum,
						unsigned& rtpTimestamp,
						ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
						void* serverRequestAlternativeByteHandlerClientData) {

  StreamState* streamState = (StreamState*)streamToken;
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (streamState != NULL) {
    streamState->startPlaying(destinations, env, clientSessionId,
			      rtcpRRHandler, rtcpRRHandlerClientData,
			      serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
    RTPSink* rtpSink = streamState->rtpSink(); // alias
    if (rtpSink != NULL) {
      rtpSeqNum = rtpSink->currentSeqNo();
      rtpTimestamp = rtpSink->presetNextTimestamp();
    }

	if (fPauseStatus)
	{
		resumeStreamSource();
		fPauseStatus = False;
	}
  }

}

void	OnDemandServerMediaSubsession::pauseStreamSource()
{
	
}
void  OnDemandServerMediaSubsession::resumeStreamSource()
{
	
}

void OnDemandServerMediaSubsession::pauseStream(unsigned /*clientSessionId*/,
						void* streamToken) {
  // Pausing isn't allowed if multiple clients are receiving data from
  // the same source:
  //if (fReuseFirstSource) return;		//2017.12.13

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL) streamState->pause();

  pauseStreamSource();
  fPauseStatus = True;
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/,
					       void* streamToken, double& seekNPT, double streamDuration, u_int64_t& numBytes) {
  numBytes = 0; // by default: unknown

  // Seeking isn't allowed if multiple clients are receiving data from the same source:
  //if (fReuseFirstSource) return;		//2017.12.13

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL && streamState->mediaSource() != NULL) {
    seekStreamSource(streamState->mediaSource(), seekNPT, streamDuration, numBytes);

    streamState->startNPT() = (float)seekNPT;
    RTPSink* rtpSink = streamState->rtpSink(); // alias
    if (rtpSink != NULL) rtpSink->resetPresentationTimes();
  }
}

void OnDemandServerMediaSubsession::seekStream(unsigned /*clientSessionId*/,
					       void* streamToken, char*& absStart, char*& absEnd) {
  // Seeking isn't allowed if multiple clients are receiving data from the same source:
  //if (fReuseFirstSource) return;		//2017.12.13

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL && streamState->mediaSource() != NULL) {
    seekStreamSource(streamState->mediaSource(), absStart, absEnd);
  }
}

void OnDemandServerMediaSubsession::nullSeekStream(unsigned /*clientSessionId*/, void* streamToken,
						   double streamEndTime, u_int64_t& numBytes) {
  numBytes = 0; // by default: unknown

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL && streamState->mediaSource() != NULL) {
    // Because we're not seeking here, get the current NPT, and remember it as the new 'start' NPT:
    streamState->startNPT() = getCurrentNPT(streamToken);

    double duration = streamEndTime - streamState->startNPT();
    if (duration < 0.0) duration = 0.0;
    setStreamSourceDuration(streamState->mediaSource(), duration, numBytes);

    RTPSink* rtpSink = streamState->rtpSink(); // alias
    if (rtpSink != NULL) rtpSink->resetPresentationTimes();
  }
}

void OnDemandServerMediaSubsession::setStreamScale(unsigned /*clientSessionId*/,
						   void* streamToken, float scale) {
  // Changing the scale factor isn't allowed if multiple clients are receiving data
  // from the same source:
  //if (fReuseFirstSource) return;		//2017.12.13

  StreamState* streamState = (StreamState*)streamToken;
  if (streamState != NULL && streamState->mediaSource() != NULL) {
    setStreamSourceScale(streamState->mediaSource(), scale);
  }
}

float OnDemandServerMediaSubsession::getCurrentNPT(void* streamToken) {
  do {
    if (streamToken == NULL) break;

    StreamState* streamState = (StreamState*)streamToken;
    RTPSink* rtpSink = streamState->rtpSink();
    if (rtpSink == NULL) break;

    return streamState->startNPT()
      + (rtpSink->mostRecentPresentationTime().tv_sec - rtpSink->initialPresentationTime().tv_sec)
      + (rtpSink->mostRecentPresentationTime().tv_usec - rtpSink->initialPresentationTime().tv_usec)/1000000.0f;
  } while (0);

  return 0.0;
}

FramedSource* OnDemandServerMediaSubsession::getStreamSource(void* streamToken) {
  if (streamToken == NULL) return NULL;

  StreamState* streamState = (StreamState*)streamToken;
  return streamState->mediaSource();
}

void OnDemandServerMediaSubsession
::getRTPSinkandRTCP(void* streamToken,
		    RTPSink const*& rtpSink, RTCPInstance const*& rtcp) {
  if (streamToken == NULL) {
    rtpSink = NULL;
    rtcp = NULL;
    return;
  }

  StreamState* streamState = (StreamState*)streamToken;
  rtpSink = streamState->rtpSink();
  rtcp = streamState->rtcpInstance();
}

void OnDemandServerMediaSubsession::deleteStream(UsageEnvironment  *pEnv, unsigned clientSessionId,
						 void*& streamToken) 
{

//envir().LockEnvir("OnDemandServerMediaSubsession::deleteStream", (unsigned long long)this);
	
  StreamState* streamState = (StreamState*)streamToken;

	_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] OnDemandServerMediaSubsession::deleteStream: 0x%X  tally: %d   sessionId: %d\n", 
		(pEnv ? pEnv->GetEnvirName() : "pEnv为空"),
		streamState, ++tally, clientSessionId);

  // Look up (and remove) the destinations for this client session:
  Destinations* destinations
    = (Destinations*)(fDestinationsHashTable->Lookup((char const*)clientSessionId));
  if (destinations != NULL) {
    fDestinationsHashTable->Remove((char const*)clientSessionId);

    // Stop streaming to these destinations:
    if (streamState != NULL) streamState->endPlaying(destinations, clientSessionId);
  }

  // Delete the "StreamState" structure if it's no longer being used:
  if (streamState != NULL) {
    if (streamState->referenceCount() > 0) --streamState->referenceCount();
    if (streamState->referenceCount() == 0) {

		if (streamState)
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] OnDemandServerMediaSubsession::deleteStream: 0x%X    fLastStreamToken: 0x%X referenceCount=0, delete streamState\n", 
				(pEnv ? pEnv->GetEnvirName() : "pEnv = NULL"),
				streamState, fLastStreamToken);

			UsageEnvironment *pTmpEnv = streamState->GetEnvir();
			if (pTmpEnv != pEnv)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"############ ERROR  envir 错误  删除会导致异常.\n");
			}

		  delete streamState;
		}
		streamToken = NULL;
    }
	else
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] OnDemandServerMediaSubsession::deleteStream: 0x%X  referenceCount[%d]>0, no delete. \n", 
			(pEnv ? pEnv->GetEnvirName() : "pEnv = NULL"),
			streamState, streamState->referenceCount() );
	}
  }

  // Finally, delete the destinations themselves:
  delete destinations;
  destinations = NULL;

  //envir().UnlockEnvir("OnDemandServerMediaSubsession::deleteStream", (unsigned long long)this);
}

char const* OnDemandServerMediaSubsession
::getAuxSDPLine(RTPSink* rtpSink, FramedSource* /*inputSource*/) {
  // Default implementation:
  return rtpSink == NULL ? NULL : rtpSink->auxSDPLine();
}

void OnDemandServerMediaSubsession::seekStreamSource(FramedSource* /*inputSource*/,
						     double& /*seekNPT*/, double /*streamDuration*/, u_int64_t& numBytes) {
  // Default implementation: Do nothing
  numBytes = 0;
}

void OnDemandServerMediaSubsession::seekStreamSource(FramedSource* /*inputSource*/,
						     char*& absStart, char*& absEnd) {
  // Default implementation: do nothing (but delete[] and assign "absStart" and "absEnd" to NULL, to show that we don't handle this)
  delete[] absStart; absStart = NULL;
  delete[] absEnd; absEnd = NULL;
}

void OnDemandServerMediaSubsession
::setStreamSourceScale(FramedSource* /*inputSource*/, float /*scale*/) {
  // Default implementation: Do nothing
}

void OnDemandServerMediaSubsession
::setStreamSourceDuration(FramedSource* /*inputSource*/, double /*streamDuration*/, u_int64_t& numBytes) {
  // Default implementation: Do nothing
  numBytes = 0;
}

void OnDemandServerMediaSubsession::closeStreamSource(FramedSource *inputSource) {
  Medium::close(inputSource);
}

Groupsock* OnDemandServerMediaSubsession
::createGroupsock(LIVE_NET_ADDRESS_INADDR const& addr, Port port, int clientSock, LIVE_IP_VER_ENUM ipVer) {
  // Default implementation; may be redefined by subclasses:
  return new Groupsock(envir(), addr, port, 255, clientSock, ipVer);
}

RTCPInstance* OnDemandServerMediaSubsession
::createRTCP(Groupsock* RTCPgs, unsigned totSessionBW, /* in kbps */
	     unsigned char const* cname, RTPSink* sink) {
  // Default implementation; may be redefined by subclasses:
  return RTCPInstance::createNew(envir(), RTCPgs, totSessionBW, cname, sink, NULL/*we're a server*/);
}

void OnDemandServerMediaSubsession
::setRTCPAppPacketHandler(RTCPAppHandlerFunc* handler, void* clientData) {
  fAppHandlerTask = handler;
  fAppHandlerClientData = clientData;
}

void OnDemandServerMediaSubsession
::sendRTCPAppPacket(u_int8_t subtype, char const* name,
		    u_int8_t* appDependentData, unsigned appDependentDataSize) {
  StreamState* streamState = (StreamState*)fLastStreamToken;
  if (streamState != NULL) {
    streamState->sendRTCPAppPacket(subtype, name, appDependentData, appDependentDataSize);
  }
}

void OnDemandServerMediaSubsession
::setSDPLinesFromRTPSink(RTPSink* rtpSink, FramedSource* inputSource, unsigned estBitrate, LIVE_IP_VER_ENUM ipVer) {
  if (rtpSink == NULL) return;

  char const* mediaType = rtpSink->sdpMediaType();
  unsigned char rtpPayloadType = rtpSink->rtpPayloadType();
  AddressString ipAddressStr(fServerAddressForSDP, ipVer);
  char* rtpmapLine = rtpSink->rtpmapLine();
  char const* rtcpmuxLine = fMultiplexRTCPWithRTP ? "a=rtcp-mux\r\n" : "";
  char const* rangeLine = rangeSDPLine();
  char const* auxSDPLine = getAuxSDPLine(rtpSink, inputSource);
  if (auxSDPLine == NULL) auxSDPLine = "";

  char const* const sdpFmtV4 =
    "m=%s %u RTP/AVP %d\r\n"
    "c=IN IP4 %s\r\n"
    "b=AS:%u\r\n"
    "%s"
    "%s"
    "%s"
    "%s"
    "a=control:%s\r\n";

  char const* const sdpFmtV6 =
    "m=%s %u RTP/AVP %d\r\n"
    "c=IN IP6 %s\r\n"
    "b=AS:%u\r\n"
    "%s"
    "%s"
    "%s"
    "%s"
    "a=control:%s\r\n";

  unsigned sdpFmtSize = (unsigned)strlen(ipVer==LIVE_IP_VER_4?sdpFmtV4:sdpFmtV6)
    + (unsigned)strlen(mediaType) + 5 /* max short len */ + 3 /* max char len */
    + (unsigned)strlen(ipAddressStr.val())
    + 20 /* max int len */
    + (unsigned)strlen(rtpmapLine)
    + (unsigned)strlen(rtcpmuxLine)
    + (unsigned)strlen(rangeLine)
    + (unsigned)strlen(auxSDPLine)
    + (unsigned)strlen(trackId());
  char* sdpLines = new char[sdpFmtSize];
  sprintf(sdpLines, ipVer==LIVE_IP_VER_4?sdpFmtV4:sdpFmtV6,
	  mediaType, // m= <media>
	  fPortNumForSDP, // m= <port>
	  rtpPayloadType, // m= <fmt list>
	  ipAddressStr.val(), // c= address
	  estBitrate, // b=AS:<bandwidth>
	  rtpmapLine, // a=rtpmap:... (if present)
	  rtcpmuxLine, // a=rtcp-mux:... (if present)
	  rangeLine, // a=range:... (if present)
	  auxSDPLine, // optional extra SDP line
	  trackId()); // a=control:<track-id>
  delete[] (char*)rangeLine; delete[] rtpmapLine;

  delete[] fSDPLines; fSDPLines = strDup(sdpLines);
  delete[] sdpLines;
}


////////// StreamState implementation //////////

static void afterPlayingStreamState(void* clientData) {
  StreamState* streamState = (StreamState*)clientData;
  if (streamState->streamDuration() == 0.0) {
    // When the input stream ends, tear it down.  This will cause a RTCP "BYE"
    // to be sent to each client, teling it that the stream has ended.
    // (Because the stream didn't have a known duration, there was no other
    //  way for clients to know when the stream ended.)
    streamState->reclaim();
  }
  // Otherwise, keep the stream alive, in case a client wants to
  // subsequently re-play the stream starting from somewhere other than the end.
  // (This can be done only on streams that have a known duration.)
}

StreamState::StreamState(UsageEnvironment	*pEnv, OnDemandServerMediaSubsession& master,
                         Port const& serverRTPPort, Port const& serverRTCPPort,
			 RTPSink* rtpSink, BasicUDPSink* udpSink,
			 unsigned totalBW, FramedSource* mediaSource,
			 Groupsock* rtpGS, Groupsock* rtcpGS)
  : fMaster(master), fAreCurrentlyPlaying(False), fReferenceCount(1),
    fServerRTPPort(serverRTPPort), fServerRTCPPort(serverRTCPPort),
    fRTPSink(rtpSink), fUDPSink(udpSink), fStreamDuration(master.duration()),
    fTotalBW(totalBW), fRTCPInstance(NULL) /* created later */,
    fMediaSource(mediaSource), fStartNPT(0.0), fRTPgs(rtpGS), fRTCPgs(rtcpGS) {

		mFlag = LIVE_FLAG;

		fEnv = pEnv;

		//_TRACE(TRACE_LOG_DEBUG, (char *)"[%s]  Create streamState:  0x%X.\n", fEnv->GetEnvirName(), this);
}

StreamState::~StreamState() {
	mFlag = 0;
	reclaim();

	_TRACE(TRACE_LOG_DEBUG, (char *)"[%s]  Delete streamState:  0x%X.\n", fEnv->GetEnvirName(), this);
}

void StreamState::startPlaying(Destinations* dests, UsageEnvironment& env, unsigned clientSessionId,
	       TaskFunc* rtcpRRHandler, void* rtcpRRHandlerClientData,
	       ServerRequestAlternativeByteHandler* serverRequestAlternativeByteHandler,
	       void* serverRequestAlternativeByteHandlerClientData) {
  if (dests == NULL) return;


  if (fEnv != &env)
  {


	  _TRACE(TRACE_LOG_DEBUG, (char *)"##############StreamState::startPlaying ERROR   env不相同...\n");
  }

  if (fRTCPInstance == NULL && fRTPSink != NULL) {
    // Create (and start) a 'RTCP instance' for this RTP sink:
    fRTCPInstance = fMaster.createRTCP(fRTCPgs, fTotalBW, (unsigned char*)fMaster.fCNAME, fRTPSink);
        // Note: This starts RTCP running automatically
    fRTCPInstance->setAppHandler(fMaster.fAppHandlerTask, fMaster.fAppHandlerClientData);
  }

  if (dests->isTCP) {
    // Change RTP and RTCP to use the TCP socket instead of UDP:
    if (fRTPSink != NULL) {
		fRTPSink->addStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
      //fRTPSink->addStreamSocket(env, dests->tcpSocketNum, dests->rtpChannelId);		//gavin 2018.03.01
#if 1		//gavin 2018.03.01

#ifdef _DEBUG
		//_TRACE(TRACE_LOG_DEBUG, (char*)"RTPInterface::setServerRequestAlternativeByteHandler  [%s]\n", env.GetEnvirName());
#endif

      RTPInterface
	::setServerRequestAlternativeByteHandler(env, dests->tcpSocketNum,
						 serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);

	  if (0 != strcmp(fRTPSink->envir().GetEnvirName(), env.GetEnvirName()))
	  {
		//gavin 2018.03.05		尝试: 如果当前streamName工作在thread0, 而当前env的工作线程为thread1, 则将该socket添加到thread0后, 将该socket从thread1中移除
		//env.taskScheduler().disableBackgroundHandling(dests->tcpSocketNum);
	  }
#else
      RTPInterface
	::setServerRequestAlternativeByteHandler(fRTPSink->envir(), dests->tcpSocketNum,
						 serverRequestAlternativeByteHandler, serverRequestAlternativeByteHandlerClientData);
#endif
	  
        // So that we continue to handle RTSP commands from the client
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->addStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);
			LIVE_NET_ADDRESS_INADDR tmpInAddr;
			memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
			if (dests->ipVer == LIVE_IP_VER_4)
			{
				tmpInAddr.sin_addr.s_addr = dests->tcpSocketNum;
			}
			else
			{
				memcpy(&tmpInAddr.sin6_addr.s6_addr, &dests->tcpSocketNum, 4);
			}
			  fRTCPInstance->setSpecificRRHandler(tmpInAddr, dests->rtcpChannelId,
				  rtcpRRHandler, rtcpRRHandlerClientData);

    }
  } else {
    // Tell the RTP and RTCP 'groupsocks' about this destination
    // (in case they don't already have it):
    if (fRTPgs != NULL) fRTPgs->addDestination(dests->addr, dests->rtpPort, clientSessionId);
    if (fRTCPgs != NULL && !(fRTCPgs == fRTPgs && dests->rtcpPort.num() == dests->rtpPort.num())) {
      fRTCPgs->addDestination(dests->addr, dests->rtcpPort, clientSessionId);
    }
    if (fRTCPInstance != NULL) {
			LIVE_NET_ADDRESS_INADDR tmpInAddr;
			memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
			if (dests->ipVer == LIVE_IP_VER_4)
			{
				tmpInAddr.sin_addr.s_addr = dests->addr.sin_addr.s_addr;
			}
			else
			{
				memcpy(&tmpInAddr.sin6_addr.s6_addr, &dests->addr.sin6_addr.s6_addr, sizeof(tmpInAddr.sin6_addr.s6_addr));
			}

			  fRTCPInstance->setSpecificRRHandler(tmpInAddr, dests->rtcpPort,
				  rtcpRRHandler, rtcpRRHandlerClientData);

    }
  }

  if (fRTCPInstance != NULL) {
    // Hack: Send an initial RTCP "SR" packet, before the initial RTP packet, so that receivers will (likely) be able to
    // get RTCP-synchronized presentation times immediately:
    fRTCPInstance->sendReport();
  }

  if (!fAreCurrentlyPlaying && fMediaSource != NULL) 
  {
	if (fRTPSink != NULL && fMediaSource->GetFlag() == LIVE_FLAG)
	{
		fRTPSink->startPlaying(*fMediaSource, afterPlayingStreamState, this);
		fAreCurrentlyPlaying = True;
    } 
	else if (fUDPSink != NULL && fMediaSource->GetFlag() == LIVE_FLAG)
	{
		char sztmp[128] = {0};
		sprintf(sztmp, "Flag: 0x%X", mFlag);
		FILE *f = fopen("udpsinkStartPlay.txt", "wb");
		if (NULL != f)
		{
			fwrite(sztmp, 1, (int)strlen(sztmp), f);
			fclose(f);
		}

		fUDPSink->startPlaying(*fMediaSource, afterPlayingStreamState, this);
		fAreCurrentlyPlaying = True;
    }
  }
}

void StreamState::pause() {
  if (fRTPSink != NULL) fRTPSink->stopPlaying();
  if (fUDPSink != NULL) fUDPSink->stopPlaying();
  fAreCurrentlyPlaying = False;
}

void StreamState::endPlaying(Destinations* dests, unsigned clientSessionId) {
#if 0
  // The following code is temporarily disabled, because it erroneously sends RTCP "BYE"s to all clients if multiple
  // clients are streaming from the same data source (i.e., if "reuseFirstSource" is True), and we don't want that to happen
  // if we're being called as a result of a single one of these clients having sent a "TEARDOWN" (rather than the whole stream
  // having been closed, for all clients).
  // This will be fixed for real later.
  if (fRTCPInstance != NULL) {
    // Hack: Explicitly send a RTCP "BYE" packet now, because the code below will prevent that from happening later,
    // when "fRTCPInstance" gets deleted:
    fRTCPInstance->sendBYE();
  }
#endif

  if (dests->isTCP) {
    if (fRTPSink != NULL) {
      RTPInterface::clearServerRequestAlternativeByteHandler(fRTPSink->envir(), dests->tcpSocketNum);
      fRTPSink->removeStreamSocket(dests->tcpSocketNum, dests->rtpChannelId);
    }
    if (fRTCPInstance != NULL) {
      fRTCPInstance->removeStreamSocket(dests->tcpSocketNum, dests->rtcpChannelId);

			LIVE_NET_ADDRESS_INADDR tmpInAddr;
			if (dests->ipVer == LIVE_IP_VER_4)
			{
				tmpInAddr.sin_addr.s_addr = dests->tcpSocketNum;
			}
			else
			{
				memcpy(&tmpInAddr.sin6_addr.s6_addr, &dests->tcpSocketNum, 4);
			}

      fRTCPInstance->unsetSpecificRRHandler(tmpInAddr, dests->rtcpChannelId);
    }
  } else {
    // Tell the RTP and RTCP 'groupsocks' to stop using these destinations:
    if (fRTPgs != NULL) fRTPgs->removeDestination(clientSessionId);
    if (fRTCPgs != NULL && fRTCPgs != fRTPgs) fRTCPgs->removeDestination(clientSessionId);
    if (fRTCPInstance != NULL) {

			LIVE_NET_ADDRESS_INADDR tmpInAddr;
			if (dests->ipVer == LIVE_IP_VER_4)
			{
				tmpInAddr.sin_addr.s_addr = dests->addr.sin_addr.s_addr;
			}
			else
			{
				memcpy(&tmpInAddr.sin6_addr.s6_addr, &dests->addr.sin6_addr.s6_addr, 4);
			}


		fRTCPInstance->unsetSpecificRRHandler(tmpInAddr, dests->rtcpPort);
    }
  }
}

void StreamState::sendRTCPAppPacket(u_int8_t subtype, char const* name,
				    u_int8_t* appDependentData, unsigned appDependentDataSize) {
  if (fRTCPInstance != NULL) {
    fRTCPInstance->sendAppPacket(subtype, name, appDependentData, appDependentDataSize);
  }
}

void StreamState::reclaim() {
  // Delete allocated media objects
  Medium::close(fRTCPInstance) /* will send a RTCP BYE */; fRTCPInstance = NULL;
  Medium::close(fRTPSink); fRTPSink = NULL;
  Medium::close(fUDPSink); fUDPSink = NULL;

  if (NULL != fMediaSource && fMediaSource->GetFlag() == LIVE_FLAG) 
  {
	fMaster.closeStreamSource(fMediaSource); 
  }
  fMediaSource = NULL;

  if (NULL != fMaster.fLastStreamToken)
  {
	if (fMaster.fLastStreamToken == this) 
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"[%s] delete fLastStreamToken: 0x%X.\n",  fEnv->GetEnvirName(), this);
		fMaster.fLastStreamToken = NULL;
	}
  }

  delete fRTPgs;
  if (fRTCPgs != fRTPgs) delete fRTCPgs;
  fRTPgs = NULL; fRTCPgs = NULL;
}
