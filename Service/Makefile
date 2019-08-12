#WEB related components
WEB_TARGET = Tmp/WEB/openZmeter.js
WEB_SOURCE = $(shell find WEB -type f ! -path "WEB/node_modules/*")

#Resources related components
RESOURCES_TARGET = Tmp/ResourceFiles.o
RESOURCES_SOURCE = $(shell find Resources ! -type d -print | sort) $(WEB_TARGET)

#Executable related components
EXECUTABLE_TARGET = Tmp/openZmeter
EXECUTABLE_SOURCE = Source/Tools.cpp Source/Main.cpp Source/Database.cpp Source/HTTPServer.cpp Source/Devices/DeviceManager.cpp Source/Devices/Device.cpp Source/Analyzers/Analyzer.cpp Source/Tariffs/Tariff.cpp 
CFLAGS += -Wno-psabi -Wall -std=c++11 -I Source -I Source/Devices -I Source/Analyzers -I Source/Tariffs 
LDFLAGS += -lstdc++ -lpq -lmicrohttpd -lz -lpthread -lcurl

#Add libs (cctz, nlohmann)
EXECUTABLE_SOURCE += $(shell find Libs/cctz -name "*.cpp" ! -type d -print)
BUILD = $(shell date +%Y%m%d.%H%M)
CFLAGS += -D__BUILD__="\"$(BUILD)\"" -I Libs -I Source/Support 
CC?=gcc
LD?=ld
EXECUTABLE_OBJS	+= $(EXECUTABLE_SOURCE:%=Tmp/%.o)
#=======================================================================================================================
default: READCONFIG $(EXECUTABLE_TARGET)

clean:
	@rm -rf Tmp

READCONFIG:
ifdef CONFIG
	@mkdir -p Tmp
	@echo "Reading configuration from '$(CONFIG)'"
include $(CONFIG)
else
	@echo "Use make CONFIG={config_file}"
	@false
endif
ifeq ($(DEBUG), true)
	CFLAGS += -O0 -g -g3 -ggdb -fstack-protector-all
else
	CFLAGS  += -O2 -Wall -fno-rtti -fopt-info-vec -s -flto -ftree-vectorize
	LDFLAGS += -flto
endif
ifeq ($(DEVICE_V1_00), true)
	EXECUTABLE_SOURCE += Source/Devices/V1_00/DeviceV1_00.cpp
	ANALYZER_SOFT = true
	LDFLAGS += -lusb-1.0
endif
ifeq ($(DEVICE_V1_01), true)
	EXECUTABLE_SOURCE += Source/Devices/V1_01/DeviceV1_01.cpp
	ANALYZER_SOFT = true
	LDFLAGS += -lusb-1.0
endif
ifeq ($(DEVICE_V2_01), true)
	EXECUTABLE_SOURCE += Source/Devices/V2_01/DeviceV2_01.cpp
	ANALYZER_SOFT = true
	LDFLAGS += -lusb-1.0
endif
ifeq ($(DEVICE_VIRTUAL), true)
	EXECUTABLE_SOURCE += Source/Devices/Virtual/DeviceVirtual.cpp
	ANALYZER_SOFT = true
endif
ifeq ($(DEVICE_REMOTE), true)
	EXECUTABLE_SOURCE += Source/Devices/Remote/DeviceRemote.cpp Source/Analyzers/Remote/AnalyzerRemote.cpp
	CFLAGS += -I Source/Analyzers/Remote
endif
ifeq ($(ANALYZER_SOFT), true)
	EXECUTABLE_SOURCE += Source/Analyzers/Soft/AnalyzerSoft.cpp Source/Analyzers/Soft/FFT.cpp
	CFLAGS += -I Source/Analyzers/Soft
	LDFLAGS += -lfftw3f
endif
ifeq ($(OS_TARGET), OPENWRT)
	EXECUTABLE_SOURCE += Source/Support/OpenWRT/OpenWRT.cpp
	CFLAGS += -I Source/Support/OpenWRT -DOPENWRT
	LDFLAGS += -ldbus-1
endif
ifeq ($(OS_TARGET), DEBIAN)
	EXECUTABLE_SOURCE += Source/Support/Debian/Debian.cpp
	CFLAGS += -I Source/Support/Debian -DDEBIAN
	LDFLAGS += -ldbus-1
endif
ifeq ($(TARIFF_ESP), true)
	EXECUTABLE_SOURCE += Source/Tariffs/ESP/TariffPlugin_ESP.cpp
