#include "LiveJPEGStreamSource.h"

LiveJPEGStreamSource*
LiveJPEGStreamSource::createNew(FramedSource* inputSource) {
  return new LiveJPEGStreamSource(inputSource);
}

LiveJPEGStreamSource::LiveJPEGStreamSource(FramedSource* inputSource)
  : JPEGVideoSource(inputSource->envir()),
    fLastWidth(0), fLastHeight(0) {
  fSource = inputSource;
}

LiveJPEGStreamSource::~LiveJPEGStreamSource() {
  Medium::close(fSource);
}

void LiveJPEGStreamSource::doGetNextFrame() {
  fSource->getNextFrame(fBuffer, sizeof fBuffer,
			LiveJPEGStreamSource::afterGettingFrame, this,
			FramedSource::handleClosure, this);
}

u_int8_t LiveJPEGStreamSource::type() {
  return 1;
};

u_int8_t LiveJPEGStreamSource::qFactor() {
  return 255; // indicates that quantization tables are dynamic
};

u_int8_t LiveJPEGStreamSource::width() {
  return fLastWidth;
}

u_int8_t LiveJPEGStreamSource::height() {
  return fLastHeight;
}

u_int8_t const* LiveJPEGStreamSource::quantizationTables(u_int8_t& precision,
							   u_int16_t& length) {
  precision = 0;
  length = fLastQuantizationTableSize;
  return fLastQuantizationTable;
}
 
void LiveJPEGStreamSource
::afterGettingFrame(void* clientData, unsigned frameSize,
		    unsigned numTruncatedBytes,
		    struct timeval presentationTime,
		    unsigned durationInMicroseconds) {
  LiveJPEGStreamSource* source = (LiveJPEGStreamSource*)clientData;
  source->afterGettingFrame1(frameSize, numTruncatedBytes,
			     presentationTime, durationInMicroseconds);
}

#if 1
void LiveJPEGStreamSource
::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
		     struct timeval presentationTime,
		     unsigned durationInMicroseconds) {

				 printf("framesize: %d\n", frameSize);

  fFrameSize = frameSize < fMaxSize ? frameSize : fMaxSize;
  // NOTE: Change the following if the size of the encoder's JPEG hdr changes
  unsigned const JPEGHeaderSize = 524;
#if 1
  // Look for the "SOF0" marker (0xFF 0xC0) in the header, to get the frame
  // width and height.  Also, look for the "DQT" marker(s) (0xFF 0xDB), to
  // get the quantization table(s):
  Boolean foundSOF0 = False;
  fLastQuantizationTableSize = 0;
  for (unsigned i = 0; i < JPEGHeaderSize-8; ++i) 
  {
    if (fBuffer[i] == 0xFF) {
      if (fBuffer[i+1] == 0xDB) { // DQT
	u_int16_t length = (fBuffer[i+2]<<8) | fBuffer[i+3];
	if (i+2 + length < JPEGHeaderSize) { // sanity check
	  u_int16_t tableSize = length - 3;
	  if (fLastQuantizationTableSize + tableSize > 128) { // sanity check
	    tableSize = 128 - fLastQuantizationTableSize;
	  }
	  memmove(&fLastQuantizationTable[fLastQuantizationTableSize],
		  &fBuffer[i+5], tableSize);
	  fLastQuantizationTableSize += tableSize;
	  if (fLastQuantizationTableSize == 128 && foundSOF0) break;
	      // we've found everything that we want
	  i += length; // skip over table
	}
      } else if (fBuffer[i+1] == 0xC0) { // SOF0
	fLastHeight = (fBuffer[i+5]<<5)|(fBuffer[i+6]>>3);
	fLastWidth = (fBuffer[i+7]<<5)|(fBuffer[i+8]>>3);
	foundSOF0 = True;
	if (fLastQuantizationTableSize == 128) break;
	    // we've found everything that we want
	i += 8;
      }
    }
  }
  if (fLastQuantizationTableSize == 64) {
    // Hack: We apparently saw only one quantization table.  Unfortunately,
    // media players seem to be unhappy if we don't send two (luma+chroma).
    // So, duplicate the existing table data:
    memmove(&fLastQuantizationTable[64], fLastQuantizationTable, 64);
    fLastQuantizationTableSize = 128;
  }
  if (!foundSOF0) envir() << "Failed to find SOF0 marker in JPEG header!\n";
#endif
  // Complete delivery to the client:
  fFrameSize = fFrameSize > JPEGHeaderSize ? fFrameSize - JPEGHeaderSize : 0;
  memmove(fTo, &fBuffer[JPEGHeaderSize], fFrameSize);
  fNumTruncatedBytes = numTruncatedBytes;
  fPresentationTime = presentationTime;
  fDurationInMicroseconds = durationInMicroseconds;
  //FramedSource::afterGetting(this);
}

#else

void LiveJPEGStreamSource
::afterGettingFrame1(unsigned frameSize,unsigned numTruncatedBytes,struct timeval presentationTime,unsigned durationInMicroseconds)
{
    int headerSize = 0;
    bool headerOk = false;
    fFrameSize = 0;

    for (unsigned int i = 0; i < frameSize ; ++i)
    {
        // SOF
        if ( (i+8) < frameSize  && fTo[i] == 0xFF && fTo[i+1] == 0xC0 )
        {
                m_height = (fTo[i+5]<<5)|(fTo[i+6]>>3);
                m_width = (fTo[i+7]<<5)|(fTo[i+8]>>3);
        }
        // DQT
        if ( (i+5+64) < frameSize && fTo[i] == 0xFF && fTo[i+1] == 0xDB)
        {
            if (fTo[i+4] ==0)
            {
                memcpy(m_qTable, fTo + i + 5, 64);
                m_qTable0Init = true;
            }
            else if (fTo[i+4] ==1)
            {
                memcpy(m_qTable + 64, fTo + i + 5, 64);
                m_qTable1Init = true;
            }
        }
        // End of header
        if ( (i+1) < frameSize && fTo[i] == 0x3F && fTo[i+1] == 0x00 )
        {
                headerOk = true;
                headerSize = i+2;
                break;
        }
    }

    if (headerOk)
    {
        fFrameSize = frameSize - headerSize;
        memmove( fTo, fTo + headerSize, fFrameSize );
    }

    fNumTruncatedBytes = numTruncatedBytes;
    fPresentationTime = presentationTime;
    fDurationInMicroseconds = durationInMicroseconds;
    afterGetting(this);
}
#endif