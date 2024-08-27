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
// A RTSP server
// Implementation

#include "RTSPServer.hh"
#include "RTSPCommon.hh"
#include "RTSPRegisterSender.hh"
#include "ProxyServerMediaSession.hh"
#include "Base64.hh"
#include <GroupsockHelper.hh>
#include "../../trace.h"
////////// RTSPServer implementation //////////

RTSPServer*
RTSPServer::createNew(UsageEnvironment& env, Port ourPort,
		      UserAuthenticationDatabase* authDatabase,
		      unsigned reclamationSeconds, void *_callback, void *_userptr) {
  int ourSocket4 = setUpOurSocket4(env, ourPort);
  if (ourSocket4 == -1) return NULL;

  int ourSocket6 = setUpOurSocket6(env, ourPort);
  //if (ourSocket6 == -1) return NULL;		//某些设备不支持IPV6
  
  return new RTSPServer(env, ourSocket4, ourSocket6, ourPort, authDatabase, reclamationSeconds, _callback, _userptr);
}

Boolean RTSPServer::lookupByName(UsageEnvironment& env,
				 char const* name,
				 RTSPServer*& resultServer) {
  resultServer = NULL; // unless we succeed
  
  Medium* medium;
  if (!Medium::lookupByName(env, name, medium)) return False;
  
  if (!medium->isRTSPServer()) {
    env.setResultMsg(name, " is not a RTSP server");
    return False;
  }
  
  resultServer = (RTSPServer*)medium;
  return True;
}

char* RTSPServer
::rtspURL(ServerMediaSession const* serverMediaSession, LIVE_IP_VER_ENUM ipVer, int clientSocket) const {
  char* urlPrefix = rtspURLPrefix(ipVer, clientSocket);
  char const* sessionName = serverMediaSession->streamName();
  
  char* resultURL = new char[strlen(urlPrefix) + strlen(sessionName) + 1];
  sprintf(resultURL, "%s%s", urlPrefix, sessionName);
  
  delete[] urlPrefix;
  return resultURL;
}

char* RTSPServer::rtspURLPrefix(LIVE_IP_VER_ENUM ipVer, int clientSocket) const {
  LIVE_NET_ADDRESS_SOCKADDR_IN ourAddress;
  memset(&ourAddress, 0x00, sizeof(LIVE_NET_ADDRESS_SOCKADDR_IN));

	if (ipVer == LIVE_IP_VER_4)
	{
		  if (clientSocket < 0) {
			// Use our default IP address in the URL:
			  ourAddress.saddr4.sin_addr.s_addr = ReceivingInterfaceAddr.sin_addr.s_addr != 0
				  ? ReceivingInterfaceAddr.sin_addr.s_addr
				  : ourIPAddress(envir(), ipVer).sin_addr.s_addr; // hack
		  } else {
			SOCKLEN_T namelen = sizeof ourAddress;
			getsockname(clientSocket, (struct sockaddr*)&ourAddress, &namelen);
		  }
  
  char urlBuffer[100]; // more than big enough for "rtsp://<ip-address>:<port>/"
  
  portNumBits portNumHostOrder = ntohs(fServerPort.num());
  if (portNumHostOrder == 554 /* the default port number */) {
    sprintf(urlBuffer, "rtsp://%s/", AddressString(ourAddress, ipVer).val());
  } else {
    sprintf(urlBuffer, "rtsp://%s:%hu/",
	    AddressString(ourAddress, ipVer).val(), portNumHostOrder);
  }
  
		  return strDup(urlBuffer);
	}
	else
	{
		//gavin 2018.03.15  ??????
#if 1
		  if (clientSocket < 0) {

			  LIVE_NET_ADDRESS_INADDR tmpInAddr;
			  memset(&tmpInAddr, 0x00, sizeof(LIVE_NET_ADDRESS_INADDR));
			  if (0!=memcmp(&tmpInAddr, &ReceivingInterfaceAddr, sizeof(LIVE_NET_ADDRESS_INADDR)))
			  {
				  memcpy(&ourAddress.saddr6.sin6_addr, &ReceivingInterfaceAddr.sin6_addr, sizeof(ourAddress.saddr6.sin6_addr));
			  }
			  else
			  {
				struct in6_addr addr6 = ourIPAddress(envir(), ipVer).sin6_addr;
				  memcpy(&ourAddress.saddr6.sin6_addr, &addr6, sizeof(ourAddress.saddr6.sin6_addr));
			  }
/*
			// Use our default IP address in the URL:
			  ourAddress.saddr6.sin6_addr.s6_addr = ReceivingInterfaceAddr != 0
			  ? ReceivingInterfaceAddr
			  : ourIPAddress(envir()); // hack
*/
		  } else {
			SOCKLEN_T namelen = sizeof ourAddress;
			getsockname(clientSocket, (struct sockaddr*)&ourAddress.saddr6, &namelen);
		  }
  
		  char urlBuffer[100]; // more than big enough for "rtsp://<ip-address>:<port>/"
  
		  portNumBits portNumHostOrder = ntohs(fServerPort.num());
		  if (portNumHostOrder == 554 /* the default port number */) {
			sprintf(urlBuffer, "rtsp://%s/", AddressString(ourAddress, ipVer).val());
		  } else {
			sprintf(urlBuffer, "rtsp://%s:%hu/",
				AddressString(ourAddress, ipVer).val(), portNumHostOrder);
		  }
  
		  return strDup(urlBuffer);
#endif
	}
}

UserAuthenticationDatabase* RTSPServer::setAuthenticationDatabase(UserAuthenticationDatabase* newDB) {
  UserAuthenticationDatabase* oldDB = fAuthDB;
  fAuthDB = newDB;
  
  return oldDB;
}

Boolean RTSPServer::setUpTunnelingOverHTTP(Port httpPort) {
  fHTTPServerSocket = setUpOurSocket4(envir(), httpPort);
  if (fHTTPServerSocket >= 0) {
    fHTTPServerPort = httpPort;
    envir().taskScheduler().turnOnBackgroundReadHandling(fHTTPServerSocket,
							 incomingConnectionHandlerHTTP, this);
    return True;
  }
  
  return False;
}

portNumBits RTSPServer::httpServerPortNum() const {
  return ntohs(fHTTPServerPort.num());
}

char const* RTSPServer::allowedCommandNames() {
  return "OPTIONS, DESCRIBE, SETUP, TEARDOWN, PLAY, PAUSE, GET_PARAMETER, SET_PARAMETER";
}

UserAuthenticationDatabase* RTSPServer::getAuthenticationDatabaseForCommand(char const* /*cmdName*/) {
  // default implementation
  return fAuthDB;
}

Boolean RTSPServer::specialClientAccessCheck(int /*clientSocket*/, LIVE_NET_ADDRESS_SOCKADDR_IN& /*clientAddr*/, char const* /*urlSuffix*/) {
  // default implementation
  return True;
}

Boolean RTSPServer::specialClientUserAccessCheck(int /*clientSocket*/, LIVE_NET_ADDRESS_SOCKADDR_IN& /*clientAddr*/,
						 char const* /*urlSuffix*/, char const * /*username*/) {
  // default implementation; no further access restrictions:
  return True;
}


RTSPServer::RTSPServer(UsageEnvironment& env,
		       int ourSocketV4, int ourSocketV6, Port ourPort,
		       UserAuthenticationDatabase* authDatabase,
		       unsigned reclamationSeconds, void *_callback, void *_userptr)
  : GenericMediaServer(env, ourSocketV4, ourSocketV6, ourPort, reclamationSeconds, _callback, _userptr),
    fHTTPServerSocket(-1), fHTTPServerPort(0),
    fClientConnectionsForHTTPTunneling(NULL), // will get created if needed
    fTCPStreamingDatabase(HashTable::create(ONE_WORD_HASH_KEYS)),
    fPendingRegisterOrDeregisterRequests(HashTable::create(ONE_WORD_HASH_KEYS)),
    fRegisterOrDeregisterRequestCounter(0), fAuthDB(authDatabase), fAllowStreamingRTPOverTCP(True)
{
#if ENABLE_CLIENT_LIST == 0x01
	pRtspClientInfoList = NULL;
	pRtspClientList = NULL;
	InitMutex(&clientListMutex);
	InitMutex(&osMutexTcpStreamingDatabase);
#endif
}

// A data structure that is used to implement "fTCPStreamingDatabase"
// (and the "noteTCPStreamingOnSocket()" and "stopTCPStreamingOnSocket()" member functions):
class streamingOverTCPRecord {
public:
  streamingOverTCPRecord(u_int32_t sessionId, unsigned trackNum, streamingOverTCPRecord* next)
    : fNext(next), fSessionId(sessionId), fTrackNum(trackNum) {
  }
  virtual ~streamingOverTCPRecord() {
    delete fNext;
  }

  streamingOverTCPRecord* fNext;
  u_int32_t fSessionId;
  unsigned fTrackNum;
};

RTSPServer::~RTSPServer() {
  // Turn off background HTTP read handling (if any):
  envir().taskScheduler().turnOffBackgroundReadHandling(fHTTPServerSocket);
  ::closeSocket(fHTTPServerSocket);
  
  cleanup(); // Removes all "ClientSession" and "ClientConnection" objects, and their tables.
  delete fClientConnectionsForHTTPTunneling;
  fClientConnectionsForHTTPTunneling = NULL;

  //add by gavin
#if ENABLE_CLIENT_LIST == 0x01
  if (NULL != pRtspClientInfoList)
  {
	  delete pRtspClientInfoList;
	  pRtspClientInfoList = NULL;
  }
  if (NULL != pRtspClientList)
  {
	  delete []pRtspClientList;
	  pRtspClientList = NULL;
  }
  DeinitMutex(&clientListMutex);
  DeinitMutex(&osMutexTcpStreamingDatabase);
#endif
  // Delete any pending REGISTER requests:
  RTSPRegisterOrDeregisterSender* r;
  while ((r = (RTSPRegisterOrDeregisterSender*)fPendingRegisterOrDeregisterRequests->getFirst()) != NULL) {
    delete r;
  }
  delete fPendingRegisterOrDeregisterRequests;
  fPendingRegisterOrDeregisterRequests = NULL;
  
  // Empty out and close "fTCPStreamingDatabase":
  streamingOverTCPRecord* sotcp;
  while ((sotcp = (streamingOverTCPRecord*)fTCPStreamingDatabase->getFirst()) != NULL) {
    delete sotcp;
  }
  delete fTCPStreamingDatabase;
  fTCPStreamingDatabase = NULL;
}

Boolean RTSPServer::isRTSPServer() const {
  return True;
}

void		RTSPServer::LockTcpStreamingDatabase()
{
	LockMutex(&osMutexTcpStreamingDatabase);
}
void		RTSPServer::UnlockTcpStreamingDatabase()
{
	UnlockMutex(&osMutexTcpStreamingDatabase);
}

void RTSPServer::incomingConnectionHandlerHTTP(void* instance, int /*mask*/) {
  RTSPServer* server = (RTSPServer*)instance;
  server->incomingConnectionHandlerHTTP();
}
void RTSPServer::incomingConnectionHandlerHTTP() {
  incomingConnectionHandlerOnSocket(fHTTPServerSocket, LIVE_IP_VER_4);
}

void RTSPServer
::noteTCPStreamingOnSocket(int socketNum, RTSPClientSession* clientSession, unsigned trackNum) {

	//LockClientConnection();
	//LockClientSession();
	LockTcpStreamingDatabase();

  streamingOverTCPRecord* sotcpCur
    = (streamingOverTCPRecord*)fTCPStreamingDatabase->Lookup((char const*)socketNum);
  streamingOverTCPRecord* sotcpNew
    = new streamingOverTCPRecord(clientSession->fOurSessionId, trackNum, sotcpCur);
  fTCPStreamingDatabase->Add((char const*)socketNum, sotcpNew);

  UnlockTcpStreamingDatabase();
  //UnlockClientSession();
  //UnlockClientConnection();
}

void RTSPServer
::unnoteTCPStreamingOnSocket(int socketNum, RTSPClientSession* clientSession, unsigned trackNum) {
  if (socketNum < 0) return;

  //LockClientConnection();

  LockTcpStreamingDatabase();

  do
  {
	  streamingOverTCPRecord* sotcpHead
		= (streamingOverTCPRecord*)fTCPStreamingDatabase->Lookup((char const*)socketNum);
	  if (sotcpHead == NULL) break;

	  // Look for a record of the (session,track); remove it if found:
	  streamingOverTCPRecord* sotcp = sotcpHead;
	  streamingOverTCPRecord* sotcpPrev = sotcpHead;
	  do {
		if (sotcp->fSessionId == clientSession->fOurSessionId && sotcp->fTrackNum == trackNum) break;
		sotcpPrev = sotcp;
		sotcp = sotcp->fNext;
	  } while (sotcp != NULL);
	  if (sotcp == NULL) break; // not found
  
	  if (sotcp == sotcpHead) {
		// We found it at the head of the list.  Remove it and reinsert the tail into the hash table:
		sotcpHead = sotcp->fNext;
		sotcp->fNext = NULL;
		delete sotcp;

		if (sotcpHead == NULL) {
		  // There were no more entries on the list.  Remove the original entry from the hash table:
		  fTCPStreamingDatabase->Remove((char const*)socketNum);
		} else {
		  // Add the rest of the list into the hash table (replacing the original):
		  fTCPStreamingDatabase->Add((char const*)socketNum, sotcpHead);
		}
	  } else {
		// We found it on the list, but not at the head.  Unlink it:
		sotcpPrev->fNext = sotcp->fNext;
		sotcp->fNext = NULL;
		delete sotcp;
	  }
  }while (0);

  UnlockTcpStreamingDatabase();
  //UnlockClientConnection();

}

void RTSPServer::stopTCPStreamingOnSocket(UsageEnvironment *pEnv, int socketNum, int *clientTrackNum, char *clientSessionIdStr){//, Boolean assignSink) {
  // Close any stream that is streaming over "socketNum" (using RTP/RTCP-over-TCP streaming):

	RTSPClientSession	*pClientSession = NULL;

	//LockClientConnection();
	//LockClientSession();

	LockTcpStreamingDatabase();

	do
	{
	  streamingOverTCPRecord* sotcp
		= (streamingOverTCPRecord*)fTCPStreamingDatabase->Lookup((char const*)socketNum);
	  if (sotcp != NULL) {		//rtp over tcp
		do {
		  RTSPClientSession* clientSession = (RTSPServer::RTSPClientSession*)lookupClientSession(sotcp->fSessionId);
		  if (clientSession != NULL)
		  {
			  //clientSession->SetAssignSink(assignSink);
			  //clientSession->deleteStreamByTrack(pEnv, sotcp->fTrackNum, False, clientTrackNum);

			  if (pEnv == clientSession->pEnvironment)
			  {
					clientSession->deleteStreamByTrack(clientSession->pEnvironment, sotcp->fTrackNum, False, clientTrackNum);
			  }
			  else
			  {
				  FILE *f = fopen("stopTCPStreamingOnSocket.txt", "wb");
				  if (NULL != f)
				  {
					  char *s = "RTSPServer::stopTCPStreamingOnSocket.  说明session冲突";
					  fwrite(s, 1, (int)strlen(s), f);
					  fclose(f);
				  }

				  //这里不同，说明sessionId冲突了 ??
				  _TRACE(TRACE_LOG_DEBUG, (char *)"==============SessionID冲突========BEGIN=====%d\n", sotcp->fSessionId);

				  int i = 0;
				  HashTable::Iterator* iter = HashTable::Iterator::create(*GetClientSessions());
				  GenericMediaServer::ClientSession* clientSession;
				  char const* key; // dummy
				  while ((clientSession = (GenericMediaServer::ClientSession*)(iter->next(key))) != NULL) 
				  {
					  _TRACE(TRACE_LOG_DEBUG, (char *)"SessionID[%d]:  %d[0x%X]    [%s]\n", i, 
											  clientSession->GetSessionId(), clientSession->GetSessionId(), 
											  clientSession->GetEnvir()->GetEnvirName());
					  i++;
				  }
				  delete iter;
				  _TRACE(TRACE_LOG_DEBUG, (char *)"==============SessionID冲突=========END====%d\n", sotcp->fSessionId);
			  }
		  }

		  streamingOverTCPRecord* sotcpNext = sotcp->fNext;
		  sotcp->fNext = NULL;
		  delete sotcp;
		  sotcp = sotcpNext;
		} while (sotcp != NULL);
		fTCPStreamingDatabase->Remove((char const*)socketNum);
	  }
	  else if ( (clientTrackNum) && (*clientTrackNum==0))		//rtp over udp
	  {
		  pClientSession = (RTSPServer::RTSPClientSession*)lookupClientSession(clientSessionIdStr);
	  }
	}while (0);

	//UnlockClientSession();
	//UnlockClientConnection();

	
	if (pClientSession != NULL) 
	{
		delete pClientSession;
	}

	UnlockTcpStreamingDatabase();
}



