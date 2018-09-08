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
	${OBJECTDIR}/Source/AnalyzerInterface.o \
	${OBJECTDIR}/Source/Analyzers/AnalyzerRemote.o \
	${OBJECTDIR}/Source/Analyzers/AnalyzerSoft.o \
	${OBJECTDIR}/Source/Database.o \
	${OBJECTDIR}/Source/DeviceInterface.o \
	${OBJECTDIR}/Source/Drivers/DeviceRemote.o \
	${OBJECTDIR}/Source/Drivers/DeviceV1.o \
	${OBJECTDIR}/Source/Drivers/DeviceV2.o \
	${OBJECTDIR}/Source/Drivers/DeviceVirtual.o \
	${OBJECTDIR}/Source/FFT.o \
	${OBJECTDIR}/Source/HTTPServer.o \
	${OBJECTDIR}/Source/Main.o \
	${OBJECTDIR}/Source/NetworkManager.o \
	${OBJECTDIR}/Source/Prices.o \
	${OBJECTDIR}/Source/Tools.o \
	${OBJECTDIR}/Source/ZeroCrossing.o


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

${OBJECTDIR}/Source/AnalyzerInterface.o: Source/AnalyzerInterface.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/AnalyzerInterface.o Source/AnalyzerInterface.cpp

${OBJECTDIR}/Source/Analyzers/AnalyzerRemote.o: Source/Analyzers/AnalyzerRemote.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/Analyzers
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Analyzers/AnalyzerRemote.o Source/Analyzers/AnalyzerRemote.cpp

${OBJECTDIR}/Source/Analyzers/AnalyzerSoft.o: Source/Analyzers/AnalyzerSoft.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/Analyzers
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Analyzers/AnalyzerSoft.o Source/Analyzers/AnalyzerSoft.cpp

${OBJECTDIR}/Source/Database.o: Source/Database.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Database.o Source/Database.cpp

${OBJECTDIR}/Source/DeviceInterface.o: Source/DeviceInterface.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/DeviceInterface.o Source/DeviceInterface.cpp

${OBJECTDIR}/Source/Drivers/DeviceRemote.o: Source/Drivers/DeviceRemote.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/Drivers
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Drivers/DeviceRemote.o Source/Drivers/DeviceRemote.cpp

${OBJECTDIR}/Source/Drivers/DeviceV1.o: Source/Drivers/DeviceV1.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/Drivers
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Drivers/DeviceV1.o Source/Drivers/DeviceV1.cpp

${OBJECTDIR}/Source/Drivers/DeviceV2.o: Source/Drivers/DeviceV2.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/Drivers
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Drivers/DeviceV2.o Source/Drivers/DeviceV2.cpp

${OBJECTDIR}/Source/Drivers/DeviceVirtual.o: Source/Drivers/DeviceVirtual.cpp
	${MKDIR} -p ${OBJECTDIR}/Source/Drivers
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Drivers/DeviceVirtual.o Source/Drivers/DeviceVirtual.cpp

${OBJECTDIR}/Source/FFT.o: Source/FFT.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/FFT.o Source/FFT.cpp

${OBJECTDIR}/Source/HTTPServer.o: Source/HTTPServer.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/HTTPServer.o Source/HTTPServer.cpp

${OBJECTDIR}/Source/Main.o: Source/Main.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Main.o Source/Main.cpp

${OBJECTDIR}/Source/NetworkManager.o: Source/NetworkManager.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/NetworkManager.o Source/NetworkManager.cpp

${OBJECTDIR}/Source/Prices.o: Source/Prices.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Prices.o Source/Prices.cpp

${OBJECTDIR}/Source/Tools.o: Source/Tools.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/Tools.o Source/Tools.cpp

${OBJECTDIR}/Source/ZeroCrossing.o: Source/ZeroCrossing.cpp
	${MKDIR} -p ${OBJECTDIR}/Source
	$(COMPILE.cc) -Werror -s -I/usr/include/libxml2 -I/usr/include/dbus-1.0 -I/usr/lib/x86_64-linux-gnu/dbus-1.0/include -I/usr/lib/arm-linux-gnueabihf/dbus-1.0/include -ISource -std=c++98 -o ${OBJECTDIR}/Source/ZeroCrossing.o Source/ZeroCrossing.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}

# Subprojects
.clean-subprojects:
