#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux
CND_DLIB_EXT=so
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/BaseList.o \
	${OBJECTDIR}/BasicTaskSchedulerEpoll.o \
	${OBJECTDIR}/CreateDump.o \
	${OBJECTDIR}/EasyRtspServerAPI.o \
	${OBJECTDIR}/LiveAudioServerMediaSubsession.o \
	${OBJECTDIR}/LiveH264VideoServerMediaSubsession.o \
	${OBJECTDIR}/LiveH265VideoServerMediaSubsession.o \
	${OBJECTDIR}/LiveJPEGStreamSource.o \
	${OBJECTDIR}/LiveJPEGVideoServerMediaSubsession.o \
	${OBJECTDIR}/LiveMetadataServerMediaSubsession.o \
	${OBJECTDIR}/LiveRtspServer.o \
	${OBJECTDIR}/LiveServerMediaSession.o \
	${OBJECTDIR}/LiveServerMediaSubsession.o \
	${OBJECTDIR}/LiveSource.o \
	${OBJECTDIR}/MJPEGVideoSource.o \
	${OBJECTDIR}/aes256.o \
	${OBJECTDIR}/libRTSPServerAPI.o \
	${OBJECTDIR}/live/BasicUsageEnvironment/BasicHashTable.o \
	${OBJECTDIR}/live/BasicUsageEnvironment/BasicTaskScheduler.o \
	${OBJECTDIR}/live/BasicUsageEnvironment/BasicTaskScheduler0.o \
	${OBJECTDIR}/live/BasicUsageEnvironment/BasicUsageEnvironment.o \
	${OBJECTDIR}/live/BasicUsageEnvironment/BasicUsageEnvironment0.o \
	${OBJECTDIR}/live/BasicUsageEnvironment/DelayQueue.o \
	${OBJECTDIR}/live/UsageEnvironment/HashTable.o \
	${OBJECTDIR}/live/UsageEnvironment/UsageEnvironment.o \
	${OBJECTDIR}/live/UsageEnvironment/strDup.o \
	${OBJECTDIR}/live/groupsock/GroupEId.o \
	${OBJECTDIR}/live/groupsock/Groupsock.o \
	${OBJECTDIR}/live/groupsock/GroupsockHelper.o \
	${OBJECTDIR}/live/groupsock/IOHandlers.o \
	${OBJECTDIR}/live/groupsock/NetAddress.o \
	${OBJECTDIR}/live/groupsock/NetInterface.o \
	${OBJECTDIR}/live/groupsock/inet.o \
	${OBJECTDIR}/live/liveMedia/AC3AudioFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/AC3AudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/AC3AudioRTPSource.o \
	${OBJECTDIR}/live/liveMedia/AC3AudioStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/ADTSAudioFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/ADTSAudioFileSource.o \
	${OBJECTDIR}/live/liveMedia/AMRAudioFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/AMRAudioFileSink.o \
	${OBJECTDIR}/live/liveMedia/AMRAudioFileSource.o \
	${OBJECTDIR}/live/liveMedia/AMRAudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/AMRAudioRTPSource.o \
	${OBJECTDIR}/live/liveMedia/AMRAudioSource.o \
	${OBJECTDIR}/live/liveMedia/AVIFileSink.o \
	${OBJECTDIR}/live/liveMedia/AudioInputDevice.o \
	${OBJECTDIR}/live/liveMedia/AudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/Base64.o \
	${OBJECTDIR}/live/liveMedia/BasicUDPSink.o \
	${OBJECTDIR}/live/liveMedia/BasicUDPSource.o \
	${OBJECTDIR}/live/liveMedia/BitVector.o \
	${OBJECTDIR}/live/liveMedia/ByteStreamFileSource.o \
	${OBJECTDIR}/live/liveMedia/ByteStreamMemoryBufferSource.o \
	${OBJECTDIR}/live/liveMedia/ByteStreamMultiFileSource.o \
	${OBJECTDIR}/live/liveMedia/DVVideoFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/DVVideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/DVVideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/DVVideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/DeviceSource.o \
	${OBJECTDIR}/live/liveMedia/DigestAuthentication.o \
	${OBJECTDIR}/live/liveMedia/EBMLNumber.o \
	${OBJECTDIR}/live/liveMedia/FileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/FileSink.o \
	${OBJECTDIR}/live/liveMedia/FramedFileSource.o \
	${OBJECTDIR}/live/liveMedia/FramedFilter.o \
	${OBJECTDIR}/live/liveMedia/FramedSource.o \
	${OBJECTDIR}/live/liveMedia/GSMAudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/GenericMediaServer.o \
	${OBJECTDIR}/live/liveMedia/H261VideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/H263plusVideoFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/H263plusVideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/H263plusVideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/H263plusVideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/H263plusVideoStreamParser.o \
	${OBJECTDIR}/live/liveMedia/H264VideoFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/H264VideoFileSink.o \
	${OBJECTDIR}/live/liveMedia/H264VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/H264VideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/H264VideoStreamDiscreteFramer.o \
	${OBJECTDIR}/live/liveMedia/H264VideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/H264or5VideoFileSink.o \
	${OBJECTDIR}/live/liveMedia/H264or5VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/H264or5VideoStreamDiscreteFramer.o \
	${OBJECTDIR}/live/liveMedia/H264or5VideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/H265VideoFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/H265VideoFileSink.o \
	${OBJECTDIR}/live/liveMedia/H265VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/H265VideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/H265VideoStreamDiscreteFramer.o \
	${OBJECTDIR}/live/liveMedia/H265VideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/InputFile.o \
	${OBJECTDIR}/live/liveMedia/JPEGVideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/JPEGVideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/JPEGVideoSource.o \
	${OBJECTDIR}/live/liveMedia/Locale.o \
	${OBJECTDIR}/live/liveMedia/MP3ADU.o \
	${OBJECTDIR}/live/liveMedia/MP3ADURTPSink.o \
	${OBJECTDIR}/live/liveMedia/MP3ADURTPSource.o \
	${OBJECTDIR}/live/liveMedia/MP3ADUTranscoder.o \
	${OBJECTDIR}/live/liveMedia/MP3ADUdescriptor.o \
	${OBJECTDIR}/live/liveMedia/MP3ADUinterleaving.o \
	${OBJECTDIR}/live/liveMedia/MP3AudioFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MP3AudioMatroskaFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MP3FileSource.o \
	${OBJECTDIR}/live/liveMedia/MP3Internals.o \
	${OBJECTDIR}/live/liveMedia/MP3InternalsHuffman.o \
	${OBJECTDIR}/live/liveMedia/MP3InternalsHuffmanTable.o \
	${OBJECTDIR}/live/liveMedia/MP3StreamState.o \
	${OBJECTDIR}/live/liveMedia/MP3Transcoder.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2AudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2AudioRTPSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2AudioStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2Demux.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2DemuxedElementaryStream.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2DemuxedServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2FileServerDemux.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2VideoFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2VideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2VideoStreamDiscreteFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEG1or2VideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEG2IndexFromTransportStream.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamAccumulator.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFromESSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFromPESSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamIndexFile.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamMultiplexor.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamTrickModeFilter.o \
	${OBJECTDIR}/live/liveMedia/MPEG2TransportUDPServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MPEG4ESVideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/MPEG4ESVideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG4GenericRTPSink.o \
	${OBJECTDIR}/live/liveMedia/MPEG4GenericRTPSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG4LATMAudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/MPEG4LATMAudioRTPSource.o \
	${OBJECTDIR}/live/liveMedia/MPEG4VideoFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/MPEG4VideoStreamDiscreteFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEG4VideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEGVideoStreamFramer.o \
	${OBJECTDIR}/live/liveMedia/MPEGVideoStreamParser.o \
	${OBJECTDIR}/live/liveMedia/MatroskaDemuxedTrack.o \
	${OBJECTDIR}/live/liveMedia/MatroskaFile.o \
	${OBJECTDIR}/live/liveMedia/MatroskaFileParser.o \
	${OBJECTDIR}/live/liveMedia/MatroskaFileServerDemux.o \
	${OBJECTDIR}/live/liveMedia/MatroskaFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/Media.o \
	${OBJECTDIR}/live/liveMedia/MediaSession.o \
	${OBJECTDIR}/live/liveMedia/MediaSink.o \
	${OBJECTDIR}/live/liveMedia/MediaSource.o \
	${OBJECTDIR}/live/liveMedia/MultiFramedRTPSink.o \
	${OBJECTDIR}/live/liveMedia/MultiFramedRTPSource.o \
	${OBJECTDIR}/live/liveMedia/OggDemuxedTrack.o \
	${OBJECTDIR}/live/liveMedia/OggFile.o \
	${OBJECTDIR}/live/liveMedia/OggFileParser.o \
	${OBJECTDIR}/live/liveMedia/OggFileServerDemux.o \
	${OBJECTDIR}/live/liveMedia/OggFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/OggFileSink.o \
	${OBJECTDIR}/live/liveMedia/OnDemandServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/OutputFile.o \
	${OBJECTDIR}/live/liveMedia/PassiveServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/ProxyServerMediaSession.o \
	${OBJECTDIR}/live/liveMedia/QCELPAudioRTPSource.o \
	${OBJECTDIR}/live/liveMedia/QuickTimeFileSink.o \
	${OBJECTDIR}/live/liveMedia/QuickTimeGenericRTPSource.o \
	${OBJECTDIR}/live/liveMedia/RTCP.o \
	${OBJECTDIR}/live/liveMedia/RTPInterface.o \
	${OBJECTDIR}/live/liveMedia/RTPSink.o \
	${OBJECTDIR}/live/liveMedia/RTPSource.o \
	${OBJECTDIR}/live/liveMedia/RTSPClient.o \
	${OBJECTDIR}/live/liveMedia/RTSPCommon.o \
	${OBJECTDIR}/live/liveMedia/RTSPRegisterSender.o \
	${OBJECTDIR}/live/liveMedia/RTSPServer.o \
	${OBJECTDIR}/live/liveMedia/RTSPServerRegister.o \
	${OBJECTDIR}/live/liveMedia/RTSPServerSupportingHTTPStreaming.o \
	${OBJECTDIR}/live/liveMedia/ServerMediaSession.o \
	${OBJECTDIR}/live/liveMedia/SimpleRTPSink.o \
	${OBJECTDIR}/live/liveMedia/SimpleRTPSource.o \
	${OBJECTDIR}/live/liveMedia/StreamParser.o \
	${OBJECTDIR}/live/liveMedia/StreamReplicator.o \
	${OBJECTDIR}/live/liveMedia/T140TextRTPSink.o \
	${OBJECTDIR}/live/liveMedia/TCPStreamSink.o \
	${OBJECTDIR}/live/liveMedia/TextRTPSink.o \
	${OBJECTDIR}/live/liveMedia/TheoraVideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/TheoraVideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/VP8VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/VP8VideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/VP9VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/VP9VideoRTPSource.o \
	${OBJECTDIR}/live/liveMedia/VideoRTPSink.o \
	${OBJECTDIR}/live/liveMedia/VorbisAudioRTPSink.o \
	${OBJECTDIR}/live/liveMedia/VorbisAudioRTPSource.o \
	${OBJECTDIR}/live/liveMedia/WAVAudioFileServerMediaSubsession.o \
	${OBJECTDIR}/live/liveMedia/WAVAudioFileSource.o \
	${OBJECTDIR}/live/liveMedia/ourMD5.o \
	${OBJECTDIR}/live/liveMedia/rtcp_from_spec.o \
	${OBJECTDIR}/live/liveMedia/uLawAudioFilter.o \
	${OBJECTDIR}/live_ipv4_ipv6.o \
	${OBJECTDIR}/livebase64.o \
	${OBJECTDIR}/livebufferqueue.o \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/osmutex.o \
	${OBJECTDIR}/osthread.o \
	${OBJECTDIR}/sync_shm.o \
	${OBJECTDIR}/trace.o