//add by gavin
#if ENABLE_CLIENT_LIST == 0x01
int	RTSPServer::GetRTSPClientList(RTSP_CLIENT_INFO_T	**pClientInfo, int *clientNum)
{
	if (NULL == pRtspClientList)
	{
		pRtspClientList = new RTSP_CLIENT_INFO_T[MAX_CLIENT_NUM];
		if (NULL == pRtspClientList)		return -2;
	}

	if (NULL != clientNum)		*clientNum = 0;
	if (NULL == pRtspClientInfoList)			return 0;

	memset(pRtspClientList, 0x00, sizeof(RTSP_CLIENT_INFO_T) * MAX_CLIENT_NUM);

	LockMutex(&clientListMutex);

	int index = 0;
	for (int i=0; i<MAX_CLIENT_NUM; i++)
	{
		if (NULL == pRtspClientInfoList[i].ptr)		continue;

		pRtspClientList[index].connection = pRtspClientInfoList[i].clientInfo.connection;
		strcpy(pRtspClientList[index].ipaddr, pRtspClientInfoList[i].clientInfo.ipaddr);
		pRtspClientList[index].port = pRtspClientInfoList[i].clientInfo.port;

		strcpy(pRtspClientList[index].session, pRtspClientInfoList[i].clientInfo.session);
		strcpy(pRtspClientList[index].name, pRtspClientInfoList[i].clientInfo.name);

		index ++;
	}

	UnlockMutex(&clientListMutex);

	if (NULL != pClientInfo)	*pClientInfo = pRtspClientList;
	if (NULL != clientNum)		*clientNum = index;

	return 0;
}
#endif
////////// RTSPServer::RTSPClientConnection implementation //////////

RTSPServer::RTSPClientConnection
::RTSPClientConnection(RTSPServer& ourServer, int clientSocket, LIVE_NET_ADDRESS_SOCKADDR_IN clientAddr, LIVE_IP_VER_ENUM ipVer)
  : GenericMediaServer::ClientConnection(ourServer, clientSocket, clientAddr, ipVer),
    fOurRTSPServer(ourServer), fClientInputSocket(fOurSocket), fClientOutputSocket(fOurSocket),
    fIsActive(True), fRecursionCount(0), fOurSessionCookie(NULL) {
  resetRequestBuffer();

  clientRequestTrackNum = 0;
  memset(fClientSessionIdStr, 0x00, sizeof(fClientSessionIdStr));

#if ENABLE_CLIENT_LIST == 0x01
  //add by gavin
  LockMutex(&fOurRTSPServer.clientListMutex);
  if (NULL == fOurRTSPServer.pRtspClientInfoList)
  {
	  fOurRTSPServer.pRtspClientInfoList = new LIVE_CLIENT_INFO_T[MAX_CLIENT_NUM];
	  if (NULL != fOurRTSPServer.pRtspClientInfoList)
	  {
		  memset(fOurRTSPServer.pRtspClientInfoList, 0x00, sizeof(LIVE_CLIENT_INFO_T)*MAX_CLIENT_NUM);
	  }
  }

  if (NULL != fOurRTSPServer.pRtspClientInfoList)
  {
	  for (int i=0; i<MAX_CLIENT_NUM; i++)
	  {
		  if (NULL == fOurRTSPServer.pRtspClientInfoList[i].ptr)
		  {
			  if (LIVE_IP_VER_4 == ipVer)
			  {
				  //strcpy(fOurRTSPServer.pRtspClientInfoList[i].clientInfo.ipaddr, inet_ntoa(clientAddr.saddr4.sin_addr));
				  //网络地址变成字符串
				  inet_ntop(AF_INET, &clientAddr.saddr4.sin_addr, fOurRTSPServer.pRtspClientInfoList[i].clientInfo.ipaddr, INET_ADDRSTRLEN);

				  fOurRTSPServer.pRtspClientInfoList[i].clientInfo.port = ntohs(clientAddr.saddr4.sin_port);

				  _TRACE(TRACE_LOG_INFO, (char*)"RTSPServer::RTSPClientConnection  [%s:%d]   ptr[0x%X] clientsocket[%d]\n", 
									fOurRTSPServer.pRtspClientInfoList[i].clientInfo.ipaddr, 
									fOurRTSPServer.pRtspClientInfoList[i].clientInfo.port, this, clientSocket);
			  }
			  else if (LIVE_IP_VER_6 == ipVer)
			  {
				  inet_ntop(AF_INET6, &clientAddr.saddr6.sin6_addr, fOurRTSPServer.pRtspClientInfoList[i].clientInfo.ipaddr, INET6_ADDRSTRLEN);

				  fOurRTSPServer.pRtspClientInfoList[i].clientInfo.port = ntohs(clientAddr.saddr6.sin6_port);
			  }

			  fOurRTSPServer.pRtspClientInfoList[i].sock = clientSocket;
			  fOurRTSPServer.pRtspClientInfoList[i].ptr = this;

			  

			  break;
		  }
	  }
  }
  UnlockMutex(&fOurRTSPServer.clientListMutex);

#endif


  fOurServer.envir().taskScheduler().turnOnBackgroundReadHandling(fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
/*
#ifdef LIVE_MULTI_THREAD_ENABLE
  if (NULL != pClientConnectionEnv && assignSink)
  {
	pClientConnectionEnv->taskScheduler().turnOnBackgroundReadHandling(fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
  }
  else if (! assignSink)
  {
	  fOurServer.envir().taskScheduler().turnOnBackgroundReadHandling(fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
  }
#else
  {
	  envir().taskScheduler().turnOnBackgroundReadHandling(fOurSocket,  (TaskScheduler::BackgroundHandlerProc*)&GenericMediaServer::ClientConnection::incomingRequestHandler, this);
  }
#endif
*/
}

RTSPServer::RTSPClientConnection::~RTSPClientConnection() {

	if (GetFlag() != LIVE_FLAG)
	{
		printf("############[ERROR]RTSPClientConnection::~RTSPClientConnection() [0x%X]", this);
		return;
	}

	UsageEnvironment	*pTmpEnv = pClientConnectionEnv;
	if (NULL == pTmpEnv)
	{
		pTmpEnv = &fOurServer.envir();
	}

	_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] RTSPServer::RTSPClientConnection::~RTSPClientConnection() ptr[0x%X] socket[%d] session[%s]  BEGIN...\n", 
		pTmpEnv->GetEnvirName(),  this,
		fClientInputSocket, fClientSessionIdStr);

  if (fOurSessionCookie != NULL) {
    // We were being used for RTSP-over-HTTP tunneling. Also remove ourselves from the 'session cookie' hash table before we go:
    fOurRTSPServer.fClientConnectionsForHTTPTunneling->Remove(fOurSessionCookie);
    delete[] fOurSessionCookie;
	fOurSessionCookie = NULL;
  }
#if ENABLE_CLIENT_LIST == 0x01
  //add by gavin
  LockMutex(&fOurRTSPServer.clientListMutex);
  if (NULL != fOurRTSPServer.pRtspClientInfoList)
  {
	  for (int i=0; i<MAX_CLIENT_NUM; i++)
	  {
		  if (this == fOurRTSPServer.pRtspClientInfoList[i].ptr)
		  {
			  memset(&fOurRTSPServer.pRtspClientInfoList[i], 0x00, sizeof(LIVE_CLIENT_INFO_T));
			  break;
		  }
	  }
  }
  UnlockMutex(&fOurRTSPServer.clientListMutex);
#endif
  
  closeSocketsRTSP(pTmpEnv);

  _TRACE(TRACE_LOG_DEBUG, (char*)"[%s] RTSPServer::RTSPClientConnection::~RTSPClientConnection() ptr[0x%X]  END...\n",pTmpEnv->GetEnvirName(),  this);

}

// Handler routines for specific RTSP commands:

void RTSPServer::RTSPClientConnection::handleCmd_OPTIONS() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 200 OK\r\nCSeq: %s\r\n%sPublic: %s\r\n\r\n",
	   fCurrentCSeq, dateHeader(), fOurRTSPServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection
::handleCmd_GET_PARAMETER(char const* /*fullRequestStr*/) {
  // By default, we implement "GET_PARAMETER" (on the entire server) just as a 'no op', and send back a dummy response.
  // (If you want to handle this type of "GET_PARAMETER" differently, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientConnection", and then reimplement this virtual function in your subclass.)
  setRTSPResponse("200 OK", LIVEMEDIA_LIBRARY_VERSION_STRING);
}

void RTSPServer::RTSPClientConnection
::handleCmd_SET_PARAMETER(char const* /*fullRequestStr*/) {
  // By default, we implement "SET_PARAMETER" (on the entire server) just as a 'no op', and send back an empty response.
  // (If you want to handle this type of "SET_PARAMETER" differently, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientConnection", and then reimplement this virtual function in your subclass.)
  setRTSPResponse("200 OK");
}

int RTSPServer::RTSPClientConnection
::handleCmd_DESCRIBE(UsageEnvironment *pEnv, char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr, LIVE_THREAD_TASK_T **pThreadTask) 
{
	int handleCmdRet = 0;
	 
	//fOurServer.LockServerMediaSession();

	//Boolean bLock = False;

  ServerMediaSession* session = NULL;
  char* sdpDescription = NULL;
  char* rtspURL = NULL;
  do {
    char urlTotalSuffix[2*RTSP_PARAM_STRING_MAX];
        // enough space for urlPreSuffix/urlSuffix'\0'
    urlTotalSuffix[0] = '\0';
    if (urlPreSuffix[0] != '\0') {
      strcat(urlTotalSuffix, urlPreSuffix);
      strcat(urlTotalSuffix, "/");
    }
    strcat(urlTotalSuffix, urlSuffix);
    
    if (!authenticationOK("DESCRIBE", urlTotalSuffix, fullRequestStr)) break;
    
    // We should really check that the request contains an "Accept:" #####
    // for "application/sdp", because that's what we're sending back #####
    
	

	//_TRACE(TRACE_LOG_DEBUG, "handleCmd_DESCRIBE  socket[%d]\n", this->fOurSocket);


	#ifdef LIVE_MULTI_THREAD_ENABLE


	//如果当前是主线程,则进入到查找通道流程
	if (pEnv->GetEnvirId() == MAIN_THREAD_ID)
	{
		//fOurServer.LockClientConnection();	//Lock

		UsageEnvironment  *pChEnv = fOurServer.GetEnvBySuffix(pEnv, urlTotalSuffix, this, pThreadTask, True);
		if (NULL == pChEnv)
		{
			//fOurServer.UnlockClientConnection();		//Unlock

			handleCmdRet = -1;

			this->pClientConnectionEnv = NULL;
			handleCmd_notFound();


			//_TRACE(TRACE_LOG_DEBUG, (char *)"#############  this->assignSink: False && pEnv=NULL   CmdNotFund\n");

			break;
		}
		else
		{
			_TRACE(TRACE_LOG_DEBUG, (char*)"[%s]Set socket[%d] Assign to [%d:%s]\n", pEnv->GetEnvirName(), this->fOurSocket, pChEnv->GetEnvirId(), pChEnv->GetEnvirName());

			//将socket从主线程移到工作线程中
			//UsageEnvironment  *pMainEnv = &envir();
			pEnv->taskScheduler().disableBackgroundHandling(fOurSocket);
			//envir().taskScheduler().disableBackgroundHandling(fOurSocket);

			//OnIncomingRequestHandler(pChEnv, 0, 0);		//将socket添加到对应线程中

			//fOurServer.UnlockClientConnection();		//Unlock

			//fOurServer.UnlockServerMediaSession();

			return MAIN_THREAD_ID;
		}

		break;
	}

	#endif


    // Begin by looking up the "ServerMediaSession" object for the specified "urlTotalSuffix":

	
	
	session = fOurServer.lookupServerMediaSession(pEnv, 1, this, urlTotalSuffix, True);
	if ( session == NULL || session->GetFlag()!=LIVE_FLAG) {

		//pChEnv->taskScheduler().disableBackgroundHandling(fOurSocket);

		_TRACE(TRACE_LOG_DEBUG, (char*)"socket[%d] in[%s], The source is not ready[%s]\n", this->fOurSocket, pEnv->GetEnvirName(), urlTotalSuffix);


		//此处减少引用计数, 不影响ClientSession::~ClientSession()中的减少引用计数, 
		//执行到此处, 说明当前还未创建ClientSession
		pEnv->DecrementReferenceCount();
		//_TRACE(TRACE_LOG_WARNING, (char*)"######## handleCmd_DESCRIBE 减少引用计数.\n", pEnv->GetReferenceCount());
		this->pClientConnectionEnv = NULL;

		//gavin 2018.09.10
		//_TRACE(TRACE_LOG_DEBUG, (char *)"[%s]未找到相应session.   复位该通道: %s\n", pEnv->GetEnvirName(), urlTotalSuffix);
		//fOurServer.ResetStreamNameInWorkThread(pEnv, True);//, urlTotalSuffix);


		//_TRACE(TRACE_LOG_DEBUG, (char *)"#############  this->assignSink: False && pEnv=NULL\n");

		handleCmdRet = -1;

		//envir().taskScheduler().disableBackgroundHandling(fOurSocket);

		//fOurServer.ResetEnvBySuffix(urlSuffix, this);

		handleCmd_notFound();

		break;
	}


	//fOurServer.LockServerMediaSession(pEnv->GetEnvirName(), "handleCmd_DESCRIBE", (unsigned long long)this);
	//bLock = True;
   
	//gavin 2018.06.25
	memset(mStreamName, 0x00, sizeof(mStreamName));
	strcpy(mStreamName, urlTotalSuffix);

	//gavin 2018.03.29
	//将socket从主线程移到工作线程中
	//envir().taskScheduler().disableBackgroundHandling(fOurSocket);
	//OnIncomingRequestHandler(pEnv, 0, 0);		//将socket添加到对应线程中

    // Increment the "ServerMediaSession" object's reference count, in case someone removes it
    // while we're using it:
    session->incrementReferenceCount();

    // Then, assemble a SDP description for this session:
    sdpDescription = session->generateSDPDescription(this->fOurSocket, fOurIPVer);
    if (sdpDescription == NULL) {

		pEnv->SetStreamStatus(0x00);		//2019.05.22

      // This usually means that a file name that was specified for a
      // "ServerMediaSubsession" does not exist.
      setRTSPResponse("404 File Not Found, Or In Incorrect Format");

	  //fOurServer.UnlockServerMediaSession(pEnv->GetEnvirName(), "handleCmd_DESCRIBE", (unsigned long long)this);
	  //bLock = False;

      break;
    }

    unsigned sdpDescriptionSize = (unsigned int)strlen(sdpDescription);
    
    // Also, generate our RTSP URL, for the "Content-Base:" header
    // (which is necessary to ensure that the correct URL gets used in subsequent "SETUP" requests).
    rtspURL = fOurRTSPServer.rtspURL(session, fOurIPVer, fClientInputSocket);
    
    snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	     "RTSP/1.0 200 OK\r\nCSeq: %s\r\n"
	     "%s"
	     "Content-Base: %s/\r\n"
	     "Content-Type: application/sdp\r\n"
	     "Content-Length: %d\r\n\r\n"
	     "%s",
	     fCurrentCSeq,
	     dateHeader(),
	     rtspURL,
	     sdpDescriptionSize,
	     sdpDescription);
  } while (0);
  
  if (session != NULL) {
    // Decrement its reference count, now that we're done using it:
    session->decrementReferenceCount();
    if (session->referenceCount() == 0 && session->deleteWhenUnreferenced()) {
      fOurServer.removeServerMediaSession(pEnv, session, True);
    }

	pEnv->SetStreamStatus(0x01);
	//session->SetStreamStatus(1);		//add by gavin 2018.03.29
  }

  //if (bLock)
  {
	//fOurServer.UnlockServerMediaSession(pEnv->GetEnvirName(), "handleCmd_DESCRIBE", (unsigned long long)this);
  }

  delete[] sdpDescription;
  delete[] rtspURL;


  

  return handleCmdRet;
}

