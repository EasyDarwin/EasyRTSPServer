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
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/EasyRTSPServer_Demo.o \
	${OBJECTDIR}/getsps.o \
	${OBJECTDIR}/main.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-LEasyRTSPClient -LEasyRTSPServer -Wl,-rpath,'.' Lib/x64/libeasystreamclient.a Lib/x64/libavfilter.a Lib/x64/libavformat.a Lib/x64/libavcodec.a Lib/x64/libavutil.a Lib/x64/libpostproc.a Lib/x64/libswresample.a Lib/x64/libswscale.a Lib/x64/libbz2.a Lib/x64/libeasyrtmp.a Lib/x64/libeasyrtspserver.a Lib/x64/libfreetype.a Lib/x64/libz.a Lib/x64/libx264.a

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libeasystreamclient.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libavfilter.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libavformat.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libavcodec.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libavutil.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libpostproc.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libswresample.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libswscale.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libbz2.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libeasyrtmp.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libeasyrtspserver.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libfreetype.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libz.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: Lib/x64/libx264.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easyrtspserver_demo ${OBJECTFILES} ${LDLIBSOPTIONS} -lpthread -ldl

${OBJECTDIR}/EasyRTSPServer_Demo.o: EasyRTSPServer_Demo.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./EasyCommon -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/EasyRTSPServer_Demo.o EasyRTSPServer_Demo.cpp

${OBJECTDIR}/getsps.o: getsps.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./EasyCommon -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/getsps.o getsps.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I./EasyCommon -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

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
