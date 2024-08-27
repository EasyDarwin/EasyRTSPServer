#ifndef _LIVE_SERVER_MEDIA_SUBSESSION_H
#define _LIVE_SERVER_MEDIA_SUBSESSION_H

#include <OnDemandServerMediaSubsession.hh>
#include "LiveSource.h"

class LiveServerMediaSubsession: public OnDemandServerMediaSubsession 
{
protected: // we're a virtual base class
  LiveServerMediaSubsession(UsageEnvironment& env, LiveSource *liveSource, char *streamName, char *mediaTypeName, unsigned estimatedBitrate, RTSP_MEDIA_INFO_T *mediainfo, Boolean reuseFirstSource, unsigned int mediaType, portNumBits defaultPort);
  virtual ~LiveServerMediaSubsession();


  FramedSource* createAudioSource(UsageEnvironment& env, RTSP_MEDIA_INFO_T *pMediaInfo, LiveSource *liveSource);	//´´½¨ÒôÆµ source
#if 0
  virtual void pauseStream(unsigned clientSessionId, void* streamToken);
  virtual void seekStream(unsigned clientSessionId, void* streamToken, double& seekNPT,
			  double streamDuration, u_int64_t& numBytes);
     // This routine is used to seek by relative (i.e., NPT) time.
     // "streamDuration", if >0.0, specifies how much data to stream, past "seekNPT".  (If <=0.0, all remaining data is streamed.)
     // "numBytes" returns the size (in bytes) of the data to be streamed, or 0 if unknown or unlimited.
  virtual void seekStream(unsigned clientSessionId, void* streamToken, char*& absStart, char*& absEnd);
     // This routine is used to seek by 'absolute' time.
     // "absStart" should be a string of the form "YYYYMMDDTHHMMSSZ" or "YYYYMMDDTHHMMSS.<frac>Z".
     // "absEnd" should be either NULL (for no end time), or a string of the same form as "absStart".
     // These strings may be modified in-place, or can be reassigned to a newly-allocated value (after delete[]ing the original).
  virtual void nullSeekStream(unsigned clientSessionId, void* streamToken,
			      double streamEndTime, u_int64_t& numBytes);
#endif
     // Called whenever we're handling a "PLAY" command without a specified start time.
  //virtual void setStreamScale(unsigned clientSessionId, void* streamToken, float scale);

  //gavin 2017.12.08
  //virtual float getCurrentNPT(void* streamToken);
  virtual void getAbsoluteTimeRange(char*& absStartTime, char*& absEndTime);// const;
  virtual void testScaleFactor(float& scale);
  virtual void seekStreamSource(FramedSource* inputSource, char*& absStart, char*& absEnd);
  virtual void	pauseStreamSource();
  virtual void  resumeStreamSource();
protected:
  LiveSource *fLiveSource;
  unsigned fEstimatedKbps;

  //RTSP_PLAY_CONTROL_INFO_T	durationCtlInfo;

  unsigned int mMediaType;

  RTSP_MEDIA_INFO_T	*mMediaInfo;
  void		*mCallbackPtr;
  void		*mUserPtr;


  virtual void setStreamSourceScale(FramedSource* inputSource, float scale);

//SS_QUEUE_OBJ_T	*mVideoQueue;
//SS_QUEUE_OBJ_T	*mAudioQueue;
};

#endif