static void lookForHeader(char const* headerName, char const* source, unsigned sourceLen, char* resultStr, unsigned resultMaxSize) {
  resultStr[0] = '\0';  // by default, return an empty string
  unsigned headerNameLen = (unsigned)strlen(headerName);
  for (int i = 0; i < (int)(sourceLen-headerNameLen); ++i) {
    if (strncmp(&source[i], headerName, headerNameLen) == 0 && source[i+headerNameLen] == ':') {
      // We found the header.  Skip over any whitespace, then copy the rest of the line to "resultStr":
      for (i += headerNameLen+1; i < (int)sourceLen && (source[i] == ' ' || source[i] == '\t'); ++i) {}
      for (unsigned j = i; j < sourceLen; ++j) {
	if (source[j] == '\r' || source[j] == '\n') {
	  // We've found the end of the line.  Copy it to the result (if it will fit):
	  if (j-i+1 > resultMaxSize) break;
	  char const* resultSource = &source[i];
	  char const* resultSourceEnd = &source[j];
	  while (resultSource < resultSourceEnd) *resultStr++ = *resultSource++;
	  *resultStr = '\0';
	  break;
	}
      }
    }
  }
}

void RTSPServer::RTSPClientConnection::handleCmd_bad() {
  // Don't do anything with "fCurrentCSeq", because it might be nonsense
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 400 Bad Request\r\n%sAllow: %s\r\n\r\n",
	   dateHeader(), fOurRTSPServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection::handleCmd_notSupported() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 405 Method Not Allowed\r\nCSeq: %s\r\n%sAllow: %s\r\n\r\n",
	   fCurrentCSeq, dateHeader(), fOurRTSPServer.allowedCommandNames());
}

void RTSPServer::RTSPClientConnection::handleCmd_notFound() {
  setRTSPResponse("404 Stream Not Found");
}

void RTSPServer::RTSPClientConnection::handleCmd_sessionNotFound() {
  setRTSPResponse("454 Session Not Found");
}

void RTSPServer::RTSPClientConnection::handleCmd_unsupportedTransport() {
  setRTSPResponse("461 Unsupported Transport");
}

Boolean RTSPServer::RTSPClientConnection::parseHTTPRequestString(char* resultCmdName, unsigned resultCmdNameMaxSize,
								 char* urlSuffix, unsigned urlSuffixMaxSize,
								 char* sessionCookie, unsigned sessionCookieMaxSize,
								 char* acceptStr, unsigned acceptStrMaxSize) {
  // Check for the limited HTTP requests that we expect for specifying RTSP-over-HTTP tunneling.
  // This parser is currently rather dumb; it should be made smarter #####
  char const* reqStr = (char const*)fRequestBuffer;
  unsigned const reqStrSize = fRequestBytesAlreadySeen;
  
  // Read everything up to the first space as the command name:
  Boolean parseSucceeded = False;
  unsigned i;
  for (i = 0; i < resultCmdNameMaxSize-1 && i < reqStrSize; ++i) {
    char c = reqStr[i];
    if (c == ' ' || c == '\t') {
      parseSucceeded = True;
      break;
    }
    
    resultCmdName[i] = c;
  }
  resultCmdName[i] = '\0';
  if (!parseSucceeded) return False;
  
  // Look for the string "HTTP/", before the first \r or \n:
  parseSucceeded = False;
  for (; i < reqStrSize-5 && reqStr[i] != '\r' && reqStr[i] != '\n'; ++i) {
    if (reqStr[i] == 'H' && reqStr[i+1] == 'T' && reqStr[i+2]== 'T' && reqStr[i+3]== 'P' && reqStr[i+4]== '/') {
      i += 5; // to advance past the "HTTP/"
      parseSucceeded = True;
      break;
    }
  }
  if (!parseSucceeded) return False;
  
  // Get the 'URL suffix' that occurred before this:
  unsigned k = i-6;
  while (k > 0 && reqStr[k] == ' ') --k; // back up over white space
  unsigned j = k;
  while (j > 0 && reqStr[j] != ' ' && reqStr[j] != '/') --j;
  // The URL suffix is in position (j,k]:
  if (k - j + 1 > urlSuffixMaxSize) return False; // there's no room> 
  unsigned n = 0;
  while (++j <= k) urlSuffix[n++] = reqStr[j];
  urlSuffix[n] = '\0';
  
  // Look for various headers that we're interested in:
  lookForHeader("x-sessioncookie", &reqStr[i], reqStrSize-i, sessionCookie, sessionCookieMaxSize);
  lookForHeader("Accept", &reqStr[i], reqStrSize-i, acceptStr, acceptStrMaxSize);
  
  return True;
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_notSupported() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 405 Method Not Allowed\r\n%s\r\n\r\n",
	   dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_notFound() {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 404 Not Found\r\n%s\r\n\r\n",
	   dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_OPTIONS() {
#ifdef DEBUG
  fprintf(stderr, "Handled HTTP \"OPTIONS\" request\n");
#endif
  // Construct a response to the "OPTIONS" command that notes that our special headers (for RTSP-over-HTTP tunneling) are allowed:
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 200 OK\r\n"
	   "%s"
	   "Access-Control-Allow-Origin: *\r\n"
	   "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n"
	   "Access-Control-Allow-Headers: x-sessioncookie, Pragma, Cache-Control\r\n"
	   "Access-Control-Max-Age: 1728000\r\n"
	   "\r\n",
	   dateHeader());
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_TunnelingGET(char const* sessionCookie) {
  // Record ourself as having this 'session cookie', so that a subsequent HTTP "POST" command (with the same 'session cookie')
  // can find us:
  if (fOurRTSPServer.fClientConnectionsForHTTPTunneling == NULL) {
    fOurRTSPServer.fClientConnectionsForHTTPTunneling = HashTable::create(STRING_HASH_KEYS);
  }
  delete[] fOurSessionCookie; fOurSessionCookie = strDup(sessionCookie);
  fOurRTSPServer.fClientConnectionsForHTTPTunneling->Add(sessionCookie, (void*)this);
#ifdef DEBUG
  fprintf(stderr, "Handled HTTP \"GET\" request (client output socket: %d)\n", fClientOutputSocket);
#endif
  
  // Construct our response:
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "HTTP/1.1 200 OK\r\n"
	   "%s"
	   "Cache-Control: no-cache\r\n"
	   "Pragma: no-cache\r\n"
	   "Content-Type: application/x-rtsp-tunnelled\r\n"
	   "\r\n",
	   dateHeader());
}

Boolean RTSPServer::RTSPClientConnection
::handleHTTPCmd_TunnelingPOST(char const* sessionCookie, unsigned char const* extraData, unsigned extraDataSize) {
  // Use the "sessionCookie" string to look up the separate "RTSPClientConnection" object that should have been used to handle
  // an earlier HTTP "GET" request:
  if (fOurRTSPServer.fClientConnectionsForHTTPTunneling == NULL) {
    fOurRTSPServer.fClientConnectionsForHTTPTunneling = HashTable::create(STRING_HASH_KEYS);
  }
  RTSPServer::RTSPClientConnection* prevClientConnection
    = (RTSPServer::RTSPClientConnection*)(fOurRTSPServer.fClientConnectionsForHTTPTunneling->Lookup(sessionCookie));
  if (prevClientConnection == NULL) {
    // There was no previous HTTP "GET" request; treat this "POST" request as bad:
    handleHTTPCmd_notSupported();
    fIsActive = False; // triggers deletion of ourself
    return False;
  }
#ifdef DEBUG
  fprintf(stderr, "Handled HTTP \"POST\" request (client input socket: %d)\n", fClientInputSocket);
#endif
  
  // Change the previous "RTSPClientSession" object's input socket to ours.  It will be used for subsequent requests:
  prevClientConnection->changeClientInputSocket(fClientInputSocket, extraData, extraDataSize);
  fClientInputSocket = fClientOutputSocket = -1; // so the socket doesn't get closed when we get deleted
  return True;
}

void RTSPServer::RTSPClientConnection::handleHTTPCmd_StreamingGET(char const* /*urlSuffix*/, char const* /*fullRequestStr*/) {
  // By default, we don't support requests to access streams via HTTP:
  handleHTTPCmd_notSupported();
}

void RTSPServer::RTSPClientConnection::resetRequestBuffer() {
  ClientConnection::resetRequestBuffer();
  
  fLastCRLF = &fRequestBuffer[-3]; // hack: Ensures that we don't think we have end-of-msg if the data starts with <CR><LF>
  fBase64RemainderCount = 0;
}

void RTSPServer::RTSPClientConnection::closeSocketsRTSP(UsageEnvironment *pEnv) {
  // First, tell our server to stop any streaming that it might be doing over our output socket:

	if (pClientConnectionEnv && pClientConnectionEnv != pEnv)
	{
		FILE *f = fopen("env.txt", "wb");
		if (NULL != f)
		{
			char sztmp[128] = {0};
			sprintf(sztmp, "pClientConnectionEnv[0x%X] != pEnv[0x%X]  flag[%0xX]\n", pClientConnectionEnv, pEnv, mFlag);
			
			fwrite(sztmp, 1, (int)strlen(sztmp), f);
			fclose(f);
		}

		_TRACE(TRACE_LOG_DEBUG, (char *)"########################## envir ERROR.. RTSPServer::RTSPClientConnection::closeSocketsRTSP\n");
		return;
	}

	UsageEnvironment *pTmpEnv = pClientConnectionEnv;
	if (NULL == pTmpEnv)
	{
		if (pEnv != &fOurServer.envir())
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"########################## envir ###### ERROR.. RTSPServer::RTSPClientConnection::closeSocketsRTSP\n");
		}
		pTmpEnv = &fOurServer.envir();
	}

	//fOurRTSPServer.LockServerMediaSession(pEnv->GetEnvirName(), "RTSPClientConnection::closeSocketsRTSP", (unsigned long long)this);

	fOurRTSPServer.stopTCPStreamingOnSocket(pEnv, fClientOutputSocket, &clientRequestTrackNum, fClientSessionIdStr);//, assignSink);

	//fOurRTSPServer.UnlockServerMediaSession(pEnv->GetEnvirName(), "RTSPClientConnection::closeSocketsRTSP", (unsigned long long)this);

  // Turn off background handling on our input socket (and output socket, if different); then close it (or them):
  if (fClientOutputSocket != fClientInputSocket) {
	  /*
#ifdef LIVE_MULTI_THREAD_ENABLE
	  if (NULL != pEnv && assignSink)
	  {
		  _TRACE(TRACE_LOG_DEBUG, (char*)"RTSPServer::RTSPClientConnection::closeSocketsRTSP 从[%s]中移除该Socket[%d].\n", pEnv->GetEnvirName(), fClientOutputSocket);

		pEnv->taskScheduler().disableBackgroundHandling(fClientOutputSocket);
	  }
	  else if (! assignSink)
	  {
		  _TRACE(TRACE_LOG_DEBUG, (char*)"RTSPServer::RTSPClientConnection::closeSocketsRTSP 从[%s]中移除该Socket[%d].\n", fOurServer.envir().GetEnvirName(), fClientOutputSocket);

		  fOurServer.envir().taskScheduler().disableBackgroundHandling(fClientOutputSocket);
	  }
#else
	  {
		envir().taskScheduler().disableBackgroundHandling(fClientOutputSocket);
	  }
#endif
	  */
    ::closeSocket(fClientOutputSocket);
  }
  fClientOutputSocket = -1;
  
  closeSockets(); // closes fClientInputSocket
}

void RTSPServer::RTSPClientConnection::handleAlternativeRequestByte(void* instance, u_int8_t requestByte) {
  RTSPClientConnection* connection = (RTSPClientConnection*)instance;
  connection->handleAlternativeRequestByte1(requestByte);
}

void RTSPServer::RTSPClientConnection::handleAlternativeRequestByte1(u_int8_t requestByte) {

	UsageEnvironment *pTmpEnv = pClientConnectionEnv;
	if (NULL == pTmpEnv)	pTmpEnv = &fOurServer.envir();

  if (requestByte == 0xFF) {
    // Hack: The new handler of the input TCP socket encountered an error reading it.  Indicate this:
	  handleRequestBytes(-1, pTmpEnv);
/*
#ifdef LIVE_MULTI_THREAD_ENABLE
	  if (NULL != pClientConnectionEnv && assignSink)
	  {
			
	  }
	  else if (! assignSink)
	  {
		  handleRequestBytes(-1, &fOurServer.envir());
	  }
#else
	  {
		handleRequestBytes(-1, &envir());
	  }
#endif
	  */
  } else if (requestByte == 0xFE) {
    // Another hack: The new handler of the input TCP socket no longer needs it, so take back control of it:

	  pTmpEnv->taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							  incomingRequestHandler, this);

	  /*
#ifdef LIVE_MULTI_THREAD_ENABLE
	  if (NULL != pClientConnectionEnv && assignSink)
	  {
		  pClientConnectionEnv->taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							  incomingRequestHandler, this);
	  }
	  else if (! assignSink)
	  {
		envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							  incomingRequestHandler, this);
	  }
#else
	  {
		envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							  incomingRequestHandler, this);
	  }
#endif
	  */
  } else {
    // Normal case: Add this character to our buffer; then try to handle the data that we have buffered so far:
    if (fRequestBufferBytesLeft == 0 || fRequestBytesAlreadySeen >= REQUEST_BUFFER_SIZE) return;
    fRequestBuffer[fRequestBytesAlreadySeen] = requestByte;

	handleRequestBytes(1, pTmpEnv);

	/*
#ifdef LIVE_MULTI_THREAD_ENABLE
	if (NULL != pClientConnectionEnv && assignSink)
	{
		handleRequestBytes(1, pClientConnectionEnv);
	}
	else if (! assignSink)
	{
		handleRequestBytes(1, &envir());
	}
#else
	{
		handleRequestBytes(1, &envir());
	}
#endif
	*/
  }
}