endif
ifeq ($(TARIFF_NONE), true)
	EXECUTABLE_SOURCE += Source/Tariffs/NONE/TariffPlugin_NONE.cpp
endif

$(WEB_TARGET):	 $(WEB_SOURCE)
	@echo "Building WEB interface"
	@if [ ! -d "WEB/node_modules" ]; then cd WEB; npm install; fi
	@cd WEB; npm run --silent build

$(RESOURCES_TARGET): $(RESOURCES_SOURCE) $(WEB_TARGET)
	@echo  "Building resource file"
	@rm -f $(RESOURCES_TARGET) Tmp/ResourceFiles
	@printf "%s\000" $(words $(RESOURCES_SOURCE)) > Tmp/ResourceFiles
	@for FILE in $(RESOURCES_SOURCE); \
	do \
		echo "  Append " $$FILE ; \
		printf '%s\000%d\000' `echo $$FILE | cut -d'/' -f2-` `stat --printf="%s" $$FILE` >> Tmp/ResourceFiles ; \
		cat $$FILE >> Tmp/ResourceFiles ; \
	done
	@cd Tmp; $(LD) -r -b binary -o ResourceFiles.o ResourceFiles
	@rm -f Tmp/ResourceFiles
	
$(EXECUTABLE_TARGET): $(EXECUTABLE_OBJS) $(RESOURCES_TARGET)
	@echo "Linking target" $(TARGET_EXEC)
	$(CC)  $(EXECUTABLE_OBJS) $(RESOURCES_TARGET) -o $(EXECUTABLE_TARGET) $(LDFLAGS)

Tmp/%.cpp.o: %.cpp
	@mkdir -p $(dir $@)
	@echo "Building" $@
	$(CC) $(CFLAGS) -c $< -o $@


#include dirs
#INC_DIRS += /usr/include/postgresql /usr/include/dbus-1.0 /usr/lib/x86_64-linux-gnu/dbus-1.0/include /usr/include/libxml2 \
#	   /usr/local/pgsql/include /usr/lib/arm-linux-gnueabihf/dbus-1.0/include /usr/include/nlohmann
#LIB_DIRS = /usr/local/pgsql/lib

#General options
#CC       ?= gcc
#LD       = ld

#LDFLAGS  += -Wl,-Bstatic -lstdc++ -Wl,-Bdynamic     -lm 

#Specific options
#debug: CPPFLAGS += -DOZM_DEBUG
#release: CPPFLAGS += 

#Source and targets files
#TARGET_EXEC  = openZmeter
#BUILD_DIR    = Tmp
#MAINSOURCES  = $(shell find ./Source -maxdepth 2 -name "*.cpp" ! -type d -print) $(shell find ./Libs -name "*.cpp" -o -name "*.cc" ! -type d -print)
#MAINSOURCES += $(shell find ./Source -mindepth 3 -name "*.cpp" ! -type d -print)
#RESOURCES    = $(shell find ./Resources ! -type d -print | sort)
#RESTARGET    = $(BUILD_DIR)/ResourceFiles.o
#RESTMP       = $(BUILD_DIR)/ResourceFiles
#OBJS         = $(MAINSOURCES:%=$(BUILD_DIR)/%.o)
#INC_DIRS    += $(shell find ./Source -type d) $(shell find ./Libs -type d)
#INC_FLAGS   += $(addprefix -I,$(INC_DIRS))
#LDFLAGS     += $(addprefix -L,$(LIB_DIRS))
#CFLAGS    += $(INC_FLAGS)
#debug: $(OBJS) $(RESTARGET)
#	@echo "Linking debug target" $(TARGET_EXEC)
#	$(CC) $(CFLAGS) $(OBJS) $(RESTARGET) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)
#	
#release: $(OBJS) $(RESTARGET)
#	@echo "Linking release target" $(TARGET_EXEC)
#	$(CC) $(CFLAGS) $(OBJS) $(RESTARGET) -o $(BUILD_DIR)/$(TARGET_EXEC) $(LDFLAGS)
#
#
#package: $(BUILD_DIR)/$(TARGET_EXEC)
#	@cp -r Package $(BUILD_DIR)
#	@mkdir -p $(BUILD_DIR)/Package/usr/sbin/$(TARGET_EXEC)
#	@cp $(BUILD_DIR)/$(TARGET_EXEC) $(BUILD_DIR)/Package/usr/sbin/$(TARGET_EXEC)
#	@dpkg-deb -b $(BUILD_DIR)/Package $(BUILD_DIR)/openZmeter.deb