# C Compiler Flags
CFLAGS=-g -fPIC -fvisibility=hidden

# CC Compiler Flags
CCFLAGS=-g -fPIC -fvisibility=hidden -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DALLOW_SERVER_PORT_REUSE -D_EPOLL_
CXXFLAGS=-g -fPIC -fvisibility=hidden -DSOCKLEN_T=socklen_t -DNO_SSTREAM=1 -D_LARGEFILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -DALLOW_SERVER_PORT_REUSE -D_EPOLL_

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=-fPIC

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libEasyRTSPServer.${CND_DLIB_EXT}

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libEasyRTSPServer.${CND_DLIB_EXT}: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libEasyRTSPServer.${CND_DLIB_EXT} ${OBJECTFILES} ${LDLIBSOPTIONS} -lpthread -shared -fPIC

${OBJECTDIR}/BaseList.o: BaseList.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BaseList.o BaseList.cpp

${OBJECTDIR}/BasicTaskSchedulerEpoll.o: BasicTaskSchedulerEpoll.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BasicTaskSchedulerEpoll.o BasicTaskSchedulerEpoll.cpp

${OBJECTDIR}/CreateDump.o: CreateDump.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CreateDump.o CreateDump.cpp

${OBJECTDIR}/EasyRtspServerAPI.o: EasyRtspServerAPI.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyRtspServerAPI.o EasyRtspServerAPI.cpp

