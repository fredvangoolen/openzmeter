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
	${OBJECTDIR}/Source/CaptureV2/DeviceV2.o \
	${OBJECTDIR}/Source/Database.o \
	${OBJECTDIR}/Source/DeviceInterface.o \
	${OBJECTDIR}/Source/Devices.o \
	${OBJECTDIR}/Source/Events.o \
	${OBJECTDIR}/Source/FFT.o \
	${OBJECTDIR}/Source/HTTPServer.o \
	${OBJECTDIR}/Source/Main.o \
	${OBJECTDIR}/Source/NetworkManager.o \
	${OBJECTDIR}/Source/Prices.o \
	${OBJECTDIR}/Source/Stats.o \
	${OBJECTDIR}/Source/Tools.o


# C Compiler Flags
CFLAGS=-march=native -Os

# CC Compiler Flags
CCFLAGS=-march=native -Os
CXXFLAGS=-march=native -Os

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-L/usr/local/lib

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openZmeter

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openZmeter: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/openZmeter ${OBJECTFILES} ${LDLIBSOPTIONS} -lfftw3f -lpthread -lusb-1.0 -lpq -lmicrohttpd -ldbus-1 -ljson-c -lz -lcurl -s

${OBJECTDIR}/Source/CaptureV2/DeviceV2.o: Source/CaptureV2/DeviceV2.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/CaptureV2
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/CaptureV2/DeviceV2.o Source/CaptureV2/DeviceV2.cpp

${OBJECTDIR}/Source/Database.o: Source/Database.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Database.o Source/Database.cpp

${OBJECTDIR}/Source/DeviceInterface.o: Source/DeviceInterface.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/DeviceInterface.o Source/DeviceInterface.cpp

${OBJECTDIR}/Source/Devices.o: Source/Devices.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Devices.o Source/Devices.cpp

${OBJECTDIR}/Source/Events.o: Source/Events.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Events.o Source/Events.cpp

${OBJECTDIR}/Source/FFT.o: Source/FFT.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/FFT.o Source/FFT.cpp

${OBJECTDIR}/Source/HTTPServer.o: Source/HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/HTTPServer.o Source/HTTPServer.cpp

${OBJECTDIR}/Source/Main.o: Source/Main.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Main.o Source/Main.cpp

${OBJECTDIR}/Source/NetworkManager.o: Source/NetworkManager.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/NetworkManager.o Source/NetworkManager.cpp

${OBJECTDIR}/Source/Prices.o: Source/Prices.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Prices.o Source/Prices.cpp

${OBJECTDIR}/Source/Stats.o: Source/Stats.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Stats.o Source/Stats.cpp

${OBJECTDIR}/Source/Tools.o: Source/Tools.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -std=c++98 -o ${OBJECTDIR}/Source/Tools.o Source/Tools.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
