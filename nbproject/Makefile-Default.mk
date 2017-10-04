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
	${OBJECTDIR}/Analyzer.o \
	${OBJECTDIR}/BasicCapture.o \
	${OBJECTDIR}/CaptureInterface.o \
	${OBJECTDIR}/Events.o \
	${OBJECTDIR}/FFT.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/Prices.o \
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
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/servicev2

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/servicev2: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	gcc -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/servicev2 ${OBJECTFILES} ${LDLIBSOPTIONS} -lm -lfftw3f -lpthread -lrt -lpq -lcurl -lxml2 -lusb-1.0 -lmicrohttpd -lstdc++

${OBJECTDIR}/Analyzer.o: Analyzer.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Analyzer.o Analyzer.cpp

${OBJECTDIR}/BasicCapture.o: BasicCapture.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/BasicCapture.o BasicCapture.cpp

${OBJECTDIR}/CaptureInterface.o: CaptureInterface.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/CaptureInterface.o CaptureInterface.cpp

${OBJECTDIR}/Events.o: Events.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -D_DEFAULT_SOURCE -D_GNU_SOURCE -I/usr/local/pgsql/include/ -I/usr/include/libxml2 -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Events.o Events.c

${OBJECTDIR}/FFT.o: FFT.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -D_DEFAULT_SOURCE -D_GNU_SOURCE -I/usr/local/pgsql/include/ -I/usr/include/libxml2 -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/FFT.o FFT.c

${OBJECTDIR}/Main.o: Main.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -D_DEFAULT_SOURCE -D_GNU_SOURCE -I/usr/local/pgsql/include/ -I/usr/include/libxml2 -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Main.o Main.c

${OBJECTDIR}/Prices.o: Prices.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -D_DEFAULT_SOURCE -D_GNU_SOURCE -I/usr/local/pgsql/include/ -I/usr/include/libxml2 -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Prices.o Prices.c

${OBJECTDIR}/RequestServer.o: RequestServer.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -D_DEFAULT_SOURCE -D_GNU_SOURCE -I/usr/local/pgsql/include/ -I/usr/include/libxml2 -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/RequestServer.o RequestServer.c

${OBJECTDIR}/Resources.o: Resources.c
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.c) -g -D_DEFAULT_SOURCE -D_GNU_SOURCE -I/usr/local/pgsql/include/ -I/usr/include/libxml2 -std=c99 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Resources.o Resources.c

${OBJECTDIR}/Tools.o: Tools.cpp
	${MKDIR} -p ${OBJECTDIR}
	${RM} "$@.d"
	$(COMPILE.cc) -g -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Tools.o Tools.cpp

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
