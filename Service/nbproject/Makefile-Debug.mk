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
	${OBJECTDIR}/Source/Analyzer.o \
	${OBJECTDIR}/Source/CaptureMono/CaptureMono.o \
	${OBJECTDIR}/Source/CaptureV2/CaptureV2.o \
	${OBJECTDIR}/Source/CaptureVirtual/CaptureVirtual.o \
	${OBJECTDIR}/Source/Database.o \
	${OBJECTDIR}/Source/HTTPServer.o \
	${OBJECTDIR}/Source/Main.o \
	${OBJECTDIR}/Source/NetworkManager.o \
	${OBJECTDIR}/Source/Prices.o \
	${OBJECTDIR}/Source/Tools.o


# C Compiler Flags
CFLAGS=-O0 -march=native

# CC Compiler Flags
CCFLAGS=-O0
CXXFLAGS=-O0

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/lib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/service ${OBJECTFILES} ${LDLIBSOPTIONS} -lfftw3f -lpthread -lusb-1.0 -lpq -lmicrohttpd -ljson-c -lz -lcurl -ldbus-1

${OBJECTDIR}/Source/Analyzer.o: Source/Analyzer.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/Analyzer.o Source/Analyzer.cpp

${OBJECTDIR}/Source/CaptureMono/CaptureMono.o: Source/CaptureMono/CaptureMono.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/CaptureMono
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/CaptureMono/CaptureMono.o Source/CaptureMono/CaptureMono.cpp

${OBJECTDIR}/Source/CaptureV2/CaptureV2.o: Source/CaptureV2/CaptureV2.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/CaptureV2
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/CaptureV2/CaptureV2.o Source/CaptureV2/CaptureV2.cpp

${OBJECTDIR}/Source/CaptureVirtual/CaptureVirtual.o: Source/CaptureVirtual/CaptureVirtual.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/CaptureVirtual
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/CaptureVirtual/CaptureVirtual.o Source/CaptureVirtual/CaptureVirtual.cpp

${OBJECTDIR}/Source/Database.o: Source/Database.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/Database.o Source/Database.cpp

${OBJECTDIR}/Source/HTTPServer.o: Source/HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/HTTPServer.o Source/HTTPServer.cpp

${OBJECTDIR}/Source/Main.o: Source/Main.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/Main.o Source/Main.cpp

${OBJECTDIR}/Source/NetworkManager.o: Source/NetworkManager.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/NetworkManager.o Source/NetworkManager.cpp

${OBJECTDIR}/Source/Prices.o: Source/Prices.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/Prices.o Source/Prices.cpp

${OBJECTDIR}/Source/Tools.o: Source/Tools.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	${RM} "$@.d"
	$(COMPILE.cc) -g -Werror -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/Source/Tools.o Source/Tools.cpp

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