void RTSPServer::RTSPClientConnection::handleRequestBytes(int newBytesRead, UsageEnvironment *pEnv) {
  int numBytesRemaining = 0;
  ++fRecursionCount;

  if (GetFlag() != LIVE_FLAG)		return;

  //_TRACE(TRACE_LOG_INFO, (char*)"++fRecursionCount: %d\n", fRecursionCount);
  
  do {
    RTSPServer::RTSPClientSession* clientSession = NULL;

    if (newBytesRead < 0 || (unsigned)newBytesRead >= fRequestBufferBytesLeft) {
      // Either the client socket has died, or the request was too big for us.
      // Terminate this connection:
#ifdef DEBUG
      fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() read %d new bytes (of %d); terminating connection!\n", this, newBytesRead, fRequestBufferBytesLeft);
#endif
      fIsActive = False;
      break;
    }
    
    Boolean endOfMsg = False;
    unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
#ifdef DEBUG
    ptr[newBytesRead] = '\0';
    fprintf(stderr, "RTSPClientConnection[%p]::handleRequestBytes() %s %d new bytes:%s\n",
	    this, numBytesRemaining > 0 ? "processing" : "read", newBytesRead, ptr);
#endif
    
    if (fClientOutputSocket != fClientInputSocket && numBytesRemaining == 0) {
      // We're doing RTSP-over-HTTP tunneling, and input commands are assumed to have been Base64-encoded.
      // We therefore Base64-decode as much of this new data as we can (i.e., up to a multiple of 4 bytes).
      
      // But first, we remove any whitespace that may be in the input data:
      unsigned toIndex = 0;
      for (int fromIndex = 0; fromIndex < newBytesRead; ++fromIndex) {
	char c = ptr[fromIndex];
	if (!(c == ' ' || c == '\t' || c == '\r' || c == '\n')) { // not 'whitespace': space,tab,CR,NL
	  ptr[toIndex++] = c;
	}
      }
      newBytesRead = toIndex;
      
      unsigned numBytesToDecode = fBase64RemainderCount + newBytesRead;
      unsigned newBase64RemainderCount = numBytesToDecode%4;
      numBytesToDecode -= newBase64RemainderCount;
      if (numBytesToDecode > 0) {
	ptr[newBytesRead] = '\0';
	unsigned decodedSize;
	unsigned char* decodedBytes = base64Decode((char const*)(ptr-fBase64RemainderCount), numBytesToDecode, decodedSize);
#ifdef DEBUG
	fprintf(stderr, "Base64-decoded %d input bytes into %d new bytes:", numBytesToDecode, decodedSize);
	for (unsigned k = 0; k < decodedSize; ++k) fprintf(stderr, "%c", decodedBytes[k]);
	fprintf(stderr, "\n");
#endif
	
	// Copy the new decoded bytes in place of the old ones (we can do this because there are fewer decoded bytes than original):
	unsigned char* to = ptr-fBase64RemainderCount;
	for (unsigned i = 0; i < decodedSize; ++i) *to++ = decodedBytes[i];
	
	// Then copy any remaining (undecoded) bytes to the end:
	for (unsigned j = 0; j < newBase64RemainderCount; ++j) *to++ = (ptr-fBase64RemainderCount+numBytesToDecode)[j];
	
	newBytesRead = decodedSize - fBase64RemainderCount + newBase64RemainderCount;
	  // adjust to allow for the size of the new decoded data (+ remainder)
	delete[] decodedBytes;
      }
      fBase64RemainderCount = newBase64RemainderCount;
    }
    
    unsigned char* tmpPtr = fLastCRLF + 2;
    if (fBase64RemainderCount == 0) { // no more Base-64 bytes remain to be read/decoded
      // Look for the end of the message: <CR><LF><CR><LF>
      if (tmpPtr < fRequestBuffer) tmpPtr = fRequestBuffer;
      while (tmpPtr < &ptr[newBytesRead-1]) {
	if (*tmpPtr == '\r' && *(tmpPtr+1) == '\n') {
	  if (tmpPtr - fLastCRLF == 2) { // This is it:
	    endOfMsg = True;
	    break;
	  }
	  fLastCRLF = tmpPtr;
	}
	++tmpPtr;
      }
    }
    
	//add by gavin
	int mainThread_newBytesRead = newBytesRead;

    fRequestBufferBytesLeft -= newBytesRead;
    fRequestBytesAlreadySeen += newBytesRead;

	//add by gavin 2019.04.04
	if (fRequestBytesAlreadySeen < 1 || fRequestBytesAlreadySeen>=REQUEST_BUFFER_SIZE)
	{
		fIsActive = False;
		break;
	}
    
    if (!endOfMsg) break; // subsequent reads will be needed to complete the request
    
    // Parse the request string into command name and 'CSeq', then handle the command:
    fRequestBuffer[fRequestBytesAlreadySeen] = '\0';
    char cmdName[RTSP_PARAM_STRING_MAX];
    char urlPreSuffix[RTSP_PARAM_STRING_MAX];
    char urlSuffix[RTSP_PARAM_STRING_MAX];
    char cseq[RTSP_PARAM_STRING_MAX];
    char sessionIdStr[RTSP_PARAM_STRING_MAX];
    unsigned contentLength = 0;
    fLastCRLF[2] = '\0'; // temporarily, for parsing
    Boolean parseSucceeded = parseRTSPRequestString((char*)fRequestBuffer, (unsigned int)(fLastCRLF+2 - fRequestBuffer),
						    cmdName, sizeof cmdName,
						    urlPreSuffix, sizeof urlPreSuffix,
						    urlSuffix, sizeof urlSuffix,
						    cseq, sizeof cseq,
						    sessionIdStr, sizeof sessionIdStr,
						    contentLength);
    fLastCRLF[2] = '\r'; // restore its value
    Boolean playAfterSetup = False;

	int iProcRet = 0;

    if (parseSucceeded) {
#ifdef DEBUG
      fprintf(stderr, "parseRTSPRequestString() succeeded, returning cmdName \"%s\", urlPreSuffix \"%s\", urlSuffix \"%s\", CSeq \"%s\", Content-Length %u, with %d bytes following the message.\n", cmdName, urlPreSuffix, urlSuffix, cseq, contentLength, ptr + newBytesRead - (tmpPtr + 2));
#endif
      // If there was a "Content-Length:" header, then make sure we've received all of the data that it specified:
      if (ptr + newBytesRead < tmpPtr + 2 + contentLength) break; // we still need more data; subsequent reads will give it to us 
      
      // If the request included a "Session:" id, and it refers to a client session that's
      // current ongoing, then use this command to indicate 'liveness' on that client session:
      Boolean const requestIncludedSessionId = sessionIdStr[0] != '\0';

	  
	if (requestIncludedSessionId)
	{
		fOurRTSPServer.LockClientSession();		//Lock

		clientSession  = (RTSPServer::RTSPClientSession*)(fOurRTSPServer.lookupClientSession(sessionIdStr));

		fOurRTSPServer.UnlockClientSession();	//Unlock

		if (clientSession != NULL && clientSession->GetFlag()==LIVE_FLAG)
		{
			clientSession->noteLiveness();
		}
		else
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"###### clientSession is NULL[0x%X]..  sessionIdStr[%s]\n", clientSession, sessionIdStr);
		}
	}
	else
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"###### request not include sessionId[%s]  requestStr: %s\n", sessionIdStr, fRequestBuffer);
	}
   
      // We now have a complete RTSP request.
      // Handle the specified command (beginning with commands that are session-independent):
      fCurrentCSeq = cseq;
      if (strcmp(cmdName, "OPTIONS") == 0) {
	// If the "OPTIONS" command included a "Session:" id for a session that doesn't exist,
	// then treat this as an error:
	if (requestIncludedSessionId && clientSession == NULL) {
	  handleCmd_sessionNotFound();
	} else {
	  // Normal case:
	  handleCmd_OPTIONS();
	}
      } else if (urlPreSuffix[0] == '\0' && urlSuffix[0] == '*' && urlSuffix[1] == '\0') {
	// The special "*" URL means: an operation on the entire server.  This works only for GET_PARAMETER and SET_PARAMETER:
	if (strcmp(cmdName, "GET_PARAMETER") == 0) {
	  handleCmd_GET_PARAMETER((char const*)fRequestBuffer);
	} else if (strcmp(cmdName, "SET_PARAMETER") == 0) {
	  handleCmd_SET_PARAMETER((char const*)fRequestBuffer);
	} else {
	  handleCmd_notSupported();
	}
      } else if (strcmp(cmdName, "DESCRIBE") == 0) {

		  LIVE_THREAD_TASK_T *pThreadTask = NULL;

		  int describeRet = handleCmd_DESCRIBE(pEnv, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer, &pThreadTask);
			if ( describeRet < 0 )
			{
				fIsActive = False;
			}
			else if (describeRet == MAIN_THREAD_ID)		//此处的describeRet返回为10000, 直接用MAIN_THREAD_ID代替
			{
				fRequestBufferBytesLeft+=mainThread_newBytesRead;
				fRequestBytesAlreadySeen -= mainThread_newBytesRead;

				if (NULL != pThreadTask)
				{
					//pThreadTask->handleDescribe += 1;
				}

				break;
			}
      } else if (strcmp(cmdName, "SETUP") == 0) {
	Boolean areAuthenticated = True;

	if (!requestIncludedSessionId) {
	  // No session id was present in the request.
	  // So create a new "RTSPClientSession" object for this request.

	  // But first, make sure that we're authenticated to perform this command:
	  char urlTotalSuffix[2*RTSP_PARAM_STRING_MAX];
	      // enough space for urlPreSuffix/urlSuffix'\0'
	  urlTotalSuffix[0] = '\0';
	  if (urlPreSuffix[0] != '\0') {
	    strcat(urlTotalSuffix, urlPreSuffix);
	    strcat(urlTotalSuffix, "/");
	  }
	  strcat(urlTotalSuffix, urlSuffix);
	  if (authenticationOK("SETUP", urlTotalSuffix, (char const*)fRequestBuffer)) {
		  memset(fClientSessionIdStr, 0x00, sizeof(fClientSessionIdStr));

		  if (pEnv->GetEnvirId() == MAIN_THREAD_ID)
		  {
			  _TRACE(TRACE_LOG_DEBUG, (char *)"################  ERROR  ################ SETUP be main thread proc...\n");
			  fIsActive = False;
			  break;
		  }
		  
	    clientSession
	      = (RTSPServer::RTSPClientSession*)fOurRTSPServer.createNewClientSessionWithId(pEnv, fClientSessionIdStr, this);
	  } else {
	    areAuthenticated = False;
	  }
	}
	if (clientSession != NULL) {
		initSessionFlag = 0x01;			//已初始化session  2019.05.20
	  clientSession->handleCmd_SETUP(this, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
	  playAfterSetup = clientSession->fStreamAfterSETUP;
	} else if (areAuthenticated) {
	  handleCmd_sessionNotFound();
	}
      } else if (strcmp(cmdName, "TEARDOWN") == 0
		 || strcmp(cmdName, "PLAY") == 0
		 || strcmp(cmdName, "PAUSE") == 0
		 || strcmp(cmdName, "GET_PARAMETER") == 0
		 || strcmp(cmdName, "SET_PARAMETER") == 0) {
	if (clientSession != NULL) {
	  iProcRet = clientSession->handleCmd_withinSession(this, cmdName, urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
	  if (iProcRet == 0xFF)		fIsActive = False;
	} else {
		_TRACE(TRACE_LOG_DEBUG, (char *)"###### cmdName[%s] Session Not Found: %s...\n", cmdName, fRequestBuffer);
	  handleCmd_sessionNotFound();
	}
      } else if (strcmp(cmdName, "REGISTER") == 0 || strcmp(cmdName, "DEREGISTER") == 0) {
	// Because - unlike other commands - an implementation of this command needs
	// the entire URL, we re-parse the command to get it:
	char* url = strDupSize((char*)fRequestBuffer);
	if (sscanf((char*)fRequestBuffer, "%*s %s", url) == 1) {
	  // Check for special command-specific parameters in a "Transport:" header:
	  Boolean reuseConnection, deliverViaTCP;
	  char* proxyURLSuffix;
	  parseTransportHeaderForREGISTER((const char*)fRequestBuffer, reuseConnection, deliverViaTCP, proxyURLSuffix);

	  handleCmd_REGISTER(cmdName, url, urlSuffix, (char const*)fRequestBuffer, reuseConnection, deliverViaTCP, proxyURLSuffix);
	  delete[] proxyURLSuffix;
	} else {
	  handleCmd_bad();
	}
	delete[] url;
      } else {
	// The command is one that we don't handle:

#ifdef _DEBUG
		  __TRACE_TO_FILE("1_err.txt", (char *)fRequestBuffer, (int)(fLastCRLF+2 - fRequestBuffer));
#endif

	handleCmd_notSupported();
      }
    } else {
#ifdef DEBUG
      fprintf(stderr, "parseRTSPRequestString() failed; checking now for HTTP commands (for RTSP-over-HTTP tunneling)...\n");
#endif
      // The request was not (valid) RTSP, but check for a special case: HTTP commands (for setting up RTSP-over-HTTP tunneling):
      char sessionCookie[RTSP_PARAM_STRING_MAX];
      char acceptStr[RTSP_PARAM_STRING_MAX];
      *fLastCRLF = '\0'; // temporarily, for parsing
      parseSucceeded = parseHTTPRequestString(cmdName, sizeof cmdName,
					      urlSuffix, sizeof urlPreSuffix,
					      sessionCookie, sizeof sessionCookie,
					      acceptStr, sizeof acceptStr);
      *fLastCRLF = '\r';
      if (parseSucceeded) {
#ifdef DEBUG
	fprintf(stderr, "parseHTTPRequestString() succeeded, returning cmdName \"%s\", urlSuffix \"%s\", sessionCookie \"%s\", acceptStr \"%s\"\n", cmdName, urlSuffix, sessionCookie, acceptStr);
#endif
	// Check that the HTTP command is valid for RTSP-over-HTTP tunneling: There must be a 'session cookie'.
	Boolean isValidHTTPCmd = True;
	if (strcmp(cmdName, "OPTIONS") == 0) {
	  handleHTTPCmd_OPTIONS();
	} else if (sessionCookie[0] == '\0') {
	  // There was no "x-sessioncookie:" header.  If there was an "Accept: application/x-rtsp-tunnelled" header,
	  // then this is a bad tunneling request.  Otherwise, assume that it's an attempt to access the stream via HTTP.
	  if (strcmp(acceptStr, "application/x-rtsp-tunnelled") == 0) {
	    isValidHTTPCmd = False;
	  } else {
	    handleHTTPCmd_StreamingGET(urlSuffix, (char const*)fRequestBuffer);
	  }
	} else if (strcmp(cmdName, "GET") == 0) {
	  handleHTTPCmd_TunnelingGET(sessionCookie);
	} else if (strcmp(cmdName, "POST") == 0) {
	  // We might have received additional data following the HTTP "POST" command - i.e., the first Base64-encoded RTSP command.
	  // Check for this, and handle it if it exists:
	  unsigned char const* extraData = fLastCRLF+4;
	  unsigned extraDataSize = (unsigned)(&fRequestBuffer[fRequestBytesAlreadySeen] - extraData);
	  if (handleHTTPCmd_TunnelingPOST(sessionCookie, extraData, extraDataSize)) {
	    // We don't respond to the "POST" command, and we go away:
	    fIsActive = False;
	    break;
	  }
	} else {
	  isValidHTTPCmd = False;
	}
	if (!isValidHTTPCmd) {
	  handleHTTPCmd_notSupported();
	}
      } else {
#ifdef DEBUG
	fprintf(stderr, "parseHTTPRequestString() failed!\n");
#endif
	handleCmd_bad();
      }
    }
  


	//if (iProcRet == 0x00)
	{
	#ifdef DEBUG
		fprintf(stderr, "sending response: %s", fResponseBuffer);
	#endif
		send(fClientOutputSocket, (char const*)fResponseBuffer, (int)strlen((char*)fResponseBuffer), 0);
    
		if (playAfterSetup) {
		  // The client has asked for streaming to commence now, rather than after a
		  // subsequent "PLAY" command.  So, simulate the effect of a "PLAY" command:
		  clientSession->handleCmd_withinSession(this, "PLAY", urlPreSuffix, urlSuffix, (char const*)fRequestBuffer);
		}
    
		// Check whether there are extra bytes remaining in the buffer, after the end of the request (a rare case).
		// If so, move them to the front of our buffer, and keep processing it, because it might be a following, pipelined request.
		unsigned requestSize = (unsigned)((fLastCRLF+4-fRequestBuffer) + contentLength);
		numBytesRemaining = fRequestBytesAlreadySeen - requestSize;
		resetRequestBuffer(); // to prepare for any subsequent request
    
		if (numBytesRemaining > 0) {
		  memmove(fRequestBuffer, &fRequestBuffer[requestSize], numBytesRemaining);
		  newBytesRead = numBytesRemaining;
		}
	}
	/*
	else
	{
		//fIsActive = False;
		//删除这前，先从主线程中移除
		if (pEnv->GetEnvirId() != MAIN_THREAD_ID)		//不为主线程
		{
			//_TRACE(TRACE_LOG_WARNING, (char*)"当前[%s]. 从主线程中移除该socket[%d]\n", pEnv->GetEnvirName(), fClientInputSocket);
			envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
		}

//		if ( 0 != strcmp(envir()

//		envir().taskScheduler().disableBackgroundHandling(fClientOutputSocket);

		if (NULL != pEnv)	pEnv->taskScheduler().disableBackgroundHandling(fClientInputSocket);

#ifdef _DEBUG
		int tmpClientInputSocket = fClientInputSocket;
		_TRACE(TRACE_LOG_WARNING, (char*)"RTSPServer::RTSPClientConnection::handleRequestBytes  Delete Client[0x%X] threadIdx[%s]  socket[%d] BEGIN\n", this, pEnv->GetEnvirName(), tmpClientInputSocket);
		delete this;
		_TRACE(TRACE_LOG_WARNING, (char*)"RTSPServer::RTSPClientConnection::handleRequestBytes  Delete Client[0x%X] threadIdx[%s]  socket[%d] END\n", this, pEnv->GetEnvirName(), tmpClientInputSocket);
#else
		delete this;
#endif
	}
	*/
  } while (numBytesRemaining > 0);
  
  --fRecursionCount;

  //_TRACE(TRACE_LOG_INFO, (char*)"--fRecursionCount: %d\n", fRecursionCount);

  if (!fIsActive) 
  {
    if (fRecursionCount > 0)
	{
		closeSockets(); 
	}
	else
	{
		//删除这前，先从主线程中移除
		if (pEnv && pEnv->GetEnvirId() != MAIN_THREAD_ID)		//不为主线程
		{
			//_TRACE(TRACE_LOG_WARNING, "当前[%s]. 从主线程中移除该socket[%d]\n", pEnv->GetEnvirName(), fClientInputSocket);
			fOurServer.envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
		}

//		if ( 0 != strcmp(envir()

//		envir().taskScheduler().disableBackgroundHandling(fClientOutputSocket);



		if (NULL != pEnv)	pEnv->taskScheduler().disableBackgroundHandling(fClientInputSocket);

#ifdef _DEBUG
		int tmpClientInputSocket = fClientInputSocket;
		_TRACE(TRACE_LOG_WARNING, (char*)"[%s] RTSPServer::RTSPClientConnection::handleRequestBytes  Delete Client[0x%X]   socket[%d] BEGIN\n", pEnv->GetEnvirName(), this, tmpClientInputSocket);
		delete this;
		_TRACE(TRACE_LOG_WARNING, (char*)"[%s] RTSPServer::RTSPClientConnection::handleRequestBytes  Delete Client[0x%X]   socket[%d] END\n", pEnv->GetEnvirName(), this, tmpClientInputSocket);
#else
		delete this;
#endif
	}
    // Note: The "fRecursionCount" test is for a pathological situation where we reenter the event loop and get called recursively
    // while handling a command (e.g., while handling a "DESCRIBE", to get a SDP description).
    // In such a case we don't want to actually delete ourself until we leave the outermost call.
  }
}