${OBJECTDIR}/LiveAudioServerMediaSubsession.o: LiveAudioServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveAudioServerMediaSubsession.o LiveAudioServerMediaSubsession.cpp

${OBJECTDIR}/LiveH264VideoServerMediaSubsession.o: LiveH264VideoServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveH264VideoServerMediaSubsession.o LiveH264VideoServerMediaSubsession.cpp

${OBJECTDIR}/LiveH265VideoServerMediaSubsession.o: LiveH265VideoServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveH265VideoServerMediaSubsession.o LiveH265VideoServerMediaSubsession.cpp

${OBJECTDIR}/LiveJPEGStreamSource.o: LiveJPEGStreamSource.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveJPEGStreamSource.o LiveJPEGStreamSource.cpp

${OBJECTDIR}/LiveJPEGVideoServerMediaSubsession.o: LiveJPEGVideoServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveJPEGVideoServerMediaSubsession.o LiveJPEGVideoServerMediaSubsession.cpp

${OBJECTDIR}/LiveMetadataServerMediaSubsession.o: LiveMetadataServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveMetadataServerMediaSubsession.o LiveMetadataServerMediaSubsession.cpp

${OBJECTDIR}/LiveRtspServer.o: LiveRtspServer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveRtspServer.o LiveRtspServer.cpp

${OBJECTDIR}/LiveServerMediaSession.o: LiveServerMediaSession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveServerMediaSession.o LiveServerMediaSession.cpp

${OBJECTDIR}/LiveServerMediaSubsession.o: LiveServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveServerMediaSubsession.o LiveServerMediaSubsession.cpp

${OBJECTDIR}/LiveSource.o: LiveSource.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/LiveSource.o LiveSource.cpp

${OBJECTDIR}/MJPEGVideoSource.o: MJPEGVideoSource.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/MJPEGVideoSource.o MJPEGVideoSource.cpp

${OBJECTDIR}/aes256.o: aes256.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/aes256.o aes256.cpp

${OBJECTDIR}/libRTSPServerAPI.o: libRTSPServerAPI.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/libRTSPServerAPI.o libRTSPServerAPI.cpp

${OBJECTDIR}/live/BasicUsageEnvironment/BasicHashTable.o: live/BasicUsageEnvironment/BasicHashTable.cpp
	${MKDIR} -p ${OBJECTDIR}/live/BasicUsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/BasicUsageEnvironment/BasicHashTable.o live/BasicUsageEnvironment/BasicHashTable.cpp

${OBJECTDIR}/live/BasicUsageEnvironment/BasicTaskScheduler.o: live/BasicUsageEnvironment/BasicTaskScheduler.cpp
	${MKDIR} -p ${OBJECTDIR}/live/BasicUsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/BasicUsageEnvironment/BasicTaskScheduler.o live/BasicUsageEnvironment/BasicTaskScheduler.cpp

${OBJECTDIR}/live/BasicUsageEnvironment/BasicTaskScheduler0.o: live/BasicUsageEnvironment/BasicTaskScheduler0.cpp
	${MKDIR} -p ${OBJECTDIR}/live/BasicUsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/BasicUsageEnvironment/BasicTaskScheduler0.o live/BasicUsageEnvironment/BasicTaskScheduler0.cpp

${OBJECTDIR}/live/BasicUsageEnvironment/BasicUsageEnvironment.o: live/BasicUsageEnvironment/BasicUsageEnvironment.cpp
	${MKDIR} -p ${OBJECTDIR}/live/BasicUsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/BasicUsageEnvironment/BasicUsageEnvironment.o live/BasicUsageEnvironment/BasicUsageEnvironment.cpp

${OBJECTDIR}/live/BasicUsageEnvironment/BasicUsageEnvironment0.o: live/BasicUsageEnvironment/BasicUsageEnvironment0.cpp
	${MKDIR} -p ${OBJECTDIR}/live/BasicUsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/BasicUsageEnvironment/BasicUsageEnvironment0.o live/BasicUsageEnvironment/BasicUsageEnvironment0.cpp

${OBJECTDIR}/live/BasicUsageEnvironment/DelayQueue.o: live/BasicUsageEnvironment/DelayQueue.cpp
	${MKDIR} -p ${OBJECTDIR}/live/BasicUsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/BasicUsageEnvironment/DelayQueue.o live/BasicUsageEnvironment/DelayQueue.cpp

${OBJECTDIR}/live/UsageEnvironment/HashTable.o: live/UsageEnvironment/HashTable.cpp
	${MKDIR} -p ${OBJECTDIR}/live/UsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/UsageEnvironment/HashTable.o live/UsageEnvironment/HashTable.cpp

${OBJECTDIR}/live/UsageEnvironment/UsageEnvironment.o: live/UsageEnvironment/UsageEnvironment.cpp
	${MKDIR} -p ${OBJECTDIR}/live/UsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/UsageEnvironment/UsageEnvironment.o live/UsageEnvironment/UsageEnvironment.cpp

${OBJECTDIR}/live/UsageEnvironment/strDup.o: live/UsageEnvironment/strDup.cpp
	${MKDIR} -p ${OBJECTDIR}/live/UsageEnvironment
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/UsageEnvironment/strDup.o live/UsageEnvironment/strDup.cpp

${OBJECTDIR}/live/groupsock/GroupEId.o: live/groupsock/GroupEId.cpp
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/GroupEId.o live/groupsock/GroupEId.cpp

${OBJECTDIR}/live/groupsock/Groupsock.o: live/groupsock/Groupsock.cpp
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/Groupsock.o live/groupsock/Groupsock.cpp

${OBJECTDIR}/live/groupsock/GroupsockHelper.o: live/groupsock/GroupsockHelper.cpp
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/GroupsockHelper.o live/groupsock/GroupsockHelper.cpp

${OBJECTDIR}/live/groupsock/IOHandlers.o: live/groupsock/IOHandlers.cpp
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/IOHandlers.o live/groupsock/IOHandlers.cpp

${OBJECTDIR}/live/groupsock/NetAddress.o: live/groupsock/NetAddress.cpp
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/NetAddress.o live/groupsock/NetAddress.cpp

${OBJECTDIR}/live/groupsock/NetInterface.o: live/groupsock/NetInterface.cpp
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/NetInterface.o live/groupsock/NetInterface.cpp

${OBJECTDIR}/live/groupsock/inet.o: live/groupsock/inet.c
	${MKDIR} -p ${OBJECTDIR}/live/groupsock
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Ilive/groupsock/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/groupsock/inet.o live/groupsock/inet.c

${OBJECTDIR}/live/liveMedia/AC3AudioFileServerMediaSubsession.o: live/liveMedia/AC3AudioFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AC3AudioFileServerMediaSubsession.o live/liveMedia/AC3AudioFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/AC3AudioRTPSink.o: live/liveMedia/AC3AudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AC3AudioRTPSink.o live/liveMedia/AC3AudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/AC3AudioRTPSource.o: live/liveMedia/AC3AudioRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AC3AudioRTPSource.o live/liveMedia/AC3AudioRTPSource.cpp

${OBJECTDIR}/live/liveMedia/AC3AudioStreamFramer.o: live/liveMedia/AC3AudioStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AC3AudioStreamFramer.o live/liveMedia/AC3AudioStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/ADTSAudioFileServerMediaSubsession.o: live/liveMedia/ADTSAudioFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ADTSAudioFileServerMediaSubsession.o live/liveMedia/ADTSAudioFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/ADTSAudioFileSource.o: live/liveMedia/ADTSAudioFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ADTSAudioFileSource.o live/liveMedia/ADTSAudioFileSource.cpp

${OBJECTDIR}/live/liveMedia/AMRAudioFileServerMediaSubsession.o: live/liveMedia/AMRAudioFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AMRAudioFileServerMediaSubsession.o live/liveMedia/AMRAudioFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/AMRAudioFileSink.o: live/liveMedia/AMRAudioFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AMRAudioFileSink.o live/liveMedia/AMRAudioFileSink.cpp

${OBJECTDIR}/live/liveMedia/AMRAudioFileSource.o: live/liveMedia/AMRAudioFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AMRAudioFileSource.o live/liveMedia/AMRAudioFileSource.cpp

${OBJECTDIR}/live/liveMedia/AMRAudioRTPSink.o: live/liveMedia/AMRAudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AMRAudioRTPSink.o live/liveMedia/AMRAudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/AMRAudioRTPSource.o: live/liveMedia/AMRAudioRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AMRAudioRTPSource.o live/liveMedia/AMRAudioRTPSource.cpp

${OBJECTDIR}/live/liveMedia/AMRAudioSource.o: live/liveMedia/AMRAudioSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AMRAudioSource.o live/liveMedia/AMRAudioSource.cpp

${OBJECTDIR}/live/liveMedia/AVIFileSink.o: live/liveMedia/AVIFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AVIFileSink.o live/liveMedia/AVIFileSink.cpp

${OBJECTDIR}/live/liveMedia/AudioInputDevice.o: live/liveMedia/AudioInputDevice.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AudioInputDevice.o live/liveMedia/AudioInputDevice.cpp

${OBJECTDIR}/live/liveMedia/AudioRTPSink.o: live/liveMedia/AudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/AudioRTPSink.o live/liveMedia/AudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/Base64.o: live/liveMedia/Base64.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/Base64.o live/liveMedia/Base64.cpp

${OBJECTDIR}/live/liveMedia/BasicUDPSink.o: live/liveMedia/BasicUDPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/BasicUDPSink.o live/liveMedia/BasicUDPSink.cpp

${OBJECTDIR}/live/liveMedia/BasicUDPSource.o: live/liveMedia/BasicUDPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/BasicUDPSource.o live/liveMedia/BasicUDPSource.cpp

${OBJECTDIR}/live/liveMedia/BitVector.o: live/liveMedia/BitVector.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/BitVector.o live/liveMedia/BitVector.cpp

${OBJECTDIR}/live/liveMedia/ByteStreamFileSource.o: live/liveMedia/ByteStreamFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ByteStreamFileSource.o live/liveMedia/ByteStreamFileSource.cpp

${OBJECTDIR}/live/liveMedia/ByteStreamMemoryBufferSource.o: live/liveMedia/ByteStreamMemoryBufferSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ByteStreamMemoryBufferSource.o live/liveMedia/ByteStreamMemoryBufferSource.cpp

${OBJECTDIR}/live/liveMedia/ByteStreamMultiFileSource.o: live/liveMedia/ByteStreamMultiFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ByteStreamMultiFileSource.o live/liveMedia/ByteStreamMultiFileSource.cpp

${OBJECTDIR}/live/liveMedia/DVVideoFileServerMediaSubsession.o: live/liveMedia/DVVideoFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/DVVideoFileServerMediaSubsession.o live/liveMedia/DVVideoFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/DVVideoRTPSink.o: live/liveMedia/DVVideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/DVVideoRTPSink.o live/liveMedia/DVVideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/DVVideoRTPSource.o: live/liveMedia/DVVideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/DVVideoRTPSource.o live/liveMedia/DVVideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/DVVideoStreamFramer.o: live/liveMedia/DVVideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/DVVideoStreamFramer.o live/liveMedia/DVVideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/DeviceSource.o: live/liveMedia/DeviceSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/DeviceSource.o live/liveMedia/DeviceSource.cpp

${OBJECTDIR}/live/liveMedia/DigestAuthentication.o: live/liveMedia/DigestAuthentication.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/DigestAuthentication.o live/liveMedia/DigestAuthentication.cpp

${OBJECTDIR}/live/liveMedia/EBMLNumber.o: live/liveMedia/EBMLNumber.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/EBMLNumber.o live/liveMedia/EBMLNumber.cpp

${OBJECTDIR}/live/liveMedia/FileServerMediaSubsession.o: live/liveMedia/FileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/FileServerMediaSubsession.o live/liveMedia/FileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/FileSink.o: live/liveMedia/FileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/FileSink.o live/liveMedia/FileSink.cpp

${OBJECTDIR}/live/liveMedia/FramedFileSource.o: live/liveMedia/FramedFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/FramedFileSource.o live/liveMedia/FramedFileSource.cpp

${OBJECTDIR}/live/liveMedia/FramedFilter.o: live/liveMedia/FramedFilter.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/FramedFilter.o live/liveMedia/FramedFilter.cpp

