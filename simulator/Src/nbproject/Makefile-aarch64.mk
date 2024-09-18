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
NM=aarch64-linux-gnu-nm
AR=aarch64-linux-gnu-ar
RANLIB=aarch64-linux-gnu-ranlib
CC=aarch64-linux-gnu-gcc
CCC=aarch64-linux-gnu-g++
CXX=aarch64-linux-gnu-g++
#FC=aarch64-linux-gnu-gprof
AS=aarch64-linux-gnu-as

# Macros
CND_PLATFORM=aarch64
CND_DLIB_EXT=so
CND_CONF=aarch64
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main_easyrtmp.o


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
LDLIBSOPTIONS=-L../Lib/${CND_CONF}

INCLUDE = -I../Include \
          -I./Include 

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_CONF}/easystreamclient

${CND_CONF}/easystreamclient: ${OBJECTFILES}
	${MKDIR} -p ${CND_CONF}
	${LINK.cc} -o ${CND_CONF}/easystreamclient ${OBJECTFILES} ${LDLIBSOPTIONS} -leasystreamclient -lavfilter -lavformat -lpostproc -lavcodec -lswscale -lswresample -lavutil -lm -ldl -lstdc++ -pthread  -lrt -leasyrtmp -lx264 -lfdk-aac -lbz2 -lz

${OBJECTDIR}/main_easyrtmp.o: main_easyrtmp.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -IInclude -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/main_easyrtmp.o main_easyrtmp.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_CONF}/easystreamclient

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