static Boolean parseAuthorizationHeader(char const* buf,
					char const*& username,
					char const*& realm,
					char const*& nonce, char const*& uri,
					char const*& response, char const *&password) {
  // Initialize the result parameters to default values:
  username = realm = nonce = uri = response = password = NULL;
  
  int iAuthType = 0;	//Basic

  // First, find "Authorization:"
  while (1) {
    if (*buf == '\0') return False; // not found
    if (_strncasecmp(buf, "Authorization: Digest ", 22) == 0)
	{
		iAuthType = 0x01;
		break;
	}
	if (_strncasecmp(buf, "Authorization: Basic ", 21) == 0)
	{
		iAuthType = 0x00;
		break;
	}
    ++buf;
  }
  

  if (iAuthType == 0x00)		//Basic
  {
		char client_username[128] = {0};
		sscanf(buf, "Authorization: Basic %s", client_username);
		if ( (int)strlen(client_username) < 2)		return False;

		unsigned decodedSize=0;
		unsigned char* decodedBytes = base64Decode((char const*)(client_username), (int)strlen(client_username), decodedSize);
		if (NULL == decodedBytes)		return False;

		char userauth[2][36];
		memset(&userauth[0], 0x00, sizeof(userauth));
		int index = 0, offset=0;
		for (unsigned int i=0; i<decodedSize; i++)
		{
			if (decodedBytes[i]==':')
			{
				index ++;
				offset=0;
			}
			else
			{
				userauth[index][offset++] = decodedBytes[i];
			}
		}

		delete[] decodedBytes;

		username = strDup(userauth[0]);
		password = strDup(userauth[1]);
  }
  else		//Digest
  {
	  // Then, run through each of the fields, looking for ones we handle:
	  char const* fields = buf + 22;
	  while (*fields == ' ') ++fields;
	  char* parameter = strDupSize(fields);
	  char* value = strDupSize(fields);
	  while (1) {
		value[0] = '\0';
		if (sscanf(fields, "%[^=]=\"%[^\"]\"", parameter, value) != 2 &&
		sscanf(fields, "%[^=]=\"\"", parameter) != 1) {
		  break;
		}
		if (strcmp(parameter, "username") == 0) {
		  username = strDup(value);
		} else if (strcmp(parameter, "realm") == 0) {
		  realm = strDup(value);
		} else if (strcmp(parameter, "nonce") == 0) {
		  nonce = strDup(value);
		} else if (strcmp(parameter, "uri") == 0) {
		  uri = strDup(value);
		} else if (strcmp(parameter, "response") == 0) {
		  response = strDup(value);
		}
    
		fields += strlen(parameter) + 2 /*="*/ + strlen(value) + 1 /*"*/;
		while (*fields == ',' || *fields == ' ') ++fields;
			// skip over any separating ',' and ' ' chars
		if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
	  }
	  delete[] parameter; delete[] value;
  }

  return True;
}

Boolean RTSPServer::RTSPClientConnection
::authenticationOK(char const* cmdName, char const* urlSuffix, char const* fullRequestStr) {
  if (!fOurRTSPServer.specialClientAccessCheck(fClientInputSocket, fClientAddr, urlSuffix)) {
    setRTSPResponse("401 Unauthorized");
    return False;
  }
  
  // If we weren't set up with an authentication database, we're OK:
  UserAuthenticationDatabase* authDB = fOurRTSPServer.getAuthenticationDatabaseForCommand(cmdName);
  if (authDB == NULL) return True;
  
  char const* username = NULL; char const* realm = NULL; char const* nonce = NULL;
  char const* uri = NULL; char const* response = NULL;
  char const* get_password = NULL;
  Boolean success = False;


	if (AUTHENTICATION_TYPE_BASIC == authDB->authenticationType)
	{
		do
		{
			if (!parseAuthorizationHeader(fullRequestStr,
							username, realm, nonce, uri, response, get_password)
			|| username == NULL) {
				break;
			}

			char const* password = authDB->lookupPassword(username);
		#ifdef DEBUG
			fprintf(stderr, "lookupPassword(%s) returned password %s\n", username, password);
		#endif
			if (password == NULL) break;
	
			if ( ((int)strlen(get_password) > 0) &&  (0 == strcmp(get_password, password)) )
			{
				success = True;
			}
		}while (0);

		delete[] (char*)username;
		delete[] (char*)get_password;
		if (success) return True;
	}
	else
	{


	  do {
		// To authenticate, we first need to have a nonce set up
		// from a previous attempt:
		if (fCurrentAuthenticator.nonce() == NULL) break;
    
		// Next, the request needs to contain an "Authorization:" header,
		// containing a username, (our) realm, (our) nonce, uri,
		// and response string:

		if (!parseAuthorizationHeader(fullRequestStr,
						username, realm, nonce, uri, response, get_password)
		|| username == NULL
		|| realm == NULL || strcmp(realm, fCurrentAuthenticator.realm()) != 0
		|| nonce == NULL || strcmp(nonce, fCurrentAuthenticator.nonce()) != 0
		|| uri == NULL || response == NULL) {
			break;
		}

		// Next, the username has to be known to us:
		char const* password = authDB->lookupPassword(username);
	#ifdef DEBUG
		fprintf(stderr, "lookupPassword(%s) returned password %s\n", username, password);
	#endif
		if (password == NULL) break;
		fCurrentAuthenticator.setUsernameAndPassword(username, password, authDB->passwordsAreMD5());
    
		// Finally, compute a digest response from the information that we have,
		// and compare it to the one that we were given:
		char const* ourResponse
		  = fCurrentAuthenticator.computeDigestResponse(cmdName, uri);
		success = (strcmp(ourResponse, response) == 0);
		fCurrentAuthenticator.reclaimDigestResponse(ourResponse);
	  } while (0);
  
	  delete[] (char*)realm; delete[] (char*)nonce;
	  delete[] (char*)uri; delete[] (char*)response;
  
	  if (success) {
		// The user has been authenticated.
		// Now allow subclasses a chance to validate the user against the IP address and/or URL suffix.
		if (!fOurRTSPServer.specialClientUserAccessCheck(fClientInputSocket, fClientAddr, urlSuffix, username)) {
		  // Note: We don't return a "WWW-Authenticate" header here, because the user is valid,
		  // even though the server has decided that they should not have access.
		  setRTSPResponse("401 Unauthorized");
		  delete[] (char*)username;
		  return False;
		}
	  }
	  delete[] (char*)username;
	  if (success) return True;
  
	  // If we get here, we failed to authenticate the user.
	  // Send back a "401 Unauthorized" response, with a new random nonce:
	  fCurrentAuthenticator.setRealmAndRandomNonce(authDB->realm());
	}

  if (AUTHENTICATION_TYPE_BASIC == authDB->authenticationType)
  {
	  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		   "RTSP/1.0 401 Unauthorized\r\n"
		   "CSeq: %s\r\n"
		   "%s"
		   "WWW-Authenticate: Basic realm=\"%s\"\r\n\r\n",
		   fCurrentCSeq,
		   dateHeader(),
		   authDB->realm());
		   //fCurrentAuthenticator.realm());
  }
  else
  {
	  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
		   "RTSP/1.0 401 Unauthorized\r\n"
		   "CSeq: %s\r\n"
		   "%s"
		   "WWW-Authenticate: Digest realm=\"%s\", nonce=\"%s\"\r\n\r\n",
		   fCurrentCSeq,
		   dateHeader(),
		   fCurrentAuthenticator.realm(), fCurrentAuthenticator.nonce());
  }

  return False;
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr) {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s\r\n",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader());
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, u_int32_t sessionId) {
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "Session: %08X\r\n\r\n",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader(),
	   sessionId);
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, char const* contentStr) {
  if (contentStr == NULL) contentStr = "";
  unsigned const contentLen = (unsigned)strlen(contentStr);
  
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "Content-Length: %d\r\n\r\n"
	   "%s",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader(),
	   contentLen,
	   contentStr);
}

void RTSPServer::RTSPClientConnection
::setRTSPResponse(char const* responseStr, u_int32_t sessionId, char const* contentStr) {
  if (contentStr == NULL) contentStr = "";
  unsigned const contentLen = (unsigned)strlen(contentStr);
  
  snprintf((char*)fResponseBuffer, sizeof fResponseBuffer,
	   "RTSP/1.0 %s\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "Session: %08X\r\n"
	   "Content-Length: %d\r\n\r\n"
	   "%s",
	   responseStr,
	   fCurrentCSeq,
	   dateHeader(),
	   sessionId,
	   contentLen,
	   contentStr);
}

void RTSPServer::RTSPClientConnection
::changeClientInputSocket(int newSocketNum, unsigned char const* extraData, unsigned extraDataSize) {


	UsageEnvironment	*pTmpEnv = pClientConnectionEnv;
	if (NULL == pTmpEnv)	pTmpEnv = &fOurServer.envir();

	pTmpEnv->taskScheduler().disableBackgroundHandling(fClientInputSocket);
	fClientInputSocket = newSocketNum;
	pTmpEnv->taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							incomingRequestHandler, this);

