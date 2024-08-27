

#ifndef _LIVE_H264_VIDEO_SERVER_MEDIA_SUBSESSION_H
#define _LIVE_H264_VIDEO_SERVER_MEDIA_SUBSESSION_H

#include "LiveServerMediaSubsession.h"
#include "RtspTypes.h"

class LiveH264VideoServerMediaSubsession: public LiveServerMediaSubsession {
public:
  static LiveH264VideoServerMediaSubsession*  createNew(UsageEnvironment& env, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *LiveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort);

  void setDoneFlag() { fDoneFlag = ~0; }
  void checkForAuxSDPLine1();

  void	IncreaseCount()	{mCounter ++;}
  int		GetCounter()		{return mCounter;}

private:
  LiveH264VideoServerMediaSubsession(UsageEnvironment& env,  unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *LiveSource, char *streamName, char *mediaTypeName, Boolean reuseFirstSource, portNumBits defaultPort);
      // called only by createNew()
  virtual ~LiveH264VideoServerMediaSubsession();

private: // redefined virtual functions
  virtual char const* getAuxSDPLine(RTPSink* rtpSink,
				    FramedSource* inputSource);
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
					      unsigned& estBitrate);
  virtual RTPSink* createNewRTPSink(Groupsock* rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
				    FramedSource* inputSource);

private:
  char fDoneFlag; // used when setting up 'SDPlines'
  RTPSink* fDummyRTPSink; // ditto

  int		mCounter;			//gavin 计时器计数
};

#endif
