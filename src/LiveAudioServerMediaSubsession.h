

#ifndef _LIVE_AUDIO_SERVER_MEDIA_SUBSESSION_H
#define _LIVE_AUDIO_SERVER_MEDIA_SUBSESSION_H

#include "LiveServerMediaSubsession.h"
#include "RtspTypes.h"

class LiveAudioServerMediaSubsession: public LiveServerMediaSubsession 
{
public:
  static LiveAudioServerMediaSubsession*  createNew(UsageEnvironment& env, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort);

  void setDoneFlag() { fDoneFlag = ~0; }
  void checkForAuxSDPLine1();

private:
  LiveAudioServerMediaSubsession(UsageEnvironment& env,  RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort);
      // called only by createNew()
  virtual ~LiveAudioServerMediaSubsession();

private: // redefined virtual functions
  //virtual char const* getAuxSDPLine(RTPSink* rtpSink,   FramedSource* inputSource);
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,  unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,  unsigned char rtpPayloadTypeIfDynamic,  FramedSource* inputSource);

private:
  char fDoneFlag; // used when setting up 'SDPlines'
  RTPSink* fDummyRTPSink; // ditto

  unsigned fSamplingFrequency;
  unsigned fNumChannels;
};

#endif