/*
#ifdef LIVE_MULTI_THREAD_ENABLE
	if (NULL != pClientConnectionEnv && assignSink)
	{
		_TRACE(TRACE_LOG_DEBUG, (char*)"RTSPClientConnection::changeClientInputSocket 从[%s]中移除该Socket[%d].\n", pClientConnectionEnv->GetEnvirName(), fClientInputSocket);

		pClientConnectionEnv->taskScheduler().disableBackgroundHandling(fClientInputSocket);
		fClientInputSocket = newSocketNum;
		pClientConnectionEnv->taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
								incomingRequestHandler, this);
	}
	else if (! assignSink)
	{
		_TRACE(TRACE_LOG_DEBUG, (char*)"RTSPClientConnection::changeClientInputSocket 从[%s]中移除该Socket[%d].\n", envir().GetEnvirName(), fClientInputSocket);


	  envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
	  fClientInputSocket = newSocketNum;
	  envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							incomingRequestHandler, this);
	}
#else
	{
	  envir().taskScheduler().disableBackgroundHandling(fClientInputSocket);
	  fClientInputSocket = newSocketNum;
	  envir().taskScheduler().setBackgroundHandling(fClientInputSocket, SOCKET_READABLE|SOCKET_EXCEPTION,
							incomingRequestHandler, this);
	}
#endif
	*/
  // Also write any extra data to our buffer, and handle it:
  if (extraDataSize > 0 && extraDataSize <= fRequestBufferBytesLeft/*sanity check; should always be true*/) {
    unsigned char* ptr = &fRequestBuffer[fRequestBytesAlreadySeen];
    for (unsigned i = 0; i < extraDataSize; ++i) {
      ptr[i] = extraData[i];
    }

	handleRequestBytes(extraDataSize, pTmpEnv);
	/*
#ifdef LIVE_MULTI_THREAD_ENABLE
	if (NULL != pClientConnectionEnv && assignSink)
	{
		handleRequestBytes(extraDataSize, pClientConnectionEnv);
	}
	else if (! assignSink)
	{
		handleRequestBytes(extraDataSize, &envir());
	}
#else
	{
		handleRequestBytes(extraDataSize, &envir());
	}
#endif
	*/
  }
}


////////// RTSPServer::RTSPClientSession implementation //////////

RTSPServer::RTSPClientSession
::RTSPClientSession(RTSPServer& ourServer, u_int32_t sessionId, UsageEnvironment	*pEnv)
  : GenericMediaServer::ClientSession(ourServer, sessionId, pEnv),
    fOurRTSPServer(ourServer), fIsMulticast(False), fStreamAfterSETUP(False),
    fTCPStreamIdCount(0), fNumStreamStates(0), fStreamStates(NULL) {

	_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] RTSPServer::RTSPClientSession::RTSPClientSession()   NEW...ptr[0x%X]\n", pEnv->GetEnvirName(), this);
}

RTSPServer::RTSPClientSession::~RTSPClientSession() {

	_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] RTSPServer::RTSPClientSession::~RTSPClientSession() ptr[0x%X] BEGIN...\n", pEnvironment->GetEnvirName(), this);
	
  reclaimStreamStates();

  _TRACE(TRACE_LOG_DEBUG, (char*)"[%s] RTSPServer::RTSPClientSession::~RTSPClientSession() ptr[0x%X] END...\n", pEnvironment->GetEnvirName(), this);
}

void RTSPServer::RTSPClientSession::deleteStreamByTrack(UsageEnvironment *pEnv, unsigned trackNum, Boolean lockFlag, int *clientTrackNum) {

	lockClientFlag = lockFlag;

	if (pEnvironment != pEnv)
	{
		_TRACE(TRACE_LOG_DEBUG, (char *)"################ envir ERROR... RTSPServer::RTSPClientSession::deleteStreamByTrack\n");
	}

  if (trackNum >= fNumStreamStates) return; // sanity check; shouldn't happen
  if (fStreamStates[trackNum].subsession != NULL) {
    fStreamStates[trackNum].subsession->deleteStream(pEnv, fOurSessionId, fStreamStates[trackNum].streamToken);
    fStreamStates[trackNum].subsession = NULL;
  }
  
  // Optimization: If all subsessions have now been deleted, then we can delete ourself now:
  Boolean noSubsessionsRemain = True;
  for (unsigned i = 0; i < fNumStreamStates; ++i) {
    if (fStreamStates[i].subsession != NULL) {
      noSubsessionsRemain = False;
      break;
    }
  }

  if (NULL!=clientTrackNum)
  {
	  if (*clientTrackNum > 0)		*clientTrackNum -= 1;
	  if (*clientTrackNum == 0)		noSubsessionsRemain = True;
  }

  if (noSubsessionsRemain) delete this;
}

void RTSPServer::RTSPClientSession::reclaimStreamStates() {

	if (lockClientFlag)		fOurRTSPServer.LockClientSession();


	//fOurRTSPServer.LockServerMediaSession();

	if (NULL != fStreamStates)
	{
	  for (unsigned i = 0; i < fNumStreamStates; ++i) {
		if (fStreamStates[i].subsession != NULL) {
		  fOurRTSPServer.unnoteTCPStreamingOnSocket(fStreamStates[i].tcpSocketNum, this, i);
		  fStreamStates[i].subsession->deleteStream(pEnvironment, fOurSessionId, fStreamStates[i].streamToken);
		}
	  }
	  delete[] fStreamStates; fStreamStates = NULL;
	}
	fNumStreamStates = 0;

	//fOurRTSPServer.UnlockServerMediaSession();

	if (lockClientFlag)		fOurRTSPServer.UnlockClientSession();
}

typedef enum StreamingMode {
  RTP_UDP,
  RTP_TCP,
  RAW_UDP
} StreamingMode;

static void parseTransportHeader(char const* buf,
				 StreamingMode& streamingMode,
				 char*& streamingModeString,
				 char*& destinationAddressStr,
				 u_int8_t& destinationTTL,
				 portNumBits& clientRTPPortNum, // if UDP
				 portNumBits& clientRTCPPortNum, // if UDP
				 unsigned char& rtpChannelId, // if TCP
				 unsigned char& rtcpChannelId // if TCP
				 ) {
  // Initialize the result parameters to default values:
  streamingMode = RTP_UDP;
  streamingModeString = NULL;
  destinationAddressStr = NULL;
  destinationTTL = 255;
  clientRTPPortNum = 0;
  clientRTCPPortNum = 1;
  rtpChannelId = rtcpChannelId = 0xFF;
  
  portNumBits p1, p2;
  unsigned ttl, rtpCid, rtcpCid;
  
  // First, find "Transport:"
  while (1) {
    if (*buf == '\0') return; // not found
    if (*buf == '\r' && *(buf+1) == '\n' && *(buf+2) == '\r') return; // end of the headers => not found
    if (_strncasecmp(buf, "Transport:", 10) == 0) break;
    ++buf;
  }
  
  // Then, run through each of the fields, looking for ones we handle:
  char const* fields = buf + 10;
  while (*fields == ' ') ++fields;
  char* field = strDupSize(fields);
  while (sscanf(fields, "%[^;\r\n]", field) == 1) {
    if (strcmp(field, "RTP/AVP/TCP") == 0) {
      streamingMode = RTP_TCP;
    } else if (strcmp(field, "RAW/RAW/UDP") == 0 ||
	       strcmp(field, "MP2T/H2221/UDP") == 0) {
      streamingMode = RAW_UDP;
      streamingModeString = strDup(field);
    } else if (_strncasecmp(field, "destination=", 12) == 0) {
      delete[] destinationAddressStr;
      destinationAddressStr = strDup(field+12);
    } else if (sscanf(field, "ttl%u", &ttl) == 1) {
      destinationTTL = (u_int8_t)ttl;
    } else if (sscanf(field, "client_port=%hu-%hu", &p1, &p2) == 2) {
      clientRTPPortNum = p1;
      clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p2; // ignore the second port number if the client asked for raw UDP
    } else if (sscanf(field, "client_port=%hu", &p1) == 1) {
      clientRTPPortNum = p1;
      clientRTCPPortNum = streamingMode == RAW_UDP ? 0 : p1 + 1;
    } else if (sscanf(field, "interleaved=%u-%u", &rtpCid, &rtcpCid) == 2) {
      rtpChannelId = (unsigned char)rtpCid;
      rtcpChannelId = (unsigned char)rtcpCid;
    }
    
    fields += strlen(field);
    while (*fields == ';' || *fields == ' ' || *fields == '\t') ++fields; // skip over separating ';' chars or whitespace
    if (*fields == '\0' || *fields == '\r' || *fields == '\n') break;
  }
  delete[] field;
}

static Boolean parsePlayNowHeader(char const* buf) {
  // Find "x-playNow:" header, if present
  while (1) {
    if (*buf == '\0') return False; // not found
    if (_strncasecmp(buf, "x-playNow:", 10) == 0) break;
    ++buf;
  }
  
  return True;
}

void RTSPServer::RTSPClientSession
::handleCmd_SETUP(RTSPServer::RTSPClientConnection* ourClientConnection,
		  char const* urlPreSuffix, char const* urlSuffix, char const* fullRequestStr) {
  // Normally, "urlPreSuffix" should be the session (stream) name, and "urlSuffix" should be the subsession (track) name.
  // However (being "liberal in what we accept"), we also handle 'aggregate' SETUP requests (i.e., without a track name),
  // in the special case where we have only a single track.  I.e., in this case, we also handle:
  //    "urlPreSuffix" is empty and "urlSuffix" is the session (stream) name, or
  //    "urlPreSuffix" concatenated with "urlSuffix" (with "/" inbetween) is the session (stream) name.
  char const* streamName = urlPreSuffix; // in the normal case
  char const* trackId = urlSuffix; // in the normal case
  char* concatenatedStreamName = NULL; // in the normal case

  //_TRACE(TRACE_LOG_DEBUG, (char*)"handleCmd_SETUP  socket[%d  - %d]\n", ourClientConnection->fClientInputSocket, ourClientConnection->fClientOutputSocket);
  
  //fOurServer.LockServerMediaSession(pEnv->GetEnvirName(), "RTSPClientSession::handleCmd_SETUP", (unsigned long long )this);

  do {
    // First, make sure the specified stream name exists:
    ServerMediaSession* sms
      = fOurServer.lookupServerMediaSession(pEnvironment, 2, ourClientConnection, streamName, False, fOurServerMediaSession == NULL);
    if (sms == NULL) {
      // Check for the special case (noted above), before we give up:
      if (urlPreSuffix[0] == '\0') {
	streamName = urlSuffix;
      } else {
	concatenatedStreamName = new char[strlen(urlPreSuffix) + strlen(urlSuffix) + 2]; // allow for the "/" and the trailing '\0'
	sprintf(concatenatedStreamName, "%s/%s", urlPreSuffix, urlSuffix);
	streamName = concatenatedStreamName;
      }
      trackId = NULL;
      
      // Check again:
      sms = fOurServer.lookupServerMediaSession(pEnvironment, 2, ourClientConnection, streamName, False, fOurServerMediaSession == NULL);
    }
    if (sms == NULL) 
	{
      if (fOurServerMediaSession == NULL) 
	  {
		// The client asked for a stream that doesn't exist (and this session descriptor has not been used before):
		ourClientConnection->handleCmd_notFound();
      } 
	  else 
	  {
		// The client asked for a stream that doesn't exist, but using a stream id for a stream that does exist. Bad request:
		ourClientConnection->handleCmd_bad();
      }
      break;
    } 
	else 
	{
		if (fOurServerMediaSession == NULL) 
		{
			if (&sms->envir() != pEnvironment)
			{
				_TRACE(TRACE_LOG_DEBUG, (char *)"handleCmd_SETUP ERROR envir[%s] 不匹配  [%s]\n", sms->envir().GetEnvirName(), pEnvironment->GetEnvirName());

				FILE *f = fopen("handleCmd_SETUP.txt", "wb");
				if (NULL != f)
				{
					char sztmp[256] = {0};
					sprintf(sztmp, (char*)"[%s] handleCmd_SETUP error. envir不匹配 sms.envir[%s]  ptr[0x%X]\n", pEnvironment->GetEnvirName(), sms->envir().GetEnvirName(), this);

					fwrite(sztmp, 1, (int)strlen(sztmp), f);
					fclose(f);
				}
			}

			// We're accessing the "ServerMediaSession" for the first time.
			fOurServerMediaSession = sms;
			fOurServerMediaSession->incrementReferenceCount();
		}
		else if (sms != fOurServerMediaSession) 
		{
			// The client asked for a stream that's different from the one originally requested for this stream id.  Bad request:
			ourClientConnection->handleCmd_bad();
			break;
		}
	}
    
    if (fStreamStates == NULL) 
	{
      // This is the first "SETUP" for this session.  Set up our array of states for all of this session's subsessions (tracks):
      fNumStreamStates = fOurServerMediaSession->numSubsessions();
      fStreamStates = new struct streamState[fNumStreamStates];
      
      ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
      ServerMediaSubsession* subsession;
      for (unsigned i = 0; i < fNumStreamStates; ++i) 
	  {
		subsession = iter.next();
		fStreamStates[i].subsession = subsession;
		fStreamStates[i].tcpSocketNum = -1; // for now; may get set for RTP-over-TCP streaming
		fStreamStates[i].streamToken = NULL; // for now; it may be changed by the "getStreamParameters()" call that comes later
      }
    }
    
    // Look up information for the specified subsession (track):
    ServerMediaSubsession* subsession = NULL;
    unsigned trackNum;
    if (trackId != NULL && trackId[0] != '\0') { // normal case
      for (trackNum = 0; trackNum < fNumStreamStates; ++trackNum) {
	subsession = fStreamStates[trackNum].subsession;
	if (subsession != NULL && strcmp(trackId, subsession->trackId()) == 0) break;
      }
      if (trackNum >= fNumStreamStates) {
	// The specified track id doesn't exist, so this request fails:
	ourClientConnection->handleCmd_notFound();
	break;
      }
    } else {
      // Weird case: there was no track id in the URL.
      // This works only if we have only one subsession:
      if (fNumStreamStates != 1 || fStreamStates[0].subsession == NULL) {
	ourClientConnection->handleCmd_bad();
	break;
      }
      trackNum = 0;
      subsession = fStreamStates[trackNum].subsession;
    }
    // ASSERT: subsession != NULL

    void*& token = fStreamStates[trackNum].streamToken; // alias
    if (token != NULL) {
      // We already handled a "SETUP" for this track (to the same client),
      // so stop any existing streaming of it, before we set it up again:
      subsession->pauseStream(fOurSessionId, token);

	  //fOurRTSPServer.LockClientConnection();

      fOurRTSPServer.unnoteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);
      subsession->deleteStream(pEnvironment, fOurSessionId, token);

	  //fOurRTSPServer.UnlockClientConnection();
    }

    // Look for a "Transport:" header in the request string, to extract client parameters:
    StreamingMode streamingMode;
    char* streamingModeString = NULL; // set when RAW_UDP streaming is specified
    char* clientsDestinationAddressStr;
    u_int8_t clientsDestinationTTL;
    portNumBits clientRTPPortNum, clientRTCPPortNum;
    unsigned char rtpChannelId, rtcpChannelId;
    parseTransportHeader(fullRequestStr, streamingMode, streamingModeString,
			 clientsDestinationAddressStr, clientsDestinationTTL,
			 clientRTPPortNum, clientRTCPPortNum,
			 rtpChannelId, rtcpChannelId);
    if ((streamingMode == RTP_TCP && rtpChannelId == 0xFF) ||
	(streamingMode != RTP_TCP && ourClientConnection->fClientOutputSocket != ourClientConnection->fClientInputSocket)) {
      // An anomolous situation, caused by a buggy client.  Either:
      //     1/ TCP streaming was requested, but with no "interleaving=" fields.  (QuickTime Player sometimes does this.), or
      //     2/ TCP streaming was not requested, but we're doing RTSP-over-HTTP tunneling (which implies TCP streaming).
      // In either case, we assume TCP streaming, and set the RTP and RTCP channel ids to proper values:
      streamingMode = RTP_TCP;
      rtpChannelId = fTCPStreamIdCount; rtcpChannelId = fTCPStreamIdCount+1;
    }
    if (streamingMode == RTP_TCP) fTCPStreamIdCount += 2;
    
    Port clientRTPPort(clientRTPPortNum);
    Port clientRTCPPort(clientRTCPPortNum);
    
    // Next, check whether a "Range:" or "x-playNow:" header is present in the request.
    // This isn't legal, but some clients do this to combine "SETUP" and "PLAY":
    double rangeStart = 0.0, rangeEnd = 0.0;
    char* absStart = NULL; char* absEnd = NULL;
    Boolean startTimeIsNow;
    if (parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow)) {
      delete[] absStart; delete[] absEnd;
      fStreamAfterSETUP = True;
    } else if (parsePlayNowHeader(fullRequestStr)) {
      fStreamAfterSETUP = True;
    } else {
      fStreamAfterSETUP = False;
    }
    
    // Then, get server parameters from the 'subsession':
    if (streamingMode == RTP_TCP) {
      // Note that we'll be streaming over the RTSP TCP connection:
      fStreamStates[trackNum].tcpSocketNum = ourClientConnection->fClientOutputSocket;
      fOurRTSPServer.noteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);

	  //此处增加计数
	  ourClientConnection->clientRequestTrackNum ++;
    }

    LIVE_NET_ADDRESS_INADDR destinationAddress;
    u_int8_t destinationTTL = 255;
