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
CC=arm-hisiv300-linux-uclibcgnueabi-gcc
CCC=arm-hisiv300-linux-uclibcgnueabi-g++
CXX=arm-hisiv300-linux-uclibcgnueabi-g++
#FC=arm-hisiv300-linux-uclibcgnueabi-gfortran
AS=arm-hisiv300-linux-uclibcgnueabi-as

# Macros
CND_PLATFORM=hisiv300-Linux
CND_DLIB_EXT=so
CND_CONF=hisiv300
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/EasyStreamingServer.o \
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
LDLIBSOPTIONS=-L./Lib/${CND_CONF}

INCLUDE = -I../Include \
          -I./Include \
		  -I./EasyRTSPServer \
		  -I./EasyStreamClient

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easystreamingserver

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easystreamingserver: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/easystreamingserver ${OBJECTFILES} ${LDLIBSOPTIONS} -leasyrtspserver -lEasyStreamClient -lavfilter -lavformat -lavcodec -lswscale -lpostproc -lswresample -lavutil -lfdk-aac -lx264 -lm -ldl -lstdc++ -pthread -lz -lrt

${OBJECTDIR}/EasyStreamingServer.o: EasyStreamingServer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD ${INCLUDE} -MP -MF "$@.d" -o ${OBJECTDIR}/EasyStreamingServer.o EasyStreamingServer.cpp

${OBJECTDIR}/getsps.o: getsps.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD  ${INCLUDE} -MP -MF "$@.d" -o ${OBJECTDIR}/getsps.o getsps.cpp

${OBJECTDIR}/main.o: main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD  ${INCLUDE} -MP -MF "$@.d" -o ${OBJECTDIR}/main.o main.cpp

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
