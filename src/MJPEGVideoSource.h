#include "JPEGVideoSource.hh"

class MJPEGVideoSource : public JPEGVideoSource
{
        public:
                static MJPEGVideoSource* createNew (UsageEnvironment& env, FramedSource* source)
                {
                        return new MJPEGVideoSource(env,source);
                }
                virtual void doGetNextFrame()
                {
                    if (m_inputSource)
                        m_inputSource->getNextFrame(fTo, fMaxSize, afterGettingFrameSub, this, FramedSource::handleClosure, this);                     
                }
                virtual void doStopGettingFrames()
                {
                    FramedSource::doStopGettingFrames();
                    if (m_inputSource)
                        m_inputSource->stopGettingFrames();                    
                }
                static void afterGettingFrameSub(void* clientData, unsigned frameSize,unsigned numTruncatedBytes,struct timeval presentationTime,unsigned durationInMicroseconds) 
                {
                                MJPEGVideoSource* source = (MJPEGVideoSource*)clientData;
                                source->afterGettingFrame(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
                }        
                void afterGettingFrame(unsigned frameSize,unsigned numTruncatedBytes,struct timeval presentationTime,unsigned durationInMicroseconds)
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
#if 0
                    if (headerOk)
                    {
                        fFrameSize = frameSize - headerSize;
                        memmove( fTo, fTo + headerSize, fFrameSize );
                    }
#else
					fFrameSize = frameSize;
#endif
                    fNumTruncatedBytes = numTruncatedBytes;
                    fPresentationTime = presentationTime;
                    fDurationInMicroseconds = durationInMicroseconds;
                    afterGetting(this);
                }
                virtual u_int8_t type() { return 1; };
                virtual u_int8_t qFactor() { return 128; };
                virtual u_int8_t width() { return m_width; };
                virtual u_int8_t height() { return m_height; };
                u_int8_t const* quantizationTables( u_int8_t& precision, u_int16_t& length )
                {
                    length = 0;
                    precision = 0;
                    if ( m_qTable0Init && m_qTable1Init )
                    {
                        precision = 8;
                        length = sizeof(m_qTable);
                    }
                    return m_qTable;            
                }

        protected:
                MJPEGVideoSource(UsageEnvironment& env, FramedSource* source) : JPEGVideoSource(env),
                m_inputSource(source),
                m_width(0),
                m_height(0),
                m_qTable0Init(false),
                m_qTable1Init(false)
                {
                    memset(&m_qTable,0,sizeof(m_qTable));
                }
                virtual ~MJPEGVideoSource() 
                { 
                    Medium::close(m_inputSource); 
                }

        protected:
                FramedSource* m_inputSource;
                u_int8_t      m_width;
                u_int8_t      m_height;
                u_int8_t      m_qTable[128];
                bool          m_qTable0Init;
                bool          m_qTable1Init;
};