${OBJECTDIR}/live/liveMedia/FramedSource.o: live/liveMedia/FramedSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/FramedSource.o live/liveMedia/FramedSource.cpp

${OBJECTDIR}/live/liveMedia/GSMAudioRTPSink.o: live/liveMedia/GSMAudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/GSMAudioRTPSink.o live/liveMedia/GSMAudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/GenericMediaServer.o: live/liveMedia/GenericMediaServer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/GenericMediaServer.o live/liveMedia/GenericMediaServer.cpp

${OBJECTDIR}/live/liveMedia/H261VideoRTPSource.o: live/liveMedia/H261VideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H261VideoRTPSource.o live/liveMedia/H261VideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/H263plusVideoFileServerMediaSubsession.o: live/liveMedia/H263plusVideoFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H263plusVideoFileServerMediaSubsession.o live/liveMedia/H263plusVideoFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/H263plusVideoRTPSink.o: live/liveMedia/H263plusVideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H263plusVideoRTPSink.o live/liveMedia/H263plusVideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/H263plusVideoRTPSource.o: live/liveMedia/H263plusVideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H263plusVideoRTPSource.o live/liveMedia/H263plusVideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/H263plusVideoStreamFramer.o: live/liveMedia/H263plusVideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H263plusVideoStreamFramer.o live/liveMedia/H263plusVideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/H263plusVideoStreamParser.o: live/liveMedia/H263plusVideoStreamParser.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H263plusVideoStreamParser.o live/liveMedia/H263plusVideoStreamParser.cpp

${OBJECTDIR}/live/liveMedia/H264VideoFileServerMediaSubsession.o: live/liveMedia/H264VideoFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264VideoFileServerMediaSubsession.o live/liveMedia/H264VideoFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/H264VideoFileSink.o: live/liveMedia/H264VideoFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264VideoFileSink.o live/liveMedia/H264VideoFileSink.cpp

${OBJECTDIR}/live/liveMedia/H264VideoRTPSink.o: live/liveMedia/H264VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264VideoRTPSink.o live/liveMedia/H264VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/H264VideoRTPSource.o: live/liveMedia/H264VideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264VideoRTPSource.o live/liveMedia/H264VideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/H264VideoStreamDiscreteFramer.o: live/liveMedia/H264VideoStreamDiscreteFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264VideoStreamDiscreteFramer.o live/liveMedia/H264VideoStreamDiscreteFramer.cpp

${OBJECTDIR}/live/liveMedia/H264VideoStreamFramer.o: live/liveMedia/H264VideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264VideoStreamFramer.o live/liveMedia/H264VideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/H264or5VideoFileSink.o: live/liveMedia/H264or5VideoFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264or5VideoFileSink.o live/liveMedia/H264or5VideoFileSink.cpp

${OBJECTDIR}/live/liveMedia/H264or5VideoRTPSink.o: live/liveMedia/H264or5VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264or5VideoRTPSink.o live/liveMedia/H264or5VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/H264or5VideoStreamDiscreteFramer.o: live/liveMedia/H264or5VideoStreamDiscreteFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264or5VideoStreamDiscreteFramer.o live/liveMedia/H264or5VideoStreamDiscreteFramer.cpp

${OBJECTDIR}/live/liveMedia/H264or5VideoStreamFramer.o: live/liveMedia/H264or5VideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H264or5VideoStreamFramer.o live/liveMedia/H264or5VideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/H265VideoFileServerMediaSubsession.o: live/liveMedia/H265VideoFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H265VideoFileServerMediaSubsession.o live/liveMedia/H265VideoFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/H265VideoFileSink.o: live/liveMedia/H265VideoFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H265VideoFileSink.o live/liveMedia/H265VideoFileSink.cpp

${OBJECTDIR}/live/liveMedia/H265VideoRTPSink.o: live/liveMedia/H265VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H265VideoRTPSink.o live/liveMedia/H265VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/H265VideoRTPSource.o: live/liveMedia/H265VideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H265VideoRTPSource.o live/liveMedia/H265VideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/H265VideoStreamDiscreteFramer.o: live/liveMedia/H265VideoStreamDiscreteFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H265VideoStreamDiscreteFramer.o live/liveMedia/H265VideoStreamDiscreteFramer.cpp

${OBJECTDIR}/live/liveMedia/H265VideoStreamFramer.o: live/liveMedia/H265VideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/H265VideoStreamFramer.o live/liveMedia/H265VideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/InputFile.o: live/liveMedia/InputFile.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/InputFile.o live/liveMedia/InputFile.cpp

${OBJECTDIR}/live/liveMedia/JPEGVideoRTPSink.o: live/liveMedia/JPEGVideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/JPEGVideoRTPSink.o live/liveMedia/JPEGVideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/JPEGVideoRTPSource.o: live/liveMedia/JPEGVideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/JPEGVideoRTPSource.o live/liveMedia/JPEGVideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/JPEGVideoSource.o: live/liveMedia/JPEGVideoSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/JPEGVideoSource.o live/liveMedia/JPEGVideoSource.cpp

${OBJECTDIR}/live/liveMedia/Locale.o: live/liveMedia/Locale.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/Locale.o live/liveMedia/Locale.cpp

${OBJECTDIR}/live/liveMedia/MP3ADU.o: live/liveMedia/MP3ADU.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3ADU.o live/liveMedia/MP3ADU.cpp

${OBJECTDIR}/live/liveMedia/MP3ADURTPSink.o: live/liveMedia/MP3ADURTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3ADURTPSink.o live/liveMedia/MP3ADURTPSink.cpp

${OBJECTDIR}/live/liveMedia/MP3ADURTPSource.o: live/liveMedia/MP3ADURTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3ADURTPSource.o live/liveMedia/MP3ADURTPSource.cpp

${OBJECTDIR}/live/liveMedia/MP3ADUTranscoder.o: live/liveMedia/MP3ADUTranscoder.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3ADUTranscoder.o live/liveMedia/MP3ADUTranscoder.cpp

${OBJECTDIR}/live/liveMedia/MP3ADUdescriptor.o: live/liveMedia/MP3ADUdescriptor.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3ADUdescriptor.o live/liveMedia/MP3ADUdescriptor.cpp

${OBJECTDIR}/live/liveMedia/MP3ADUinterleaving.o: live/liveMedia/MP3ADUinterleaving.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3ADUinterleaving.o live/liveMedia/MP3ADUinterleaving.cpp

