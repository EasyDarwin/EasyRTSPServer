
#ifndef  __LIVE_JPEG_STREAM_SOURCE
#define __LIVE_JPEG_STREAM_SOURCE


#ifndef _JPEG_VIDEO_SOURCE_HH
#include <JPEGVideoSource.hh>
#endif

#include "LiveSource.h"

class LiveJPEGStreamSource: public JPEGVideoSource {
public:
  static LiveJPEGStreamSource* createNew(FramedSource* inputSource); 

private:
  LiveJPEGStreamSource(FramedSource* inputSource);
      // called only by createNew()

  virtual ~LiveJPEGStreamSource();

private: // redefined virtual functions
  virtual void doGetNextFrame();

  virtual u_int8_t type();
  virtual u_int8_t qFactor();
  virtual u_int8_t width();
  virtual u_int8_t height();
  virtual u_int8_t const* quantizationTables(u_int8_t& precision,
                                             u_int16_t& length);
private:
  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
                                struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                          struct timeval presentationTime,
                          unsigned durationInMicroseconds);

private:
  FramedSource* fSource;
  u_int8_t fLastWidth, fLastHeight; // actual dimensions /8
  u_int8_t fLastQuantizationTable[256];//128];
  u_int16_t fLastQuantizationTableSize;
  unsigned char fBuffer[VIDEO_MAX_FRAME_SIZE];
};

#endif
