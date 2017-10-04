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
	${OBJECTDIR}/Analyzer.o \
	${OBJECTDIR}/CaptureInterface.o \
	${OBJECTDIR}/DB.o \
	${OBJECTDIR}/Events.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/Processing.o \
	${OBJECTDIR}/RequestServer.o \
	${OBJECTDIR}/Resources.o \
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
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service ${OBJECTFILES} ${LDLIBSOPTIONS} -lm -lfftw3f -lpthread -lrt -lpq -lcurl -lxml2 -lmicrohttpd

${OBJECTDIR}/Analyzer.o: Analyzer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Analyzer.o Analyzer.cpp

${OBJECTDIR}/CaptureInterface.o: CaptureInterface.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CaptureInterface.o CaptureInterface.cpp

${OBJECTDIR}/DB.o: DB.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/DB.o DB.cpp

${OBJECTDIR}/Events.o: Events.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Events.o Events.c

${OBJECTDIR}/Main.o: Main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.c

${OBJECTDIR}/Processing.o: Processing.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Processing.o Processing.c

${OBJECTDIR}/RequestServer.o: RequestServer.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RequestServer.o RequestServer.c

${OBJECTDIR}/Resources.o: Resources.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Resources.o Resources.cpp

${OBJECTDIR}/Tools.o: Tools.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -I/usr/include/libxml2 -I/usr/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Tools.o Tools.cpp

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