${OBJECTDIR}/live/liveMedia/MP3AudioFileServerMediaSubsession.o: live/liveMedia/MP3AudioFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3AudioFileServerMediaSubsession.o live/liveMedia/MP3AudioFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MP3AudioMatroskaFileServerMediaSubsession.o: live/liveMedia/MP3AudioMatroskaFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3AudioMatroskaFileServerMediaSubsession.o live/liveMedia/MP3AudioMatroskaFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MP3FileSource.o: live/liveMedia/MP3FileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3FileSource.o live/liveMedia/MP3FileSource.cpp

${OBJECTDIR}/live/liveMedia/MP3Internals.o: live/liveMedia/MP3Internals.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3Internals.o live/liveMedia/MP3Internals.cpp

${OBJECTDIR}/live/liveMedia/MP3InternalsHuffman.o: live/liveMedia/MP3InternalsHuffman.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3InternalsHuffman.o live/liveMedia/MP3InternalsHuffman.cpp

${OBJECTDIR}/live/liveMedia/MP3InternalsHuffmanTable.o: live/liveMedia/MP3InternalsHuffmanTable.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3InternalsHuffmanTable.o live/liveMedia/MP3InternalsHuffmanTable.cpp

${OBJECTDIR}/live/liveMedia/MP3StreamState.o: live/liveMedia/MP3StreamState.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3StreamState.o live/liveMedia/MP3StreamState.cpp

${OBJECTDIR}/live/liveMedia/MP3Transcoder.o: live/liveMedia/MP3Transcoder.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MP3Transcoder.o live/liveMedia/MP3Transcoder.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2AudioRTPSink.o: live/liveMedia/MPEG1or2AudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2AudioRTPSink.o live/liveMedia/MPEG1or2AudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2AudioRTPSource.o: live/liveMedia/MPEG1or2AudioRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2AudioRTPSource.o live/liveMedia/MPEG1or2AudioRTPSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2AudioStreamFramer.o: live/liveMedia/MPEG1or2AudioStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2AudioStreamFramer.o live/liveMedia/MPEG1or2AudioStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2Demux.o: live/liveMedia/MPEG1or2Demux.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2Demux.o live/liveMedia/MPEG1or2Demux.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2DemuxedElementaryStream.o: live/liveMedia/MPEG1or2DemuxedElementaryStream.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2DemuxedElementaryStream.o live/liveMedia/MPEG1or2DemuxedElementaryStream.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2DemuxedServerMediaSubsession.o: live/liveMedia/MPEG1or2DemuxedServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2DemuxedServerMediaSubsession.o live/liveMedia/MPEG1or2DemuxedServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2FileServerDemux.o: live/liveMedia/MPEG1or2FileServerDemux.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2FileServerDemux.o live/liveMedia/MPEG1or2FileServerDemux.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2VideoFileServerMediaSubsession.o: live/liveMedia/MPEG1or2VideoFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2VideoFileServerMediaSubsession.o live/liveMedia/MPEG1or2VideoFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2VideoRTPSink.o: live/liveMedia/MPEG1or2VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2VideoRTPSink.o live/liveMedia/MPEG1or2VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2VideoRTPSource.o: live/liveMedia/MPEG1or2VideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2VideoRTPSource.o live/liveMedia/MPEG1or2VideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2VideoStreamDiscreteFramer.o: live/liveMedia/MPEG1or2VideoStreamDiscreteFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2VideoStreamDiscreteFramer.o live/liveMedia/MPEG1or2VideoStreamDiscreteFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEG1or2VideoStreamFramer.o: live/liveMedia/MPEG1or2VideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG1or2VideoStreamFramer.o live/liveMedia/MPEG1or2VideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEG2IndexFromTransportStream.o: live/liveMedia/MPEG2IndexFromTransportStream.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2IndexFromTransportStream.o live/liveMedia/MPEG2IndexFromTransportStream.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportFileServerMediaSubsession.o: live/liveMedia/MPEG2TransportFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportFileServerMediaSubsession.o live/liveMedia/MPEG2TransportFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamAccumulator.o: live/liveMedia/MPEG2TransportStreamAccumulator.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamAccumulator.o live/liveMedia/MPEG2TransportStreamAccumulator.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFramer.o: live/liveMedia/MPEG2TransportStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFramer.o live/liveMedia/MPEG2TransportStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFromESSource.o: live/liveMedia/MPEG2TransportStreamFromESSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFromESSource.o live/liveMedia/MPEG2TransportStreamFromESSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFromPESSource.o: live/liveMedia/MPEG2TransportStreamFromPESSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamFromPESSource.o live/liveMedia/MPEG2TransportStreamFromPESSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamIndexFile.o: live/liveMedia/MPEG2TransportStreamIndexFile.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamIndexFile.o live/liveMedia/MPEG2TransportStreamIndexFile.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamMultiplexor.o: live/liveMedia/MPEG2TransportStreamMultiplexor.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamMultiplexor.o live/liveMedia/MPEG2TransportStreamMultiplexor.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamTrickModeFilter.o: live/liveMedia/MPEG2TransportStreamTrickModeFilter.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportStreamTrickModeFilter.o live/liveMedia/MPEG2TransportStreamTrickModeFilter.cpp

${OBJECTDIR}/live/liveMedia/MPEG2TransportUDPServerMediaSubsession.o: live/liveMedia/MPEG2TransportUDPServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG2TransportUDPServerMediaSubsession.o live/liveMedia/MPEG2TransportUDPServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MPEG4ESVideoRTPSink.o: live/liveMedia/MPEG4ESVideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4ESVideoRTPSink.o live/liveMedia/MPEG4ESVideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/MPEG4ESVideoRTPSource.o: live/liveMedia/MPEG4ESVideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4ESVideoRTPSource.o live/liveMedia/MPEG4ESVideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG4GenericRTPSink.o: live/liveMedia/MPEG4GenericRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4GenericRTPSink.o live/liveMedia/MPEG4GenericRTPSink.cpp

${OBJECTDIR}/live/liveMedia/MPEG4GenericRTPSource.o: live/liveMedia/MPEG4GenericRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4GenericRTPSource.o live/liveMedia/MPEG4GenericRTPSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG4LATMAudioRTPSink.o: live/liveMedia/MPEG4LATMAudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4LATMAudioRTPSink.o live/liveMedia/MPEG4LATMAudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/MPEG4LATMAudioRTPSource.o: live/liveMedia/MPEG4LATMAudioRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4LATMAudioRTPSource.o live/liveMedia/MPEG4LATMAudioRTPSource.cpp