#ifdef RTSP_ALLOW_CLIENT_DESTINATION_SETTING
    if (clientsDestinationAddressStr != NULL) {
      // Use the client-provided "destination" address.
      // Note: This potentially allows the server to be used in denial-of-service
      // attacks, so don't enable this code unless you're sure that clients are
      // trusted.
      destinationAddress = our_inet_addr(clientsDestinationAddressStr);
    }
    // Also use the client-provided TTL.
    destinationTTL = clientsDestinationTTL;
#endif
    delete[] clientsDestinationAddressStr;
    Port serverRTPPort(0);
    Port serverRTCPPort(0);
    
    // Make sure that we transmit on the same interface that's used by the client (in case we're a multi-homed server):
#if 0
    struct sockaddr_in sourceAddr; SOCKLEN_T namelen = sizeof sourceAddr;
    getsockname(ourClientConnection->fClientInputSocket, (struct sockaddr*)&sourceAddr, &namelen);
    LIVE_NET_ADDRESS_INADDR origSendingInterfaceAddr = SendingInterfaceAddr;
    LIVE_NET_ADDRESS_INADDR origReceivingInterfaceAddr = ReceivingInterfaceAddr;
    // NOTE: The following might not work properly, so we ifdef it out for now:
#ifdef HACK_FOR_MULTIHOMED_SERVERS
    ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
#endif

#else
	
    struct sockaddr_in sourceAddr4; SOCKLEN_T namelen4 = sizeof sourceAddr4;
	struct sockaddr_in6 sourceAddr6; SOCKLEN_T namelen6 = sizeof sourceAddr6;
	if (ourClientConnection->fOurIPVer == LIVE_IP_VER_4)
	{
		getsockname(ourClientConnection->fClientInputSocket, (struct sockaddr*)&sourceAddr4, &namelen4);
	}
	else
	{
		getsockname(ourClientConnection->fClientInputSocket, (struct sockaddr*)&sourceAddr6, &namelen6);
	}
    LIVE_NET_ADDRESS_INADDR origSendingInterfaceAddr = SendingInterfaceAddr;
    LIVE_NET_ADDRESS_INADDR origReceivingInterfaceAddr = ReceivingInterfaceAddr;
    // NOTE: The following might not work properly, so we ifdef it out for now:
#ifdef HACK_FOR_MULTIHOMED_SERVERS
    ReceivingInterfaceAddr = SendingInterfaceAddr = sourceAddr.sin_addr.s_addr;
#endif


#endif

	LIVE_NET_ADDRESS_INADDR tmpInAddr;
	//memcpy(&tmpInAddr.sin_addr, &ourClientConnection->fClientAddr.saddr4.sin_addr, sizeof(tmpInAddr.sin_addr));
	//memcpy(&tmpInAddr.sin6_addr, &ourClientConnection->fClientAddr.saddr6.sin6_addr, sizeof(tmpInAddr.sin6_addr));

//	memcpy(&tmpInAddr.sin_addr, &sourceAddr4.sin_addr, sizeof(tmpInAddr.sin_addr));
//	memcpy(&tmpInAddr.sin6_addr, &sourceAddr6.sin6_addr, sizeof(tmpInAddr.sin6_addr));
	//2018.11.09   udp发送流
	memcpy(&tmpInAddr.sin_addr, &ourClientConnection->fClientAddr.saddr4.sin_addr, sizeof(tmpInAddr.sin_addr));
	memcpy(&tmpInAddr.sin6_addr, &ourClientConnection->fClientAddr.saddr6.sin6_addr, sizeof(tmpInAddr.sin6_addr));

	subsession->getStreamParameters(pEnvironment, fOurSessionId, tmpInAddr,
				    clientRTPPort, clientRTCPPort,
				    fStreamStates[trackNum].tcpSocketNum, rtpChannelId, rtcpChannelId,
				    destinationAddress, destinationTTL, fIsMulticast,
				    serverRTPPort, serverRTCPPort,
					fStreamStates[trackNum].streamToken, ourClientConnection->fOurSocket, ourClientConnection->fOurIPVer, (char *)fullRequestStr);
    SendingInterfaceAddr = origSendingInterfaceAddr;
    ReceivingInterfaceAddr = origReceivingInterfaceAddr;
    
    AddressString destAddrStr(destinationAddress, ourClientConnection->fOurIPVer);

	LIVE_NET_ADDRESS_SOCKADDR_IN	srcAddr;
	memset(&srcAddr, 0x00, sizeof(LIVE_NET_ADDRESS_SOCKADDR_IN));
	memcpy(&srcAddr.saddr4, &sourceAddr4, sizeof(srcAddr.saddr4));
	memcpy(&srcAddr.saddr6, &sourceAddr6, sizeof(srcAddr.saddr6));

    AddressString sourceAddrStr(srcAddr, ourClientConnection->fOurIPVer);
    char timeoutParameterString[100];
    if (fOurRTSPServer.fReclamationSeconds > 0) {
      sprintf(timeoutParameterString, ";timeout=%u", fOurRTSPServer.fReclamationSeconds);
    } else {
      timeoutParameterString[0] = '\0';
    }
    if (fIsMulticast) {
      switch (streamingMode) {
          case RTP_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: RTP/AVP;multicast;destination=%s;source=%s;port=%d-%d;ttl=%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()), destinationTTL,
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
          case RTP_TCP: {
	    // multicast streams can't be sent via TCP
	    ourClientConnection->handleCmd_unsupportedTransport();
	    break;
	  }
          case RAW_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: %s;multicast;destination=%s;source=%s;port=%d;ttl=%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(serverRTPPort.num()), destinationTTL,
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
      }
    } else {
      switch (streamingMode) {
          case RTP_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: RTP/AVP;unicast;destination=%s;source=%s;client_port=%d-%d;server_port=%d-%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()), ntohs(clientRTCPPort.num()), ntohs(serverRTPPort.num()), ntohs(serverRTCPPort.num()),
		     fOurSessionId, timeoutParameterString);

		_TRACE(TRACE_LOG_INFO, (char *)"%s", ourClientConnection->fResponseBuffer);
	    break;
	  }
          case RTP_TCP: {
	    if (!fOurRTSPServer.fAllowStreamingRTPOverTCP) {
	      ourClientConnection->handleCmd_unsupportedTransport();
	    } else {
	      snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		       "RTSP/1.0 200 OK\r\n"
		       "CSeq: %s\r\n"
		       "%s"
		       "Transport: RTP/AVP/TCP;unicast;destination=%s;source=%s;interleaved=%d-%d\r\n"
		       "Session: %08X%s\r\n\r\n",
		       ourClientConnection->fCurrentCSeq,
		       dateHeader(),
		       destAddrStr.val(), sourceAddrStr.val(), rtpChannelId, rtcpChannelId,
		       fOurSessionId, timeoutParameterString);
	    }
	    break;
	  }
          case RAW_UDP: {
	    snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
		     "RTSP/1.0 200 OK\r\n"
		     "CSeq: %s\r\n"
		     "%s"
		     "Transport: %s;unicast;destination=%s;source=%s;client_port=%d;server_port=%d\r\n"
		     "Session: %08X%s\r\n\r\n",
		     ourClientConnection->fCurrentCSeq,
		     dateHeader(),
		     streamingModeString, destAddrStr.val(), sourceAddrStr.val(), ntohs(clientRTPPort.num()), ntohs(serverRTPPort.num()),
		     fOurSessionId, timeoutParameterString);
	    break;
	  }
      }
    }
    delete[] streamingModeString;
	streamingModeString = NULL;
  } while (0);

  //fOurServer.UnlockServerMediaSession(pEnv->GetEnvirName(), "RTSPClientSession::handleCmd_SETUP", (unsigned long long )this);
  
  delete[] concatenatedStreamName;
  concatenatedStreamName = NULL;
}

