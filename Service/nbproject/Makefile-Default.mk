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
CND_CONF=Default
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/CaptureInterface.o \
	${OBJECTDIR}/CaptureMono/CaptureMono.o \
	${OBJECTDIR}/CaptureVirtual/CaptureVirtual.o \
	${OBJECTDIR}/Database.o \
	${OBJECTDIR}/Events.o \
	${OBJECTDIR}/HTTPServer.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/Tools.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service ${OBJECTFILES} ${LDLIBSOPTIONS} -lfftw3f -lpthread -lusb-1.0 -lpq -lmicrohttpd -ljson-c

${OBJECTDIR}/CaptureInterface.o: CaptureInterface.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CaptureInterface.o CaptureInterface.cpp

${OBJECTDIR}/CaptureMono/CaptureMono.o: CaptureMono/CaptureMono.cpp
	${MKDIR} -p ${OBJECTDIR}/CaptureMono
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CaptureMono/CaptureMono.o CaptureMono/CaptureMono.cpp

${OBJECTDIR}/CaptureVirtual/CaptureVirtual.o: CaptureVirtual/CaptureVirtual.cpp
	${MKDIR} -p ${OBJECTDIR}/CaptureVirtual
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CaptureVirtual/CaptureVirtual.o CaptureVirtual/CaptureVirtual.cpp

${OBJECTDIR}/Database.o: Database.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Database.o Database.cpp

${OBJECTDIR}/Events.o: Events.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -O3 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Events.o Events.c

${OBJECTDIR}/HTTPServer.o: HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/HTTPServer.o HTTPServer.cpp

${OBJECTDIR}/Main.o: Main.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.cpp

${OBJECTDIR}/Tools.o: Tools.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Tools.o Tools.cpp

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