${OBJECTDIR}/live/liveMedia/MPEG4VideoFileServerMediaSubsession.o: live/liveMedia/MPEG4VideoFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4VideoFileServerMediaSubsession.o live/liveMedia/MPEG4VideoFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/MPEG4VideoStreamDiscreteFramer.o: live/liveMedia/MPEG4VideoStreamDiscreteFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4VideoStreamDiscreteFramer.o live/liveMedia/MPEG4VideoStreamDiscreteFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEG4VideoStreamFramer.o: live/liveMedia/MPEG4VideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEG4VideoStreamFramer.o live/liveMedia/MPEG4VideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEGVideoStreamFramer.o: live/liveMedia/MPEGVideoStreamFramer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEGVideoStreamFramer.o live/liveMedia/MPEGVideoStreamFramer.cpp

${OBJECTDIR}/live/liveMedia/MPEGVideoStreamParser.o: live/liveMedia/MPEGVideoStreamParser.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MPEGVideoStreamParser.o live/liveMedia/MPEGVideoStreamParser.cpp

${OBJECTDIR}/live/liveMedia/MatroskaDemuxedTrack.o: live/liveMedia/MatroskaDemuxedTrack.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MatroskaDemuxedTrack.o live/liveMedia/MatroskaDemuxedTrack.cpp

${OBJECTDIR}/live/liveMedia/MatroskaFile.o: live/liveMedia/MatroskaFile.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MatroskaFile.o live/liveMedia/MatroskaFile.cpp

${OBJECTDIR}/live/liveMedia/MatroskaFileParser.o: live/liveMedia/MatroskaFileParser.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MatroskaFileParser.o live/liveMedia/MatroskaFileParser.cpp

${OBJECTDIR}/live/liveMedia/MatroskaFileServerDemux.o: live/liveMedia/MatroskaFileServerDemux.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MatroskaFileServerDemux.o live/liveMedia/MatroskaFileServerDemux.cpp

${OBJECTDIR}/live/liveMedia/MatroskaFileServerMediaSubsession.o: live/liveMedia/MatroskaFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MatroskaFileServerMediaSubsession.o live/liveMedia/MatroskaFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/Media.o: live/liveMedia/Media.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/Media.o live/liveMedia/Media.cpp

${OBJECTDIR}/live/liveMedia/MediaSession.o: live/liveMedia/MediaSession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MediaSession.o live/liveMedia/MediaSession.cpp

${OBJECTDIR}/live/liveMedia/MediaSink.o: live/liveMedia/MediaSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MediaSink.o live/liveMedia/MediaSink.cpp

${OBJECTDIR}/live/liveMedia/MediaSource.o: live/liveMedia/MediaSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MediaSource.o live/liveMedia/MediaSource.cpp

${OBJECTDIR}/live/liveMedia/MultiFramedRTPSink.o: live/liveMedia/MultiFramedRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MultiFramedRTPSink.o live/liveMedia/MultiFramedRTPSink.cpp

${OBJECTDIR}/live/liveMedia/MultiFramedRTPSource.o: live/liveMedia/MultiFramedRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/MultiFramedRTPSource.o live/liveMedia/MultiFramedRTPSource.cpp

${OBJECTDIR}/live/liveMedia/OggDemuxedTrack.o: live/liveMedia/OggDemuxedTrack.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OggDemuxedTrack.o live/liveMedia/OggDemuxedTrack.cpp

${OBJECTDIR}/live/liveMedia/OggFile.o: live/liveMedia/OggFile.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OggFile.o live/liveMedia/OggFile.cpp

${OBJECTDIR}/live/liveMedia/OggFileParser.o: live/liveMedia/OggFileParser.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OggFileParser.o live/liveMedia/OggFileParser.cpp

${OBJECTDIR}/live/liveMedia/OggFileServerDemux.o: live/liveMedia/OggFileServerDemux.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OggFileServerDemux.o live/liveMedia/OggFileServerDemux.cpp

${OBJECTDIR}/live/liveMedia/OggFileServerMediaSubsession.o: live/liveMedia/OggFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OggFileServerMediaSubsession.o live/liveMedia/OggFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/OggFileSink.o: live/liveMedia/OggFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OggFileSink.o live/liveMedia/OggFileSink.cpp

${OBJECTDIR}/live/liveMedia/OnDemandServerMediaSubsession.o: live/liveMedia/OnDemandServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OnDemandServerMediaSubsession.o live/liveMedia/OnDemandServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/OutputFile.o: live/liveMedia/OutputFile.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/OutputFile.o live/liveMedia/OutputFile.cpp

${OBJECTDIR}/live/liveMedia/PassiveServerMediaSubsession.o: live/liveMedia/PassiveServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/PassiveServerMediaSubsession.o live/liveMedia/PassiveServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/ProxyServerMediaSession.o: live/liveMedia/ProxyServerMediaSession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ProxyServerMediaSession.o live/liveMedia/ProxyServerMediaSession.cpp

${OBJECTDIR}/live/liveMedia/QCELPAudioRTPSource.o: live/liveMedia/QCELPAudioRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/QCELPAudioRTPSource.o live/liveMedia/QCELPAudioRTPSource.cpp

${OBJECTDIR}/live/liveMedia/QuickTimeFileSink.o: live/liveMedia/QuickTimeFileSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/QuickTimeFileSink.o live/liveMedia/QuickTimeFileSink.cpp

${OBJECTDIR}/live/liveMedia/QuickTimeGenericRTPSource.o: live/liveMedia/QuickTimeGenericRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/QuickTimeGenericRTPSource.o live/liveMedia/QuickTimeGenericRTPSource.cpp

${OBJECTDIR}/live/liveMedia/RTCP.o: live/liveMedia/RTCP.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTCP.o live/liveMedia/RTCP.cpp

${OBJECTDIR}/live/liveMedia/RTPInterface.o: live/liveMedia/RTPInterface.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTPInterface.o live/liveMedia/RTPInterface.cpp

${OBJECTDIR}/live/liveMedia/RTPSink.o: live/liveMedia/RTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTPSink.o live/liveMedia/RTPSink.cpp

${OBJECTDIR}/live/liveMedia/RTPSource.o: live/liveMedia/RTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTPSource.o live/liveMedia/RTPSource.cpp