int RTSPServer::RTSPClientSession
::handleCmd_withinSession(RTSPServer::RTSPClientConnection* ourClientConnection,
			  char const* cmdName,
			  char const* urlPreSuffix, char const* urlSuffix,
			  char const* fullRequestStr) {
  // This will either be:
  // - a non-aggregated operation, if "urlPreSuffix" is the session (stream)
  //   name and "urlSuffix" is the subsession (track) name, or
  // - an aggregated operation, if "urlSuffix" is the session (stream) name,
  //   or "urlPreSuffix" is the session (stream) name, and "urlSuffix" is empty,
  //   or "urlPreSuffix" and "urlSuffix" are both nonempty, but when concatenated, (with "/") form the session (stream) name.
  // Begin by figuring out which of these it is:

	//fOurServer.LockServerMediaSession();		//Lock


	int procRet = -1;

	UsageEnvironment	*pTmpEnv = pEnvironment;

	pTmpEnv->SetLockFlag(0x01);

	//pTmpEnv->LockEnvir("RTSPClientSession::handleCmd_withinSession", (unsigned long long)this);

	if (NULL != fOurServerMediaSession)
	{
		if (&fOurServerMediaSession->envir() != pEnvironment)
		{
			_TRACE(TRACE_LOG_DEBUG, (char *)"handleCmd_withinSession ERROR envir[%s] 不匹配  [%s]\n", fOurServerMediaSession->envir().GetEnvirName(), pEnvironment->GetEnvirName());

			FILE *f = fopen("handleCmd_withinSession.txt", "wb");
			if (NULL != f)
			{
				char sztmp[256] = {0};
				sprintf(sztmp, (char*)"[%s] handleCmd_withinSession error. envir不匹配 sms.envir[%s]  ptr[0x%X]\n", pEnvironment->GetEnvirName(), fOurServerMediaSession->envir().GetEnvirName(), this);

				fwrite(sztmp, 1, (int)strlen(sztmp), f);
				fclose(f);
			}
		}
	}

	do
	{
		ServerMediaSubsession* subsession;
		/*
		_TRACE(TRACE_LOG_DEBUG, (char*)"[%s] handleCmd_withinSession[0x%X]  cmdName[%s]  socket[%d  - %d]  %s\n", 
								pEnvironment->GetEnvirName(),
								this, cmdName, 
								ourClientConnection->fClientInputSocket, ourClientConnection->fClientOutputSocket, fullRequestStr);
  */
		if (fOurServerMediaSession == NULL) 
		{ // There wasn't a previous SETUP!
			ourClientConnection->handleCmd_notSupported();
			//return 0;
			procRet = -1;
			break;
		} 
		else if (urlSuffix[0] != '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0) 
		{
			// Non-aggregated operation.
			// Look up the media subsession whose track id is "urlSuffix":
			ServerMediaSubsessionIterator iter(*fOurServerMediaSession);
			while ((subsession = iter.next()) != NULL) 
			{
				if (strcmp(subsession->trackId(), urlSuffix) == 0) break; // success
			}
			if (subsession == NULL) 
			{ // no such track!
				ourClientConnection->handleCmd_notFound();
				procRet = -2;
				//return 0;
				break;
			}
		} 
		else if (strcmp(fOurServerMediaSession->streamName(), urlSuffix) == 0 ||
					(urlSuffix[0] == '\0' && strcmp(fOurServerMediaSession->streamName(), urlPreSuffix) == 0)) 
		{
			// Aggregated operation
			subsession = NULL;
		} 
		else if (urlPreSuffix[0] != '\0' && urlSuffix[0] != '\0') 
		{
			// Aggregated operation, if <urlPreSuffix>/<urlSuffix> is the session (stream) name:
			unsigned const urlPreSuffixLen = (unsigned)strlen(urlPreSuffix);
			if (strncmp(fOurServerMediaSession->streamName(), urlPreSuffix, urlPreSuffixLen) == 0 &&
				fOurServerMediaSession->streamName()[urlPreSuffixLen] == '/' &&
				strcmp(&(fOurServerMediaSession->streamName())[urlPreSuffixLen+1], urlSuffix) == 0) 
			{
				subsession = NULL;
			} else
			{
				ourClientConnection->handleCmd_notFound();
				//return 0;
				procRet = -3;
				break;
			}
		}
		else
		{	// the request doesn't match a known stream and/or track at all!
			ourClientConnection->handleCmd_notFound();
			//return 0;
			procRet = -4;
			break;
		}
  
		if (strcmp(cmdName, "TEARDOWN") == 0) 
		{
			//GenericMediaServer *pOurServer = &fOurServer;
			//pOurServer->LockClientConnection();			//Lock

			

			handleCmd_TEARDOWN(ourClientConnection, subsession);
			//pOurServer->UnlockClientConnection();			//Unlock
			//return MAIN_THREAD_ID;

			procRet = 0xFF;


			break;
		}
		else if (strcmp(cmdName, "PLAY") == 0) 
		{
			//fOurServer.LockServerMediaSession(ourClientConnection->pEnv->GetEnvirName(), "RTSPClientSession::handleCmd_withinSession:PLAY", (unsigned long long)this);		//Lock

			handleCmd_PLAY(ourClientConnection, subsession, fullRequestStr);
			//fOurServer.UnlockServerMediaSession(ourClientConnection->pEnv->GetEnvirName(), "RTSPClientSession::handleCmd_withinSession:PLAY", (unsigned long long)this);		//Unlock
		} 
		else if (strcmp(cmdName, "PAUSE") == 0) 
		{
			handleCmd_PAUSE(ourClientConnection, subsession);

			procRet = 0x00;
		} 
		else if (strcmp(cmdName, "GET_PARAMETER") == 0) 
		{
			handleCmd_GET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
			procRet = 0x00;
		} 
		else if (strcmp(cmdName, "SET_PARAMETER") == 0) 
		{
			handleCmd_SET_PARAMETER(ourClientConnection, subsession, fullRequestStr);
			procRet = 0x00;
		}
	}while (0);
	//fOurServer.UnlockServerMediaSession();		//Unlock

	pTmpEnv->SetLockFlag(0);
	//pTmpEnv->UnlockEnvir("RTSPClientSession::handleCmd_withinSession", (unsigned long long)this);

  return procRet;
}

void RTSPServer::RTSPClientSession
::handleCmd_TEARDOWN(RTSPServer::RTSPClientConnection* ourClientConnection,
		     ServerMediaSubsession* subsession) {

	if (NULL == fStreamStates)		return;

	unsigned i = 0;
	

	//fOurRTSPServer.LockClientSession();

	//fOurRTSPServer.LockServerMediaSession(ourClientConnection->pEnv->GetEnvirName(), "RTSPClientSession::handleCmd_TEARDOWN", (unsigned long long)this);

	for (i = 0; i < fNumStreamStates; ++i)
	{
		if (subsession == NULL /* means: aggregated operation */
			|| subsession == fStreamStates[i].subsession) 
		{
			if (fStreamStates[i].subsession != NULL) 
			{
				fOurRTSPServer.unnoteTCPStreamingOnSocket(fStreamStates[i].tcpSocketNum, this, i);
				fStreamStates[i].subsession->deleteStream(pEnvironment, fOurSessionId, fStreamStates[i].streamToken);
				fStreamStates[i].subsession = NULL;
			}
		}
	}

	//fOurRTSPServer.UnlockServerMediaSession(ourClientConnection->pEnv->GetEnvirName(), "RTSPClientSession::handleCmd_TEARDOWN", (unsigned long long)this);

	//fOurRTSPServer.UnlockClientSession();

	setRTSPResponse(ourClientConnection, "200 OK");
  
  // Optimization: If all subsessions have now been torn down, then we know that we can reclaim our object now.
  // (Without this optimization, however, this object would still get reclaimed later, as a result of a 'liveness' timeout.)
	Boolean noSubsessionsRemain = True;
	for (i = 0; i < fNumStreamStates; ++i) 
	{
		if (fStreamStates[i].subsession != NULL) 
		{
			noSubsessionsRemain = False;
			break;
		}
	}

	if (noSubsessionsRemain)
	{
		delete this;
	}
}

void RTSPServer::RTSPClientSession
::handleCmd_PLAY(RTSPServer::RTSPClientConnection* ourClientConnection,
		 ServerMediaSubsession* subsession, char const* fullRequestStr) {
  char* rtspURL
	  = fOurRTSPServer.rtspURL(fOurServerMediaSession, ourClientConnection->fOurIPVer, ourClientConnection->fClientInputSocket);
  unsigned rtspURLSize = (unsigned)strlen(rtspURL);
  
  // Parse the client's "Scale:" header, if any:
  float scale;
  Boolean sawScaleHeader = parseScaleHeader(fullRequestStr, scale);
  
  // Try to set the stream's scale factor to this value:
  if (subsession == NULL /*aggregate op*/) {
    fOurServerMediaSession->testScaleFactor(scale);
  } else {
    subsession->testScaleFactor(scale);
  }
  
  char buf[100];
  char* scaleHeader;
  if (!sawScaleHeader) {
    buf[0] = '\0'; // Because we didn't see a Scale: header, don't send one back
  } else {
    sprintf(buf, "Scale: %f\r\n", scale);
  }
  scaleHeader = strDup(buf);
  
  // Parse the client's "Range:" header, if any:
  float duration = 0.0;
  double rangeStart = 0.0, rangeEnd = 0.0;
  char* absStart = NULL; char* absEnd = NULL;
  Boolean startTimeIsNow;
  Boolean sawRangeHeader
    = parseRangeHeader(fullRequestStr, rangeStart, rangeEnd, absStart, absEnd, startTimeIsNow);
  
  if (sawRangeHeader && absStart == NULL/*not seeking by 'absolute' time*/) {
    // Use this information, plus the stream's duration (if known), to create our own "Range:" header, for the response:
    duration = subsession == NULL /*aggregate op*/
      ? fOurServerMediaSession->duration() : subsession->duration();
    if (duration < 0.0) {
      // We're an aggregate PLAY, but the subsessions have different durations.
      // Use the largest of these durations in our header
      duration = -duration;
    }
    
    // Make sure that "rangeStart" and "rangeEnd" (from the client's "Range:" header)
    // have sane values, before we send back our own "Range:" header in our response:
    if (rangeStart < 0.0) rangeStart = 0.0;
    else if (rangeStart > duration) rangeStart = duration;
    if (rangeEnd < 0.0) rangeEnd = 0.0;
    else if (rangeEnd > duration) rangeEnd = duration;
    if ((scale > 0.0 && rangeStart > rangeEnd && rangeEnd > 0.0) ||
	(scale < 0.0 && rangeStart < rangeEnd)) {
      // "rangeStart" and "rangeEnd" were the wrong way around; swap them:
      double tmp = rangeStart;
      rangeStart = rangeEnd;
      rangeEnd = tmp;
    }
  }
  
  // Create a "RTP-Info:" line.  It will get filled in from each subsession's state:
  char const* rtpInfoFmt =
    "%s" // "RTP-Info:", plus any preceding rtpInfo items
    "%s" // comma separator, if needed
    "url=%s/%s"
    ";seq=%d"
    ";rtptime=%u"
    ;
  unsigned rtpInfoFmtSize = (unsigned)strlen(rtpInfoFmt);
  char* rtpInfo = strDup("RTP-Info: ");
  unsigned i, numRTPInfoItems = 0;
  
  // Do any required seeking/scaling on each subsession, before starting streaming.
  // (However, we don't do this if the "PLAY" request was for just a single subsession
  // of a multiple-subsession stream; for such streams, seeking/scaling can be done
  // only with an aggregate "PLAY".)
  for (i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */ || fNumStreamStates == 1) {
      if (fStreamStates[i].subsession != NULL) {
	if (sawScaleHeader) {
	  fStreamStates[i].subsession->setStreamScale(fOurSessionId, fStreamStates[i].streamToken, scale);
	}
	if (absStart != NULL) {
	  // Special case handling for seeking by 'absolute' time:
	
	  fStreamStates[i].subsession->seekStream(fOurSessionId, fStreamStates[i].streamToken, absStart, absEnd);
	} else {
	  // Seeking by relative (NPT) time:
	  
	  u_int64_t numBytes;
	  if (!sawRangeHeader || startTimeIsNow) {
	    // We're resuming streaming without seeking, so we just do a 'null' seek
	    // (to get our NPT, and to specify when to end streaming):
	    fStreamStates[i].subsession->nullSeekStream(fOurSessionId, fStreamStates[i].streamToken,
							rangeEnd, numBytes);
	  } else {
	    // We do a real 'seek':
	    double streamDuration = 0.0; // by default; means: stream until the end of the media
	    if (rangeEnd > 0.0 && (rangeEnd+0.001) < duration) {
	      // the 0.001 is because we limited the values to 3 decimal places
	      // We want the stream to end early.  Set the duration we want:
	      streamDuration = rangeEnd - rangeStart;
	      if (streamDuration < 0.0) streamDuration = -streamDuration;
	          // should happen only if scale < 0.0
	    }
	    fStreamStates[i].subsession->seekStream(fOurSessionId, fStreamStates[i].streamToken,
						    rangeStart, streamDuration, numBytes);
	  }
	}
      }
    }
  }
  
  // Create the "Range:" header that we'll send back in our response.
  // (Note that we do this after seeking, in case the seeking operation changed the range start time.)
  if (absStart != NULL) {
    // We're seeking by 'absolute' time:
    if (absEnd == NULL) {
      sprintf(buf, "Range: clock=%s-\r\n", absStart);
    } else {
      sprintf(buf, "Range: clock=%s-%s\r\n", absStart, absEnd);
    }
    delete[] absStart; delete[] absEnd;
  } else {
    // We're seeking by relative (NPT) time:
    if (!sawRangeHeader || startTimeIsNow) {
      // We didn't seek, so in our response, begin the range with the current NPT (normal play time):
      float curNPT = 0.0;
      for (i = 0; i < fNumStreamStates; ++i) {
	if (subsession == NULL /* means: aggregated operation */
	    || subsession == fStreamStates[i].subsession) {
	  if (fStreamStates[i].subsession == NULL) continue;
	  float npt = fStreamStates[i].subsession->getCurrentNPT(fStreamStates[i].streamToken);
	  if (npt > curNPT) curNPT = npt;
	  // Note: If this is an aggregate "PLAY" on a multi-subsession stream,
	  // then it's conceivable that the NPTs of each subsession may differ
	  // (if there has been a previous seek on just one subsession).
	  // In this (unusual) case, we just return the largest NPT; I hope that turns out OK...
	}
      }
      rangeStart = curNPT;
    }

    if (rangeEnd == 0.0 && scale >= 0.0) {
      sprintf(buf, "Range: npt=%.3f-\r\n", rangeStart);
    } else {
      sprintf(buf, "Range: npt=%.3f-%.3f\r\n", rangeStart, rangeEnd);
    }
  }
  char* rangeHeader = strDup(buf);
  
  // Now, start streaming:
  for (i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */
	|| subsession == fStreamStates[i].subsession) {
      unsigned short rtpSeqNum = 0;
      unsigned rtpTimestamp = 0;
      if (fStreamStates[i].subsession == NULL) continue;

#ifdef LIVE_MULTI_THREAD_ENABLE
	  //将该socket切换到工作线程

	  //fOurServerMediaSession->SetStreamStatus(1);	//add by gavin 2018.04.17

	  //fOurServer.LockClientConnection();		//gavin 2018.03.14
	  //_TRACE(TRACE_LOG_DEBUG, "从主线程中移除该Socket[%d].\n", ourClientConnection->fOurSocket);
	  //envir().taskScheduler().disableBackgroundHandling(ourClientConnection->fOurSocket);
	  //fOurServer.UnlockClientConnection();		//gavin 2018.03.14

	  fStreamStates[i].subsession->startStream(*pEnvironment,
							fOurSessionId,
					       fStreamStates[i].streamToken,
					       (TaskFunc*)noteClientLiveness, this,
					       rtpSeqNum, rtpTimestamp,
					       RTSPServer::RTSPClientConnection::handleAlternativeRequestByte, ourClientConnection);
#else
	  //fStreamStates[i].subsession->startStream(*ourClientConnection->pEnv,
	  fStreamStates[i].subsession->startStream(envir(),
							fOurSessionId,
					       fStreamStates[i].streamToken,
					       (TaskFunc*)noteClientLiveness, this,
					       rtpSeqNum, rtpTimestamp,
					       RTSPServer::RTSPClientConnection::handleAlternativeRequestByte, ourClientConnection);
#endif
      const char *urlSuffix = fStreamStates[i].subsession->trackId();
      char* prevRTPInfo = rtpInfo;
      unsigned rtpInfoSize = rtpInfoFmtSize
	+ (unsigned)strlen(prevRTPInfo)
	+ 1
	+ rtspURLSize + (unsigned)strlen(urlSuffix)
	+ 5 /*max unsigned short len*/
	+ 10 /*max unsigned (32-bit) len*/
	+ 2 /*allows for trailing \r\n at final end of string*/;
      rtpInfo = new char[rtpInfoSize];
      sprintf(rtpInfo, rtpInfoFmt,
	      prevRTPInfo,
	      numRTPInfoItems++ == 0 ? "" : ",",
	      rtspURL, urlSuffix,
	      rtpSeqNum,
	      rtpTimestamp
	      );
      delete[] prevRTPInfo;
    }
  }
  if (numRTPInfoItems == 0) {
    rtpInfo[0] = '\0';
  } else {
    unsigned rtpInfoLen = (unsigned)strlen(rtpInfo);
    rtpInfo[rtpInfoLen] = '\r';
    rtpInfo[rtpInfoLen+1] = '\n';
    rtpInfo[rtpInfoLen+2] = '\0';
  }
  
  // Fill in the response:
  snprintf((char*)ourClientConnection->fResponseBuffer, sizeof ourClientConnection->fResponseBuffer,
	   "RTSP/1.0 200 OK\r\n"
	   "CSeq: %s\r\n"
	   "%s"
	   "%s"
	   "%s"
	   "Session: %08X\r\n"
	   "%s\r\n",
	   ourClientConnection->fCurrentCSeq,
	   dateHeader(),
	   scaleHeader,
	   rangeHeader,
	   fOurSessionId,
	   rtpInfo);
  delete[] rtpInfo; delete[] rangeHeader;
  delete[] scaleHeader; delete[] rtspURL;
}

void RTSPServer::RTSPClientSession::handleCmd_RTCP(RTSPServer::RTSPClientConnection* ourClientConnection, ServerMediaSubsession* subsession) 
{

#if 1
  //2016.05.24   gavin
  for (unsigned trackNum = 0; trackNum < fNumStreamStates; ++trackNum) 
  {
    if (fStreamStates[trackNum].subsession != NULL) 
	{
      if (fStreamStates[trackNum].tcpSocketNum == ourClientConnection->fClientOutputSocket)
      {
      	fOurRTSPServer.noteTCPStreamingOnSocket(fStreamStates[trackNum].tcpSocketNum, this, trackNum);
      }
    }
  }
#endif

#if 0		//gavin 2018.03.12
  unsigned i = 0;
  // Now, start streaming:
  for (i = 0; i < fNumStreamStates; ++i) 
  {
    if (subsession == NULL || subsession == fStreamStates[i].subsession) 
	{
      unsigned short rtpSeqNum = 0;
      unsigned rtpTimestamp = 0;
      if (fStreamStates[i].subsession == NULL) continue;
	  fStreamStates[i].subsession->startStream(*ourClientConnection->pEnv,  fOurSessionId,
					       fStreamStates[i].streamToken,
					       (TaskFunc*)noteClientLiveness, this,
					       rtpSeqNum, rtpTimestamp,
					       RTSPServer::RTSPClientConnection::handleAlternativeRequestByte, ourClientConnection);
    }
  }
#endif

}

void RTSPServer::RTSPClientSession
::handleCmd_PAUSE(RTSPServer::RTSPClientConnection* ourClientConnection,
		  ServerMediaSubsession* subsession) {
  for (unsigned i = 0; i < fNumStreamStates; ++i) {
    if (subsession == NULL /* means: aggregated operation */
	|| subsession == fStreamStates[i].subsession) {
      if (fStreamStates[i].subsession != NULL) {
	fStreamStates[i].subsession->pauseStream(fOurSessionId, fStreamStates[i].streamToken);
      }
    }
  }
  
  setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId);
}

void RTSPServer::RTSPClientSession
::handleCmd_GET_PARAMETER(RTSPServer::RTSPClientConnection* ourClientConnection,
			  ServerMediaSubsession* /*subsession*/, char const* /*fullRequestStr*/) {
  // By default, we implement "GET_PARAMETER" just as a 'keep alive', and send back a dummy response.
  // (If you want to handle "GET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
  setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId, LIVEMEDIA_LIBRARY_VERSION_STRING);
}

void RTSPServer::RTSPClientSession
::handleCmd_SET_PARAMETER(RTSPServer::RTSPClientConnection* ourClientConnection,
			  ServerMediaSubsession* /*subsession*/, char const* /*fullRequestStr*/) {
  // By default, we implement "SET_PARAMETER" just as a 'keep alive', and send back an empty response.
  // (If you want to handle "SET_PARAMETER" properly, you can do so by defining a subclass of "RTSPServer"
  // and "RTSPServer::RTSPClientSession", and then reimplement this virtual function in your subclass.)
  setRTSPResponse(ourClientConnection, "200 OK", fOurSessionId);
}

GenericMediaServer::ClientConnection*
RTSPServer::createNewClientConnection(int clientSocket, LIVE_NET_ADDRESS_SOCKADDR_IN clientAddr, LIVE_IP_VER_ENUM ipVer) {
  return new RTSPClientConnection(*this, clientSocket, clientAddr, ipVer);
}

GenericMediaServer::ClientSession*
RTSPServer::createNewClientSession(u_int32_t sessionId, UsageEnvironment	*pEnv) {
  return new RTSPClientSession(*this, sessionId, pEnv);//&envir());
}