${OBJECTDIR}/live/liveMedia/RTSPClient.o: live/liveMedia/RTSPClient.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTSPClient.o live/liveMedia/RTSPClient.cpp

${OBJECTDIR}/live/liveMedia/RTSPCommon.o: live/liveMedia/RTSPCommon.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTSPCommon.o live/liveMedia/RTSPCommon.cpp

${OBJECTDIR}/live/liveMedia/RTSPRegisterSender.o: live/liveMedia/RTSPRegisterSender.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTSPRegisterSender.o live/liveMedia/RTSPRegisterSender.cpp

${OBJECTDIR}/live/liveMedia/RTSPServer.o: live/liveMedia/RTSPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTSPServer.o live/liveMedia/RTSPServer.cpp

${OBJECTDIR}/live/liveMedia/RTSPServerRegister.o: live/liveMedia/RTSPServerRegister.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTSPServerRegister.o live/liveMedia/RTSPServerRegister.cpp

${OBJECTDIR}/live/liveMedia/RTSPServerSupportingHTTPStreaming.o: live/liveMedia/RTSPServerSupportingHTTPStreaming.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/RTSPServerSupportingHTTPStreaming.o live/liveMedia/RTSPServerSupportingHTTPStreaming.cpp

${OBJECTDIR}/live/liveMedia/ServerMediaSession.o: live/liveMedia/ServerMediaSession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ServerMediaSession.o live/liveMedia/ServerMediaSession.cpp

${OBJECTDIR}/live/liveMedia/SimpleRTPSink.o: live/liveMedia/SimpleRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/SimpleRTPSink.o live/liveMedia/SimpleRTPSink.cpp

${OBJECTDIR}/live/liveMedia/SimpleRTPSource.o: live/liveMedia/SimpleRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/SimpleRTPSource.o live/liveMedia/SimpleRTPSource.cpp

${OBJECTDIR}/live/liveMedia/StreamParser.o: live/liveMedia/StreamParser.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/StreamParser.o live/liveMedia/StreamParser.cpp

${OBJECTDIR}/live/liveMedia/StreamReplicator.o: live/liveMedia/StreamReplicator.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/StreamReplicator.o live/liveMedia/StreamReplicator.cpp

${OBJECTDIR}/live/liveMedia/T140TextRTPSink.o: live/liveMedia/T140TextRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/T140TextRTPSink.o live/liveMedia/T140TextRTPSink.cpp

${OBJECTDIR}/live/liveMedia/TCPStreamSink.o: live/liveMedia/TCPStreamSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/TCPStreamSink.o live/liveMedia/TCPStreamSink.cpp

${OBJECTDIR}/live/liveMedia/TextRTPSink.o: live/liveMedia/TextRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/TextRTPSink.o live/liveMedia/TextRTPSink.cpp

${OBJECTDIR}/live/liveMedia/TheoraVideoRTPSink.o: live/liveMedia/TheoraVideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/TheoraVideoRTPSink.o live/liveMedia/TheoraVideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/TheoraVideoRTPSource.o: live/liveMedia/TheoraVideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/TheoraVideoRTPSource.o live/liveMedia/TheoraVideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/VP8VideoRTPSink.o: live/liveMedia/VP8VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VP8VideoRTPSink.o live/liveMedia/VP8VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/VP8VideoRTPSource.o: live/liveMedia/VP8VideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VP8VideoRTPSource.o live/liveMedia/VP8VideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/VP9VideoRTPSink.o: live/liveMedia/VP9VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VP9VideoRTPSink.o live/liveMedia/VP9VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/VP9VideoRTPSource.o: live/liveMedia/VP9VideoRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VP9VideoRTPSource.o live/liveMedia/VP9VideoRTPSource.cpp

${OBJECTDIR}/live/liveMedia/VideoRTPSink.o: live/liveMedia/VideoRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VideoRTPSink.o live/liveMedia/VideoRTPSink.cpp

${OBJECTDIR}/live/liveMedia/VorbisAudioRTPSink.o: live/liveMedia/VorbisAudioRTPSink.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VorbisAudioRTPSink.o live/liveMedia/VorbisAudioRTPSink.cpp

${OBJECTDIR}/live/liveMedia/VorbisAudioRTPSource.o: live/liveMedia/VorbisAudioRTPSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/VorbisAudioRTPSource.o live/liveMedia/VorbisAudioRTPSource.cpp

${OBJECTDIR}/live/liveMedia/WAVAudioFileServerMediaSubsession.o: live/liveMedia/WAVAudioFileServerMediaSubsession.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/WAVAudioFileServerMediaSubsession.o live/liveMedia/WAVAudioFileServerMediaSubsession.cpp

${OBJECTDIR}/live/liveMedia/WAVAudioFileSource.o: live/liveMedia/WAVAudioFileSource.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/WAVAudioFileSource.o live/liveMedia/WAVAudioFileSource.cpp

${OBJECTDIR}/live/liveMedia/ourMD5.o: live/liveMedia/ourMD5.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/ourMD5.o live/liveMedia/ourMD5.cpp

${OBJECTDIR}/live/liveMedia/rtcp_from_spec.o: live/liveMedia/rtcp_from_spec.c
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.c) -O2 -Ilive/groupsock/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/rtcp_from_spec.o live/liveMedia/rtcp_from_spec.c

${OBJECTDIR}/live/liveMedia/uLawAudioFilter.o: live/liveMedia/uLawAudioFilter.cpp
	${MKDIR} -p ${OBJECTDIR}/live/liveMedia
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live/liveMedia/uLawAudioFilter.o live/liveMedia/uLawAudioFilter.cpp

${OBJECTDIR}/live_ipv4_ipv6.o: live_ipv4_ipv6.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/live_ipv4_ipv6.o live_ipv4_ipv6.cpp

${OBJECTDIR}/livebase64.o: livebase64.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/livebase64.o livebase64.cpp

${OBJECTDIR}/livebufferqueue.o: livebufferqueue.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/livebufferqueue.o livebufferqueue.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/osmutex.o: osmutex.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/osmutex.o osmutex.cpp

${OBJECTDIR}/osthread.o: osthread.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/osthread.o osthread.cpp

${OBJECTDIR}/sync_shm.o: sync_shm.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/sync_shm.o sync_shm.cpp

${OBJECTDIR}/trace.o: trace.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -DXLOCALE_NOT_USED -Ilive/BasicUsageEnvironment/include -Ilive/groupsock/include -Ilive/liveMedia/include -Ilive/UsageEnvironment/include -I../Include -fPIC  -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/trace.o trace.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
